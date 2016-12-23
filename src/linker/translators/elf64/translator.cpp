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

#include "linker/translators/elf64/translator.hpp"
#include "linker/translators/elf64/object_file.hpp"
#include "assembler/x86_64/assembler.hpp"
#include <ostream>
#include <iostream>
#include <sstream>
#include <common/binary.hpp>

namespace jcc {

  elf64_module_translator::elf64_module_translator ()
  {
    this->mod = nullptr;
  }

  elf64_module_translator::~elf64_module_translator ()
  {
    delete this->mod;
  }



  void
  elf64_module_translator::save (generic_module &mod, std::ostream& strm)
  {
    this->mod = &mod;
    this->build_object_file ();
    this->mod = nullptr;

    this->obj.save (strm);
  }

  std::shared_ptr<generic_module>
  elf64_module_translator::load (std::istream& strm)
  {
    this->obj.load (strm);

    auto& ehdr = this->obj.get_file_header ();

    // figure out module type
    module_type mtype = module_type::relocatable;
    switch (ehdr.e_type)
      {
      case ET_REL: mtype = module_type::relocatable; break;
      case ET_EXEC: mtype = module_type::executable; break;
      case ET_DYN: mtype = module_type::shared; break;

      default:
        throw std::runtime_error ("elf64_module_translator::load: unknown object file type");
      }

    // extract target architecture
    target_architecture arch = target_architecture::x86_64;
    if (ehdr.e_machine != 62) // x86_64
      throw std::runtime_error ("elf64_module_translator::load: unsupported architecture");

    this->mod = new generic_module (mtype, arch);
    this->parse_object_file ();

    auto ptr = std::shared_ptr<generic_module> (this->mod);
    this->mod = nullptr;
    return ptr;
  }



  void
  elf64_module_translator::build_object_file ()
  {
    this->obj.clear ();

    switch (this->mod->get_type ())
      {
      case module_type::executable: obj.set_type (ET_EXEC); break;
      case module_type::relocatable: obj.set_type (ET_REL); break;
      case module_type::shared: obj.set_type (ET_DYN); break;
      }

    obj.add_default_strtab_section (".strtab");
    obj.add_symtab_section (".symtab", *obj.get_shstrtab ());

    if (this->mod->get_type () != module_type::relocatable)
      {
        auto& dynstr = this->obj.add_strtab_section (".dynstr");
        this->obj.add_dynsym_section (".dynsym", dynstr);
      }

    for (auto s : this->mod->get_sections ())
      this->handle_section (*s);

    this->add_got_plt ();
    this->add_plt ();
    this->add_relocations ();
    this->add_dynamic_section ();
    this->set_entry_point ();
    this->add_segments ();

    this->obj.compute_layout ();

    this->fill_got_plt ();
    this->fill_plt ();
    this->fix_relocations ();
  }



  void
  elf64_module_translator::handle_section (section& s)
  {
    switch (s.get_type ())
      {
      case SECT_CODE:
        this->handle_code_section (static_cast<code_section&> (s));
        break;
      }
  }

  void
  elf64_module_translator::handle_code_section (code_section& s)
  {
    if (this->obj.has_section (".text"))
      // TODO: handle this in the future
      throw std::runtime_error ("elf64_module_translator::handle_code_section: more than one code section");

    auto& text = this->obj.add_progbits_section (".text",
      s.get_code ().data (), s.get_code ().size ());
    text.set_flags (SHF_ALLOC | SHF_EXECINSTR);

    this->sect_map[&s] = &text;
  }



