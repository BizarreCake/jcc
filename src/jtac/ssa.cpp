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

#include "jtac/ssa.hpp"
#include "jtac/assembler.hpp"
#include "jtac/data_flow.hpp"
#include <algorithm>
#include <iostream>


namespace jcc {
namespace jtac {

  /*!
    \brief Transforms the specified CFG into SSA form.
    \param cfg The control flow graph to transform.
  */
  void
  ssa_builder::transform (control_flow_graph& cfg)
  {
    this->cfg = &cfg;

    dom_analyzer da;
    this->dom_results = da.analyze (*this->cfg);

    this->find_globals (this->globals, this->def_blocks);
    this->define_initial_names ();
    this->insert_phi_functions ();
    this->rename ();

    this->cfg->set_type (control_flow_graph_type::ssa);
  }



  static bool
  _has_phi_function (const std::vector<jtac_instruction>& insts, jtac_var_id var)
  {
    for (auto& inst : insts)
      {
        if (inst.op != JTAC_SOP_ASSIGN_PHI)
          break;
        if (inst.oprs[0].type == JTAC_OPR_VAR &&
            inst.oprs[0].val.var.get_id () == var)
          return true;
      }

    return false;
  }

  /*!
     Inserts phi-functions at the start of every block that has multiple
     predecessors. A phi-function is inserted for every name that is defined
     or used in the control flow graph.
   */
  void
  ssa_builder::insert_phi_functions ()
  {
    std::map<jtac_var_id, std::set<basic_block_id>> handled_blocks;
    std::map<basic_block_id, assembler> asems;
    for (auto var : this->globals)
      {
        auto& var_blocks = this->def_blocks[var];
        std::vector<basic_block_id> work_list (var_blocks.begin (), var_blocks.end ());
        while (!work_list.empty ())
          {
            auto bid = work_list.back ();
            work_list.pop_back ();

            auto& dfs = this->dom_results.get_dfs (bid);
            for (auto df : dfs)
              {
                auto blk = this->cfg->find_block (df);
                auto& asem = asems[df];
                if (!_has_phi_function (blk->get_instructions (), var) &&
                    !_has_phi_function (asem.get_instructions (), var))
                  {
                    // insert phi function to the beginning of the block.
                    auto& phi = asem.emit_assign_phi (jtac_var (var));
                    for (size_t i = 0; i < blk->get_prev ().size (); ++i)
                      phi.push_extra (jtac_var (var));

                    if (handled_blocks.find (var) == handled_blocks.end () ||
                        handled_blocks[var].find (df) == handled_blocks[var].end ())
                      {
                        work_list.push_back (df);
                        handled_blocks[var].insert (df);
                      }
                  }
              }
          }
      }

    for (auto& p : asems)
      {
        auto blk = this->cfg->find_block (p.first);
        auto& insts = p.second.get_instructions ();
        blk->push_instructions_front (insts.begin (), insts.end ());
      }
  }

  //! \brief Initializes the stack/counter for the first block.
  void
  ssa_builder::define_initial_names ()
  {
    auto root = this->cfg->get_root ();
    auto undef_globals = this->globals;
    for (auto& inst : root->get_instructions ())
      {
        if (is_opcode_assign (inst.op) && inst.oprs[0].type == JTAC_OPR_VAR)
          undef_globals.erase (inst.oprs[0].val.var.get_id ());
      }

    for (auto var : undef_globals)
      this->new_name (var);
  }

