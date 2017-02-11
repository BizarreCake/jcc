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

#ifndef _JCC__JTAC__ALLOCATORS__BASIC__BASIC__H_
#define _JCC__JTAC__ALLOCATORS__BASIC__BASIC__H_

#include "jtac/allocation/allocator.hpp"
#include "jtac/allocation/basic/undirected_graph.hpp"
#include "jtac/name_map.hpp"
#include <set>


namespace jcc {
namespace jtac {

  /*!
     \class basic_register_allocator
     \brief A basic register allocator!
   */
  class basic_register_allocator: public register_allocator
  {
    control_flow_graph *cfg;

    int num_colors;

    using live_range = std::set<jtac_var_id>;
    std::vector<live_range> live_ranges;
    std::unordered_map<jtac_var_id, size_t> live_range_map;

    std::set<live_range> spilled_lrs;
    int tmp_idx;

    undirected_graph infer_graph; // inference graph

    register_allocation *res; // result goes here

    // DEBUG:
    const name_map<jtac_var_id> *var_names;

   public:
    basic_register_allocator ();

   public:
    virtual register_allocation allocate (control_flow_graph& cfg,
                                          int num_colors) override;

   private:
    /*!
       Finds all global live ranges in the underlying CFG, and maps all SSA
       names to a matching live range.
     */
    void discover_live_ranges ();

    //! \brief Joins equal live ranges together.
    void nub_live_ranges ();

    /*!
       \brief Builds the inference graph for the underlying CFG.

       The inference graph is populated with a node for every global live range
       in the CFG. Then, an edge is drawn between every two nodes whose live
       ranges interfere at some point in the CFG.
     */
    void build_inference_graph ();


    /*!
       \brief Attempts to color the inference graph.
       \returns True if the graph has been successfully colored.
     */
    bool color_graph ();

    //! \brief Picks a constrained node to remove from the inference graph.
    undirected_graph::node_id pick_constrained_node ();

    //! \brief Picks a node to spill from the inference graph.
    undirected_graph::node_id pick_node_to_spill (
        const std::unordered_map<undirected_graph::node_id, register_color>& color_map);


    //! \brief Inserts spill code for the specified live range into the CFG.
    void insert_spill_code (const live_range& lr);

    /*!
       Checks whether the specified instruction's operands contain variables
       from the the given live range.
     */
    bool contains_live_range_use (const jtac_instruction& inst, const live_range& lr);

   public:
    //! \brief DEBUG
    void print (const name_map<jtac_var_id>& var_names);

    //! \brief DEBUG
    void print_inference_graph (
        std::unordered_map<undirected_graph::node_id, register_color>& color_map);

    //! \brief DEBUG
    void set_var_names (const name_map<jtac_var_id>& var_names);
  };
}
}

#endif //_JCC__JTAC__ALLOCATORS__BASIC__BASIC__H_
