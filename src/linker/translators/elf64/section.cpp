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

#include "linker/translators/elf64/section.hpp"
#include "linker/translators/elf64/object_file.hpp"
#include <cstring>
#include <sstream>
#include <common/binary.hpp>
#include <iostream>


namespace jcc {

  elf64_section::elf64_section (elf64_object_file& obj)
    : obj (obj)
  {
    static int _next_id = 1;

    this->id = _next_id ++;
    this->index = 0;
    std::memset (&this->shdr, 0, sizeof this->shdr);
    this->vaddr_hint = 0;
  }



//------------------------------------------------------------------------------

  elf64_null_section::elf64_null_section (elf64_object_file& obj)
    : elf64_section (obj)
  { }



  void
  elf64_null_section::bake ()
  {
    this->data = std::string (24, '\0');
  }

  size_t
  elf64_null_section::compute_size ()
  {
    return 0;
  }



//------------------------------------------------------------------------------

  elf64_strtab_section::elf64_strtab_section (elf64_object_file& obj)
    : elf64_section (obj)
  {
    this->index_map[""] = 0;
    this->curr_idx = 1;
    this->data.push_back (0);

    this->shdr.sh_addralign = 1;
    this->shdr.sh_type = SHT_STRTAB;
  }



  //! \brief Checks whether the string table contains the specified string.
  bool
  elf64_strtab_section::has_string (const std::string& str) const
  {
    return this->index_map.find (str) != this->index_map.end ();
  }

  //! \brief Returns the index of the specified string in the table if it
  //!        exists; otherwise, returns -1.
  int
  elf64_strtab_section::get_string (const std::string& str) const
  {
    auto itr = this->index_map.find (str);
    if (itr == this->index_map.end ())
      return -1;
    return itr->second;
  }

  //! \brief Inserts the specified string into the table if it does not
  //!        already exists, and returns its index.
  int
  elf64_strtab_section::add_string (const std::string& str)
  {
    auto itr = this->index_map.find (str);
    if (itr != this->index_map.end ())
      return itr->second;

    for (char c : str)
      data.push_back (c);
    data.push_back (0);

    int idx = this->curr_idx;
    this->index_map[str] = idx;
    this->curr_idx += str.length () + 1;
    return idx;
  }

  //! \brief Returns the string associated with the specified index.
  const char*
  elf64_strtab_section::get_string (int idx) const
  {
    return this->data.data () + idx;
//    for (auto& p : this->index_map)
//      if (p.second == idx)
//        return p.first;
//    throw std::runtime_error ("elf64_strtab_section::get_string: index not found");
  }



  void
  elf64_strtab_section::bake ()
  {
    this->compute_size ();
    while (this->data.size () < this->shdr.sh_size)
      this->data.push_back (0);
  }

  size_t
  elf64_strtab_section::compute_size ()
  {
    this->shdr.sh_size = (elf64_xword_t)this->curr_idx;
    if (this->shdr.sh_size % 8 != 0)
      this->shdr.sh_size += (8 - (this->shdr.sh_size % 8));

    return (size_t)this->shdr.sh_size;
  }

  void
  elf64_strtab_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    this->data = std::string ((char *)raw, len);

    unsigned int idx = 0;
    while (idx < len)
      {
        char *str = (char *)(raw + idx);
        this->index_map[str] = idx;
        idx += std::strlen (str) + 1;
      }

    this->curr_idx = idx;
  }



