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

#ifndef _JCC__LINKER__TRANSLATORS__ELF64__OBJECT_FILE__H_
#define _JCC__LINKER__TRANSLATORS__ELF64__OBJECT_FILE__H_

#include "linker/translators/elf64/elf64.hpp"
#include "linker/translators/elf64/section.hpp"
#include "linker/translators/elf64/segment.hpp"
#include <vector>
#include <memory>
#include <iosfwd>

namespace jcc {

  /*!
     \class elf64_object_file
     \brief ELF64 object file.
   */
  class elf64_object_file
  {
    elf64_ehdr_t ehdr;
    std::vector<elf64_section *> sections;
    elf64_strtab_section* def_strtab; // default string table

    std::vector<elf64_segment *> segments;

    // used when loading:
    std::vector<elf64_shdr_t> shdrs;

    // entry point
    elf64_section *entry_sect;
    elf64_xword_t entry_off;

    elf64_addr_t image_base;

   public:
    inline auto& get_sections () { return this->sections; }
    inline elf64_strtab_section* get_shstrtab () const { return this->def_strtab; }
    inline elf64_null_section& get_null () { return static_cast<elf64_null_section&> (*this->sections.front ()); }

    inline const elf64_ehdr_t& get_file_header () const { return this->ehdr; }
    inline void set_type (elf64_object_file_type_t type) { this->ehdr.e_type = type; }

    inline void
    set_entry_point (elf64_section& sect, elf64_xword_t off = 0)
    { this->entry_sect = &sect; this->entry_off = off; }

    inline void set_image_base (elf64_addr_t addr) { this->image_base = addr ;}

   public:
    elf64_object_file ();
    elf64_object_file (const elf64_object_file& other) = delete; // TODO: implement
    elf64_object_file (elf64_object_file&& other);
    ~elf64_object_file ();

   private:
    //! \brief Initializes the header with default values.
    void clear_header ();

   public:
    //! \brief Inserts and returns a new string table section.
    elf64_strtab_section& add_strtab_section (const std::string& name);

    //! \brief Inserts a new string table section and marks it as the default
    //!        string table for the object file.
    elf64_strtab_section& add_default_strtab_section (const std::string& name);

    //! \brief Inserts a new symbol table section.
    elf64_symtab_section& add_symtab_section (const std::string& name,
                                              elf64_strtab_section& strtab);

    //! \brief Inserts a new dynamic symbol table section.
    elf64_symtab_section& add_dynsym_section (const std::string& name,
                                              elf64_strtab_section& strtab);

    //! \brief Inserts a new PROGBITS section.
    elf64_progbits_section& add_progbits_section (
        const std::string& name, const unsigned char *data, size_t len);

    //! \brief Inserts an .interp section.
    elf64_interp_section& add_interp_section (const std::string& interp);

    //! \brief Inserts a dynamic linking table section.
    elf64_dynamic_section& add_dynamic_section (const std::string& name,
                                                elf64_strtab_section& strtab);

    //! \brief Inserts a relocations section.
    elf64_rela_section& add_rela_section (const std::string& name,
                                          elf64_section& sect,
                                          elf64_symtab_section& symtab);


    //! \brief Inserts a new program segment.
    elf64_segment& add_segment (elf64_segment_type type);

   public:
    //! \brief Searches for a section given its name.
    elf64_section& get_section_by_name (const std::string &name);

    //! \brief Searches for the section whose file offset matches the specified offset.
    elf64_section& get_section_by_offset (elf64_off_t off);

    //! \brief Searches for the section given its ID.
    elf64_section& get_section_by_id (int id);

    //! \brief Checks whether the object file contains a section with the
    //!        specified name.
    bool has_section (const std::string& name);

    //! \brief Checks whether the object file contains a section with the
    //!        specified ID.
    bool has_section_id (int id);

    //! \brief Returns true if the specifeid section is contained in some segment.
    bool in_segment (const elf64_section& sect) const;

   public:
    /*!
       \brief Computes the offsets and virtual addresses of all sections in the
              object file.
     */
    void compute_layout ();

    /*!
       \brief Saves the object file into a stream.
     */
    void save (std::ostream& strm);

    /*!
       \brief Loads the object file from the sshdrspecified stream.
     */
    void load (std::istream& strm);

    //! \brief Resets the object file to a clean state.
    void clear ();

   private:
    void write_header (std::ostream& strm, size_t& curr_off);
    void write_program_headers (std::ostream& strm, size_t& curr_off);
    void write_section_headers (std::ostream& strm, size_t& curr_off);

    void order_sections ();
    void compute_offsets ();
    void position_segments ();
    void bake_sections ();

    void read_header (std::istream& strm);
    void read_sections (std::istream& strm);

    elf64_section* read_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_strtab_section& read_strtab_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_symtab_section& read_symtab_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_verdef_section& read_verdef_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_versym_section& read_versym_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_progbits_section& read_progbits_section (std::istream& strm, elf64_shdr_t& shdr);
    elf64_dynamic_section& read_dynamic_section (std::istream& strm, elf64_shdr_t& shdr);
    void read_section_generic (elf64_section& s, std::istream& strm, elf64_shdr_t& shdr);

    /*!
       \brief Returns the correct address alignment for the specified section.

       This is important if the given section appears first in some segment's
       section list. In that case, the section's alignment value must agree
       with that of the section.
     */
    elf64_xword_t get_section_addralign (const elf64_section& sect) const;
  };
}

#endif //_JCC__LINKER__TRANSLATORS__ELF64__OBJECT__H_
