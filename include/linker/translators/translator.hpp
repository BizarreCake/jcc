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

#ifndef _JCC__LINKER__TRANSLATORS__TRANSLATOR__H_
#define _JCC__LINKER__TRANSLATORS__TRANSLATOR__H_

#include "linker/generic_module.hpp"
#include <iosfwd>


namespace jcc {
  
  /*!
    \class module_translator
    \brief Base class for module translators.
    
    A module translator is a component that can translate between a platform-
    specific implementation of a module and a generic module.
   */
  class module_translator
  {
  public:
    virtual ~module_translator () { }
    
  public:
    /*! 
      \brief Translates the specified generic module.

      The output is emitted to the specified stream \p strm.
     */
    virtual void save (generic_module& mod, std::ostream& strm) = 0;

    /*!
       \brief Translates a platform-specific module into a generic module.
     */
    virtual std::shared_ptr<generic_module> load (std::istream& strm) = 0;
    
  public:
    /*!
      \brief Factory method for creating translators.
     */
    static std::unique_ptr<module_translator> create (const std::string& name);
  };
}

#endif

