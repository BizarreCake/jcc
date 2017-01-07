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

#include <iostream>
#include "jtac/data_flow.hpp"


namespace jcc {
namespace jtac {

  template<typename T>
  static bool
  _sets_equal (const T& a, const T& b)
  {
    if (a.size () != b.size ())
      return false;

    for (auto& e : a)
      if (b.find (e) == b.end ())
        return false;

    return true;
  }



  iterative_analyzer::~iterative_analyzer ()
  {
    for (auto& p : this->frags)
      delete p.second;
  }



  //! \brief Returns the fragment associated with the specified basic block ID.
  iterative_analyzer::fragment&
  iterative_analyzer::get_fragment (basic_block_id id)
  {
    return *this->frags[id];
  }



  /*!
     Solves the data-flow problem for the specified CFG using an iterative
     fixed-point algorithm.
   */
  void
  iterative_analyzer::solve (const control_flow_graph& cfg)
  {
    this->set_active_cfg (cfg);

    for (auto& p : this->frags)
      delete p.second;
    this->frags.clear ();

    // initialize fragments
    for (auto& blk : cfg.get_blocks ())
      this->frags[blk->get_id ()] = this->compute_init_fragment (*blk).release ();

    bool changed = true;
    while (changed)
      {
        changed = false;
        for (auto& blk : cfg.get_blocks ())
          if (this->compute_fragment (*this->frags[blk->get_id ()], *blk))
            changed = true;
      }
  }



//------------------------------------------------------------------------------

  void
  reach_def_analysis::add_block (basic_block_id id, std::set<definition>&& defs)
  {
    this->block_map[id] = std::move (defs);
  }

  //! \brief Returns the definitions reaching the specified block.
  const std::set<definition>&
  reach_def_analysis::get_block (basic_block_id id)
  {
    auto itr = this->block_map.find (id);
    if (itr == this->block_map.end ())
      throw std::runtime_error ("reach_def_analysis: unknown id");
    return itr->second;
  }



  /*!
     \brief Performs a reaching-definintions analysis on the specified CFG.
     \param cfg The control flow graph to analyze.
     \return The results of the analysis.
   */
  reach_def_analysis
  reach_def_analyzer::analyze (const control_flow_graph& cfg)
  {
    this->set_active_cfg (cfg);
    this->get_all_defs ();
    this->solve (cfg);

    reach_def_analysis result;
    for (auto& blk : cfg.get_blocks ())
      {
        auto& my_frag = static_cast<my_fragment&> (this->get_fragment (*blk));
        result.add_block (blk->get_id (), std::move (my_frag.defs));
      }

    return result;
  }

  bool
  reach_def_analyzer::compute_fragment (fragment& frag, const basic_block& blk)
  {
    auto& my_frag = static_cast<my_fragment&> (frag);
    std::set<definition> ndefs;

    for (auto& prev : blk.get_prev ())
      {
        auto kill = this->de_kill (*prev);

        auto& prev_frag = static_cast<my_fragment&> (this->get_fragment (*prev));
        auto& reaches_prev = prev_frag.defs;

        auto de = this->de_def (*prev);
        for (auto& def : de)
          ndefs.insert (def);

        for (auto& def : reaches_prev)
          if (kill.find (def) == kill.end ())
            ndefs.insert (def);
      }

    bool modified = !_sets_equal (my_frag.defs, ndefs);
    if (modified)
      my_frag.defs = std::move (ndefs);
    return modified;
  }

  std::unique_ptr<reach_def_analyzer::fragment>
  reach_def_analyzer::compute_init_fragment (const basic_block& blk)
  {
    auto frag = new my_fragment ();
    return std::unique_ptr<fragment> (frag);
  }



  //! \brief Finds all definitions that appear in the CFG.
  void
  reach_def_analyzer::get_all_defs ()
  {
    for (auto& blk : this->cfg->get_blocks ())
      {
        auto& insts = blk->get_instructions ();
        for (size_t i = 0; i < insts.size (); ++i)
          {
            auto& inst = insts[i];
            if (is_opcode_assign (inst.op))
              {
                this->all_defs.emplace_back (blk->get_id (), i);
              }
          }
      }
  }

  //! \brief Returns a list of all downward-exposed definitions in a block.
  std::set<definition>&
  reach_def_analyzer::de_def (const basic_block& blk)
  {
    auto ditr = this->de_defs.find (blk.get_id ());
    if (ditr != this->de_defs.end ())
      return ditr->second;

    std::set<jtac_var_id> enc_vars;
    auto& defs = this->de_defs[blk.get_id ()];

    auto& insts = blk.get_instructions ();
    for (int i = (int)insts.size () - 1; i >= 0; --i)
      {
        auto& inst = insts[i];
        if (is_opcode_assign (inst.op))
          {
            auto var = inst.oprs[0].val.var.get_id ();
            auto sitr = enc_vars.find (var);
            if (sitr == enc_vars.end ())
              {
                enc_vars.insert (var);
                defs.emplace (blk.get_id (), i);
              }
          }
      }

    return defs;
  }

  //! \brief Returns a list of all definitions that are obscured by a
  //!        defintion in a block.
  std::set<definition>&
  reach_def_analyzer::de_kill (const basic_block& blk)
  {
    auto ditr = this->de_kills.find (blk.get_id ());
    if (ditr != this->de_kills.end ())
      return ditr->second;

    std::set<jtac_var_id> my_vars;
    for (auto& inst : blk.get_instructions ())
      if (is_opcode_assign (inst.op))
        {
          auto var = inst.oprs[0].val.var.get_id ();
          my_vars.insert (var);
        }

    auto& defs = this->de_kills[blk.get_id ()];
    for (auto& def : this->all_defs)
      {
        auto var = this->cfg->find_block (def.first)
            ->get_instructions ()[def.second].oprs[0].val.var.get_id ();
        if (my_vars.find (var) != my_vars.end ())
          defs.insert (def);
      }

    auto de = this->de_def (blk);
    for (auto& def : de)
      defs.erase (def);

    return defs;
  }



//------------------------------------------------------------------------------

