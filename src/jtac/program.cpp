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

#include "jtac/program.hpp"


namespace jcc {
namespace jtac {

  procedure::procedure (const std::string& name)
      : name (name)
  {
  }



  //! \brief Inserts a variable name mapping.
  void
  procedure::map_var_name (const std::string& name, jtac_var_id id)
  {
    this->var_name_map[name] = id;
  }

  //! \brief Returns the variable ID associated with the specified name.
  jtac_var_id
  procedure::get_var_name_id (const std::string& name) const
  {
    auto itr = this->var_name_map.find (name);
    if (itr == this->var_name_map.end ())
      throw std::runtime_error ("procedure::get_var_name_id: could not find variable name");
    return itr->second;
  }

  //! \brief Checks whether the specified variable name is mapped to a variable ID.
  bool
  procedure::has_var_name (const std::string& name) const
  {
    return this->var_name_map.find (name) != this->var_name_map.end ();
  }



//------------------------------------------------------------------------------

  //! \brief Inserts a new procedure and returns a reference to it.
  procedure&
  program::emplace_procedure (const std::string& name)
  {
    this->procs.emplace_back (name);
    return this->procs.back ();
  }



  //! \brief Inserts a name mapping.
  void
  program::map_name (const std::string& name, jtac_name_id id)
  {
    this->name_map[name] = id;
  }

  //! \brief Returns the name ID associated with the specified name.
  jtac_name_id
  program::get_name_id (const std::string& name) const
  {
    auto itr = this->name_map.find (name);
    if (itr == this->name_map.end ())
      throw std::runtime_error ("procedure::get_name_id: could not find name");
    return itr->second;
  }

  //! \brief Checks whether the specified name is mapped to a name ID.
  bool
  program::has_name (const std::string& name) const
  {
    return this->name_map.find (name) != this->name_map.end ();
  }
}
}
