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

#include "linker/section.hpp"
#include <cstring>
#include <stdexcept>


namespace jcc {

  section::section (const std::string& name)
    : name (name)
  {
  }



//------------------------------------------------------------------------------

  progbits_section::progbits_section (const std::string& name, size_t vaddr)
      : section (name)
  {
    this->vaddr = vaddr;
  }

  progbits_section::progbits_section (const std::string& name,
      const std::vector<unsigned char>& data, size_t vaddr)
      : section (name), data (data)
  {
    this->vaddr = vaddr;
  }

  progbits_section::progbits_section (const std::string& name,
      const unsigned char *data, unsigned int len, size_t vaddr)
      : section (name)
  {
    this->vaddr = vaddr;
    this->data.resize (len);
    std::memcpy (this->data.data (), data, len);
  }



  std::unique_ptr<section>
  progbits_section::clone () const
  {
    return std::unique_ptr<section> (new progbits_section (*this));
  }

  std::unique_ptr<section>
  progbits_section::move_clone ()
  {
    return std::unique_ptr<section> (new progbits_section (std::move (*this)));
  }

  
  
//------------------------------------------------------------------------------
  
  code_section::code_section (const std::string& name, size_t vaddr)
    : progbits_section (name, vaddr)
  {
  }

  code_section::code_section (const std::string& name,
      const std::vector<unsigned char>& data, size_t vaddr)
      : progbits_section (name, data, vaddr)
  {
  }

  code_section::code_section (const std::string& name,
      const unsigned char *data, unsigned int len, size_t vaddr)
      : progbits_section (name, data, len, vaddr)
  {
  }



  //! \brief Inserts the specified relocation to the section's relocation list.
  void
  code_section::add_relocation (relocation reloc)
  {
    this->reloc_map[reloc.sym.id] = this->relocs.size ();
    this->relocs.push_back (reloc);
  }

  //! \brief Checks whether the section contains the specified relocation.
  bool
  code_section::has_relocation (relocation_symbol_id id) const
  {
    auto itr = this->reloc_map.find (id);
    return (itr != this->reloc_map.end ());
  }

  //! \brief Returns the relocation that has the specified ID.
  const relocation&
  code_section::get_relocation (relocation_symbol_id id) const
  {
    auto itr = this->reloc_map.find (id);
    if (itr == this->reloc_map.end ())
      throw std::runtime_error ("code_section::get_relocation: unknown ID");
    return this->relocs[itr->second];
  }

  
  
  std::unique_ptr<section>
  code_section::clone () const
  {
    return std::unique_ptr<section> (new code_section (*this));
  }
  
  std::unique_ptr<section>
  code_section::move_clone ()
  {
    return std::unique_ptr<section> (new code_section (std::move (*this)));
  }
}