//------------------------------------------------------------------------------

  elf64_symtab_section::elf64_symtab_section (
      elf64_object_file& obj, elf64_strtab_section& strtab)
      : elf64_section (obj), strtab (strtab)
  {
    this->shdr.sh_addralign = 8;
    this->shdr.sh_entsize = 24;
    this->shdr.sh_type = SHT_SYMTAB;
    this->next_sym_id = 0;
  }



  //! \brief Inserts a new symbol to the end of the table.
  int
  elf64_symtab_section::add_symbol (
      const std::string& name, elf64_symbol_type type, elf64_symbol_binding bind,
      int sect_id, elf64_addr_t value, elf64_xword_t size)
  {
    symbol *sym = nullptr;
    if (bind == STB_LOCAL)
      {
        this->syms_local.emplace_back ();
        sym = &this->syms_local.back ();
      }
    else
      {
        this->syms_global.emplace_back ();
        sym = &this->syms_global.back ();
      }

    sym->id = this->next_sym_id ++;
    sym->index = 0;
    sym->name = (elf64_word_t)this->strtab.add_string (name);
    sym->type = type;
    sym->bind = bind;
    sym->sect_id = sect_id;
    sym->val.is_ptr = false;
    sym->val.num = value;
    sym->size = size;

    return sym->id;
  }

  /*!
     \brief Inserts a new symbol to the end of the table.

     The value of the symbol will be set to the specified section's virtual
     address summed together with the given offset.
   */
  int
  elf64_symtab_section::add_symbol_ptr (
      const std::string& name, elf64_symbol_type type, elf64_symbol_binding bind,
      int sect_id, elf64_addr_t offset, elf64_xword_t size)
  {
    symbol *sym = nullptr;
    if (bind == STB_LOCAL)
      {
        this->syms_local.emplace_back ();
        sym = &this->syms_local.back ();
      }
    else
      {
        this->syms_global.emplace_back ();
        sym = &this->syms_global.back ();
      }

    sym->id = this->next_sym_id ++;
    sym->index = 0;
    sym->name = (elf64_word_t)this->strtab.add_string (name);
    sym->type = type;
    sym->bind = bind;
    sym->sect_id = sect_id;
    sym->val.is_ptr = true;
    sym->val.ptr.off = offset;
    sym->size = size;

    return sym->id;
  }

  //! \brief Searches the table for the ID of a symbol that has the specified
  //!        name.
  int
  elf64_symtab_section::find_symbol_id (const std::string& name) const
  {
    for (auto& sym : this->syms_local)
      if (this->strtab.get_string (sym.name) == name)
        return sym.id;
    for (auto& sym : this->syms_global)
      if (this->strtab.get_string (sym.name) == name)
        return sym.id;
    return -1;
  }

  //! \brief Returns the symbol has the specified ID.
  const elf64_symtab_section::symbol&
  elf64_symtab_section::get_symbol (int id) const
  {
    for (auto& sym : this->syms_local)
      {
        if (sym.id == id)
          return sym;
        else if (sym.id > id)
          break;
      }

    for (auto& sym : this->syms_global)
      {
        if (sym.id == id)
          return sym;
        else if (sym.id > id)
          break;
      }

    throw std::runtime_error ("elf64_symtab_section::get_symbol: ID not found");
  }



  void
  elf64_symtab_section::write_symbol (std::ostream& strm, symbol& sym)
  {
    elf64_half_t sect_idx = sym.sect_id;
    elf64_section *sect = nullptr;
    if (sect_idx != 0)
      {
        sect = &this->obj.get_section_by_id (sym.sect_id);
        sect_idx = (elf64_half_t)sect->get_index ();
      }

    bin::write_u32_le (strm, sym.name);
    bin::write_u8 (strm, ((unsigned char)sym.bind << 4) | (unsigned char)sym.type);
    bin::write_u8 (strm, 0);
    bin::write_u16_le (strm, sect_idx);
    if (sym.val.is_ptr)
      bin::write_u64_le (strm, sect->get_header ().sh_addr + sym.val.ptr.off);
    else
      bin::write_i64_le (strm, sym.val.num);
    bin::write_i64_le (strm, sym.size);
  }

  void
  elf64_symtab_section::bake ()
  {
    this->data.clear ();
    std::ostringstream ss;

    // null entry
    bin::write_zeroes (ss, 24);

    int sym_idx = 1;
    for (auto& sym : this->syms_local)
      {
        sym.index = sym_idx ++;
        this->write_symbol (ss, sym);
      }
    for (auto& sym : this->syms_global)
      {
        sym.index = sym_idx ++;
        this->write_symbol (ss, sym);
      }

    this->data = ss.str ();
    this->shdr.sh_size = this->data.size ();
    this->shdr.sh_link = (elf64_half_t)this->strtab.get_index ();
    this->shdr.sh_info = (elf64_word_t)this->syms_local.size () + 1; // +1 for null entry
  }

  size_t
  elf64_symtab_section::compute_size ()
  {
    this->shdr.sh_size = 0;

    this->shdr.sh_size += 24; // null entry
    this->shdr.sh_size += 24 * this->syms_local.size ();
    this->shdr.sh_size += 24 * this->syms_global.size ();

    return (size_t)this->shdr.sh_size;
  }

  void
  elf64_symtab_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    if (len % 24 != 0)
      throw std::runtime_error ("elf64_symtab_section::load_raw: invalid symtab section size");

    int next_idx = 1;
    for (unsigned int off = 24; off < len; off += 24)
      {
        symbol sym;
        sym.id = this->next_sym_id ++;
        sym.index = next_idx ++;
        sym.name = bin::read_u32_le (raw + off);
        sym.type = (elf64_symbol_type)(raw[off + 4] & 0xf);
        sym.bind = (elf64_symbol_binding)(raw[off + 4] >> 4);
        sym.sect_id = bin::read_u16_le (raw + off + 6);
        sym.val.is_ptr = false;
        sym.val.num = bin::read_u64_le (raw + off + 8);
        sym.size = bin::read_u64_le (raw + off + 16);

        if (sym.bind == STB_LOCAL)
          this->syms_local.push_back (sym);
        else
          this->syms_global.push_back (sym);
      }
  }