  void
  elf64_module_translator::add_dynamic_section ()
  {
    if (this->mod->get_type () != module_type::executable &&
        this->mod->get_type () != module_type::shared)
      return;

    if (this->mod->get_type () == module_type::executable)
      this->obj.add_interp_section ("/lib64/ld-linux-x86-64.so.2");

    auto& dynstr = static_cast<elf64_strtab_section&> (this->obj.get_section_by_name (".dynstr"));
    auto& dynsym = static_cast<elf64_dynsym_section&> (this->obj.get_section_by_name (".dynsym"));

    auto& dynamic = this->obj.add_dynamic_section (".dynamic", dynstr);

    for (auto& imp : this->mod->get_imports ())
      dynamic.add (DT_NEEDED, (elf64_xword_t)dynstr.add_string (imp));

    dynamic.add (DT_STRTAB, dynstr);
    dynamic.add (DT_SYMTAB, dynsym);
    dynamic.add (DT_STRSZ, dynstr.compute_size ());
    dynamic.add (DT_SYMENT, 24);
    if (this->obj.has_section (".got.plt"))
      dynamic.add (DT_PLTGOT, this->obj.get_section_by_name (".got.plt"));
    if (this->obj.has_section (".rela.plt"))
      {
        dynamic.add (DT_PLTRELSZ, 24);
        dynamic.add (DT_JMPREL, this->obj.get_section_by_name (".rela.plt"));
        dynamic.add (DT_PLTREL, DT_RELA);
      }

  }

  void
  elf64_module_translator::add_got_plt ()
  {
    if (this->mod->get_type () == module_type::relocatable)
      return;

    std::ostringstream ss;
    bin::write_zeroes (ss, 24 + 8 * this->mod->get_import_symbols ().size ());
    auto data = ss.str ();
    auto& got_plt = this->obj.add_progbits_section (".got.plt",
        (const unsigned char*)data.data (), data.size ());
    got_plt.set_flags (SHF_ALLOC | SHF_WRITE);
    got_plt.set_alignment (8);
  }

  void
  elf64_module_translator::add_plt ()
  {
    if (this->mod->get_type () == module_type::relocatable)
      return;

    std::ostringstream ss;
    bin::write_zeroes (ss, 16 + 16 * this->mod->get_import_symbols ().size ());
    auto data = ss.str ();

    auto& plt = this->obj.add_progbits_section (".plt",
        (const unsigned char *)data.data (), data.size ());
    plt.set_flags (SHF_ALLOC | SHF_EXECINSTR);
  }

  void
  elf64_module_translator::add_relocations ()
  {
    for (auto s : this->mod->get_sections ())
      {
        if (s->get_type () != SECT_CODE)
          continue;

        auto& code_sect = static_cast<code_section&> (*s);
        auto& relocs = code_sect.get_relocations ();
        if (relocs.empty ())
          continue;

        elf64_symtab_section *symtab = nullptr;
        elf64_rela_section *rela = nullptr;
        if (this->mod->get_type () == module_type::relocatable)
          {
            elf64_section& text = this->obj.get_section_by_name (".text");
            symtab = static_cast<elf64_symtab_section*> (&this->obj.get_section_by_name (".symtab"));
            if (this->obj.has_section (".rela.text"))
              throw std::runtime_error ("elf64_module_translator::add_relocations: .rela.text already exists");
            rela = &this->obj.add_rela_section (".rela.text", text, *symtab);
          }
        else
          {
            symtab = static_cast<elf64_symtab_section*> (&this->obj.get_section_by_name (".dynsym"));
            if (this->obj.has_section (".rela.plt"))
              rela = static_cast<elf64_rela_section*> (&this->obj.get_section_by_name (".rela.plt"));
            else
              {
                auto& plt = this->obj.get_section_by_name (".plt");
                rela = &this->obj.add_rela_section (".rela.plt", plt, *symtab);
              }
          }

        int sym_id;
        for (auto& reloc : relocs)
          {
            switch (reloc.type)
              {
                case R_NONE:
                  break;

                case R_PC32:
                  sym_id = symtab->find_symbol_id (reloc.sym.store->get_name (reloc.sym.id));
                if (sym_id == -1)
                  sym_id = symtab->add_symbol (reloc.sym.store->get_name (reloc.sym.id),
                                              STT_FUNC, STB_GLOBAL, 0, 0);
                if (this->mod->get_type () == module_type::relocatable)
                  rela->add (R_X86_64_PC32, reloc.offset, sym_id, reloc.add);
                else
                  {
                    auto imp = this->mod->get_import_symbol (reloc.sym.id);
                    rela->add (R_X86_64_JUMP_SLOT, this->obj.get_section_by_name (".got.plt"),
                               (elf64_off_t)(24 + imp.index * 8), sym_id, 0);
                  }
                break;
              }
          }
      }
  }



