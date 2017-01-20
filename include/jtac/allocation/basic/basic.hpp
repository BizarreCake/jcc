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
#include <set>


namespace jcc {
namespace jtac {

  /*!
     \class basic_register_allocator
     \brief A basic register allocator!
   */
  class basic_register_allocator: public register_allocator
  {
    const control_flow_graph *cfg;

    using live_range = std::set<jtac_var_id>;
    std::vector<live_range> live_ranges;
    std::unordered_map<jtac_var_id, size_t> live_range_map;

    undirected_graph infer_graph; // inference graph

   public:
    basic_register_allocator ();

   public:
    virtual register_allocation allocate (const control_flow_graph& cfg) override;

   private:
    /*!
       Finds all global live ranges in the underlying CFG, and maps all SSA
       names to a matching live range.
     */
    void discover_live_ranges ();

    /*!
       \brief Builds the inference graph for the underlying CFG.

       The inference graph is populated with a node for every global live range
       in the CFG. Then, an edge is drawn between every two nodes whose live
       ranges interfere at some point in the CFG.
     */
    void build_inference_graph ();
  };
}
}

#endif //_JCC__JTAC__ALLOCATORS__BASIC__BASIC__H_
