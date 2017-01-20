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

#ifndef _JCC__LINKER__GENERIC_MODULE__H_
#define _JCC__LINKER__GENERIC_MODULE__H_

#include "linker/section.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace jcc {
  
  /*!
     \enum module_type
     \brief Enumeration of possible types of modules.
   */
  enum class module_type
  {
    // \brief Relocatable object file.
    relocatable,
    
    // \brief Executable file.
    executable,
    
    // \brief Shared object file.
    shared,
  };
  
  
  /*!
     \enum target_architecture
     \brief Enumeration of possible target machine architectures.
   */
  enum class target_architecture
  {
    // \brief AMD64 architecture
    x86_64,
  };


  /*!
     \struct module_location
     \brief Represents an address in the form of: SECTION + offset.
   */
  struct module_location
  {
    section *sect;
    size_t off;

    module_location ()
        : sect (nullptr), off (0)
    { }

    module_location (section *sect, size_t off = 0)
        : sect (sect), off (off)
    { }

    module_location (section& sect, size_t off = 0)
        : sect (&sect), off (off)
    { }

    operator bool () const
    { return this->sect || (this->off != 0); }
  };


  using version_symbol_id = int;
  using module_import_id = int;

// reserved version symbol IDs:
#define VERSION_ID_GLOBAL 1

  struct version_symbol
  {
    version_symbol_id id;
    std::string name;
  };



  /*!
     \enum symbol_type
     \brief Type of symbol being exported.
   */
  enum class export_symbol_type
  {
    data,
    function,
  };

  /*!
     \struct export_symbol
     \brief An export symbol.
   */
  struct export_symbol
  {
    std::string name; // symbol name
    export_symbol_type type;
    section *sect;    // the section in which the symbol is defined.
    size_t vaddr;     // virtual address
    version_symbol_id version;
  };


  /*!
     \struct import_symbol
     \brief A relocation with import details.
   */
  struct import_symbol
  {
    int index;
    relocation_symbol_id rel;
    module_import_id mod;
    version_symbol_id ver;
  };


  /*! 
     \class generic_module
     \brief Platform-independent module/object file.
   */
  class generic_module
  {
    module_type mtype;
    target_architecture tarch;
    module_location entry_point;
    size_t image_base;
    
    std::vector<std::unique_ptr<section>> sects;
    std::unordered_map<std::string, section *> sect_map;
    std::vector<section *> sect_ptrs;

    version_symbol_id next_ver_id;
    std::vector<version_symbol> ver_syms;
    std::unordered_map<std::string, version_symbol_id> ver_name_map;

    std::string exp_name; // export name
    std::vector<export_symbol> exp_syms;
    std::unordered_map<std::string, size_t> exp_sym_map;

    std::vector<std::string> imps;
    std::unordered_map<std::string, module_import_id> imp_map;
    std::unordered_map<relocation_symbol_id, import_symbol> imp_syms;

  public:
    inline module_type get_type () const { return this->mtype; }
    inline target_architecture get_target_architecture () const { return this->tarch; }
    
    inline module_location get_entry_point () const { return this->entry_point; }
    inline void set_entry_point (module_location loc) { this->entry_point = loc; }

    inline size_t get_image_base () const { return this->image_base; }
    inline void set_image_base (size_t addr) { this->image_base = addr; }
    
    inline std::vector<section *>& get_sections () { return this->sect_ptrs; }

    inline auto& get_export_symbols () { return this->exp_syms; }
    inline const auto& get_export_symbols () const { return this->exp_syms; }
    inline auto& get_import_symbols () { return this->imp_syms; }
    inline const auto& get_import_symbols () const { return this->imp_syms; }
    inline const auto& get_imports () const { return this->imps; }

    inline const std::string& get_export_name () const { return this->exp_name; }
    inline void set_export_name (const std::string& name) { this->exp_name = name; }

    
  public:
    explicit generic_module (module_type mtype, target_architecture tarch);
    ~generic_module ();
    
  public:
    //! \brief Inserts the specified section into the module's section list.
    void add_section (section&& sect);
    
    //! \brief Finds the section that has the specified name (\p name).
    section* find_section (const std::string& name);
    

    //! \brief Inserts a version definition symbol and returns its ID.
    version_symbol_id add_version_symbol (const std::string& name);

    //! \brief Returns the ID associated with the specified version symbol name.
    version_symbol_id get_version_symbol_id (const std::string& name) const;


    //! \brief Inserts an export symbol into the module.
    void add_export_symbol (const std::string& name, export_symbol_type type,
                            section *sect, size_t vaddr,
                            version_symbol_id version);

    //! \brief Checks whether the module contains the specified export.
    bool has_export_symbol (const std::string& name) const;

    //! \brief Returns the export symbol with the specified name.
    const export_symbol& get_export_symbol (const std::string& name) const;



    //! \brief Inserts a module import.
    module_import_id add_import (const std::string& name);

    //! \brief Checks whether the module is importing the specified module name.
    bool has_import (const std::string& name) const;

    //! \brief Returns the module import ID associated with the specified name.
    module_import_id get_import (const std::string& name) const;

    //! \brief Returns the name of the module associated with the specified
    //!        import ID.
    const std::string& get_import_name (module_import_id id) const;

    //! \brief Inserts import data for the specified relocation.
    void add_import_symbol (relocation_symbol_id rel, module_import_id mod,
                            version_symbol_id ver);

    //! \brief Returns the import symbol associated with the specified relocation.
    import_symbol get_import_symbol (relocation_symbol_id rel) const;
  };
}

#endif

