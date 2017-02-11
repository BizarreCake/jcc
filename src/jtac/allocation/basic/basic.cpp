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
#include "jtac/assembler.hpp"
#include "jtac/jtac.hpp"
#include <stack>
#include <memory>

#include "jtac/printer.hpp" // DEBUG
#include <iostream> // DEBUG


namespace jcc {
namespace jtac {

  // DEBUG
  static std::string
  _print_var (jtac_var_id var, const name_map<jtac_var_id>& var_names)
  {
    jtac::printer p;
    p.set_var_names (var_names);

    jtac_tagged_operand opr;
    opr = jtac_var (var);
    return p.print_operand (opr);
  }



  basic_register_allocator::basic_register_allocator ()
  {
    this->cfg = nullptr;
    this->num_colors = 0;
    this->tmp_idx = 0;

    this->var_names = nullptr;
  }



  register_allocation
  basic_register_allocator::allocate (control_flow_graph& cfg,
                                      int num_colors)
  {
    if (cfg.get_type () != control_flow_graph_type::ssa)
      throw std::runtime_error ("basic_register_allocation::allocate: CFG must be in SSA form");

    this->cfg = &cfg;
    this->num_colors = num_colors;
    this->spilled_lrs.clear ();
    this->tmp_idx = 0;

    register_allocation res;
    this->res = &res;

    do
      {
        this->discover_live_ranges ();
        this->build_inference_graph ();
        //if (this->color_graph ())
        //  break;

        /*
        for (size_t i = 1; i <= this->cfg->get_size (); ++i)
          {
            auto blk = cfg.find_block (i);
            jcc::jtac::printer printer;
            printer.set_var_names (*this->var_names);
            std::cout << printer.print_basic_block (*blk) << std::endl << std::endl;
          }
        std::cout << std::endl;

        this->print (*this->var_names);
        //*/
      }
    while (!this->color_graph ());

    this->res = nullptr;
    return res;
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
        if (is_opcode_assign (inst.op) || inst.op == JTAC_SOP_LOAD)
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

    this->nub_live_ranges ();
  }