  void
  dom_analysis::add_block (basic_block_id id, std::set<basic_block_id>&& doms)
  {
    this->block_map[id] = std::move (doms);
  }

  //! \brief Returns the set of blocks dominating the specified block.
  const std::set<basic_block_id>&
  dom_analysis::get_block (basic_block_id id)
  {
    auto itr = this->block_map.find (id);
    if (itr == this->block_map.end ())
      throw std::runtime_error ("dom_analysis::get_block: invalid id");
    return itr->second;
  }


  //! \brief Sets a block's immediate dominator.
  void
  dom_analysis::set_idom (basic_block_id id, basic_block_id idom)
  {
    this->idom_map[id] = idom;
  }

  //! \brief Returns the specified block's immediate dominator.
  basic_block_id
  dom_analysis::get_idom (basic_block_id id) const
  {
    auto itr = this->idom_map.find (id);
    if (itr == this->idom_map.end ())
      throw std::runtime_error ("dom_analysis::get_idom: invalid id");
    return itr->second;
  }


  //! \brief Inserts a block into a specified block's dominance frontier set.
  void
  dom_analysis::add_df (basic_block_id id, basic_block_id df)
  {
    this->df_map[id].insert (df);
  }

  //! \brief Returns the dominance frontier set of a specified block.
  std::set<basic_block_id>&
  dom_analysis::get_dfs (basic_block_id id)
  {
    return this->df_map[id];
  }



  /*!
     \brief Performs dominance analysis on the specified CFG.
     \param cfg The control flow graph to analyze.
     \return The results of the analysis.
   */
  dom_analysis
  dom_analyzer::analyze (const control_flow_graph& cfg)
  {
    this->set_active_cfg (cfg);
    this->solve (cfg);

    dom_analysis result;
    for (auto& blk : cfg.get_blocks ())
      {
        auto& my_frag = static_cast<my_fragment &> (this->get_fragment (*blk));
        result.add_block (blk->get_id (), std::move (my_frag.doms));
      }

    // compute immediate dominators
    this->compute_idoms (result);

    // compute dominance frontiers
    this->compute_dfs (result);

    return result;
  }

  //! \brief Finds all immediate dominators.
  void
  dom_analyzer::compute_idoms (dom_analysis& result)
  {
    for (auto& blk : this->cfg->get_blocks ())
      {
        auto& doms = result.get_block (blk->get_id ());
        for (auto dom : doms)
          {
            if (dom == blk->get_id ())
              continue;

            bool found = false;
            for (auto other_dom : doms)
              if (other_dom != blk->get_id () && other_dom != dom)
                {
                  auto& other_dom_doms = result.get_block (other_dom);
                  if (other_dom_doms.find (dom) != other_dom_doms.end ())
                    {
                      found = true;
                      break;
                    }
                }

            if (!found)
              result.set_idom (blk->get_id (), dom);
          }
      }
  }

  //! \brief Computes dominance frontiers.
  void
  dom_analyzer::compute_dfs (dom_analysis& result)
  {
    for (auto& blk : this->cfg->get_blocks ())
      {
        auto& prevs = blk->get_prev ();
        if (prevs.size () > 1)
          {
            auto blk_idom = result.get_idom (blk->get_id ());
            for (auto& prev : prevs)
              {
                const basic_block *curr = prev.get ();
                while (curr->get_id () != blk_idom)
                  {
                    result.add_df (curr->get_id (), blk->get_id ());
                    curr = this->cfg->find_block (result.get_idom (curr->get_id ())).get ();
                  }
              }
          }
      }
  }



  bool
  dom_analyzer::compute_fragment (fragment& frag, const basic_block& blk)
  {
    auto& my_frag = static_cast<my_fragment&> (frag);

    std::set<basic_block_id> ndoms;

    auto& prevs = blk.get_prev ();
    if (!prevs.empty ())
      {
        ndoms = static_cast<my_fragment&> (this->get_fragment (*prevs[0])).doms;
        for (size_t i = 1; i < prevs.size (); ++i)
          {
            auto& prev_frag = static_cast<my_fragment&> (this->get_fragment (*prevs[i]));
            for (auto itr = ndoms.begin (); itr != ndoms.end (); )
              {
                if (prev_frag.doms.find (*itr) == prev_frag.doms.end ())
                  itr = ndoms.erase (itr);
                else
                  ++ itr;
              }
          }
      }

    ndoms.insert (blk.get_id ());

    bool modified = !_sets_equal (my_frag.doms, ndoms);
    if (modified)
      my_frag.doms = std::move (ndoms);
    return modified;
  }

  std::unique_ptr<dom_analyzer::fragment>
  dom_analyzer::compute_init_fragment (const basic_block& blk)
  {
    auto frag = new my_fragment ();

    if (this->cfg->get_root ()->get_id () == blk.get_id ())
      frag->doms.insert (blk.get_id ());
    else
      {
        for (auto& b : this->cfg->get_blocks ())
          frag->doms.insert (b->get_id ());
      }

    return std::unique_ptr<fragment> (frag);
  }
}
}