  //! \brief Finds all variables that are live across multiple blocks.
  void
  ssa_builder::find_globals (std::set<jtac_var_id>& globals,
                             std::map<jtac_var_id, std::set<basic_block_id>>& blocks)
  {
    for (auto& blk : this->cfg->get_blocks ())
      {
        std::set<jtac_var_id> kill;
        for (auto& inst : blk->get_instructions ())
          {
            int opr_start = is_opcode_assign (inst.op) ? 1 : 0;
            int opr_end = get_operand_count (inst.op);
            for (int i = opr_start; i < opr_end; ++i)
              if (inst.oprs[i].type == JTAC_OPR_VAR &&
                  kill.find (inst.oprs[i].val.var.get_id ()) == kill.end ())
                globals.insert (inst.oprs[i].val.var.get_id ());
            if (has_extra_operands (inst.op))
              for (int i = 0; i < inst.extra.count; ++i)
                if (inst.extra.oprs[i].type == JTAC_OPR_VAR &&
                    kill.find (inst.extra.oprs[i].val.var.get_id ()) == kill.end ())
                  globals.insert (inst.extra.oprs[i].val.var.get_id ());

            if (is_opcode_assign (inst.op) && inst.oprs[0].type == JTAC_OPR_VAR)
              {
                auto var = inst.oprs[0].val.var.get_id ();
                kill.insert (var);
                blocks[var].insert (blk->get_id ());
              }
          }
      }
  }


  //! \brief Renames variables so that each definition is unique.
  void
  ssa_builder::rename ()
  {
    this->rename_block (*this->cfg->get_root ());
  }

  void
  ssa_builder::rename_block (basic_block& blk)
  {
    auto& insts = blk.get_instructions ();
    for (auto& inst : insts)
      {
        if (inst.op == JTAC_SOP_ASSIGN_PHI)
          inst.oprs[0].val.var.set_id (this->new_name (inst.oprs[0].val.var.get_id ()));
        else
          {
            int opr_start = is_opcode_assign (inst.op) ? 1 : 0;
            int opr_end = get_operand_count (inst.op);

            // rename operands
            for (int i = opr_start; i < opr_end; ++i)
              if (inst.oprs[i].type == JTAC_OPR_VAR)
                {
                  auto var = inst.oprs[i].val.var.get_id ();
                  auto& stk = this->stacks[var];
                  if (stk.empty ())
                    throw std::runtime_error ("ssa_builder:rename_block: variable used before being defined");
                  inst.oprs[i].val.var.set_id (make_var_id (var, stk.top ()));
                }
            if (has_extra_operands (inst.op))
              for (int i = 0; i < inst.extra.count; ++i)
                if (inst.extra.oprs[i].type == JTAC_OPR_VAR)
                  {
                    auto var = inst.extra.oprs[i].val.var.get_id ();
                    auto& stk = this->stacks[var];
                    if (stk.empty ())
                      throw std::runtime_error ("ssa_builder:rename_block: variable used before being defined");
                    inst.extra.oprs[i].val.var.set_id (make_var_id (var, stk.top ()));
                  }

            // rename name being assigned
            if (is_opcode_assign (inst.op) && inst.oprs[0].type == JTAC_OPR_VAR)
              inst.oprs[0].val.var.set_id (this->new_name (inst.oprs[0].val.var.get_id ()));
          }
      }

    // fill phi-function parameters
    for (auto& next : blk.get_next ())
      {
        size_t idx = 0;
        for (size_t i = 0; i < next->get_prev ().size (); ++i)
          if (next->get_prev ()[i]->get_id () == blk.get_id ())
            { idx = i; break; }

        for (auto& inst : next->get_instructions ())
          {
            if (inst.op != JTAC_SOP_ASSIGN_PHI)
              break;
            auto var = inst.extra.oprs[idx].val.var.get_id ();
            auto& stk = this->stacks[var_base (var)];
            //std::cout << "blk: " << blk.get_id () << ", next: " << next->get_id () << ", var: " << var << ", arg count: " << (int)inst.extra.count << ", idx: " << idx << ", stack: " << stk.size () << std::endl;
            if (stk.empty ())
              throw std::runtime_error ("ssa_builder::rename_block: bad");
              //// DEBUG
              //inst.extra.oprs[idx].val.var.set_id (this->new_name (inst.extra.oprs[idx].val.var.get_id ()));
            else
              inst.extra.oprs[idx].val.var.set_id (make_var_id (var_base (var), stk.top()));
          }
      }

    // recurse
    for (auto& b : this->cfg->get_blocks ())
      {
        if (b->get_id () == blk.get_id () || b->get_id () == this->cfg->get_root ()->get_id ())
          continue;

        auto idom = this->dom_results.get_idom (b->get_id ());
        if (blk.get_id () == idom)
          this->rename_block (*b);
      }

    for (auto& inst : insts)
      if (is_opcode_assign (inst.op) && inst.oprs[0].type == JTAC_OPR_VAR)
        {
          auto var = inst.oprs[0].val.var.get_id ();
          auto& stk = this->stacks[var_base (var)];
          if (!stk.empty ())
            stk.pop ();
        }
  }

