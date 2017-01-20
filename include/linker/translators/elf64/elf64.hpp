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

#ifndef _JCC__LINKER__TRANSLATORS__ELF64__ELF64__H_
#define _JCC__LINKER__TRANSLATORS__ELF64__ELF64__H_

#include <cstdint>
#include <string>


namespace jcc {

  //
  // ELF-64 data types:
  //
  typedef uint64_t elf64_addr_t;
  typedef uint64_t elf64_off_t;
  typedef uint16_t elf64_half_t;
  typedef uint32_t elf64_word_t;
  typedef int32_t elf64_sword_t;
  typedef uint64_t elf64_xword_t;
  typedef int64_t elf64_sxword_t;

#define ELF64_FILE_HEADER_SIZE    64
#define ELF64_SECTION_HEADER_SIZE 64
#define ELF64_PROGRAM_HEADER_SIZE 56


  struct elf64_ehdr_t
  {
    unsigned char e_ident[16]; //! \brief ELF identification
    elf64_half_t  e_type;      //! \brief Object file type
    elf64_half_t  e_machine;   //! \brief Machine type
    elf64_word_t  e_version;   //! \brief Object file version
    elf64_addr_t  e_entry;     //! \brief Entry point address
    elf64_off_t   e_phoff;     //! \brief Program header offset
    elf64_off_t   e_shoff;     //! \brief Section header offset
    elf64_word_t  e_flags;     //! \brief Processor-specific flags
    elf64_half_t  e_ehsize;    //! \brief ELF header size
    elf64_half_t  e_phentsize; //! \brief Size of program header entry
    elf64_half_t  e_phnum;     //! \brief Number of program header entries
    elf64_half_t  e_shentsize; //! \brief Size of section header entry
    elf64_half_t  e_shnum;     //! \brief Number of section header entries
    elf64_half_t  e_shstrndx;  //! \brief Section name string table index
  };

  enum elf64_object_file_type_t
  {
    ET_NONE = 0,  //! \brief No file type
    ET_REL  = 1,  //! \brief relocatable object file
    ET_EXEC = 2,  //! \brief Executable file
    ET_DYN  = 3,  //! \brief Shared object file
    ET_CORE = 4,  //! \brief Core file
  };



  enum elf64_section_type
  {
    SHT_NULL      = 0,  //! \brief Marks an unused section header.
    SHT_PROGBITS  = 1,  //! \brief contains information defined by the program.
    SHT_SYMTAB    = 2,  //! \brief Contains a linker symbol table.
    SHT_STRTAB    = 3,  //! \brief Contains a string table
    SHT_RELA      = 4,  //! \brief Contains “Rela” type relocation entries
    SHT_HASH      = 5,  //! \brief Contains a symbol hash table
    SHT_DYNAMIC   = 6,  //! \brief Contains dynamic linking tables
    SHT_NOTE      = 7,  //! \brief Contains note information
    SHT_NOBITS    = 8,  //! \brief Contains uninitialized space; does not occupy any space in the file
    SHT_REL       = 9,  //! \brief Contains “Rel” type relocation entries
    SHT_SHLIB     = 10, //! \brief Reserved
    SHT_DYNSYM    = 11, //! \brief Contains a dynamic loader symbol table

    SHT_GNU_VERDEF  = 0x6ffffffd, //! \brief Version definition section.
    SHT_GNU_VERNEED	= 0x6ffffffe, //! \brief Version needs section.
    SHT_GNU_VERSYM  = 0x6fffffff, //! \brief Version symbol table.

    SHT_LOOS      = 0x60000000, //! \brief Environment-specific use
    SHT_HIOS      = 0x6FFFFFFF, //! \brief Environment-specific use
    SHT_LOPROC    = 0x70000000, //! \brief Processor-specific use
    SHT_HIPROC    = 0x7FFFFFFF, //! \brief Processor-specific use
  };

  enum
  {
    SHF_WRITE     = 0x1,  //! \brief Section contains writable data
    SHF_ALLOC     = 0x2,  //! \brief Section is allocated in memory image of program
    SHF_EXECINSTR = 0x4,  //! \brief Section contains executable instructions
    SHF_INFO_LINK = 0x40, //! \brief Info contains section index.
    SHF_MASKOS    = 0x0F000000, //! \brief Environment-specific use
    SHF_MASKPROC  = 0xF0000000, //! \brief Processor-specific use
  };

