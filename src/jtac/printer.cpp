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

#include "jtac/printer.hpp"
#include <ostream>
#include <iomanip>
#include <sstream>
#include <jtac/jtac.hpp>


namespace jcc {
namespace jtac {

  static const char*
  _get_opcode_mnemonic (jtac_opcode op)
  {
    switch (op)
      {
      case JTAC_OP_UNDEF:       return "<undef>";
      case JTAC_OP_ASSIGN:      return "=";
      case JTAC_OP_ASSIGN_ADD:  return "+";
      case JTAC_OP_ASSIGN_SUB:  return "-";
      case JTAC_OP_ASSIGN_MUL:  return "*";
      case JTAC_OP_ASSIGN_DIV:  return "/";
      case JTAC_OP_ASSIGN_MOD:  return "%";
      case JTAC_OP_ASSIGN_CALL: return "call";
      case JTAC_OP_CMP:         return "cmp";
      case JTAC_OP_JMP:         return "jmp";
      case JTAC_OP_JE:          return "je";
      case JTAC_OP_JNE:         return "jne";
      case JTAC_OP_JL:          return "jl";
      case JTAC_OP_JLE:         return "jle";
      case JTAC_OP_JG:          return "jg";
      case JTAC_OP_JGE:         return "jge";
      case JTAC_OP_RET:         return "ret";
      case JTAC_OP_CALL:      return "call";

      case JTAC_SOP_ASSIGN_PHI: return "phi";
      }

    return "<>";
  }

  //! \brief Prints the specified opcode's mnemonic onto the given stream.
  void
  printer::print_mnemonic (jtac_opcode op, std::ostream& strm)
  {
    strm << _get_opcode_mnemonic (op);
  }

  //! \brief Prints the specified opcode's mnemonic into a string.
  std::string
  printer::print_mnemonic (jtac_opcode op)
  {
    return _get_opcode_mnemonic (op);
  }


  //! \brief Prints the specified instruction onto the given stream.
  void
  printer::print_instruction (const jtac_instruction& ins, std::ostream& strm)
  {
    switch (ins.op)
      {
      case JTAC_OP_UNDEF:
        strm << "<undef>";
        break;

      case JTAC_OP_ASSIGN_ADD:
      case JTAC_OP_ASSIGN_SUB:
      case JTAC_OP_ASSIGN_MUL:
      case JTAC_OP_ASSIGN_DIV:
      case JTAC_OP_ASSIGN_MOD:
        this->print_operand (ins.oprs[0], strm);
        strm << " = ";
        this->print_operand (ins.oprs[1], strm);
        strm << ' ' << _get_opcode_mnemonic (ins.op) << ' ';
        this->print_operand (ins.oprs[2], strm);
        break;

      case JTAC_OP_ASSIGN:
        this->print_operand (ins.oprs[0], strm);
        strm << " = ";
        this->print_operand (ins.oprs[1], strm);
        break;

      case JTAC_OP_CMP:
        strm << _get_opcode_mnemonic (ins.op);
        strm << ' ';
        this->print_operand (ins.oprs[0], strm);
        strm << ", ";
        this->print_operand (ins.oprs[1], strm);
        break;

      case JTAC_OP_JMP:
      case JTAC_OP_JE:
      case JTAC_OP_JNE:
      case JTAC_OP_JL:
      case JTAC_OP_JLE:
      case JTAC_OP_JG:
      case JTAC_OP_JGE:
      case JTAC_OP_RET:
        strm << _get_opcode_mnemonic (ins.op);
        strm << ' ';
        this->print_operand (ins.oprs[0], strm);
        break;

      case JTAC_SOP_ASSIGN_PHI:
        this->print_operand (ins.oprs[0], strm);
        strm << " = phi(";
        for (int i = 0; i < ins.extra.count; ++i)
          {
            this->print_operand (ins.extra.oprs[i], strm);
            if (i != (int)ins.extra.count - 1)
              strm << ", ";
          }
        strm << ')';
        break;

      case JTAC_OP_ASSIGN_CALL:
        this->print_operand (ins.oprs[0], strm);
        strm << " = call ";
        this->print_operand (ins.oprs[1], strm);
        strm << '(';
        for (int i = 0; i < ins.extra.count; ++i)
          {
            this->print_operand (ins.extra.oprs[i], strm);
            if (i != (int)ins.extra.count - 1)
              strm << ", ";
          }
        strm << ')';
        break;

      case JTAC_OP_CALL:
        strm << "call ";
        this->print_operand (ins.oprs[0], strm);
        strm << '(';
        for (int i = 0; i < ins.extra.count; ++i)
          {
            this->print_operand (ins.extra.oprs[i], strm);
            if (i != (int)ins.extra.count - 1)
              strm << ", ";
          }
        strm << ')';
        break;
      }
  }

