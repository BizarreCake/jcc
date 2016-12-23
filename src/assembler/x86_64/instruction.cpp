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

#include "assembler/x86_64/instruction.hpp"
#include <cstring>


namespace jcc {
  
  namespace x86_64 {
    
    //! /brief Returns the size of the register in bits.
    int
    reg_t::register_size () const
    {
      switch (this->code >> 4)
        {
        case 0: return 8;
        case 1: return 16;
        case 2: return 32;
        case 3: return 64;
        
        default:
          return 0;
        }
    }
  }
}