//------------------------------------------------------------------------------

  elf64_dynsym_section::elf64_dynsym_section (elf64_object_file& obj,
                                              elf64_strtab_section& strtab)
    : elf64_symtab_section (obj, strtab)
  {
    this->shdr.sh_type = SHT_DYNSYM;
  }



//------------------------------------------------------------------------------

  elf64_progbits_section::elf64_progbits_section (elf64_object_file& obj)
    : elf64_section (obj)
  {
    this->init ();
  }

  elf64_progbits_section::elf64_progbits_section (
      elf64_object_file& obj, const std::vector<unsigned char>& data)
    : elf64_section (obj), data (data)
  {
    this->init ();
  }

  elf64_progbits_section::elf64_progbits_section (
      elf64_object_file& obj, std::vector<unsigned char>&& data)
    : elf64_section (obj), data (std::move (data))
  {
    this->init ();
  }

  elf64_progbits_section ::elf64_progbits_section (
      elf64_object_file& obj, const unsigned char *data, size_t len)
    : elf64_section (obj)
  {
    this->data.resize (len);
    std::memcpy (this->data.data (), data, len);

    this->init ();
  }



  void
  elf64_progbits_section::init ()
  {
    this->shdr.sh_addralign = 0x10;
    this->shdr.sh_type = SHT_PROGBITS;
    this->shdr.sh_flags = SHF_ALLOC;
    this->shdr.sh_size = this->data.size ();
  }



  void
  elf64_progbits_section::set_data (const unsigned char *data, size_t len)
  {
    this->data.resize (len);
    std::memcpy (this->data.data (), data, len);
    this->shdr.sh_size = this->data.size ();
  }



  size_t
  elf64_progbits_section::compute_size ()
  {
    return this->data.size ();
  }

  void
  elf64_progbits_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    this->data.resize (len);
    std::memcpy (this->data.data (), raw, len);
    this->shdr.sh_size = len;
  }



//------------------------------------------------------------------------------

  elf64_interp_section::elf64_interp_section (elf64_object_file& obj,
                                              const std::string& interp)
      : elf64_section (obj), interp (interp)
  {
    this->shdr.sh_type = SHT_PROGBITS;
    this->shdr.sh_addralign = 1;
    this->shdr.sh_flags = SHF_ALLOC;

    this->shdr.sh_size = interp.size () + 1;
    this->shdr.sh_size += (16 - (this->shdr.sh_addr % 16)) % 16;
    while (this->interp.size () <= this->shdr.sh_size)
      this->interp.push_back (0);
  }



  size_t
  elf64_interp_section::compute_size ()
  {
    return (size_t)this->shdr.sh_size;
  }



