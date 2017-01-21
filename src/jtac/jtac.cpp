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

#include "jtac/jtac.hpp"
#include <stdexcept>


namespace jcc {
namespace jtac {

  //! \brief Returns true if the opcode described an instruction of the form: X = Y.
  bool
  is_opcode_assign (jtac_opcode op)
  {
    switch (op)
      {
      case JTAC_OP_ASSIGN:
      case JTAC_OP_ASSIGN_ADD:
      case JTAC_OP_ASSIGN_SUB:
      case JTAC_OP_ASSIGN_MUL:
      case JTAC_OP_ASSIGN_DIV:
      case JTAC_OP_ASSIGN_MOD:
      case JTAC_OP_ASSIGN_CALL:
      case JTAC_SOP_ASSIGN_PHI:
        return true;

      case JTAC_OP_UNDEF:
      case JTAC_OP_RET:
      case JTAC_OP_CMP:
      case JTAC_OP_JMP:
      case JTAC_OP_JE:
      case JTAC_OP_JNE:
      case JTAC_OP_JL:
      case JTAC_OP_JLE:
      case JTAC_OP_JG:
      case JTAC_OP_JGE:
      case JTAC_OP_CALL:
        return false;
      }

    throw std::runtime_error ("is_opcode_assign: unhandled opcode");
  }

  //! \brief Returns the class of the specified opcode.
  jtac_opcode_class
  get_opcode_class (jtac_opcode op)
  {
    switch (op)
      {
      case JTAC_OP_UNDEF:
        return JTAC_OPC_NONE;

      case JTAC_OP_ASSIGN:
        return JTAC_OPC_ASSIGN2;

      case JTAC_OP_ASSIGN_ADD:
      case JTAC_OP_ASSIGN_SUB:
      case JTAC_OP_ASSIGN_MUL:
      case JTAC_OP_ASSIGN_DIV:
      case JTAC_OP_ASSIGN_MOD:
        return JTAC_OPC_ASSIGN3;

      case JTAC_OP_RET:
      case JTAC_OP_JMP:
      case JTAC_OP_JE:
      case JTAC_OP_JNE:
      case JTAC_OP_JL:
      case JTAC_OP_JLE:
      case JTAC_OP_JG:
      case JTAC_OP_JGE:
        return JTAC_OPC_USE1;

      case JTAC_OP_CMP:
        return JTAC_OPC_USE2;

      case JTAC_OP_ASSIGN_CALL:
        return JTAC_OPC_ASSIGN_CALL;

      case JTAC_SOP_ASSIGN_PHI:
        return JTAC_OPC_ASSIGN_FIXED_CALL;

      case JTAC_OP_CALL:
        return JTAC_OPC_CALL;
      }

    throw std::runtime_error ("get_opcode_class: unhandled opcode");
  }

  //! \brief Returns the number of operands used by the specified opcode.
  int
  get_operand_count (jtac_opcode op)
  {
    switch (get_opcode_class (op))
      {
        case JTAC_OPC_ASSIGN_CALL:       return 2;
        case JTAC_OPC_ASSIGN_FIXED_CALL: return 1;
        case JTAC_OPC_NONE:              return 0;
        case JTAC_OPC_USE1:              return 1;
        case JTAC_OPC_USE2:              return 2;
        case JTAC_OPC_ASSIGN2:           return 2;
        case JTAC_OPC_ASSIGN3:           return 3;
        case JTAC_OPC_CALL:              return 1;
      }

    throw std::runtime_error ("has_extra_operands: unhandled opcode class");
  }

  //! \brief Checks whether the specified opcode requires extra operands.
  bool
  has_extra_operands (jtac_opcode op)
  {
    switch (get_opcode_class (op))
      {
      case JTAC_OPC_ASSIGN_CALL:
      case JTAC_OPC_ASSIGN_FIXED_CALL:
      case JTAC_OPC_CALL:
        return true;

      case JTAC_OPC_NONE:
      case JTAC_OPC_USE1:
      case JTAC_OPC_USE2:
      case JTAC_OPC_ASSIGN2:
      case JTAC_OPC_ASSIGN3:
        return false;
      }

    throw std::runtime_error ("has_extra_operands: unhandled opcode class");
  }

//------------------------------------------------------------------------------



  jtac_tagged_operand&
  jtac_tagged_operand::operator= (const jtac_tagged_operand& other)
  {
    this->type = other.type;
    switch (this->type)
      {
        case JTAC_OPR_CONST: new (&this->val.konst) jtac_const (other.val.konst); break;
        case JTAC_OPR_VAR: new (&this->val.var) jtac_var (other.val.var); break;
        case JTAC_OPR_LABEL: new (&this->val.lbl) jtac_label (other.val.lbl); break;
        case JTAC_OPR_OFFSET: new (&this->val.off) jtac_offset (other.val.off); break;
        case JTAC_OPR_NAME: new (&this->val.name) jtac_name (other.val.name); break;
      }

    return *this;
  }