  void
  elf64_module_translator::fill_got_plt ()
  {
    if (!this->obj.has_section (".got.plt"))
      return;

    std::ostringstream ss;

    // first entry points to dynamic section
    bin::write_u64_le (ss, this->obj.get_section_by_name (".dynamic").get_header ().sh_addr);
    bin::write_u64_le (ss, 0); // set by dynamic linker
    bin::write_u64_le (ss, 0); // set by dynamic linker

    auto plt_addr = this->obj.get_section_by_name (".plt").get_header ().sh_addr;
    for (size_t i = 0; i < this->mod->get_import_symbols ().size (); ++i)
      bin::write_u64_le (ss, plt_addr + 16 + i * 16 + 5);

    auto data = ss.str ();
    auto& sect = static_cast<elf64_progbits_section&> (
        this->obj.get_section_by_name (".got.plt"));
    sect.set_data ((const unsigned char *)data.data (), data.size ());
  }

  void
  elf64_module_translator::fill_plt ()
  {
    if (!this->obj.has_section (".plt"))
      return;

    auto plt_addr = this->obj.get_section_by_name (".plt").get_header ().sh_addr;
    auto got_plt_addr = this->obj.get_section_by_name (".got.plt").get_header ().sh_addr;

    using namespace x86_64;

    assembler asem;
    int64_t off = 0;

    int lbl_start = asem.make_and_mark_label ();

    // push GOT[1]
    asem.emit_push (mem_t (SS_QWORD, REG_RIP, 1, REG_NONE, 4,
                           (got_plt_addr + 8) - (plt_addr + off) - 6));
    off = asem.get_size ();

    // jmp GOT[2]
    asem.emit_jmp (mem_t (SS_QWORD, REG_RIP, 1, REG_NONE, 4,
                          (got_plt_addr + 16) - (plt_addr + off) - 6));

    while (asem.get_size () < 16)
      asem.emit_nop ();
    off = 16;

    // emit PLT entries
    auto& imp_syms = this->mod->get_import_symbols ();
    for (size_t i = 0; i < imp_syms.size (); ++i)
      {
        auto entry_addr = got_plt_addr + 24 + i*8;

        // jump to corresponding GOT entry
        asem.emit_push (mem_t (SS_QWORD, REG_RIP, 1, REG_NONE, 4,
                               entry_addr - (plt_addr + off) - 6));

        // push relocation index
        asem.emit_push (imm_t (SS_DWORD, 4, i));

        // jump to PLT[0]
        asem.emit_jmp (lbl_t (lbl_start));

        while (asem.get_size () % 16 != 0)
          asem.emit_nop ();

        off += 16;
      }

    asem.fix_labels ();

    auto& sect = static_cast<elf64_progbits_section&> (
        this->obj.get_section_by_name (".plt"));
    sect.set_data (asem.get_data (), asem.get_size ());
  }


