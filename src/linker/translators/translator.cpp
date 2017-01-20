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

#include "linker/translators/translator.hpp"
#include <unordered_map>
#include <stdexcept>

// translators:
#include "linker/translators/elf64/translator.hpp"


namespace jcc {

  static std::unique_ptr<module_translator>
  _create_elf64 ()
    { return std::make_unique<elf64_module_translator> (); }
  
  /*!
    \brief Factory method for creating translators.
   */
  std::unique_ptr<module_translator>
  module_translator::create (const std::string& name)
  {
    using create_fn = std::unique_ptr<module_translator> (*) ();
    static std::unordered_map<std::string, create_fn> _map {
      { "elf64", &_create_elf64 },
    };
    
    auto itr = _map.find (name);
    if (itr == _map.end ())
      throw std::runtime_error ("module_translator::create: unknown translator");
    return itr->second ();
  }
}

