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

#ifndef _JCC__JTAC__DATA_FLOW__H_
#define _JCC__JTAC__DATA_FLOW__H_

#include "jtac/control_flow.hpp"
#include <map>
#include <memory>
#include <set>


namespace jcc {
namespace jtac {

  /*!
     \class iterative_analysis
     \brief Base class for a special class of data-flow analyzers.

     Serves as the base class for global data-flow analyzers whose problems
     can be solved using an iterative fixed-point algorithm.
   */
  class iterative_analyzer
  {
   protected:
    struct fragment
    {
      virtual ~fragment () { }
    };

   private:
    std::map<basic_block_id, fragment *> frags;

   protected:
    const control_flow_graph *cfg;

    const control_flow_graph& get_active_cfg () const { return *this->cfg; }
    void set_active_cfg (const control_flow_graph& cfg) { this->cfg = &cfg; }

   public:
    virtual ~iterative_analyzer ();

   protected:
    /*!
     * \brief Computes a fragment for the specified basic block.
     * \return True if the fragment has been modified.
     */
    virtual bool compute_fragment (fragment& frag, const basic_block& blk) = 0;

    //! \brief Initializes a fragment for the first time.
    virtual std::unique_ptr<fragment> compute_init_fragment (const basic_block& blk) = 0;

    //! \brief Returns the fragment associated with the specified basic block ID.
    fragment& get_fragment (basic_block_id id);

    fragment&
    get_fragment (const basic_block& blk)
    { return this->get_fragment (blk.get_id ()); }

   protected:
    /*!
       Solves the data-flow problem for the specified CFG using an iterative
       fixed-point algorithm.
     */
    void solve (const control_flow_graph& cfg);
  };



//------------------------------------------------------------------------------

  using definition = std::pair<basic_block_id, size_t>;

  class reach_def_analysis
  {
    std::unordered_map<basic_block_id, std::set<definition>> block_map;

   public:
    void add_block (basic_block_id id, std::set<definition>&& defs);

    //! \brief Returns the definitions reaching the specified block.
    const std::set<definition>& get_block (basic_block_id id);
  };

  /*!
     \class reach_def_analyzer
     \brief Computes reaching definitions.
   */
  class reach_def_analyzer: public iterative_analyzer
  {
   private:
    struct my_fragment: public fragment
    {
      std::set<definition> defs;
    };

   private:
    std::vector<definition> all_defs;
    std::map<basic_block_id, std::set<definition>> de_defs, de_kills;

   public:
    /*!
       \brief Performs a reaching-definintions analysis on the specified CFG.
       \param cfg The control flow graph to analyze.
       \return The results of the analysis.
     */
    reach_def_analysis analyze (const control_flow_graph& cfg);

   protected:
    virtual bool compute_fragment (fragment& frag, const basic_block& blk) override;

    virtual std::unique_ptr<fragment> compute_init_fragment (const basic_block& blk) override;

   private:
    //! \brief Finds all definitions that appear in the CFG.
    void get_all_defs ();

    //! \brief Returns a list of all downward-exposed definitions in a block.
    std::set<definition>& de_def (const basic_block& blk);

    //! \brief Returns a list of all definitions that are obscured by a
    //!        defintion in a block.
    std::set<definition>& de_kill (const basic_block& blk);
  };



//------------------------------------------------------------------------------

  /*!
     \class dom_analysis
     \brief Dominance analysis results.
   */
  class dom_analysis
  {
    std::unordered_map<basic_block_id, std::set<basic_block_id>> block_map;
    std::unordered_map<basic_block_id, basic_block_id> idom_map;
    std::unordered_map<basic_block_id, std::set<basic_block_id>> df_map;

   public:
    void add_block (basic_block_id id, std::set<basic_block_id>&& doms);

    //! \brief Returns the set of blocks dominating the specified block.
    const std::set<basic_block_id>& get_block (basic_block_id id);


    //! \brief Sets a block's immediate dominator.
    void set_idom (basic_block_id id, basic_block_id idom);

    //! \brief Returns the specified block's immediate dominator.
    basic_block_id get_idom (basic_block_id id) const;


    //! \brief Inserts a block into a specified block's dominance frontier set.
    void add_df (basic_block_id id, basic_block_id df);

    //! \brief Returns the dominance frontier set of a specified block.
    std::set<basic_block_id>& get_dfs (basic_block_id id);
  };

  /*!
     \class dom_analyzer
     \brief Dominance analyzer.
   */
  class dom_analyzer: public iterative_analyzer
  {
   private:
    struct my_fragment: public fragment
    {
      std::set<basic_block_id> doms;
    };

   public:
    /*!
       \brief Performs dominance analysis on the specified CFG.
       \param cfg The control flow graph to analyze.
       \return The results of the analysis.
     */
    dom_analysis analyze (const control_flow_graph& cfg);

   private:
    //! \brief Finds all immediate dominators.
    void compute_idoms (dom_analysis& result);

    //! \brief Computes dominance frontiers.
    void compute_dfs (dom_analysis& result);

   protected:
    virtual bool compute_fragment (fragment& frag, const basic_block& blk) override;

    virtual std::unique_ptr<fragment> compute_init_fragment (const basic_block& blk) override;
  };
}
}

#endif //_JCC__JTAC__DATA_FLOW__H_
