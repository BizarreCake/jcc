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

#ifndef _JCC__JTAC__PROGRAM__H_
#define _JCC__JTAC__PROGRAM__H_

#include "jtac/jtac.hpp"
#include <vector>
#include <string>


namespace jcc {
namespace jtac {

  /*!
     \class procedure
     \brief A JTAC procedure/function.
   */
  class procedure
  {
    std::string name;
    std::vector<jtac_var_id> params;
    std::vector<jtac_instruction> body;

   public:
    inline const std::string& get_name () const { return this->name; }

    inline const auto& get_params () const { return this->params; }
    inline auto& get_params () { return this->params; }

    inline const auto& get_body () const { return this->body; }
    inline auto& get_body () { return this->body; }

   public:
    procedure (const std::string& name);

   public:
  };


  /*!
     \class program
     \brief Represents a JTAC program.

     As opposed to a stricly linear array of instructions, a program
     logically divides JTAC instructions into functions/procedures and may
     contain other relevant metadata (such as imports or exports, etc...)
   */
  class program
  {
    std::vector<procedure> procs;
  };
}
}

#endif //_JCC__JTAC__PROGRAM__H_