  //! \brief Fixes relocations in code.
  void
  elf64_module_translator::fix_relocations ()
  {
    if (!this->obj.has_section (".got.plt"))
      return;

    auto plt_addr = this->obj.get_section_by_name (".plt").get_header ().sh_addr;

    auto& imp_syms = this->mod->get_import_symbols ();
    for (auto& p : imp_syms)
      {
        auto sym = p.second;
        auto& s = this->find_section_with_relocation (sym.rel);
        auto& reloc = s.get_relocation (sym.rel);
        auto plt_entry_addr = plt_addr + 16 + sym.index*8;

        // get corresponding ELF section
        auto& sect = static_cast<elf64_progbits_section&> (*this->sect_map[&s]);

        // fix relocation
        unsigned char* data = sect.get_data ();
        switch (reloc.size)
          {
          case 4:
            bin::write_u32_le (data + reloc.offset,
                               plt_entry_addr - (sect.get_header ().sh_addr + reloc.offset) + reloc.add);
            break;

          default:
            throw std::runtime_error ("elf64_module_translator::fix_relocations: unhandled relocation size");
          }
      }
  }

  //! \brief Searches for the section that contains the specified relocation.
  code_section&
  elf64_module_translator::find_section_with_relocation (relocation_symbol_id id)
  {
    for (auto s : this->mod->get_sections ())
      if (s->get_type () == SECT_CODE)
        {
          auto &sect = static_cast<code_section &> (*s);
          if (sect.has_relocation (id))
            return sect;
        }

    throw std::runtime_error ("elf64_module_translator::find_section_with_relocation: could not find section");
  }



  void
  elf64_module_translator::set_entry_point ()
  {
    if (!this->obj.has_section (".text"))
      return;

    auto& text = this->obj.get_section_by_name (".text");

    this->obj.set_entry_point (text);

    auto& symtab = static_cast<elf64_symtab_section&> (this->obj.get_section_by_name (".symtab"));
    symtab.add_symbol_ptr ("_start", STT_FUNC, STB_GLOBAL, text.get_id ());
  }



  void
  elf64_module_translator::add_segments ()
  {
    if (this->mod->get_type () != module_type::executable &&
        this->mod->get_type () != module_type::shared)
      return;

    if (this->obj.has_section (".interp"))
      this->obj.add_segment (PT_INTERP)
          .add_section (obj.get_section_by_name (".interp"));

    auto& main_seg = this->obj.add_segment (PT_LOAD);
    if (this->obj.has_section (".interp"))
      main_seg.add_section (obj.get_section_by_name (".interp"));
    main_seg.add_section (obj.get_section_by_name (".dynsym"))
        .add_section (obj.get_section_by_name (".dynstr"));
    if (this->obj.has_section (".rela.plt"))
      main_seg.add_section (obj.get_section_by_name (".rela.plt"));
//    if (this->obj.has_section (".plt"))
//      main_seg.add_section (obj.get_section_by_name (".plt"));
    main_seg.add_section (obj.get_section_by_name (".text"));
    main_seg.set_flags (PF_R | PF_X);

    auto& dyn_load_seg = this->obj.add_segment (PT_LOAD);
//    if (this->obj.has_section (".got.plt"))
//      dyn_load_seg.add_section (this->obj.get_section_by_name (".got.plt"));
    dyn_load_seg.add_section (obj.get_section_by_name (".dynamic"));
    dyn_load_seg.set_flags (PF_R | PF_W);

    this->obj.add_segment (PT_DYNAMIC)
        .add_section (obj.get_section_by_name (".dynamic"))
        .set_flags (PF_R | PF_W);

    if (this->obj.has_section (".plt"))
      this->obj.add_segment (PT_LOAD)
          .add_section (obj.get_section_by_name (".plt"))
          .set_flags (PF_R | PF_X);
    if (this->obj.has_section (".got.plt"))
      this->obj.add_segment (PT_LOAD)
          .add_section (obj.get_section_by_name (".got.plt"))
          .set_flags (PF_R | PF_W);
  }



  void
  elf64_module_translator::parse_object_file ()
  {
    this->parse_version_definitions ();
    this->parse_sections ();
    this->parse_exports ();
  }


