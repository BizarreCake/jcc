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

#ifndef _JCC__LINKER__TRANSLATORS__ELF64__TRANSLATOR__H_
#define _JCC__LINKER__TRANSLATORS__ELF64__TRANSLATOR__H_

#include "linker/translators/translator.hpp"
#include "linker/translators/elf64/object_file.hpp"
#include <unordered_map>


namespace jcc {

  /*!
    \class elf64_module_translator
    \brief A module translator that produces ELF-64 object files.
   */
  class elf64_module_translator: public module_translator
  {
   private:
    generic_module *mod;
    elf64_object_file obj;

    std::unordered_map<section *, elf64_section *> sect_map;
  
   public:
    elf64_module_translator ();
    ~elf64_module_translator ();
    
   public:
    virtual void save (generic_module& mod, std::ostream& strm) override;

    virtual std::shared_ptr<generic_module> load (std::istream& strm) override;

   private:
    void build_object_file ();

    void handle_section (section& s);
    void handle_code_section (code_section& s);

    void add_got_plt ();
    void add_plt ();
    void add_relocations ();
    void add_dynamic_section ();
    void fill_got_plt ();
    void fill_plt ();

    //! \brief Fixes relocations in code.
    void fix_relocations ();

    void set_entry_point ();

    void add_segments ();

   private:
    void parse_object_file ();
    void parse_sections ();
    void parse_progbits_section (elf64_progbits_section& s);
    void parse_dynamic_section (elf64_dynamic_section& s);
    void parse_version_definitions ();
    void parse_exports ();


   private:
    //! \brief Searches for the section that contains the specified relocation.
    code_section& find_section_with_relocation (relocation_symbol_id id);
  };
}

#endif

