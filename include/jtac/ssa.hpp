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

#ifndef _JCC__JTAC__SSA__H_
#define _JCC__JTAC__SSA__H_

#include "jtac/jtac.hpp"
#include "jtac/control_flow.hpp"
#include "jtac/data_flow.hpp"
#include <set>
#include <map>
#include <stack>


namespace jcc {
namespace jtac {

  /*!
     \class ssa_builder
     \brief Transforms control flow graphs into SSA form.
   */
  class ssa_builder
  {
    control_flow_graph *cfg;

    std::set<jtac_var_id> globals;
    std::map<jtac_var_id, std::set<basic_block_id>> def_blocks;
    dom_analysis dom_results;

    // used when renaming:
    std::map<jtac_var_id, int> counters;
    std::map<jtac_var_id, std::stack<int>> stacks;

   public:
    /*!
       \brief Transforms the specified CFG into SSA form.
       \param cfg The control flow graph to transform.
     */
    void transform (control_flow_graph& cfg);

   private:
    /*!
       Inserts phi-functions at the start of every block that has multiple
       predecessors. A phi-function is inserted for every name that is defined
       or used in the control flow graph.
     */
    void insert_phi_functions ();

    //! \brief Finds all variables that are live across multiple blocks.
    void find_globals (std::set<jtac_var_id>& globals,
                       std::map<jtac_var_id, std::set<basic_block_id>>& blocks);

    //! \brief Renames variables so that each definition is unique.
    void rename ();

    void rename_block (basic_block& blk);

    jtac_var_id new_name (jtac_var_id base);

    //! \brief Returns a list of all variables defined or used in the CFG.
    std::set<jtac_var_id> enum_vars ();
  };
}
}

#endif //_JCC__JTAC__SSA__H_
