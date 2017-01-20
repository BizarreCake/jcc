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

#include "linker/generic_module.hpp"
#include <stdexcept>


namespace jcc {
  
  generic_module::generic_module (module_type mtype, target_architecture tarch)
  {
    this->mtype = mtype;
    this->tarch = tarch;
    this->image_base = 0x400000;
    this->next_ver_id = 2;
  }

  generic_module::~generic_module ()
  {
    
  }
  
  
  
  //! \brief Inserts the specified section into the module's section list.
  void
  generic_module::add_section (section&& sect_)
  {
    if (this->find_section (sect_.get_name ()))
      throw std::runtime_error ("generic_module::add_section: name collision");
    
    this->sects.push_back (sect_.move_clone ());
    auto& sect = this->sects.back ();
    this->sect_map[sect->get_name ()] = sect.get ();
    this->sect_ptrs.push_back (sect.get ());
  }
  
  //! \brief Finds the section that has the specified name (\p name).
  section*
  generic_module::find_section (const std::string& name)
  {
    auto itr = this->sect_map.find (name);
    return (itr == this->sect_map.end ()) ? nullptr : itr->second;
  }



  //! \brief Inserts a version definition symbol and returns its ID.
  version_symbol_id
  generic_module::add_version_symbol (const std::string& name)
  {
    this->ver_syms.emplace_back ();
    auto& ver = this->ver_syms.back ();

    ver.id = this->next_ver_id ++;
    ver.name = name;
    this->ver_name_map[ver.name] = ver.id;

    return ver.id;
  }

  //! \brief Returns the ID associated with the specified version symbol name.
  version_symbol_id
  generic_module::get_version_symbol_id (const std::string& name) const
  {
    auto itr = this->ver_name_map.find (name);
    if (itr == this->ver_name_map.end ())
      throw std::runtime_error ("generic_module::get_version_symbol_id: unknown version");

    return itr->second;
  }


  //! \brief Inserts an export symbol into the module.
  void
  generic_module::add_export_symbol (const std::string& name,
                                     export_symbol_type type,section *sect,
                                     size_t vaddr, version_symbol_id version)
  {
    this->exp_sym_map[name] = this->exp_syms.size ();
    this->exp_syms.push_back ({name, type, sect, vaddr, version});

  }

  //! \brief Checks whether the module contains the specified export.
  bool
  generic_module::has_export_symbol (const std::string& name) const
  {
    auto itr = this->exp_sym_map.find (name);
    return (itr != this->exp_sym_map.end ());
  }

  //! \brief Returns the export symbol with the specified name.
  const export_symbol&
  generic_module::get_export_symbol (const std::string& name) const
  {
    auto itr = this->exp_sym_map.find (name);
    if (itr == this->exp_sym_map.end ())
      throw std::runtime_error ("generic_module::get_export_symbol: could not find name");
    return this->exp_syms[itr->second];
  }


  //! \brief Inserts a module import.
  module_import_id
  generic_module::add_import (const std::string& name)
  {
    module_import_id id = (module_import_id)this->imps.size ();
    this->imps.push_back (name);
    this->imp_map[name] = id;
    return id;
  }

  //! \brief Checks whether the module is importing the specified module name.
  bool
  generic_module::has_import (const std::string& name) const
  {
    auto itr = this->imp_map.find (name);
    return (itr != this->imp_map.end ());
  }

  //! \brief Returns the module import ID associated with the specified name.
  module_import_id
  generic_module::get_import (const std::string& name) const
  {
    auto itr = this->imp_map.find (name);
    if (itr == this->imp_map.end ())
      throw std::runtime_error ("generic_module::get_import: could not find module name");
    return itr->second;
  }

  //! \brief Returns the name of the module associated with the specified
  //!        import ID.
  const std::string&
  generic_module::get_import_name (module_import_id id) const
  {
    if (id >= (int)this->imps.size ())
      throw std::runtime_error ("generic_module::get_import_name: id out of range");
    return this->imps[id];
  }

  //! \brief Inserts import data for the specified relocation.
  void
  generic_module::add_import_symbol (
      relocation_symbol_id rel, module_import_id mod, version_symbol_id ver)
  {
    int index = (int)this->imp_syms.size ();
    this->imp_syms[rel] = { .index = index, .rel = rel, .mod = mod, .ver = ver };
  }

  //! \brief Returns the import symbol associated with the specified relocation.
  import_symbol
  generic_module::get_import_symbol (relocation_symbol_id rel) const
  {
    auto itr = this->imp_syms.find (rel);
    if (itr == this->imp_syms.end ())
      throw std::runtime_error ("generic_module::get_import_symbol: unknown relocation");

    return itr->second;
  }
}

