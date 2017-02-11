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

#ifndef _JCC__JTAC__PRINTER__H_
#define _JCC__JTAC__PRINTER__H_

#include "jtac/jtac.hpp"
#include "jtac/control_flow.hpp"
#include "jtac/name_map.hpp"
#include <iosfwd>


namespace jcc {
namespace jtac {

  /*!
     \class printer
     \brief JTAC pretty printer.
   */
  class printer
  {
    size_t base;
    size_t inst_idx;
    const name_map<jtac_var_id> *var_names;

   public:
    printer ();

   public:
    //! \brief Sets the name table used when printing variables.
    void set_var_names (const name_map<jtac_var_id>& var_names);

    //! \brief Drops the currently set variable name table.
    void reset_var_names ();

   public:
    //! \brief Prints the specified opcode's mnemonic onto the given stream.
    void print_mnemonic (jtac_opcode op, std::ostream& strm);

    //! \brief Prints the specified opcode's mnemonic into a string.
    std::string print_mnemonic (jtac_opcode op);


    //! \brief Prints the specified instruction onto the given stream.
    void print_instruction (const jtac_instruction& ins, std::ostream& strm);

    //! \brief Prints the specified instruction into a string.
    std::string print_instruction (const jtac_instruction& ins);


    //! \brief Prints the specified basic block onto the given stream.
    void print_basic_block (const basic_block& blk, std::ostream& strm);

    //! \brief Prints the specified basic block into a string.
    std::string print_basic_block (const basic_block& blk);


    //! \brief Prints the specified operand onto the given stream.
    void print_operand (const jtac_tagged_operand& opr, std::ostream& strm);

    //! \brief Prints the specified operand into a string.
    std::string print_operand (const jtac_tagged_operand& opr);
  };
}
}

#endif //_JCC__JTAC__PRINTER__H_
