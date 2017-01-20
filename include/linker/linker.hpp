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

#ifndef _JCC__LINKER__LINKER__H_
#define _JCC__LINKER__LINKER__H_

#include "linker/generic_module.hpp"


namespace jcc {

  /*!
     \class link_error
     \brief Thrown by the linker in cases of failure.
   */
  class link_error: public std::runtime_error
  {
   public:
    link_error (const std::string& str)
        : std::runtime_error (str)
    { }
  };



  /*!
     \class linker
     \brief The generic module linker.
   */
  class linker
  {
    std::vector<generic_module *> mods;

    generic_module *out; // output module
    generic_module *main; // main input module (containg program entry point)

    // the linker uses its own relocation store
    std::shared_ptr<relocation_symbol_store> rstore;

   public:
    linker ();
    ~linker ();

   public:
    //! \brief Inserts the specified module as input.
    void add_module (generic_module& mod);

   public:
    /*!
       \brief Links all input modules together.
     */
    std::shared_ptr<generic_module> link ();

   private:
    //! \brief Finds the module that contains the symbol _start.
    generic_module& find_main_module ();

    //! \brief Finds the input module that contains the specified export symbol name.
    generic_module& find_module_containing_export (const std::string& name);


    //! \brief Constructs the output module's sections.
    void add_sections ();

    //! \brief Inserts the specified section into the output module.
    void add_section (section& sect);
    void add_code_section (code_section& sect);
  };
}

#endif //_JCC__LINKER__LINKER__H_
