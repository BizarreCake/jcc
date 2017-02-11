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
#include "jtac/name_map.hpp"
#include <vector>
#include <string>
#include <unordered_map>


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
    name_map<jtac_var_id> var_names;

   public:
    inline auto& get_var_names () { return this->var_names; }
    inline const auto& get_var_names () const { return this->var_names; }

    inline const std::string& get_name () const { return this->name; }

    inline const auto& get_params () const { return this->params; }
    inline auto& get_params () { return this->params; }

    inline const auto& get_body () const { return this->body; }
    inline auto& get_body () { return this->body; }

   public:
    procedure (const std::string& name);

   public:
    template<typename Iterator>
    void insert_instructions (Iterator start, Iterator end)
    { this->body.insert (this->body.end (), start, end ); }
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
    name_map<jtac_name_id> names;

   public:
    inline auto& get_names () { return this->names; }
    inline const auto& get_names () const { return this->names; }

    inline auto& get_procedures () { return this->procs; }
    inline const auto& get_procedures () const { return this->procs; }

   public:
    //! \brief Inserts a new procedure and returns a reference to it.
    procedure& emplace_procedure (const std::string& name);
  };
}
}

#endif //_JCC__JTAC__PROGRAM__H_
