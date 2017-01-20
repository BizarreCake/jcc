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

#include "linker/linker.hpp"


namespace jcc {

  linker::linker ()
  {
    this->out = nullptr;
    this->rstore = std::make_shared<relocation_symbol_store> ();
  }

  linker::~linker ()
  {
    delete this->out;
  }



  //! \brief Inserts the specified module as input.
  void
  linker::add_module (generic_module& mod)
  {
    this->mods.push_back (&mod);
  }



  /*!
     \brief Links all input modules together.
   */
  std::shared_ptr<generic_module>
  linker::link ()
  {
    this->main = &this->find_main_module ();
    this->out = new generic_module (module_type::executable, this->main->get_target_architecture ());

    this->add_sections ();

    auto ptr = std::shared_ptr<generic_module> (this->out);
    this->out = nullptr;
    return ptr;
  }



  //! \brief Finds the module that contains the symbol _start.
  generic_module&
  linker::find_main_module ()
  {
    for (auto mod : this->mods)
      if (mod->get_entry_point ())
        return *mod;

    throw link_error ("could not find a module containing the program's entry point");
  }

  //! \brief Finds the input module that contains the specified export symbol name.
  generic_module&
  linker::find_module_containing_export (const std::string& name)
  {
    generic_module *cmod = nullptr;
    for (auto mod : this->mods)
      if (mod->has_export_symbol (name))
        {
          if (cmod)
            throw link_error ("symbol ambiguity: " + name);
          cmod = mod;
        }

    if (!cmod)
      throw link_error ("could not find module containing symbol: " + name);
    return *cmod;
  }



  //! \brief Constructs the output module's sections.
  void
  linker::add_sections ()
  {
    for (auto mod : this->mods)
      {
        if (mod->get_type () != module_type::relocatable)
          continue;

        for (auto sect : mod->get_sections ())
          this->add_section (*sect);
      }
  }

  //! \brief Inserts the specified section into the output module.
  void
  linker::add_section (section& sect)
  {
    switch (sect.get_type ())
      {
      case SECT_PROGBITS:
        throw std::runtime_error ("linker::add_section: PROGBITS section not handled");

      case SECT_CODE:
        this->add_code_section (static_cast<code_section&> (sect));
        break;
      }
  }

  void
  linker::add_code_section (code_section& sect_in)
  {
    if (this->out->find_section (sect_in.get_name ()))
      throw std::runtime_error ("linker::add_code_section: attempting to add section with same name twice");

    this->out->add_section (code_section (sect_in));
    code_section& sect = static_cast<code_section&> (
        *this->out->find_section (sect_in.get_name ()));

    // handle relocations
    for (auto& reloc : sect.get_relocations ())
      {
        // move relocation into linker's store
        reloc.sym = this->rstore->get (reloc.sym.store->get_name (reloc.sym.id));

        auto& sym_name = reloc.sym.store->get_name (reloc.sym.id);
        auto& mod = this->find_module_containing_export (sym_name);
        if (mod.get_type () != module_type::shared)
          throw std::runtime_error ("linker::add_code_section: relocations from non-shared objects not handled yet");

        module_import_id mod_id;
        if (this->out->has_import (mod.get_export_name ()))
          mod_id = this->out->get_import (mod.get_export_name ());
        else
          mod_id = this->out->add_import (mod.get_export_name ());

        auto& exp_sym = mod.get_export_symbol (sym_name);
        this->out->add_import_symbol (reloc.sym.id, mod_id, exp_sym.version);
      }
  }
}