//------------------------------------------------------------------------------

  elf64_dynamic_section::elf64_dynamic_section (elf64_object_file& obj,
                                                elf64_strtab_section& strtab)
      : elf64_section (obj), strtab (strtab)
  {
    this->shdr.sh_type = SHT_DYNAMIC;
    this->shdr.sh_addralign = 8;
    this->shdr.sh_flags = SHF_ALLOC | SHF_WRITE;
    this->shdr.sh_entsize = 0x10;
    this->shdr.sh_size = 0x10; // size of null entry
  }



  //! \brief Inserts an entry to the end of the table.
  void
  elf64_dynamic_section::add (elf64_dynamic_tag tag, elf64_xword_t val)
  {
    this->entries.emplace_back ();
    auto& ent = this->entries.back ();
    this->shdr.sh_size += 0x10;

    ent.tag = tag;
    ent.is_ptr = false;
    ent.val = val;
  }

  /*!
     \brief Inserts an entry to the end of the table.
     The value of the entry being the sum of the specified section's
     virtual address and the given offset.
   */
  void
  elf64_dynamic_section::add (elf64_dynamic_tag tag, elf64_section& sect,
                              elf64_xword_t off)
  {
    this->entries.emplace_back ();
    auto& ent = this->entries.back ();
    this->shdr.sh_size += 0x10;

    ent.tag = tag;
    ent.is_ptr = true;
    ent.ptr.sect = &sect;
    ent.ptr.off = off;
  }



  void
  elf64_dynamic_section::bake ()
  {
    this->shdr.sh_link = (elf64_word_t)this->strtab.get_index ();

    std::ostringstream ss;
    for (auto& ent : this->entries)
      {
        bin::write_i64_le (ss, ent.tag);
        if (ent.is_ptr)
          bin::write_u64_le (ss, ent.ptr.sect->get_header ().sh_addr + ent.ptr.off);
        else
          bin::write_i64_le (ss, ent.val);
      }

    // include null entry
    bin::write_i64_le (ss, DT_NULL);
    bin::write_i64_le (ss, 0);

    this->data = ss.str ();
  }

  size_t
  elf64_dynamic_section::compute_size ()
  {
    return (size_t)this->shdr.sh_size;
  }

  void
  elf64_dynamic_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    if (len % 16 != 0)
      throw std::runtime_error ("elf64_dynamic_setion::load_raw: invalid section length");

    for (unsigned int off = 0; off < len; off += 16)
      {
        elf64_xword_t tag = bin::read_u64_le (raw + off);
        if (tag == DT_NULL)
          break;

        elf64_xword_t val = bin::read_u64_le (raw + off + 8);
        this->add ((elf64_dynamic_tag)tag, val);
      }
  }



//------------------------------------------------------------------------------

  elf64_rela_section::elf64_rela_section (elf64_object_file& obj,
                                          elf64_section& sect,
                                          elf64_symtab_section& symtab)
    : elf64_section (obj), sect (sect), symtab (symtab)
  {
    this->shdr.sh_type = SHT_RELA;
    this->shdr.sh_addralign = 8;
    this->shdr.sh_entsize = 0x18;
    this->shdr.sh_flags = SHF_ALLOC | SHF_INFO_LINK;
  }



  //! \brief Inserts a relocation entry.
  void
  elf64_rela_section::add (elf64_relocation_type type, elf64_section& sect,
                           elf64_off_t offset, int sym_id, elf64_sxword_t add)
  {
    this->entries.push_back ({ type, &sect, offset, sym_id, add });
    this->shdr.sh_size += 0x18;
  }

  //! \brief Inserts a zero-based relocation entry.
  void
  elf64_rela_section::add (elf64_relocation_type type, elf64_off_t offset,
                           int sym_id, elf64_sxword_t add)
  {
    this->entries.push_back ({ type, nullptr, offset, sym_id, add });
    this->shdr.sh_size += 0x18;
  }



  void
  elf64_rela_section::bake ()
  {
    this->shdr.sh_info = (elf64_word_t)this->sect.get_index ();
    this->shdr.sh_link = (elf64_word_t)this->symtab.get_index ();

    std::ostringstream ss;
    for (auto& e : this->entries)
      {
        auto& sym = this->symtab.get_symbol (e.sym_id);
        bin::write_u64_le (ss, (e.sect ? e.sect->get_header ().sh_addr : 0) + e.offset);
        bin::write_u64_le (ss, ((elf64_xword_t)sym.index << 32) | e.type);
        bin::write_i64_le (ss, e.add);
      }

    this->data = ss.str ();
  }

  size_t
  elf64_rela_section::compute_size ()
  {
    return (size_t)this->shdr.sh_size;
  }



