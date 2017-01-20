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

#ifndef _JCC__JTAC__ALLOCATION__ALLOCATOR__H_
#define _JCC__JTAC__ALLOCATION__ALLOCATOR__H_

#include "jtac/control_flow.hpp"
#include <unordered_map>


namespace jcc {
namespace jtac {

  //! \brief Stores the ID of a virtual register.
  using register_color = int;

  /*!
     \class register_allocation
     \brief Stores the results returned by a register allocator.
   */
  class register_allocation
  {
    std::unordered_map<jtac_var_id, register_color> color_map;
  };



  /*!
     \class register_allocator
     \brief Base class for register allocators.
   */
  class register_allocator
  {
   public:
    virtual ~register_allocator () { }

   public:
    /*!
       \brief Allocates registers.

       Processes the specified control flow graph and allocates registers or
       decides to spill to memory variables in the CFG.
     */
    virtual register_allocation allocate (const control_flow_graph& cfg) = 0;
  };
}
}

#endif //_JCC__JTAC__ALLOCATION__ALLOCATOR__H_
