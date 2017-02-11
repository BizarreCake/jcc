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

   public:
    //! \brief Sets the color of the specified variable.
    void set_color (jtac_var_id var, register_color col);

    //! \brief Returns the color of the specified variable.
    register_color get_color (jtac_var_id var) const;
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
       \brief Performs register allocation.

       Processes the specified control flow graph and determines which
       variables get mapped to what registers, and which variables get spilled
       into memory.

       NOTE: The control graph is transformed to contain the necessary spill
             code.

       \param cfg        The control flow graph to process.
       \param num_colors Max amount of physical registers available.
     */
    virtual register_allocation allocate (control_flow_graph& cfg,
                                          int num_colors) = 0;
  };
}
}

#endif //_JCC__JTAC__ALLOCATION__ALLOCATOR__H_
