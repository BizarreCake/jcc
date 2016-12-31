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

#include "jtac/ssa.hpp"


namespace jcc {
namespace jtac {

  /*!
     \brief Builds a new CFG in maximal SSA form.
     \param cfg The control flow graph to transform.
     \return A copy of the CFG in maximal SSA form.
   */
  control_flow_graph
  ssa_builder::build_maximal (const control_flow_graph& cfg)
  {

  }
}
}
