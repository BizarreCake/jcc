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
  std::shared_ptr<basic_block>
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
        auto blk = std::make_shared<basic_block> ();

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
            auto target_idx = p.first + 1 + last.oprs[0].val.off.get_offset ();
            if (blocks.find (target_idx) != blocks.end ())
              {
                auto& target = blocks[target_idx];
                target->add_prev (blk);
                blk->add_next (target);
              }
          }

        auto next_idx = p.first + 1;
        auto& next_blk = blocks[next_idx];
        next_blk->add_prev (blk);
        blk->add_next (next_blk);
      }

    return blocks[0];
  }
}