//------------------------------------------------------------------------------

  elf64_verdef_section::elf64_verdef_section (elf64_object_file& obj,
                                              elf64_strtab_section& strtab)
    : elf64_section (obj), strtab (strtab)
  {
    this->shdr.sh_type = SHT_GNU_VERDEF;
    this->shdr.sh_addralign = 8;
  }



  elf64_verdef_section::entry&
  elf64_verdef_section::add (elf64_half_t index, elf64_half_t flags,
                             const std::string& name)
  {
    this->entries.emplace_back ();
    auto& entry = this->entries.back ();

    entry.index = index;
    entry.flags = flags;
    entry.names.push_back (name);
    entry.hash = elf64_hash (name);

    return entry;
  }

  elf64_verdef_section::entry&
  elf64_verdef_section::get_version (elf64_half_t index)
  {
    for (auto& e : this->entries)
      if (e.index == index)
        return e;

    throw std::runtime_error ("elf64_verdef_section::get_version: version index not found");
  }

  const std::string&
  elf64_verdef_section::get_version_name (elf64_half_t index)
  {
    return this->get_version (index).names[0];
  }



  void
  elf64_verdef_section::bake ()
  {
    this->shdr.sh_link = (elf64_half_t)this->strtab.get_index ();
    this->compute_size ();

    std::ostringstream ss;
    for (size_t i = 0; i < this->entries.size (); ++i)
      {
        auto& entry = this->entries[i];

        bin::write_u16_le (ss, 1); // revision
        bin::write_u16_le (ss, entry.flags);
        bin::write_u16_le (ss, entry.index);
        bin::write_u16_le (ss, (elf64_half_t)entry.names.size ());
        bin::write_u32_le (ss, entry.hash);
        bin::write_u32_le (ss, 0x14); // offset to auxiliary entries

        // offset to next entry
        bin::write_u32_le (ss,
          (i == this->entries.size () - 1)
            ? 0
            : (0x14 + (elf64_word_t)entry.names.size () * 8));

        // auxiliary entries:
        for (size_t j = 0; j < entry.names.size (); ++j)
          {
            bin::write_u32_le (ss, (elf64_word_t)this->strtab.add_string (entry.names[j]));
            bin::write_u32_le (ss, (j == entry.names.size () - 1) ? 0 : 8);
          }
      }

  }

  size_t
  elf64_verdef_section::compute_size ()
  {
    this->shdr.sh_size = 0;
    for (auto& entry : this->entries)
      this->shdr.sh_size += 0x14 + 8 * entry.names.size ();

    return this->shdr.sh_size;
  }

  void
  elf64_verdef_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    unsigned int off = 0;
    while (off < len)
      {
        elf64_half_t flags = bin::read_u16_le (raw + off + 2);
        elf64_half_t index = bin::read_u16_le (raw + off + 4);
        elf64_half_t cnt = bin::read_u16_le (raw + off + 6);

        entry *ent = nullptr;

        elf64_sword_t aux = bin::read_i32_le (raw + off + 12);
        unsigned int aoff = off + aux;
        for (elf64_half_t i = 0; i < cnt; ++i)
          {
            elf64_word_t name_idx = bin::read_u32_le (raw + aoff);
            auto name = this->strtab.get_string (name_idx);

            if (i == 0)
              ent = &this->add (index, flags, name);
            else
              ent->add_predecessor (name);

            elf64_sword_t next = bin::read_i32_le (raw + aoff + 4);
            aoff += next;
          }

        elf64_sword_t next = bin::read_i32_le (raw + off + 16);
        if (next == 0)
          break;
        off += next;
      }
  }



//------------------------------------------------------------------------------

  elf64_versym_section::elf64_versym_section (elf64_object_file& obj,
                                              elf64_dynsym_section& dynsym)
    : elf64_section (obj), dynsym (dynsym)
  {
    this->shdr.sh_type = SHT_GNU_VERSYM;
    this->shdr.sh_addralign = 2;
    this->shdr.sh_entsize = 2;
  }



  void
  elf64_versym_section::set_entry (int index, elf64_half_t val)
  {
    if (index >= (int)this->entries.size ())
      this->entries.resize ((unsigned)index + 1, 0);
    this->entries[index] = val;
  }

  elf64_half_t
  elf64_versym_section::get_entry (int index) const
  {
    if (index >= (int)this->entries.size ())
      return 0;
    return this->entries[index];
  }



  void
  elf64_versym_section::bake ()
  {
    std::ostringstream ss;
    for (elf64_half_t e : this->entries)
      bin::write_u16_le (ss, e);

    if (this->entries.size () < this->dynsym.get_count ())
      {
        size_t rem = this->dynsym.get_count () - this->entries.size ();
        while (rem --> 0)
          bin::write_u16_le (ss, 0);
      }

    this->data = ss.str ();
    this->shdr.sh_size = this->data.size ();
    this->shdr.sh_link = (elf64_half_t)this->dynsym.get_index ();
  }

  size_t
  elf64_versym_section::compute_size ()
  {
    this->shdr.sh_size = this->dynsym.get_count () * 2;
    return this->shdr.sh_size;
  }

  void
  elf64_versym_section::load_raw (const unsigned char *raw, unsigned int len)
  {
    if (len % 2 != 0)
      throw std::runtime_error ("elf64_versym_section::load_raw: invalid section length");

    unsigned int index = 0;
    this->entries.resize (len / 2);
    for (unsigned int off = 0; off < len; off += 2)
      {
        elf64_half_t e = bin::read_u16_le (raw + off);
        this->entries[index++] = e;
      }
  }
}
