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



//------------------------------------------------------------------------------

  //! \brief Inserts a new procedure and returns a reference to it.
  procedure&
  program::emplace_procedure (const std::string& name)
  {
    this->procs.emplace_back (name);
    return this->procs.back ();
  }
}
}
