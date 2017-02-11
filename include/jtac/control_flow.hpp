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

#ifndef _JCC__JTAC__CONTROL_FLOW__H_
#define _JCC__JTAC__CONTROL_FLOW__H_

#include "jtac/jtac.hpp"
#include <vector>
#include <memory>
#include <unordered_map>


namespace jcc {
namespace jtac {

  /*!
     \class basic_block
     \brief A straight-line piece of code without any jumps.
   */
  class basic_block
  {
    basic_block_id id;
    std::vector<jtac_instruction> insts;
    size_t base;

    std::vector<std::shared_ptr<basic_block>> prev;
    std::vector<std::shared_ptr<basic_block>> next;

   public:
    inline basic_block_id get_id () const { return this->id; }

    inline auto& get_instructions () { return this->insts; }
    inline const auto& get_instructions () const { return this->insts; }
    inline void clear_instructions () { this->insts.clear (); }

    inline const auto& get_prev () const { return this->prev; }
    inline const auto& get_next () const { return this->next; }

    inline size_t get_base () const { return this->base; }
    inline void set_base (size_t base) { this->base = base; }

   public:
    basic_block (basic_block_id id);

   public:
    //! \brief Inserts the specified instruction to the end of the block.
    void push_instruction (const jtac_instruction& inst);

    //! \brief Inserts the specified instruction to the beginning of the block.
    void push_instruction_front (const jtac_instruction& inst);

    //! \brief Inserts a range of instructions to the beginning of the block.
    template<typename Itr>
    void
    push_instructions_front (Itr start, Itr end)
    { this->insts.insert (this->insts.begin (), start, end); }

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

    std::unordered_map<basic_block_id, std::shared_ptr<basic_block>> block_map;
    std::vector<std::shared_ptr<basic_block>> blocks;

   public:
    inline control_flow_graph_type get_type () const { return this->type; }
    inline void set_type (control_flow_graph_type type) { this->type = type; }

    inline auto& get_root () { return this->root; }
    inline const auto& get_root () const { return this->root; }

    inline auto& get_blocks () { return this->blocks; }
    inline const auto& get_blocks () const { return this->blocks; }

    inline size_t get_size () const { return this->blocks.size (); }

   public:
    control_flow_graph (control_flow_graph_type type,
                        std::shared_ptr<basic_block> root);

   public:
    //! \brief Inserts the specified <id, block> pair to the CFG.
    void map_block (basic_block_id id,
                    std::shared_ptr<basic_block> blk);

    //! \brief Searches for a block in the CFG by ID.
    std::shared_ptr<basic_block> find_block (basic_block_id id);
    std::shared_ptr<const basic_block> find_block (basic_block_id id) const;
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
