/*
 * jcc - A compiler framework.
 * Copyright (C) 2016 Jacob Zhitomirsky
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "jtac/assembler.hpp"


namespace jcc {

  jtac_assembler::jtac_assembler ()
  {
    this->pos = 0;
    this->next_lbl_id = 1;
  }



  //! \brief Creates and returns a new unique label ID.
  jtac_label_id
  jtac_assembler::make_label ()
  {
    return this->next_lbl_id ++;
  }

  //! \brief Sets the position of the specified label ID to the current position.
  void
  jtac_assembler::mark_label (jtac_label_id id)
  {
    this->lbl_fixes[id] = this->pos;
  }

  //! \brief Calls make_label() and mark_label() in succession.
  jtac_label_id
  jtac_assembler::make_and_mark_label ()
  {
    auto lbl = this->make_label ();
    this->mark_label (lbl);
    return lbl;
  }



  //! \brief Overwrites or inserts a new instruction and returns it.
  jtac_instruction&
  jtac_assembler::put_instruction ()
  {
    if (this->pos < this->insts.size ())
      return this->insts[this->pos];

    this->insts.emplace_back ();
    return this->insts.back ();
  }

  static void
  _set_operand (jtac_instruction& inst, int index, const jtac_operand& opr)
  {
    inst.oprs[index].type = opr.get_type ();
    switch (opr.get_type ())
      {
      case JTAC_OPR_CONST:
        inst.oprs[index].val.konst = static_cast<const jtac_const&> (opr);
        break;

      case JTAC_OPR_VAR:
        inst.oprs[index].val.var = static_cast<const jtac_var&> (opr);
        break;

      case JTAC_OPR_LABEL:
        inst.oprs[index].val.lbl = static_cast<const jtac_label&> (opr);
        break;

      case JTAC_OPR_OFFSET:
        inst.oprs[index].val.off = static_cast<const jtac_offset&> (opr);
        break;
      }
  }

  //! \brief Emits a standard instruction in the form of: r = a <op> b
  void
  jtac_assembler::emit_basic3 (jtac_opcode op, const jtac_operand& r,
                              const jtac_operand& a, const jtac_operand& b)
  {
    auto& inst = this->put_instruction ();
    inst.op = op;
    _set_operand (inst, 0, r);
    _set_operand (inst, 1, a);
    _set_operand (inst, 2, b);
  }

  //! \brief Emits a binary instruction in the form of: a <op> b
  void
  jtac_assembler::emit_basic2 (jtac_opcode op, const jtac_operand& a,
                               const jtac_operand& b)
  {
    auto& inst = this->put_instruction ();
    inst.op = op;
    _set_operand (inst, 0, a);
    _set_operand (inst, 1, b);
  }

  //! \brief Emits an instruction that takes a single operand.
  void
  jtac_assembler::emit_basic1 (jtac_opcode op, const jtac_operand& opr)
  {
    if (opr.get_type () == JTAC_OPR_LABEL)
      {
        auto& lbl = static_cast<const jtac_label&> (opr);
        this->lbl_uses.push_back ({ lbl.get_id (), this->pos });
      }

    auto& inst = this->put_instruction ();
    inst.op = op;
    _set_operand (inst, 0, opr);
  }
}
