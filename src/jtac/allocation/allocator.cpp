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

#include "jtac/allocation/allocator.hpp"
#include <stdexcept>


namespace jcc {
namespace jtac {

  //! \brief Sets the color of the specified variable.
  void
  register_allocation::set_color (jtac_var_id var, register_color col)
  {
    this->color_map[var] = col;
  }

  //! \brief Returns the color of the specified variable.
  register_color
  register_allocation::get_color (jtac_var_id var) const
  {
    auto itr = this->color_map.find (var);
    if (itr == this->color_map.end ())
      throw std::runtime_error ("register_allocation::get_color: variable has no color");

    return itr->second;
  }
}
}