  struct elf64_shdr_t
  {
    elf64_word_t   sh_name;      //! \brief Section name
    elf64_word_t   sh_type;      //! \brief Section type
    elf64_xword_t  sh_flags;     //! \brief Section attributes
    elf64_addr_t   sh_addr;      //! \brief Virtual address in memory
    elf64_off_t    sh_offset;    //! \brief Offset in file
    elf64_xword_t  sh_size;      //! \brief Size of section
    elf64_word_t   sh_link;      //! \brief Link to other section
    elf64_word_t   sh_info;      //! \brief Miscellaneous information
    elf64_xword_t  sh_addralign; //! \brief Address alignment boundary
    elf64_xword_t  sh_entsize;   //! \brief Size of entries, if section has table
  };


  enum elf64_segment_type
  {
    PT_NULL    = 0,          //! \brief Unused entry
    PT_LOAD    = 1,          //! \brief Loadable segment
    PT_DYNAMIC = 2,          //! \brief Dynamic linking tables
    PT_INTERP  = 3,          //! \brief Program interpreter path name
    PT_NOTE    = 4,          //! \brief Note sections
    PT_SHLIB   = 5,          //! \brief Reserved
    PT_PHDR    = 6,          //! \brief Program header table
    PT_LOOS    = 0x60000000, //! \brief Environment-specific use
    PT_HIOS    = 0x6FFFFFFF, //! \brief Environment-specific use
    PT_LOPROC  = 0x70000000, //! \brief Processor-specific use
    PT_HIPROC  = 0x7FFFFFFF, //! \brief Processor-specific use
  };

  enum
  {
    PF_X        = 0x1,        //! \brief Execute permission
    PF_W        = 0x2,        //! \brief Write permission
    PF_R        = 0x4,        //! \brief Read permission
    PF_MASKOS   = 0x00FF0000, //! \brief These flag bits are reserved for environment-specific use
    PF_MASKPROC = 0xFF000000, //! \brief These flag bits are reserved for processor-specific use
  };

  struct elf64_phdr_t
  {
    elf64_word_t  p_type;   //! \brief Type of segment
    elf64_word_t  p_flags;  //! \brief Segment attributes
    elf64_off_t   p_offset; //! \brief Offset in file
    elf64_addr_t  p_vaddr;  //! \brief Virtual address in memory
    elf64_addr_t  p_paddr;  //! \brief Reserved
    elf64_xword_t p_filesz; //! \brief Size of segment in file
    elf64_xword_t p_memsz;  //! \brief Size of segment in memory
    elf64_xword_t p_align;  //! \brief Alignment of segment
  };


  enum elf64_symbol_binding
  {
    STB_LOCAL  = 0,  //! \brief Not visible outside the object file
    STB_GLOBAL = 1,  //! \brief Global symbol, visible to all object files
    STB_WEAK   = 2,  //! \brief Global scope, but with lower precedence than global symbols
    STB_LOOS   = 10, //! \brief Environment-specific use
    STB_HIOS   = 12, //! \brief Environment-specific use
    STB_LOPROC = 13, //! \brief Processor-specific use
    STB_HIPROC = 15, //! \brief Processor-specific use
  };

  enum elf64_symbol_type
  {
    STT_NOTYPE  = 0,  //! \brief No type specified (e.g., an absolute symbol)
    STT_OBJECT  = 1,  //! \brief Data object
    STT_FUNC    = 2,  //! \brief Function entry point
    STT_SECTION = 3,  //! \brief Symbol is associated with a section
    STT_FILE    = 4,  //! \brief Source file associated with the object file
    STT_LOOS    = 10, //! \brief Environment-specific use
    STT_HIOS    = 12, //! \brief Environment-specific use
    STT_LOPROC  = 13, //! \brief Processor-specific use
    STT_HIPROC  = 15, //! \brief Processor-specific use
  };

  struct elf64_sym_t
  {
    elf64_word_t  st_name;  //! \brief Symbol name
    unsigned char st_info;  //! \brief Type and binding attributes
    unsigned char st_other; //! \brief Reserved
    elf64_half_t  st_shndx; //! \brief Section table index
    elf64_addr_t  st_value; //! \brief Symbol value
    elf64_xword_t st_size;  //! \brief Size of object.
  };