  //! \brief Joins equal live ranges together.
  void
  basic_register_allocator::nub_live_ranges ()
  {
    std::map<live_range, int> lrs;
    for (auto& lr : this->live_ranges)
      if (lrs.find (lr) == lrs.end ())
        {
          int idx = (int)lrs.size ();
          lrs[lr] = idx;
        }

    std::vector<live_range> ord_lrs;
    ord_lrs.resize (lrs.size ());
    for (auto& p : lrs)
      ord_lrs[p.second] = p.first;

    std::vector<jtac_var_id> vars;
    for (auto& p : this->live_range_map)
      vars.push_back (p.first);
    for (auto var : vars)
      this->live_range_map[var] = (size_t)lrs[this->live_ranges[this->live_range_map[var]]];

    this->live_ranges = ord_lrs;

    std::cout << "Discovered live ranges:" << std::endl;
    for (size_t i = 0; i < this->live_ranges.size (); ++i)
      {
        std::cout << "    LR#" << (i + 1) << ": ";
        for (auto var : this->live_ranges[i])
          std::cout << _print_var (var, *this->var_names) << ' ';
        std::cout << std::endl;
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

    std::cout << "Building inference graph:" << std::endl;
    for (auto& blk : this->cfg->get_blocks ())
      {
        std::set<size_t> live_now;
        for (auto var : live_results.get_live_out (blk->get_id ()))
          live_now.insert (this->live_range_map[var]);

        auto& insts = blk->get_instructions ();
        for (auto itr = insts.rbegin (); itr != insts.rend (); ++itr)
          {
            auto& inst = *itr;

            {
              printer p;
              p.set_var_names (*this->var_names);
              std::cout << "    inst: " << p.print_instruction (inst) << std::endl;
            }

            if (inst.op == JTAC_SOP_STORE || inst.op == JTAC_SOP_UNLOAD)
              {
                if (inst.oprs[0].type == JTAC_OPR_VAR)
                  live_now.insert (this->live_range_map[inst.oprs[0].val.var.get_id ()]);
              }
            else if (inst.op == JTAC_SOP_LOAD)
              {
                auto lr_dest = this->live_range_map[inst.oprs[0].val.var.get_id ()];
                for (auto lr : live_now)
                  if (lr != lr_dest)
                    this->infer_graph.add_edge (
                        (undirected_graph::node_id)lr_dest, (undirected_graph::node_id)lr);

                live_now.erase (lr_dest);
              }
            else
              {
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

            {
              std::cout << "    LiveNow: ";
              for (auto lri : live_now)
                std::cout << "LR#" << (lri + 1) << " ";
              std::cout << std::endl;
            }
          }
      }
  }


  /*!
     \brief Attempts to color the inference graph.
     \returns True if the graph has been successfully colored.
   */
  bool
  basic_register_allocator::color_graph ()
  {
    //
    // Pick out nodes from the inference graph until it is empty.
    //
    std::stack<std::unique_ptr<undirected_graph::node>> stk;
    while (!this->infer_graph.empty ())
      {
        // pick node to remove from graph
        undirected_graph::node_id id;
        if (this->infer_graph.has_less_k (this->num_colors))
          // pick an unconstrained node to remove from the graph.
          id = this->infer_graph.find_less_k (this->num_colors);
        else
          {
            // no unconstrained nodes left in the graph.
            // carefully pick a constrained node.
            id = this->pick_constrained_node ();
          }

        stk.emplace (new undirected_graph::node (this->infer_graph.get_node (id)));
        this->infer_graph.remove_node (id);
      }

    std::cout << "Reconstructing graph:" << std::endl;

    //
    // Reconstruct inference graph, coloring nodes at the same time.
    //
    std::unordered_map<undirected_graph::node_id, register_color> color_map;
    while (!stk.empty ())
      {
        this->print_inference_graph (color_map);

        // insert node back into the graph.
        auto& ptr = stk.top ();
        this->infer_graph.add_node (ptr->value);
        for (auto id : ptr->nodes)
          this->infer_graph.add_edge (ptr->value, id);

        // color node
        std::set<register_color> avail;
        for (int i = 0; i < this->num_colors; ++i)
          avail.insert ((register_color)i);
        for (auto n : ptr->nodes)
          {
            auto itr = color_map.find (n);
            if (itr != color_map.end ())
              avail.erase (itr->second);
          }
        if (!avail.empty ())
          color_map[ptr->value] = *avail.begin ();

        stk.pop ();
      }

    this->print_inference_graph (color_map);

    if (color_map.size () != this->infer_graph.size ())
      {
        // not all nodes colored.
        // spill.

        auto id = this->pick_node_to_spill (color_map);
        this->insert_spill_code (this->live_ranges[id]);

//        for (auto n : this->infer_graph.get_nodes ())
//          if (color_map.find (n->value) != color_map.end ())
//            {
//              this->insert_spill_code (this->live_ranges[n->value]);
//            }

        return false;
      }

    for (auto& p : this->live_range_map)
      {
        auto lr_id = p.second;
        this->res->set_color (p.first, color_map[lr_id]);
      }

    return true;
  }

  //! \brief Picks a constrained node to remove from the inference graph.
  undirected_graph::node_id
  basic_register_allocator::pick_constrained_node ()
  {
    //
    // TODO
    //
    return this->infer_graph.get_nodes ()[0]->value;
  }

  //! \brief Picks a node to spill from the inference graph.
  undirected_graph::node_id
  basic_register_allocator::pick_node_to_spill (
      const std::unordered_map<undirected_graph::node_id, register_color>& color_map)
  {
    //
    // TODO
    //
    for (auto n : this->infer_graph.get_nodes ())
      if (color_map.find (n->value) == color_map.end ())
        {
          auto& lr = this->live_ranges[n->value];
          if (this->spilled_lrs.find (lr) != this->spilled_lrs.end ())
            continue;

          this->spilled_lrs.insert (lr);
          return n->value;
        }


    throw std::runtime_error ("basic_register_allocator::pick_node_to_spill: node not found");
  }


  /*!
       Checks whether the specified instruction's operands contain variables
       from the the given live range.
     */
  bool
  basic_register_allocator::contains_live_range_use (const jtac_instruction& inst,
                                                     const live_range& lr)
  {
    int opr_start = is_opcode_assign (inst.op) ? 1 : 0;
    int opr_end = get_operand_count (inst.op);
    for (int i = opr_start; i < opr_end; ++i)
      if (inst.oprs[i].type == JTAC_OPR_VAR
          && lr.find (inst.oprs[i].val.var.get_id ()) != lr.end ())
        return true;

    if (has_extra_operands (inst.op))
      for (int i = 0; i < inst.extra.count; ++i)
        if (inst.extra.oprs[i].type == JTAC_OPR_VAR
            && lr.find (inst.extra.oprs[i].val.var.get_id ()) != lr.end ())
          return true;

    return false;
  }

  //! \brief Spills the specified live range.
  void
  basic_register_allocator::insert_spill_code (const live_range& lr)
  {
    std::cout << "Spilling live range: ";
    for (auto var : lr)
      std::cout << this->var_names->get_name (var_base (var)) << "_" << var_subscript (var) << " ";
    std::cout << std::endl;

    assembler asem;
    for (auto& blk : this->cfg->get_blocks ())
      {
        std::vector<jtac_instruction> insts;
        for (auto& inst : blk->get_instructions ())
          {
            if (inst.op == JTAC_SOP_ASSIGN_PHI)
              {
                if (!(inst.oprs[0].type == JTAC_OPR_VAR
                    && lr.find (inst.oprs[0].val.var.get_id ()) != lr.end ()))
                  {
                    bool found = false;
                    for (int i = 0; i < inst.extra.count; ++i)
                      if (inst.extra.oprs[i].type == JTAC_OPR_VAR
                          && lr.find (inst.extra.oprs[i].val.var.get_id ()) != lr.end ())
                        { found = true; break; }
                    if (!found)
                      insts.push_back (inst);
                  }
                continue;
              }

            bool need_store = false;
            bool need_load = false;
            auto tmp_var = make_var_id (var_base (*lr.begin ()), 0, this->tmp_idx + 1);

            if (is_opcode_assign (inst.op))
              {
                // append store after definitions of variables in the live range.
                if (inst.oprs[0].type == JTAC_OPR_VAR
                    && lr.find (inst.oprs[0].val.var.get_id ()) != lr.end ())
                  {
                    need_store = true;

                    // replace destination variable with temporary variable
                    inst.oprs[0] = jtac_var (tmp_var);
                  }
              }

            // wrap uses of varaibles in the live range with load+unload
            if (this->contains_live_range_use (inst, lr))
              {
                need_load = true;

                // replace uses with the temporary variable
                int opr_start = is_opcode_assign (inst.op) ? 1 : 0;
                int opr_end = get_operand_count (inst.op);
                for (int i = opr_start; i < opr_end; ++i)
                  if (inst.oprs[i].type == JTAC_OPR_VAR
                      && lr.find (inst.oprs[i].val.var.get_id ()) != lr.end ())
                    inst.oprs[i] = jtac_var (tmp_var);
                if (has_extra_operands (inst.op))
                  for (int i = 0; i < inst.extra.count; ++i)
                    if (inst.extra.oprs[i].type == JTAC_OPR_VAR
                        && lr.find (inst.extra.oprs[i].val.var.get_id ()) != lr.end ())
                      inst.extra.oprs[i] = jtac_var (tmp_var);
              }

            if (need_load || need_store)
              ++ this->tmp_idx;

            if (need_load)
              {
                // load
                auto& si = asem.emit_load (jtac_var (tmp_var));
                for (auto var : lr)
                  si.push_extra (jtac_var (var));
                insts.push_back (asem.get_instructions ().back ());
                asem.clear ();
              }

            insts.push_back (inst);

            if (need_store)
              {
                // store
                asem.emit_store (jtac_var (tmp_var));
                insts.push_back (asem.get_instructions ().back ());
                asem.clear ();
              }
            else if (need_load)
              {
                // unload
                asem.emit_unload (jtac_var (tmp_var));
                insts.push_back (asem.get_instructions ().back ());
                asem.clear ();
              }
          }

        blk->clear_instructions ();
        blk->push_instructions_front (insts.begin (), insts.end ());
      }
  }



  /*!
     \brief Prints the state of the allocator.

     NOTE: This is a debug function.
   */
  void
  basic_register_allocator::print (const name_map<jtac_var_id>& var_names)
  {
    //
    // print live ranges
    //
    std::cout << "Live ranges:" << std::endl;
    for (size_t i = 0; i < this->live_ranges.size (); ++i)
      {
        std::cout << "    LR#" << (i + 1) << ": ";
        for (auto var : this->live_ranges[i])
          std::cout << _print_var (var, var_names) << ' ';
        std::cout << std::endl;
      }
    std::cout << std::endl;

    //
    // print inference graph
    //
    std::cout << "Inference graph:" << std::endl;
    for (auto n : this->infer_graph.get_nodes ())
      {
        std::cout << "    LR#" << (n->value + 1) << " interferes with: ";
        for (auto an : n->nodes)
          std::cout << "LR#" << (an + 1) << " ";
        std::cout << std::endl;
      }
    std::cout << std::endl;
  }

  //! \brief DEBUG
  void
  basic_register_allocator::print_inference_graph (
      std::unordered_map<undirected_graph::node_id, register_color>& color_map)
  {
    std::cout << "    --------------------" << std::endl;
    for (auto n : this->infer_graph.get_nodes ())
      {
        std::cout << "    LR#" << (n->value + 1);
        if (color_map.find (n->value) != color_map.end ())
          std::cout << '[' << color_map[n->value] << ']';
        else
          std::cout << "[]";
        std::cout << ": ";
        for (auto id : n->nodes)
          {
            auto& other = infer_graph.get_node (id);
            std::cout << "LR#" << (other.value + 1);
            if (color_map.find (other.value) != color_map.end ())
              std::cout << '[' << color_map[other.value] << ']';
            else
              std::cout << "[]";
            std::cout << ' ';
          }
        std::cout << std::endl;
      }

    std::cout << std::endl;
  }

  //! \brief DEBUG
  void
  basic_register_allocator::set_var_names (const name_map<jtac_var_id>& var_names)
  {
    this->var_names = &var_names;
  }
}
}
