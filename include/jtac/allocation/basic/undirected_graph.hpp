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

#ifndef _JCC__JTAC__ALLOCATORS__BASIC__UNDIRECTED_GRAPH__H_
#define _JCC__JTAC__ALLOCATORS__BASIC__UNDIRECTED_GRAPH__H_

#include <vector>
#include <unordered_map>
#include <set>


namespace jcc {
namespace jtac {

  /*!
     \class undirected_graph
     \brief Used as an inference graph during register allocation.
   */
  class undirected_graph
  {
   public:
    using node_id = long;

    struct node
    {
      node_id value;
      std::set<node_id> nodes; // attached nodes
    };

   public:
    std::vector<node *> nodes;
    std::unordered_map<node_id, node *> node_map;

   public:
    inline const auto& get_nodes () const { return this->nodes; }

    inline size_t size () const { return this->nodes.size (); }
    inline bool empty () const { return this->size () == 0; }

   public:
    undirected_graph ();
    ~undirected_graph ();

   public:
    //! \brief Inserts a new lone node.
    void add_node (node_id val);

    //! \brief Links between two nodes.
    void add_edge (node_id a, node_id b);

    //! \brief Removes a specified node along with its edges.
    void remove_node (node_id id);

    //! \brief Removes all edges and nodes.
    void clear ();


    //! \brief Checks whether the graph contains a node of degree less than K.
    bool has_less_k (int k) const;

    //! \brief Returns a node of degree less than K.
    node_id find_less_k (int k) const;

    //! \brief Returns the node associated with the specified ID.
    node& get_node (node_id id);
  };
}
}

#endif //_JCC__JTAC__ALLOCATORS__BASIC__INFERENCE_GRAPH__H_
