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

#include "assembler/relocation.hpp"
#include <stdexcept>


namespace jcc {

  relocation_symbol_store::relocation_symbol_store ()
  {
  }



  relocation_symbol
  relocation_symbol_store::get (const std::string& name)
  {
    auto itr = this->index_map.find (name);
    if (itr != this->index_map.end ())
      return { .store = this, .id = itr->second };

    int id = (int)this->names.size ();
    this->names.push_back (name);
    this->index_map[name] = id;
    return { .store = this, .id = id };
  }

  //! \brief Returns the name associated with the specified symbol ID.
  const std::string&
  relocation_symbol_store::get_name (relocation_symbol_id id) const
  {
    if (id >= (int)this->names.size ())
      throw std::runtime_error ("relocation_symbol_store::get_name: id out of range");
    return this->names[id];
  }
}