  enum elf64_dynamic_tag
  {
    DT_NULL         = 0,    //! \brief ignored Marks the end of the dynamic array
    DT_NEEDED       = 1,    //! \brief d_val The string table offset of the name of a needed library.
    DT_PLTRELSZ     = 2,    //! \brief d_val Total size, in bytes, of the relocation entries associated with the procedure linkage table.
    DT_PLTGOT       = 3,    //! \brief d_ptr Contains an address associated with the linkage table. The specific meaning of this field is processor-dependent.
    DT_HASH         = 4,    //! \brief d_ptr Address of the symbol hash table, described below.
    DT_STRTAB       = 5,    //! \brief d_ptr Address of the dynamic string table.
    DT_SYMTAB       = 6,    //! \brief d_ptr Address of the dynamic symbol table.
    DT_RELA         = 7,    //! \brief d_ptr Address of a relocation table with Elf64_Rela entries.
    DT_RELASZ       = 8,    //! \brief d_val Total size, in bytes, of the DT_RELA relocation table.
    DT_RELAENT      = 9,    //! \brief d_val Size, in bytes, of each DT_RELA relocation entry.
    DT_STRSZ        = 10,   //! \brief d_val Total size, in bytes, of the string table.
    DT_SYMENT       = 11,   //! \brief d_val Size, in bytes, of each symbol table entry.
    DT_INIT         = 12,   //! \brief d_ptr Address of the initialization function.
    DT_FINI         = 13,   //! \brief d_ptr Address of the termination function.
    DT_SONAME       = 14,   //! \brief d_val The string table offset of the name of this shared object.
    DT_RPATH        = 15,   //! \brief d_val The string table offset of a shared library search path string.
    DT_SYMBOLIC     = 16,   //! \brief ignored The presence of this dynamic table entry modifies the symbol resolution algorithm for references within the library. Symbols defined within the library are used toresolve references before the dynamic linker searches the usual search path.
    DT_REL          = 17,   //! \brief d_ptr Address of a relocation table with Elf64_Rel entries.
    DT_RELSZ        = 18,   //! \brief d_val Total size, in bytes, of the DT_REL relocation table.
    DT_RELENT       = 19,   //! \brief d_val Size, in bytes, of each DT_REL relocation entry.
    DT_PLTREL       = 20,   //! \brief d_val Type of relocation entry used for the procedure linkage table. The d_val member contains either DT_REL or DT_RELA.
    DT_DEBUG        = 21,   //! \brief d_ptr Reserved for debugger use.
    DT_TEXTREL      = 22,   //! \brief ignored The presence of this dynamic table entry signals that the relocation table contains relocations for a non-writable segment.
    DT_JMPREL       = 23,   //! \brief d_ptr Address of the relocations associated with the procedure linkage table.
    DT_BIND_NOW     = 24,   //! \brief ignored The presence of this dynamic table entry signals that the dynamic loader should process all relocations for this object before transferring control to the program.
    DT_INIT_ARRAY   = 25,   //! \brief d_ptr Pointer to an array of pointers to initialization functions.
    DT_FINI_ARRAY   = 26,   //! \brief d_ptr Pointer to an array of pointers to termination functions.
    DT_INIT_ARRAYSZ = 27,   //! \brief d_val Size, in bytes, of the array of initialization functions.
    DT_FINI_ARRAYSZ = 28,   //! \brief d_val Size, in bytes, of the array of termination functions.
    DT_LOOS   = 0x60000000, //! \brief Defines a range of dynamic table tags that are reserved for environment-specific use.
    DT_HIOS   = 0x6FFFFFFF,
    DT_LOPROC = 0x70000000, //! \brief Defines a range of dynamic table tags that are reserved for processor-specific use.
    DT_HIPROC = 0x7FFFFFFF,
  };


  enum elf64_relocation_type
  {
    R_X86_64_NONE,
    R_X86_64_64,
    R_X86_64_PC32,
    R_X86_64_GOT32,
    R_X86_64_PLT32,
    R_X86_64_COPY,
    R_X86_64_GLOB_DAT,
    R_X86_64_JUMP_SLOT,
    R_X86_64_RELATIVE,
    R_X86_64_GOTPCREL,
    R_X86_64_32,
    R_X86_64_32S,
    R_X86_64_16,
    R_X86_64_PC16,
    R_X86_64_8,
    R_X86_64_PC8,
    R_X86_64_DTPMOD64,
    R_X86_64_DTPOFF64,
    R_X86_64_TPOFF64,
    R_X86_64_TLSGD,
    R_X86_64_TLSLD,
    R_X86_64_DTPOFF32,
    R_X86_64_GOTTPOFF,
    R_X86_64_TPOFF32,
    R_X86_64_PC64,
    R_X86_64_GOTOFF64,
    R_X86_64_GOTPC32,
    R_X86_64_SIZE32,
    R_X86_64_SIZE64,
    R_X86_64_GOTPC32_TLSDESC,
    R_X86_64_TLSDESC_CALL,
    R_X86_64_TLSDESC,
    R_X86_64_IRELATIVE,
  };



  /*!
     \brief The hash function used in ELF64 files.
   */
  elf64_word_t elf64_hash (const std::string& name);
}

#endif //_JCC__LINKER__TRANSLATORS__ELF64__ELF64__H_