  jtac_var_id
  ssa_builder::new_name (jtac_var_id base)
  {
    ++ this->counters[base];
    int i = this->counters[base];
    this->stacks[base].push (i);
    return make_var_id (base, i);
  }



  //! \brief Returns a list of all variables defined or used in the CFG.
  std::set<jtac_var_id>
  ssa_builder::enum_vars ()
  {
    std::set<jtac_var_id> vars;
    for (auto& blk : this->cfg->get_blocks ())
      {
        auto& insts = blk->get_instructions ();
        for (auto& inst : insts)
          {
            switch (inst.op)
              {
              case JTAC_OP_UNDEF:
              case JTAC_OP_RETN:
              case JTAC_SOP_LOAD:
              case JTAC_SOP_STORE:
              case JTAC_SOP_UNLOAD:
                break;

              case JTAC_OP_JMP:
              case JTAC_OP_JE:
              case JTAC_OP_JNE:
              case JTAC_OP_JL:
              case JTAC_OP_JLE:
              case JTAC_OP_JG:
              case JTAC_OP_JGE:
              case JTAC_OP_RET:
                if (inst.oprs[0].type == JTAC_OPR_VAR)
                  vars.insert (inst.oprs[0].val.var.get_id ());
                break;

              case JTAC_OP_ASSIGN:
              case JTAC_OP_CMP:
                for (int i = 0; i < 2; ++i)
                  if (inst.oprs[i].type == JTAC_OPR_VAR)
                    vars.insert (inst.oprs[i].val.var.get_id ());
                break;

              case JTAC_OP_ASSIGN_ADD:
              case JTAC_OP_ASSIGN_SUB:
              case JTAC_OP_ASSIGN_MUL:
              case JTAC_OP_ASSIGN_DIV:
              case JTAC_OP_ASSIGN_MOD:
                for (int i = 0; i < 3; ++i)
                  if (inst.oprs[i].type == JTAC_OPR_VAR)
                    vars.insert (inst.oprs[i].val.var.get_id ());
                break;

              case JTAC_OP_ASSIGN_CALL:
                for (int i = 0; i < 2; ++i)
                  if (inst.oprs[i].type == JTAC_OPR_VAR)
                    vars.insert (inst.oprs[i].val.var.get_id ());
                for (int i = 0; i < inst.extra.count; ++i)
                    if (inst.extra.oprs[i].type == JTAC_OPR_VAR)
                      vars.insert (inst.extra.oprs[i].val.var.get_id ());
                break;

              case JTAC_SOP_ASSIGN_PHI:
                if (inst.oprs[0].type == JTAC_OPR_VAR)
                  vars.insert (inst.oprs[0].val.var.get_id ());
                for (int i = 0; i < inst.extra.count; ++i)
                  if (inst.extra.oprs[i].type == JTAC_OPR_VAR)
                    vars.insert (inst.extra.oprs[i].val.var.get_id ());
                break;

              case JTAC_OP_CALL:
                if (inst.oprs[0].type == JTAC_OPR_VAR)
                  vars.insert (inst.oprs[0].val.var.get_id ());
                for (int i = 0; i < inst.extra.count; ++i)
                  if (inst.extra.oprs[i].type == JTAC_OPR_VAR)
                    vars.insert (inst.extra.oprs[i].val.var.get_id ());
                break;
              }
          }
      }
    return vars;
  }
}
}
