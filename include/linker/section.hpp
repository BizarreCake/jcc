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

#ifndef _JCC__LINKER__SECTION__H_
#define _JCC__LINKER__SECTION__H_

#include "assembler/relocation.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>


namespace jcc {
  
  enum section_type
  {
    SECT_PROGBITS,
    SECT_CODE,
  };
  
  
  
  /*!
     \class section
     \brief Base class for platform-independent module sections.
     
      In reality, a section might represent an actual section in the object
      file or a program segment, depending on the underlying implementation.
      But in its essence, a section is just a block of data, usually stored
      contiguously in the object file, coupled with some associated parameters.
   */
  class section
  {
  protected:
    std::string name;
  
  public:
    inline const std::string& get_name () const { return this->name; }
  
  public:
    section (const std::string& name);
    virtual ~section () { }
    
  public:
    virtual std::unique_ptr<section> clone () const = 0;
    virtual std::unique_ptr<section> move_clone () = 0;
    
    virtual section_type get_type () const = 0;
  };


  /*!
     \class progbits_section
     \brief A section that contains program data.
   */
  class progbits_section: public section
  {
   protected:
    std::vector<unsigned char> data;
    size_t vaddr;

   public:
    inline std::vector<unsigned char>& get_data () { return this->data; }
    inline const std::vector<unsigned char>& get_data () const { return this->data; }

    inline size_t get_vaddr () const { return this->vaddr; }
    inline void set_vaddr (size_t vaddr) { this->vaddr = vaddr; }

   public:
    progbits_section (const std::string& name, size_t vaddr = 0);
    progbits_section (const std::string& name,
                      const std::vector<unsigned char>& data, size_t vaddr);
    progbits_section (const std::string& name,
                      const unsigned char *data, unsigned int len, size_t vaddr);

   public:
    virtual std::unique_ptr<section> clone () const override;
    virtual std::unique_ptr<section> move_clone () override;

    virtual section_type get_type () const override { return SECT_PROGBITS; }
  };
  
  
  
  /*!
     \class code_section
     \brief A section that contains code.
    
     Along with the code, also stores relevant parameters like the code's
     virtual address in memory, permissions, etc...
   */
  class code_section: public progbits_section
  {
    std::vector<relocation> relocs;
    std::unordered_map<relocation_symbol_id, size_t> reloc_map;
    
  public:
    inline std::vector<unsigned char>& get_code () { return this->data; }
    inline const std::vector<unsigned char>& get_code () const { return this->data; }

    inline std::vector<relocation>& get_relocations () { return this->relocs; }
    inline const std::vector<relocation>& get_relocations () const { return this->relocs; }

  public:
    code_section (const std::string& name, size_t vaddr = 0);
    code_section (const std::string& name,
                  const std::vector<unsigned char>& data, size_t vaddr);
    code_section (const std::string& name,
                  const unsigned char *data, unsigned int len, size_t vaddr);

   public:
    //! \brief Inserts the specified relocation to the section's relocation list.
    void add_relocation (relocation reloc);

    //! \brief Checks whether the section contains the specified relocation.
    bool has_relocation (relocation_symbol_id id) const;

    //! \brief Returns the relocation that has the specified ID.
    const relocation& get_relocation (relocation_symbol_id id) const;


  public:
    virtual std::unique_ptr<section> clone () const override;
    virtual std::unique_ptr<section> move_clone () override;
    
    virtual section_type get_type () const override { return SECT_CODE; }
  };
}

#endif

