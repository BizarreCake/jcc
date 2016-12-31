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

#ifndef _JCC__JTAC__CONTROL_FLOW__H_
#define _JCC__JTAC__CONTROL_FLOW__H_

#include "jtac/jtac.hpp"
#include <vector>
#include <memory>


namespace jcc {
namespace jtac {

  //! \brief Used to identify basic blocks.
  using basic_block_id = int;

  /*!
     \class basic_block
     \brief A straight-line piece of code without any jumps.
   */
  class basic_block
  {
    basic_block_id id;
    std::vector<jtac_instruction> insts;

    std::vector<std::shared_ptr<basic_block>> prev;
    std::vector<std::shared_ptr<basic_block>> next;

   public:
    inline basic_block_id get_id () const { return this->id; }
    inline const auto& get_instructions () const { return this->insts; }

    inline const auto& get_prev () const { return this->prev; }
    inline const auto& get_next () const { return this->next; }

   public:
    basic_block (basic_block_id id);

   public:
    //! \brief Inserts the specified instruction to the end of the block.
    void push_instruction (const jtac_instruction& inst);

    //! \brief Inserts a basic block to this block's list of predecessor blocks.
    void add_prev (std::shared_ptr<basic_block> blk);

    //! \brief Inserts a basic block to this block's list of successor blocks.
    void add_next (std::shared_ptr<basic_block> blk);
  };



  /*!
     \enum control_flow_graph_type
     \brief Describes the type of a CFG's contents.
   */
  enum class control_flow_graph_type
  {
    normal,
    ssa,      //! \brief The CFG is in SSA form.
  };

  /*!
     \class control_flow_graph
     \brief A control flow graph!
   */
  class control_flow_graph
  {
    control_flow_graph_type type;
    std::shared_ptr<basic_block> root;

   public:
    inline control_flow_graph_type get_type () const { return this->type; }

    inline auto& get_root () { return this->root; }
    inline const auto& get_root () const { return this->root; }

   public:
    control_flow_graph (control_flow_graph_type type,
                        std::shared_ptr<basic_block> root);
  };



  /*!
     \class control_flow_analyzer
     \brief Performs control flow analysis.
   */
  class control_flow_analyzer
  {
    basic_block_id next_blk_id;

   public:
    control_flow_analyzer ();

   public:
    /*!
       \brief Builds a control flow graph.
     */
    control_flow_graph build_graph (
        const std::vector<jtac_instruction>& insts);

   public:
    //! \brief Static method for convenience.
    static control_flow_graph make_cfg (const std::vector<jtac_instruction>& insts);
  };
}
}

#endif //_JCC__JTAC__CONTROL_FLOW__H_
