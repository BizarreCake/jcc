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

#include "linker/translators/elf64/object_file.hpp"
#include <cstring>
#include <stdexcept>
#include <ostream>
#include <common/binary.hpp>
#include <iostream>
#include <algorithm>
#include <set>

namespace jcc {

  elf64_object_file::elf64_object_file ()
  {
    this->clear ();
  }

  elf64_object_file::elf64_object_file (elf64_object_file&& other)
    : ehdr (other.ehdr), sections (std::move (other.sections)),
      segments (std::move (other.segments))
  {
    this->def_strtab = other.def_strtab;
    other.def_strtab = nullptr;

    this->entry_sect = other.entry_sect;
    other.entry_sect = nullptr;

    this->entry_off = other.entry_off;
    this->image_base = other.image_base;
  }

  elf64_object_file::~elf64_object_file ()
  {
    for (auto s : this->sections)
      delete s;

    for (auto seg : this->segments)
      delete seg;
  }



  //! \brief Initializes the header with default values.
  void
  elf64_object_file::clear_header ()
  {
    // e_ident
    std::memset (this->ehdr.e_ident, 0, 16);
    this->ehdr.e_ident[0] = 0x7f;
    std::strcpy ((char *)this->ehdr.e_ident + 1, "ELF");
    this->ehdr.e_ident[4] = 2; // EI_CLASS: ELFCLASS64
    this->ehdr.e_ident[5] = 1; // EI_DATA: ELFDATA2LSB
    this->ehdr.e_ident[6] = 1; // EI_VERSION: EV_CURRENT
    this->ehdr.e_ident[7] = 0; // EI_OSABI: ELFOSABI_SYSV
    this->ehdr.e_ident[8] = 0; // EI_ABIVERSION

    this->ehdr.e_type = ET_NONE;
    this->ehdr.e_machine = 62; // EM_X86_64
    this->ehdr.e_version = 1; // EV_CURRENT
    this->ehdr.e_entry = 0;
    this->ehdr.e_phoff = 0;
    this->ehdr.e_shoff = 0;
    this->ehdr.e_flags = 0;
    this->ehdr.e_ehsize = ELF64_FILE_HEADER_SIZE;
    this->ehdr.e_phentsize = ELF64_PROGRAM_HEADER_SIZE;
    this->ehdr.e_phnum = 0;
    this->ehdr.e_shentsize = ELF64_SECTION_HEADER_SIZE;
    this->ehdr.e_shnum = 0;
    this->ehdr.e_shstrndx = 0;
  }



  //! \brief Inserts and returns a new string table section.
  elf64_strtab_section&
  elf64_object_file::add_strtab_section (const std::string& name)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_strtab_section: No default string table set");

    auto s = new elf64_strtab_section (*this);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    hdr.sh_addralign = 1;
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a new string table section and marks it as the default
  //!        string table for the object file.
  elf64_strtab_section&
  elf64_object_file::add_default_strtab_section (const std::string& name)
  {
    auto s = new elf64_strtab_section (*this);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    this->def_strtab = s;
    this->ehdr.e_shstrndx = (elf64_half_t)s->get_index ();

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)s->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a new symbol table section.
  elf64_symtab_section&
  elf64_object_file::add_symtab_section (const std::string& name,
                                         elf64_strtab_section& strtab)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_symtab_section: No default string table set");

    auto s = new elf64_symtab_section (*this, strtab);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a new dynamic symbol table section.
  elf64_symtab_section&
  elf64_object_file::add_dynsym_section (const std::string& name,
                                         elf64_strtab_section& strtab)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_dynsym_section: No default string table set");

    auto s = new elf64_dynsym_section (*this, strtab);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a new PROGBITS section.
  elf64_progbits_section&
  elf64_object_file::add_progbits_section (
      const std::string& name, const unsigned char *data, size_t len)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_progbits_section: No default string table set");

    auto s = new elf64_progbits_section (*this, data, len);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts an .interp section.
  elf64_interp_section&
  elf64_object_file::add_interp_section (const std::string& interp)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_interp_section: No default string table set");

