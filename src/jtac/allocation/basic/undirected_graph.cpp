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

#include "jtac/allocation/basic/undirected_graph.hpp"
#include <stdexcept>


namespace jcc {
namespace jtac {

  undirected_graph::undirected_graph ()
  {
  }

  undirected_graph::~undirected_graph ()
  {
    this->clear ();
  }



  //! \brief Inserts a new lone node.
  void
  undirected_graph::add_node (node_id val)
  {
    if (this->node_map.find (val) != this->node_map.end ())
      throw std::runtime_error ("undirected_graph::add_node: node already exists");

    auto n = new node ();
    n->value = val;

    this->nodes.push_back (n);
    this->node_map[val] = n;
  }

  //! \brief Links between two nodes.
  void
  undirected_graph::add_edge (node_id a, node_id b)
  {
    auto itr_first = this->node_map.find (a);
    if (itr_first == this->node_map.end ())
      throw std::runtime_error ("undirected_graph::add_edge: cannot find node");

    auto itr_second = this->node_map.find (b);
    if (itr_second == this->node_map.end ())
      throw std::runtime_error ("undirected_graph::add_edge: cannot find node");

    auto first = itr_first->second;
    auto second = itr_second->second;
    first->nodes.insert (second->value);
    second->nodes.insert (first->value);
  }

  //! \brief Removes a specified node along with its edges.
  void
  undirected_graph::remove_node (node_id id)
  {
    this->node_map.erase (id);
    for (auto itr = this->nodes.begin (); itr != this->nodes.end (); ++itr)
      {
        auto n = *itr;
        if (n->value == id)
          {
            for (auto other : this->nodes)
              if (other->value != id)
                other->nodes.erase (id);

            delete n;
            itr = this->nodes.erase (itr);
            break;
          }
      }
  }

  //! \brief Removes all edges and nodes.
  void
  undirected_graph::clear ()
  {
    for (auto n : this->nodes)
      delete n;

    this->nodes.clear ();
    this->node_map.clear ();
  }


  //! \brief Checks whether the graph contains a node of degree less than K.
  bool
  undirected_graph::has_less_k (int k) const
  {
    for (auto n : this->nodes)
      if ((int)n->nodes.size () < k)
        return true;

    return false;
  }

  //! \brief Returns a node of degree less than K.
  undirected_graph::node_id
  undirected_graph::find_less_k (int k) const
  {
    for (auto n : this->nodes)
      if ((int)n->nodes.size () < k)
        return n->value;

    throw std::runtime_error ("undirected_graph::find_less_k: node not found");
  }


  //! \brief Returns the node associated with the specified ID.
  undirected_graph::node&
  undirected_graph::get_node (node_id id)
  {
    auto itr = this->node_map.find (id);
    if (itr == this->node_map.end ())
      throw std::runtime_error ("undirected_graph::get_node: node not found");

    return *itr->second;
  }
}
}
