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

#include "jtac/allocation/basic/basic.hpp"
#include "jtac/data_flow.hpp"


namespace jcc {
namespace jtac {

  basic_register_allocator::basic_register_allocator ()
  {
    this->cfg = nullptr;
  }



  register_allocation
  basic_register_allocator::allocate (const control_flow_graph& cfg)
  {
    if (cfg.get_type () != control_flow_graph_type::ssa)
      throw std::runtime_error ("basic_register_allocation::allocate: CFG must be in SSA form");

    this->cfg = &cfg;
    this->discover_live_ranges ();
    this->build_inference_graph ();
  }



  /*!
     Finds all global live ranges in the underlying CFG, and maps all SSA
     names to a matching live range.
   */
  void
  basic_register_allocator::discover_live_ranges ()
  {
    this->live_range_map.clear ();
    this->live_ranges.clear ();

    std::unordered_map<jtac_var_id, live_range> lr_map;

    for (auto& blk : this->cfg->get_blocks ())
      {
        for (auto& inst : blk->get_instructions ())
          {
            if (inst.op != JTAC_SOP_ASSIGN_PHI)
              continue;

            auto dest_var = inst.oprs[0].val.var.get_id ();

            live_range lr;
            lr.insert (dest_var);

            // insert the sets associated with the operands in to the final
            // live range.
            for (unsigned i = 0; i < inst.extra.count; ++i)
              {
                auto opr = inst.extra.oprs[i].val.var.get_id ();
                auto itr = lr_map.find (opr);
                if (itr == lr_map.end ())
                  lr.insert (opr);
                else
                  {
                    auto& opr_lr = itr->second;
                    lr.insert (opr_lr.begin (), opr_lr.end ());
                  }
              }

            // update operands' sets.
            for (unsigned i = 0; i < inst.extra.count; ++i)
              {
                auto opr = inst.extra.oprs[i].val.var.get_id ();
                lr_map[opr] = lr;
              }

            lr_map[dest_var] = std::move (lr);
          }
      }

    for (auto& p : lr_map)
      {
        auto& lr = p.second;
        size_t lr_id = this->live_ranges.size ();

        for (auto var : lr)
          this->live_range_map[var] = lr_id;
        this->live_ranges.push_back (std::move (lr));
      }

    // create a live range for variables that weren't handled
    for (auto& blk : this->cfg->get_blocks ())
      for (auto& inst : blk->get_instructions ())
        if (is_opcode_assign (inst.op))
          {
            auto var = inst.oprs[0].val.var.get_id ();
            if (this->live_range_map.find (var) == this->live_range_map.end ())
              {
                live_range lr;
                lr.insert (var);
                this->live_range_map[var] = this->live_ranges.size ();
                this->live_ranges.push_back (std::move (lr));
              }
          }
  }


  /*!
     \brief Builds the inference graph for the underlying CFG.

     The inference graph is populated with a node for every global live range
     in the CFG. Then, an edge is drawn between every two nodes whose live
     ranges interfere at some point in the CFG.
   */
  void
  basic_register_allocator::build_inference_graph ()
  {
    this->infer_graph.clear ();

    // insert a node for every global live range
    for (size_t i = 0; i < this->live_ranges.size (); ++i)
      this->infer_graph.add_node ((undirected_graph::node_id)i);

    live_analyzer la;
    auto live_results = la.analyze (*this->cfg);

    for (auto& blk : this->cfg->get_blocks ())
      {
        std::set<size_t> live_now;
        for (auto var : live_results.get_live_out (blk->get_id ()))
          live_now.insert (this->live_range_map[var]);

        auto& insts = blk->get_instructions ();
        for (auto itr = insts.rbegin (); itr != insts.rend (); ++itr)
          {
            auto& inst = *itr;
            int opr_start = is_opcode_assign (inst.op) ? 1 : 0;
            int opr_end = get_operand_count (inst.op);

            if (is_opcode_assign (inst.op))
              {
                auto lr_dest = this->live_range_map[inst.oprs[0].val.var.get_id ()];
                for (auto lr : live_now)
                  if (lr != lr_dest)
                    this->infer_graph.add_edge (
                        (undirected_graph::node_id)lr_dest, (undirected_graph::node_id)lr);

                live_now.erase (lr_dest);
              }

            // insert operands into LiveNow set.
            for (int i = opr_start; i < opr_end; ++i)
              if (inst.oprs[i].type == JTAC_OPR_VAR)
                live_now.insert (this->live_range_map[inst.oprs[i].val.var.get_id ()]);
            if (has_extra_operands (inst.op))
              for (unsigned i = 0; i < inst.extra.count; ++i)
                if (inst.extra.oprs[i].type == JTAC_OPR_VAR)
                  live_now.insert (this->live_range_map[inst.extra.oprs[i].val.var.get_id ()]);

          }
      }
  }
}
}
