/*
 * jcc - A compiler framework.
 * Copyright (C) 2016-2017 Jacob Zhitomirsky
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

#ifndef _JCC__JTAC__ASSEMBLER__H_
#define _JCC__JTAC__ASSEMBLER__H_

#include "jtac/jtac.hpp"
#include <vector>
#include <cstddef>
#include <unordered_map>


namespace jcc {
namespace jtac {

  /*!
     \class assembler
     \brief JTAC assembler.
   */
  class assembler {
    struct label_use {
      jtac_label_id lbl;
      size_t pos;
    };

   private:
    std::vector<jtac_instruction> insts;
    size_t pos;

    jtac_label_id next_lbl_id;
    std::unordered_map<jtac_label_id, size_t> lbl_fixes;
    std::vector<label_use> lbl_uses;

   public:
    inline const auto &get_instructions () const
    { return this->insts; }

    inline size_t get_pos () const
    { return this->pos; }
    inline void set_pos (size_t pos)
    { this->pos = pos; }

   public:
    assembler ();

   public:
    //! \brief Resets the state of the assembler.
    void clear ();

   public:
    //! \brief Creates and returns a new unique label ID.
    jtac_label_id make_label ();

    //! \brief Sets the position of the specified label ID to the current position.
    void mark_label (jtac_label_id id);

    //! \brief Calls make_label() and mark_label() in succession.
    jtac_label_id make_and_mark_label ();

    //! \brief Updates label references where the label location is known.
    void fix_labels ();

   public:
    //
    // emit methods:
    //

#define DEF_BASIC3(NAME, OPC)                                         \
    inline void                                                       \
    emit_assign_##NAME (const jtac_operand& r, const jtac_operand& a, \
                        const jtac_operand& b)                        \
    { this->emit_basic3 ((OPC), r, a, b); }

    DEF_BASIC3(add, JTAC_OP_ASSIGN_ADD)
    DEF_BASIC3(sub, JTAC_OP_ASSIGN_SUB)
    DEF_BASIC3(mul, JTAC_OP_ASSIGN_MUL)
    DEF_BASIC3(div, JTAC_OP_ASSIGN_DIV)
    DEF_BASIC3(mod, JTAC_OP_ASSIGN_MOD)

#undef DEF_BASIC

#define DEF_BASIC2(NAME, OPC)                                         \
    inline void                                                       \
    emit_##NAME (const jtac_operand& a, const jtac_operand& b)        \
    { this->emit_basic2 ((OPC), a, b); }

    DEF_BASIC2(assign, JTAC_OP_ASSIGN)
    DEF_BASIC2(cmp, JTAC_OP_CMP)

#undef DEF_BASIC2

#define DEF_BASIC1(NAME, OPC)                                         \
    inline void                                                       \
    emit_##NAME (const jtac_operand& opr)                             \
    { this->emit_basic1 ((OPC), opr); }

    DEF_BASIC1(jmp, JTAC_OP_JMP)
    DEF_BASIC1(je, JTAC_OP_JE)
    DEF_BASIC1(jne, JTAC_OP_JNE)
    DEF_BASIC1(jl, JTAC_OP_JL)
    DEF_BASIC1(jle, JTAC_OP_JLE)
    DEF_BASIC1(jg, JTAC_OP_JG)
    DEF_BASIC1(jge, JTAC_OP_JGE)
    DEF_BASIC1(ret, JTAC_OP_RET)

#undef DEF_BASIC1

    jtac_instruction& emit_call (const jtac_operand& target);

    jtac_instruction& emit_assign_call (const jtac_operand& dest,
                                        const jtac_operand& target);

    jtac_instruction& emit_assign_phi (const jtac_operand& dest);

   private:
    //! \brief Overwrites or inserts a new instruction and returns it.
    jtac_instruction &put_instruction ();

    //! \brief Emits a standard instruction in the form of: r = a <op> b
    void emit_basic3 (jtac_opcode op, const jtac_operand &r,
                      const jtac_operand &a, const jtac_operand &b);

    //! \brief Emits a binary instruction in the form of: a <op> b
    void emit_basic2 (jtac_opcode op, const jtac_operand &a,
                      const jtac_operand &b);

    //! \brief Emits an instruction that takes a single operand.
    void emit_basic1 (jtac_opcode op, const jtac_operand &opr);
  };

}
}

#endif //_JCC__JTAC__ASSEMBLER__H_