  jtac_tagged_operand&
  jtac_tagged_operand::operator= (jtac_tagged_operand&& other)
  {
    this->type = other.type;
    switch (this->type)
      {
      case JTAC_OPR_CONST: new (&this->val.konst) jtac_const (std::move (other.val.konst)); break;
      case JTAC_OPR_VAR: new (&this->val.var) jtac_var (std::move (other.val.var)); break;
      case JTAC_OPR_LABEL: new (&this->val.lbl) jtac_label (std::move (other.val.lbl)); break;
      case JTAC_OPR_OFFSET: new (&this->val.off) jtac_offset (std::move (other.val.off)); break;
      case JTAC_OPR_NAME: new (&this->val.name) jtac_name (std::move (other.val.name)); break;
      }

    return *this;
  }

  jtac_tagged_operand&
  jtac_tagged_operand::operator= (const jtac_operand& opr)
  {
    this->type = opr.get_type ();
    switch (opr.get_type ())
      {
      case JTAC_OPR_CONST:
        new (&this->val.konst) jtac_const (static_cast<const jtac_const&> (opr));
        break;

      case JTAC_OPR_VAR:
        new (&this->val.var) jtac_var (static_cast<const jtac_var&> (opr));
        break;

      case JTAC_OPR_LABEL:
        new (&this->val.lbl) jtac_label (static_cast<const jtac_label&> (opr));
        break;

      case JTAC_OPR_OFFSET:
        new (&this->val.off) jtac_offset (static_cast<const jtac_offset&> (opr));
        break;

      case JTAC_OPR_NAME:
        new (&this->val.name) jtac_name (static_cast<const jtac_name&> (opr));
        break;
      }

    return *this;
  }



  jtac_operand&
  tagged_operand_to_operand (jtac_tagged_operand& opr)
  {
    switch (opr.type)
      {
      case JTAC_OPR_VAR: return opr.val.var;
      case JTAC_OPR_CONST: return opr.val.konst;
      case JTAC_OPR_LABEL: return opr.val.lbl;
      case JTAC_OPR_OFFSET: return opr.val.off;
      case JTAC_OPR_NAME: return opr.val.name;
      }

    throw std::runtime_error ("tagged_operand_to_operand: unhandled operand type");
  }



  jtac_instruction::jtac_instruction ()
  {
    this->extra.cap = 0;
  }

  jtac_instruction::jtac_instruction (const jtac_instruction& other)
  { *this = other; }

  jtac_instruction::jtac_instruction (jtac_instruction&& other)
  { *this = std::move (other); }



  jtac_instruction&
  jtac_instruction::operator= (const jtac_instruction& other)
  {
    this->op = other.op;
    this->oprs[0] = other.oprs[0];
    this->oprs[1] = other.oprs[1];
    this->oprs[2] = other.oprs[2];
    this->extra.count = other.extra.count;
    this->extra.cap = other.extra.cap;
    if (this->extra.cap)
      {
        this->extra.oprs = new jtac_tagged_operand[this->extra.cap];
        for (int i = 0; i < this->extra.count; ++i)
          this->extra.oprs[i] = other.extra.oprs[i];
      }

    return *this;
  }

  jtac_instruction&
  jtac_instruction::operator= (jtac_instruction&& other)
  {
    this->op = other.op;
    this->oprs[0] = std::move (other.oprs[0]);
    this->oprs[1] = std::move (other.oprs[1]);
    this->oprs[2] = std::move (other.oprs[2]);
    this->extra.count = other.extra.count;
    this->extra.cap = other.extra.cap;
    this->extra.oprs = other.extra.oprs;
    other.extra.count = 0;
    other.extra.cap = 0;
    other.extra.oprs = nullptr;

    return *this;
  }



  //! \brief Inserts the specified operand into the instruction's "extra" list.
  jtac_instruction&
  jtac_instruction::push_extra (const jtac_operand& opr)
  {
    if (this->extra.count == this->extra.cap)
      {
        int ncap = (int)this->extra.cap * 2;
        if (ncap > 255) ncap = 255;
        this->extra.cap = (unsigned char)ncap;
        jtac_tagged_operand *tmp = new jtac_tagged_operand[ncap];
        for (int i = 0; this->extra.count; ++i)
          tmp[i] = std::move (this->extra.oprs[i]);
        delete[] this->extra.oprs;
        this->extra.oprs = tmp;
      }

    this->extra.oprs[this->extra.count ++] = opr;
    return *this;
  }
}
}
