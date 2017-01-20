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

#include "linker/translators/elf64/elf64.hpp"


namespace jcc {

  /*!
     \brief The hash function used in ELF64 files.
   */
  elf64_word_t
  elf64_hash (const std::string& name)
  {
    elf64_word_t h = 0, g;

    for (unsigned char c : name)
      {
        h = (h << 4) + c;
        if ((g = h & 0xf0000000))
          h ^= g >> 24;
      }

    return h;
  }
}
