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

#ifndef _JCC__JTAC__NAME_MAP__H_
#define _JCC__JTAC__NAME_MAP__H_

#include <string>
#include <unordered_map>
#include <stdexcept>


namespace jcc {
namespace jtac {

  template<typename T>
  class name_map
  {
    std::unordered_map<std::string, T> name_map;
    std::unordered_map<T, std::string> value_map;

   public:
    //! \brief Inserts a new mapping.
    void
    insert (const std::string &name, T val)
    {
      this->name_map[name] = val;
      this->value_map[val] = name;
    }


    //! \brief Returns the value associated with the specified name.
    T
    get (const std::string& name) const
    {
      auto itr = this->name_map.find (name);
      if (itr == this->name_map.end ())
        throw std::runtime_error ("name_map::get: could not find name");
      return itr->second;
    }

    //! \brief Checks whether the name map contains the specified name.
    bool
    has_name (const std::string &name) const
    { return this->name_map.find (name) != this->name_map.end (); }


    //! \brief Returns the name associated with the specified value.
    std::string
    get_name (T val) const
    {
      auto itr = this->value_map.find (val);
      if (itr == this->value_map.end ())
        throw std::runtime_error ("name_map::get_name: could not find value");
      return itr->second;
    }

    //! \brief Checks whether the name map contains the specified value.
    bool
    has_value (T val) const
    { return this->value_map.find (val) != this->value_map.end (); }
  };
}
}

#endif //_JCC__JTAC__NAME_MAP__H_