  //! \brief Prints the specified instruction into a string.
  std::string
  printer::print_instruction (const jtac_instruction& ins)
  {
    std::ostringstream ss;
    this->base = 0;
    this->inst_idx = 0;
    this->print_instruction (ins, ss);
    return ss.str ();
  }


  static int
  _log10 (int n)
  { return (n < 10) ? 0 : 1 + _log10 (n / 10); }

  //! \brief Prints the specified basic block onto the given stream.
  void
  printer::print_basic_block (const basic_block& blk, std::ostream& strm)
  {
    this->base = blk.get_base ();

    auto& insts = blk.get_instructions ();
    int lpad = _log10 ((int)insts.size ()) + 1;

    strm << "Basic Block #" << blk.get_id () << '\n';
    strm << std::string ((size_t)(14 + _log10 (blk.get_id ())), '-') << '\n';
    for (size_t i = 0; i < insts.size (); ++i)
      {
        this->inst_idx = this->base + i;
        strm << std::setw (lpad) << std::setfill ('0') << (this->inst_idx) << ": "
             << std::setfill (' ');
        this->print_instruction (insts[i], strm);
        strm << '\n';
      }

    // print names of attached blocks
    strm << std::string ((size_t)(14 + _log10 (blk.get_id ())), '-') << '\n';

    strm << "Prev:";
    for (auto& b : blk.get_prev ())
      strm << " #" << b->get_id ();
    if (blk.get_prev ().empty ())
      strm << " none";
    strm << '\n';

    strm << "Next:";
    for (auto& b : blk.get_next ())
      strm << " #" << b->get_id ();
    if (blk.get_next ().empty ())
      strm << " none";
  }

  //! \brief Prints the specified basic block into a string.
  std::string
  printer::print_basic_block (const basic_block& blk)
  {
    std::ostringstream ss;
    this->print_basic_block (blk, ss);
    return ss.str ();
  }



  //! \brief Prints the specified operand onto the given stream.
  void
  printer::print_operand (const jtac_tagged_operand& opr, std::ostream& strm)
  {
    switch (opr.type)
      {
      case jtac_operand_type::JTAC_OPR_CONST:
        strm << opr.val.konst.get_value ();
        break;

      case jtac_operand_type::JTAC_OPR_VAR:
        {
          auto var = opr.val.var.get_id ();
          strm << 't' << var_base (var);
          if (var_subscript (var) != 0)
            strm << '_' << var_subscript (var);
        }
        break;

      case jtac_operand_type::JTAC_OPR_OFFSET:
        strm << (this->inst_idx + 1 + opr.val.off.get_offset ());
        break;

      case jtac_operand_type::JTAC_OPR_LABEL:
        strm << 'L' << opr.val.lbl.get_id ();
        break;

      case jtac_operand_type::JTAC_OPR_NAME:
        // TODO: use name mapping
        strm << "<name #" << opr.val.name.get_id () << '>';
        break;
      }
  }
}
}