    auto s = new elf64_interp_section (*this,interp);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (".interp");
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a dynamic linking table section.
  elf64_dynamic_section&
  elf64_object_file::add_dynamic_section (const std::string& name,
                                          elf64_strtab_section& strtab)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_dynamic_section: No default string table set");

    auto s = new elf64_dynamic_section (*this, strtab);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }

  //! \brief Inserts a relocations section.
  elf64_rela_section&
  elf64_object_file::add_rela_section (const std::string& name,
                                       elf64_section& sect,
                                       elf64_symtab_section& symtab)
  {
    if (!this->def_strtab)
      throw std::runtime_error ("add_rela_section: No default string table set");

    auto s = new elf64_rela_section (*this, sect, symtab);
    s->set_index ((int)this->sections.size ());
    this->sections.push_back (s);

    auto& hdr = s->get_header ();
    hdr.sh_name = (elf64_word_t)this->def_strtab->add_string (name);
    ++ this->ehdr.e_shnum;

    return *s;
  }


  //! \brief Inserts a new program segment.
  elf64_segment&
  elf64_object_file::add_segment (elf64_segment_type type)
  {
    auto seg = new elf64_segment ();
    this->segments.push_back (seg);

    auto& hdr = seg->get_header ();
    hdr.p_type = type;
    ++ this->ehdr.e_phnum;

    return *seg;
  }



  //! \brief Searches for a section given its name.
  elf64_section&
  elf64_object_file::get_section_by_name (const std::string &name)
  {
    for (auto s : this->sections)
      if (this->def_strtab->get_string (name) == (int)s->get_header ().sh_name)
        return *s;

    throw std::runtime_error ("elf64_object_file::get_section_by_name: section not found");
  }

  //! \brief Searches for the section whose file offset matches the specified offset.
  elf64_section&
  elf64_object_file::get_section_by_offset (elf64_off_t off)
  {
    for (auto s : this->sections)
      if (s->get_header ().sh_offset == off)
        return *s;

    throw std::runtime_error ("elf64_object_file::get_section_by_offset: section not found");
  }

  //! \brief Searches for the section given its ID.
  elf64_section&
  elf64_object_file::get_section_by_id (int id)
  {
    for (auto s : this->sections)
      if (s->get_id () == id)
        return *s;

    throw std::runtime_error ("elf64_object_file::get_section_by_id: section not found");
  }

  //! \brief Checks whether the object file contains a section with the
  //!        specified name.
  bool
  elf64_object_file::has_section (const std::string& name)
  {
    for (auto s : this->sections)
      if (this->def_strtab->get_string (name) == (int)s->get_header ().sh_name)
        return true;

    return false;
  }

  //! \brief Checks whether the object file contains a section with the
  //!        specified ID.
  bool
  elf64_object_file::has_section_id (int id)
  {
    for (auto s : this->sections)
      if (s->get_id () == id)
        return true;

    return false;
  }

  //! \brief Returns true if the specifeid section is contained in some segment.
  bool
  elf64_object_file::in_segment (const elf64_section& sect) const
  {
    for (auto seg : this->segments)
      if (seg->has_section (sect))
        return true;

    return false;
  }



  static int
  _gcd (int a, int b)
  { return (b == 0) ? a : _gcd (b, a % b); }

  static inline int
  _lcm (int a, int b)
  { return a * b / _gcd (a, b); }

  /*!
     \brief Returns the correct address alignment for the specified section.

     This is important if the given section appears first in some segment's
     section list. In that case, the section's alignment value must agree
     with that of the section.
   */
  elf64_xword_t
  elf64_object_file::get_section_addralign (const elf64_section& sect) const
  {
    elf64_xword_t align = sect.get_header ().sh_addralign;
    for (auto seg : this->segments)
      {
        if (seg->get_sections ().front () == &sect)
          {
            auto seg_align = seg->get_header ().p_align;
            align = (elf64_xword_t)_lcm ((int)align, (int)seg_align);
          }
      }

    return align;
  }



  /*!
     \brief Computes the offsets and virtual addresses of all sections in the
            object file.
   */
  void
  elf64_object_file::compute_layout ()
  {
    this->order_sections ();
    this->compute_offsets ();
    this->position_segments ();
  }

  void
  elf64_object_file::order_sections ()
  {
    std::unordered_map<elf64_section *, std::set<elf64_section *>> behind_map;
    for (auto seg : this->segments)
      {
        auto& seg_sections = seg->get_sections ();
        for (size_t i = 0; i < seg_sections.size (); ++i)
          {
            auto s = seg_sections[i];
            behind_map[s]; // create entry if one does not exist already
            for (size_t j = 0; j < i; ++j)
              behind_map[s].insert (seg_sections[j]);
          }
      }

    // take vaddr hints into consideration
    for (auto s : this->sections)
      {
        auto this_hint = s->get_vaddr_hint ();
        if (this_hint != 0)
          {
            for (auto other : this->sections)
              {
                auto that_hint = other->get_vaddr_hint ();
                if (that_hint != 0 && that_hint < this_hint)
                  behind_map[s].insert (other);
              }
          }
      }

    std::vector<elf64_section *> sects;
    sects.push_back (this->sections[0]); // null section is always first

    while (!behind_map.empty ())
      {
        // find a section that has no other sections behind it
        bool found = false;
        for (auto& p : behind_map)
          if (p.second.empty ())
            {
              sects.push_back (p.first);
              for (auto& p2 : behind_map)
                p2.second.erase (p.first);
              behind_map.erase (p.first);

              found = true;
              break;
            }

        if (!found)
          throw std::runtime_error ("elf64_object_file::order_sections: order collision");
      }

    // push all other sections not appearing in any segment to the end of the
    // section list.
    for (auto s : this->sections)
      if (std::find (sects.begin (), sects.end (), s) == sects.end ())
        sects.push_back (s);

    // update indices
    for (size_t i = 0; i < sects.size (); ++i)
      sects[i]->set_index ((int)i);

    this->sections.clear ();
    while (!sects.empty ())
      {
        auto s = sects[0];

        bool in_seg = false;
        for (auto seg : this->segments)
          if (seg->has_section (*s))
            {
              auto& seg_sections = seg->get_sections ();

              // ensure section order
              if (seg_sections[0] != s)
                {
                  auto itr_sects = std::find (this->sections.begin (), this->sections.end (), seg_sections[0]);
                  if (itr_sects == this->sections.end ())
                    throw std::runtime_error ("elf64_object_file::position_sections: bad section order");

                  auto itr_seg = seg_sections.begin ();
                  while (itr_sects != this->sections.end () && itr_seg != seg_sections.end ())
                    {
                      if (*itr_sects != *itr_seg)
                        break;
                      ++ itr_sects;
                      ++ itr_seg;
                    }

                  for (; itr_seg != seg_sections.end (); ++itr_seg)
                    {
                      auto itr = std::find (sects.begin (), sects.end (), *itr_seg);
                      if (itr == sects.end ())
                        throw std::runtime_error ("elf64_object_file::position_sections: bad section order");

                      sects.erase (itr);
                      itr_sects = this->sections.insert (itr_sects, *itr_seg) + 1;
                    }
                }
              else
                {
                  this->sections.push_back (s);
                  sects.erase (sects.begin ());

                  for (size_t i = 1; i < seg_sections.size (); ++i)
                    if (seg_sections[i]->get_index () < seg_sections[i - 1]->get_index () + 1)
                      throw std::runtime_error ("elf64_object_file::position_sections: bad section order");

                  for (size_t i = 1; i < seg_sections.size (); ++i)
                    {
                      auto next = seg_sections[i];
                      sects.erase (std::remove (sects.begin (), sects.end (), next), sects.end ());
                      this->sections.push_back (next);
                    }
                }

              in_seg = true;
              break;
            }

        if (!in_seg)
          {
            this->sections.push_back (s);
            sects.erase (sects.begin ());
          }
      }

    // update correct indices
    for (size_t i = 0; i < this->sections.size (); ++i)
      this->sections[i]->set_index ((int)i);
    this->ehdr.e_shstrndx = (elf64_half_t)this->def_strtab->get_index ();
  }

  void
  elf64_object_file::compute_offsets ()
  {
    size_t curr_off = ELF64_FILE_HEADER_SIZE;

    // program headers come right after file header
    if (this->segments.empty ())
      this->ehdr.e_phoff = 0;
    else
      {
        this->ehdr.e_phoff = curr_off;
        curr_off += this->segments.size () * ELF64_PROGRAM_HEADER_SIZE;
        curr_off += ELF64_PROGRAM_HEADER_SIZE; // account for PHDR segment
      }

    size_t curr_addr = this->image_base + curr_off;

    for (size_t i = 1; i < this->sections.size (); ++i)
      {
        auto s = this->sections[i];
        auto& hdr = s->get_header ();

        hdr.sh_offset = curr_off;
        if (this->in_segment (*s))
          {
            auto hint = s->get_vaddr_hint ();
            if (hint != 0)
              curr_addr = hint;

            hdr.sh_addr = curr_addr;

            auto align = this->get_section_addralign (*s);
            if (align > 1)
              {
                // align virtual address with file offset modulo address alignment value
                int add = (int)((hdr.sh_addr - hdr.sh_offset) % align);
                hdr.sh_offset += add;
                curr_off = hdr.sh_offset;

                if (hdr.sh_offset % align != 0)
                  {
                    hdr.sh_offset += align - (hdr.sh_offset % align);
                    hdr.sh_addr += align - (hdr.sh_addr % align);
                    curr_off = hdr.sh_offset;
                    curr_addr = hdr.sh_addr;
                  }
              }
          }
        else
          hdr.sh_addr = 0; // don't give a virtual address if not in segment

        auto sh_size = s->compute_size ();
        curr_off += sh_size;
        curr_addr += sh_size;
      }

    if (curr_off % 8 != 0)
      curr_off += 8 - (curr_off % 8);

    this->ehdr.e_shoff = curr_off;
  }

  void
  elf64_object_file::position_segments ()
  {
    for (auto seg : this->segments)
      {
        // make sure the segment's sections are all consecutive
        auto& seg_sections = seg->get_sections ();
        for (size_t i = 1; i < seg_sections.size (); ++i)
          if (seg_sections[i]->get_index () != seg_sections[i - 1]->get_index () + 1)
            throw std::runtime_error ("elf64_object_file::bake_segments: sections in segment must be consecutive");

        auto& first = seg_sections.front ()->get_header ();
        auto& last = seg_sections.back ()->get_header ();

        auto& phdr = seg->get_header ();
        phdr.p_offset = first.sh_offset;
        phdr.p_vaddr = first.sh_addr;
        phdr.p_paddr = 0;
        phdr.p_filesz = last.sh_offset + last.sh_size - first.sh_offset;
        phdr.p_memsz = last.sh_addr + last.sh_size - first.sh_addr;
      }
  }


  static int
  _section_bake_index (elf64_section_type type)
  {
    switch (type)
      {
      case SHT_STRTAB:
        return 0;
      case SHT_PROGBITS:
        return 1;
      case SHT_SYMTAB:
      case SHT_DYNSYM:
        return 2;
      case SHT_GNU_VERDEF:
        return 3;
      case SHT_GNU_VERSYM:
        return 4;
      case SHT_REL:
      case SHT_RELA:
        return 5;

      default:
        return 6;
      }
  }

  void
  elf64_object_file::bake_sections ()
  {
    auto sections = this->sections;
    std::sort (sections.begin (), sections.end (),
      [] (const elf64_section *a, const elf64_section *b) {
        return _section_bake_index ((elf64_section_type)a->get_header ().sh_type)
               < _section_bake_index ((elf64_section_type)b->get_header ().sh_type);
      });

    for (auto s : sections)
      s->bake ();
  }



  /*!
     \brief Saves the object file into a stream.
   */
  void
  elf64_object_file::save (std::ostream& strm)
  {
    size_t curr_off = 0;

    this->bake_sections ();

    this->write_header (strm, curr_off);

    // write program headers
    this->write_program_headers (strm, curr_off);

    // write section data
    for (size_t i = 1; i < this->sections.size (); ++i)
      {
        auto s = this->sections[i];
        auto& hdr = s->get_header ();

        if (curr_off < hdr.sh_offset)
          {
            // insert padding
            bin::write_zeroes (strm, (int)(hdr.sh_offset - curr_off));
            curr_off = hdr.sh_offset;
          }

        auto data = s->get_data ();
        strm.write (data, hdr.sh_size);
        curr_off += hdr.sh_size;
      }

    // insert padding before section headers if necessary
    if (curr_off < this->ehdr.e_shoff)
      {
        bin::write_zeroes (strm, (int)(this->ehdr.e_shoff - curr_off));
        curr_off = this->ehdr.e_shoff;
      }

    // write section headers
    this->write_section_headers (strm, curr_off);
  }

  void
  elf64_object_file::write_header (std::ostream& strm, size_t& curr_off)
  {
    auto& ehdr = this->ehdr;

    if (this->entry_sect)
      ehdr.e_entry = this->entry_sect->get_header ().sh_addr + this->entry_off;
    else
      ehdr.e_entry = 0;

    if (this->segments.empty ())
      ehdr.e_phnum = 0;
    else
      ehdr.e_phnum = (elf64_half_t)(this->segments.size () + 1); // +1 for PHDR segment

    strm.write ((char *)ehdr.e_ident, 16);
    bin::write_u16_le (strm, ehdr.e_type);
    bin::write_u16_le (strm, ehdr.e_machine);
    bin::write_u32_le (strm, ehdr.e_version);
    bin::write_u64_le (strm, ehdr.e_entry);
    bin::write_u64_le (strm, ehdr.e_phoff);
    bin::write_u64_le (strm, ehdr.e_shoff);
    bin::write_u32_le (strm, ehdr.e_flags);
    bin::write_u16_le (strm, ehdr.e_ehsize);
    bin::write_u16_le (strm, ehdr.e_phentsize);
    bin::write_u16_le (strm, ehdr.e_phnum);
    bin::write_u16_le (strm, ehdr.e_shentsize);
    bin::write_u16_le (strm, ehdr.e_shnum);
    bin::write_u16_le (strm, ehdr.e_shstrndx);

    curr_off += ELF64_FILE_HEADER_SIZE;
  }


  static void
  _write_program_header (std::ostream& strm, elf64_phdr_t& hdr)
  {
    bin::write_u32_le (strm, hdr.p_type);
    bin::write_u32_le (strm, hdr.p_flags);
    bin::write_u64_le (strm, hdr.p_offset);
    bin::write_u64_le (strm, hdr.p_vaddr);
    bin::write_u64_le (strm, hdr.p_paddr);
    bin::write_u64_le (strm, hdr.p_filesz);
    bin::write_u64_le (strm, hdr.p_memsz);
    bin::write_u64_le (strm, hdr.p_align);
  }

  void
  elf64_object_file::write_program_headers (std::ostream& strm, size_t& curr_off)
  {
    if (this->segments.empty ())
      return;

    // write PHDR segment
    {
      elf64_phdr_t hdr;
      hdr.p_offset = this->ehdr.e_phoff;
      hdr.p_vaddr = this->image_base + hdr.p_offset;
      hdr.p_paddr = 0;
      hdr.p_filesz = (this->segments.size () + 1) * ELF64_PROGRAM_HEADER_SIZE;
      hdr.p_memsz = hdr.p_filesz;
      hdr.p_flags = PF_R | PF_X;
      hdr.p_align = 8;
      hdr.p_type = PT_PHDR;
      _write_program_header (strm, hdr);
      curr_off += ELF64_PROGRAM_HEADER_SIZE;
    }

    for (size_t i = 0; i < this->segments.size (); ++i)
      {
        auto seg = this->segments[i];
        auto& hdr = seg->get_header ();
        _write_program_header (strm, hdr);
        curr_off += ELF64_PROGRAM_HEADER_SIZE;
      }
  }


  void
  elf64_object_file::write_section_headers (std::ostream& strm, size_t& curr_off)
  {
    for (size_t i = 0; i < this->sections.size (); ++i)
      {
        auto s = this->sections[i];
        auto& hdr = s->get_header ();

        bin::write_u32_le (strm, hdr.sh_name);
        bin::write_u32_le (strm, hdr.sh_type);
        bin::write_u64_le (strm, hdr.sh_flags);
        bin::write_u64_le (strm, hdr.sh_addr);
        bin::write_u64_le (strm, hdr.sh_offset);
        bin::write_u64_le (strm, hdr.sh_size);
        bin::write_u32_le (strm, hdr.sh_link);
        bin::write_u32_le (strm, hdr.sh_info);
        bin::write_u64_le (strm, hdr.sh_addralign);
        bin::write_u64_le (strm, hdr.sh_entsize);
        curr_off += ELF64_SECTION_HEADER_SIZE;
      }
  }



  //! \brief Resets the object file to a clean state.
  void
  elf64_object_file::clear ()
  {
    for (auto s : this->sections)
      delete s;
    this->sections.clear ();

    for (auto seg : this->segments)
      delete seg;
    this->segments.clear ();

    this->clear_header ();
    this->def_strtab = nullptr;

    this->image_base = 0x400000;
    this->entry_sect = nullptr;
    this->entry_off = 0;

    // create null section
    this->sections.push_back (new elf64_null_section (*this));
    this->sections.back ()->set_index (0);
    this->ehdr.e_shnum = 1;
  }



  /*!
     \brief Loads the object file from the specified stream.
   */
  void
  elf64_object_file::load (std::istream& strm)
  {
    this->clear ();

    this->read_header (strm);
    this->read_sections (strm);
  }

  void
  elf64_object_file::read_header (std::istream& strm)
  {
    strm.seekg (0, std::ios_base::beg);
    strm.read ((char *)this->ehdr.e_ident, 16);
    this->ehdr.e_type = bin::read_u16_le (strm);
    this->ehdr.e_machine = bin::read_u16_le (strm);
    this->ehdr.e_version = bin::read_u32_le (strm);
    this->ehdr.e_entry = bin::read_u64_le (strm);
    this->ehdr.e_phoff = bin::read_u64_le (strm);
    this->ehdr.e_shoff = bin::read_u64_le (strm);
    this->ehdr.e_flags = bin::read_u32_le (strm);
    this->ehdr.e_ehsize = bin::read_u16_le (strm);
    this->ehdr.e_phentsize = bin::read_u16_le (strm);
    this->ehdr.e_phnum = bin::read_u16_le (strm);
    this->ehdr.e_shentsize = bin::read_u16_le (strm);
    this->ehdr.e_shnum = bin::read_u16_le (strm);
    this->ehdr.e_shstrndx = bin::read_u16_le (strm);
  }


  static elf64_shdr_t
  _read_section_header (std::istream& strm)
  {
    elf64_shdr_t shdr;

    shdr.sh_name = bin::read_u32_le (strm);
    shdr.sh_type = bin::read_u32_le (strm);
    shdr.sh_flags = bin::read_u64_le (strm);
    shdr.sh_addr = bin::read_u64_le (strm);
    shdr.sh_offset = bin::read_u64_le (strm);
    shdr.sh_size = bin::read_u64_le (strm);
    shdr.sh_link = bin::read_u32_le (strm);
    shdr.sh_info = bin::read_u32_le (strm);
    shdr.sh_addralign = bin::read_u64_le (strm);
    shdr.sh_entsize = bin::read_u64_le (strm);

    return shdr;
  }

  void
  elf64_object_file::read_sections (std::istream& strm)
  {
    strm.seekg (this->ehdr.e_shoff, std::ios_base::beg);

    // read all section headers
    for (unsigned i = 0; i < this->ehdr.e_shnum; ++i)
      this->shdrs.push_back (_read_section_header (strm));

    // read section name string table section first.
    this->def_strtab = static_cast<elf64_strtab_section *> (
      this->read_section (strm, this->shdrs[this->ehdr.e_shstrndx]));
    this->def_strtab->set_id (this->ehdr.e_shstrndx);

    std::vector<std::pair<int, elf64_shdr_t>> process_shdrs;
    for (int i = 0; i < (int)this->shdrs.size (); ++i)
      process_shdrs.emplace_back (i, this->shdrs[i]);
    process_shdrs.erase (process_shdrs.begin () + this->ehdr.e_shstrndx);

    // sort the section headers in the order that they are going to be
    // processed.
    std::sort (process_shdrs.begin (), process_shdrs.end (),
               [] (const std::pair<int, elf64_shdr_t>& a,
                   const std::pair<int, elf64_shdr_t>& b) {
                 return _section_bake_index ((elf64_section_type)a.second.sh_type)
                        < _section_bake_index ((elf64_section_type)b.second.sh_type);
               });

    for (auto& p : process_shdrs)
      {
        auto s = this->read_section (strm, p.second);
        if (s)
          s->set_id (p.first);
      }

    this->shdrs.clear ();
  }

  elf64_section*
  elf64_object_file::read_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    switch (shdr.sh_type)
      {
      case SHT_STRTAB:
        return &this->read_strtab_section (strm, shdr);

      case SHT_PROGBITS:
        return &this->read_progbits_section (strm, shdr);

      case SHT_SYMTAB:
      case SHT_DYNSYM:
        return &this->read_symtab_section (strm, shdr);

      case SHT_GNU_VERDEF:
        return &this->read_verdef_section (strm, shdr);

      case SHT_GNU_VERSYM:
        return &this->read_versym_section (strm, shdr);

      case SHT_DYNAMIC:
        return &this->read_dynamic_section (strm, shdr);

      default:
        //throw std::runtime_error ("elf64_object_file::read_section: unknown section type");
        return nullptr;
      }
  }

  void
  elf64_object_file::read_section_generic (elf64_section& s, std::istream& strm,
                                           elf64_shdr_t& shdr)
  {
    s.get_header () = shdr;

    unsigned char *raw = new unsigned char [shdr.sh_size];
    strm.seekg (shdr.sh_offset, std::ios_base::beg);
    strm.read ((char *)raw, shdr.sh_size);
    s.load_raw (raw, (unsigned int)shdr.sh_size);
    delete[] raw;
  }

  elf64_strtab_section&
  elf64_object_file::read_strtab_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto s = new elf64_strtab_section (*this);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }

  elf64_symtab_section&
  elf64_object_file::read_symtab_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto& link_strtab = static_cast<elf64_strtab_section&> (
        this->get_section_by_offset (this->shdrs[shdr.sh_link].sh_offset));
    auto s = new elf64_symtab_section (*this, link_strtab);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }

  elf64_verdef_section&
  elf64_object_file::read_verdef_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto& link_strtab = static_cast<elf64_strtab_section&> (
        this->get_section_by_offset (this->shdrs[shdr.sh_link].sh_offset));
    auto s = new elf64_verdef_section (*this, link_strtab);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }

  elf64_versym_section&
  elf64_object_file::read_versym_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto& link_dynsym = static_cast<elf64_dynsym_section&> (
        this->get_section_by_offset (this->shdrs[shdr.sh_link].sh_offset));
    auto s = new elf64_versym_section (*this, link_dynsym);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }

  elf64_progbits_section&
  elf64_object_file::read_progbits_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto s = new elf64_progbits_section (*this);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }

  elf64_dynamic_section&
  elf64_object_file::read_dynamic_section (std::istream& strm, elf64_shdr_t& shdr)
  {
    auto& link_strtab = static_cast<elf64_strtab_section&> (
        this->get_section_by_offset (this->shdrs[shdr.sh_link].sh_offset));
    auto s = new elf64_dynamic_section (*this, link_strtab);
    this->read_section_generic (*s, strm, shdr);
    this->sections.push_back (s);
    return *s;
  }
}
