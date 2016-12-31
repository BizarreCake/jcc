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

#ifndef _JCC__JTAC__PRINTER__H_
#define _JCC__JTAC__PRINTER__H_

#include "jtac/jtac.hpp"
#include "jtac/control_flow.hpp"
#include <iosfwd>


namespace jcc {
namespace jtac {

  /*!
     \class printer
     \brief JTAC pretty printer.
   */
  class printer
  {
   public:
    //! \brief Prints the specified instruction onto the given stream.
    void print_instruction (const jtac_instruction& ins, std::ostream& strm);

    //! \brief Prints the specified instruction into a string.
    std::string print_instruction (const jtac_instruction& ins);


    //! \brief Prints the specified basic block onto the given stream.
    void print_basic_block (const basic_block& blk, std::ostream& strm);

    //! \brief Prints the specified basic block into a string.
    std::string print_basic_block (const basic_block& blk);

   private:
    //! \brief Prints the specified operand onto the given stream.
    void print_operand (const jtac_tagged_operand& opr, std::ostream& strm);
  };
}
}

#endif //_JCC__JTAC__PRINTER__H_
