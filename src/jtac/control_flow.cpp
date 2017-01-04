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

#include <jtac/jtac.hpp>
#include "jtac/control_flow.hpp"
#include <unordered_map>
#include <stdexcept>


namespace jcc {
namespace jtac {

  basic_block::basic_block (basic_block_id id)
  {
    this->id = id;
    this->base = 0;
  }



  //! \brief Inserts the specified instruction to the end of the block.
  void
  basic_block::push_instruction (const jtac_instruction& inst)
  {
    this->insts.push_back (inst);
  }

  //! \brief Inserts a basic block to this block's list of predecessor blocks.
  void
  basic_block::add_prev (std::shared_ptr<basic_block> blk)
  {
    this->prev.push_back (blk);
  }

  //! \brief Inserts a basic block to this block's list of successor blocks.
  void
  basic_block::add_next (std::shared_ptr<basic_block> blk)
  {
    this->next.push_back (blk);
  }



//------------------------------------------------------------------------------

  control_flow_graph::control_flow_graph (control_flow_graph_type type,
                                          std::shared_ptr<basic_block> root)
      : root (root)
  {
    this->type = type;
  }



  //! \brief Inserts the specified <id, block> pair to the CFG.
  void
  control_flow_graph::map_block (basic_block_id id,
                                 std::shared_ptr<basic_block> blk)
  {
    this->block_map[id] = blk;
    this->blocks.push_back (blk);
  }

  //! \brief Searches for a block in the CFG by ID.
  std::shared_ptr<basic_block>
  control_flow_graph::find_block (basic_block_id id)
  {
    auto itr = this->block_map.find (id);
    return (itr == this->block_map.end ()) ? std::shared_ptr<basic_block> ()
                                           : itr->second;
  }



//------------------------------------------------------------------------------

  control_flow_analyzer::control_flow_analyzer ()
  {
    this->next_blk_id = 1;
  }



  static bool
  _is_branch_instruction (const jtac_instruction& inst)
  {
    switch (inst.op)
      {
      case JTAC_OP_JMP:
      case JTAC_OP_JE:
      case JTAC_OP_JNE:
      case JTAC_OP_JL:
      case JTAC_OP_JLE:
      case JTAC_OP_JG:
      case JTAC_OP_JGE:
        return true;

      default:
        return false;
      }
  }

  /*!
     \brief Builds a control flow graph.
   */
  control_flow_graph
  control_flow_analyzer::build_graph (const std::vector<jtac_instruction>& insts)
  {
    // pick leaders
    std::vector<bool> leaders (insts.size (), false);
    leaders[0] = true; // first instruction is a leader
    for (size_t i = 0; i < insts.size (); ++i)
      {
        auto& inst = insts[i];
        if (_is_branch_instruction (inst))
          {
            // mark next instruction and target instruction as leaders
            if (i != insts.size () - 1)
              leaders[i + 1] = true;
            if (inst.oprs[0].type != JTAC_OPR_OFFSET)
              throw std::runtime_error ("control_flow_analyzer::build_graph: branch instruction operand is not an offset");
            leaders[i + 1 + inst.oprs[0].val.off.get_offset ()] = true;
          }
      }

    // use leaders to build basic blocks
    std::unordered_map<size_t, std::shared_ptr<basic_block>> blocks;
    for (size_t i = 0; i < insts.size (); ++i)
      {
        size_t start = i;
        auto blk = std::make_shared<basic_block> (this->next_blk_id ++);

        blk->push_instruction (insts[i++]);
        for (; i < insts.size () && !leaders[i]; ++i)
          blk->push_instruction (insts[i]);

        blocks[start] = blk;
        -- i;
      }

    // link blocks together
    for (auto& p : blocks)
      {
        auto& blk = p.second;
        auto& last = blk->get_instructions ().back ();

        if (_is_branch_instruction (last))
          {
            auto target_idx = p.first + blk->get_instructions ().size () + last.oprs[0].val.off.get_offset ();
            if (blocks.find (target_idx) != blocks.end ())
              {
                auto& target = blocks[target_idx];
                target->add_prev (blk);
                blk->add_next (target);
              }
          }
        if (last.op != JTAC_OP_JMP)
          {
            auto next_idx = p.first + blk->get_instructions ().size ();
            auto itr = blocks.find (next_idx);
            if (itr != blocks.end ())
              {
                auto& next_blk = itr->second;
                next_blk->add_prev (blk);
                blk->add_next (next_blk);
              }
          }
      }

    auto cfg = control_flow_graph (control_flow_graph_type::normal, blocks[0]);
    for (auto& p : blocks)
      {
        cfg.map_block (p.second->get_id (), p.second);
        p.second->set_base (p.first);
      }
    return cfg;
  }



  //! \brief Static method for convenience.
  control_flow_graph
  control_flow_analyzer::make_cfg (const std::vector<jtac_instruction>& insts)
  {
    control_flow_analyzer an;
    return an.build_graph (insts);
  }
}
}
