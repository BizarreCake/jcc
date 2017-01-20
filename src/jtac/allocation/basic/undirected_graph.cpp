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
    first->nodes.insert (second);
    second->nodes.insert (first);
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
}
}