  void
  elf64_module_translator::parse_sections ()
  {
    for (auto s : this->obj.get_sections ())
      {
        auto& shdr = s->get_header ();
        switch (shdr.sh_type)
          {
          case SHT_PROGBITS:
            this->parse_progbits_section (static_cast<elf64_progbits_section&> (*s));
            break;

          case SHT_DYNAMIC:
            this->parse_dynamic_section (static_cast<elf64_dynamic_section&> (*s));
            break;

          default:
            break;
          }
      }
  }

  void
  elf64_module_translator::parse_progbits_section (elf64_progbits_section& s)
  {
    auto& shdr = s.get_header ();

    if ((shdr.sh_flags & SHF_EXECINSTR) && (shdr.sh_flags & SHF_ALLOC))
      {
        code_section sect (
            this->obj.get_shstrtab ()->get_string (shdr.sh_name),
            (const unsigned char *)s.get_data (), (unsigned int)shdr.sh_size,
            shdr.sh_addr);
        this->mod->add_section (std::move (sect));
      }
    else
      {
        progbits_section sect (
            this->obj.get_shstrtab ()->get_string (shdr.sh_name),
            (const unsigned char *)s.get_data (), (unsigned int)shdr.sh_size,
            shdr.sh_addr);
        this->mod->add_section (std::move (sect));
      }
  }

  void
  elf64_module_translator::parse_dynamic_section (elf64_dynamic_section& s)
  {
    for (auto& ent : s.get_entries ())
      {
        if (ent.tag == DT_SONAME)
          {
            this->mod->set_export_name (s.get_strtab ().get_string (ent.val));
          }
      }
  }


  void
  elf64_module_translator::parse_version_definitions ()
  {
    if (!this->obj.has_section (".gnu.version_d"))
      return;

    auto& verdef = static_cast<elf64_verdef_section&> (
        this->obj.get_section_by_name (".gnu.version_d"));
    for (auto& ent : verdef.get_entries ())
      {
        this->mod->add_version_symbol (ent.names[0]);
      }
  }

  void
  elf64_module_translator::parse_exports ()
  {
    if (!this->obj.has_section (".dynsym"))
      return;

    elf64_versym_section *versym = nullptr;
    elf64_verdef_section *verdef = nullptr;
    if (this->obj.has_section (".gnu.version")
        && this->obj.has_section (".gnu.version_d"))
      {
        versym = static_cast<elf64_versym_section *> (
            &this->obj.get_section_by_name (".gnu.version"));
        verdef = static_cast<elf64_verdef_section *> (
            &this->obj.get_section_by_name (".gnu.version_d"));
      }

    auto& dynsym = static_cast<elf64_dynsym_section&> (
        this->obj.get_section_by_name (".dynsym"));
    auto& dynstr = static_cast<elf64_strtab_section&> (dynsym.get_strtab ());
    for (auto& sym : dynsym.get_global_symbols ())
      {
        if (sym.bind != STB_GLOBAL)
          continue;
        if (sym.sect_id == 0)
          continue;

        // find associated section
        if (!this->obj.has_section_id (sym.sect_id))
          continue;
        elf64_section& elf_sect = this->obj.get_section_by_id (sym.sect_id);
        section *sect = nullptr;
        if (!(sect = this->mod->find_section (
            this->obj.get_shstrtab ()->get_string (elf_sect.get_header ().sh_name))))
          continue;

        export_symbol_type type;
        if (sym.type == STT_FUNC)
          type = export_symbol_type::function;
        else
          continue;

        // get version if available
        version_symbol_id ver_id = VERSION_ID_GLOBAL;
        if (versym)
          {
            auto ver_idx = versym->get_entry (sym.index);
            if (ver_idx & 0x8000)
              continue; // hidden symbol
            auto& ver_name = verdef->get_version_name (ver_idx);
            ver_id = this->mod->get_version_symbol_id (ver_name);
          }

        this->mod->add_export_symbol (dynstr.get_string (sym.name), type,
                                      sect, sym.val.num, ver_id);
      }
  }
}

