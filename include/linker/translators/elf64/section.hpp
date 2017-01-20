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

#ifndef _JCC__LINKER__TRANSLATORS__ELF64__SECTION__H_
#define _JCC__LINKER__TRANSLATORS__ELF64__SECTION__H_

#include "linker/translators/elf64/elf64.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace jcc {

  // forward decs:
  class elf64_object_file;

  /*!
     \class elf64_section
     \brief Base class for ELF64 sections.
   */
  class elf64_section
  {
   protected:
    elf64_object_file& obj;
    int id;
    int index; // index of section in section header table
    elf64_shdr_t shdr;
    elf64_addr_t vaddr_hint;

   public:
    inline int get_id () const { return this->id; }
    inline void set_id (int id) { this->id = id; }

    inline int get_index () const { return this->index; }
    inline void set_index (int idx) { this->index = idx; }

    inline elf64_shdr_t& get_header () { return this->shdr; }
    inline const elf64_shdr_t& get_header () const { return this->shdr; }

    inline elf64_addr_t get_vaddr_hint () const { return this->vaddr_hint; }
    inline void set_vaddr_hint (elf64_addr_t hint) { this->vaddr_hint = hint; }

   public:
    elf64_section (elf64_object_file& obj);
    virtual ~elf64_section () { }

   public:
    void set_flags (elf64_word_t flags) { this->shdr.sh_flags = flags; }
    void set_alignment (elf64_xword_t align) { this->shdr.sh_addralign = align; }

   public:
    /*!
       \brief Prepares the section's binary data, and determines its size.
     */
    virtual void bake () = 0;

    //! \brief Determines the size of the section in bytes.
    virtual size_t compute_size () = 0;

    //! \brief Returns the binary data of the section (created by bake()).
    virtual const char* get_data () const = 0;

    //! \brief Loads section contents from the specified byte array.
    virtual void load_raw (const unsigned char *raw, unsigned int len) = 0;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_null_section
     \brief An all-zero section.
   */
  class elf64_null_section: public elf64_section
  {
    std::string data;

   public:
    elf64_null_section (elf64_object_file& obj);

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override { }
  };


//------------------------------------------------------------------------------

  /*!
     \class elf64_strtab_section
     \brief String table section.
   */
  class elf64_strtab_section: public elf64_section
  {
    std::unordered_map<std::string, int> index_map;
    int curr_idx;

    std::string data;

   public:
    elf64_strtab_section (elf64_object_file& obj);

   public:
    //! \brief Checks whether the string table contains the specified string.
    bool has_string (const std::string& str) const;

    //! \brief Returns the index of the specified string in the table if it
    //!        exists; otherwise, returns -1.
    int get_string (const std::string& str) const;

    //! \brief Inserts the specified string into the table if it does not
    //!        already exists, and returns its index.
    int add_string (const std::string& str);

    //! \brief Returns the string associated with the specified index.
    const char* get_string (int idx) const;

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
      { return this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_symtab_section
     \brief Symbol table section.
   */
  class elf64_symtab_section: public elf64_section
  {
   public:
    struct symbol
    {
      int id;
      int index;

      elf64_word_t name;
      elf64_symbol_type type;
      elf64_symbol_binding bind;
      int sect_id;
      elf64_xword_t size;

      struct
      {
        bool is_ptr;
        union
        {
          elf64_xword_t num;
          struct
          {
            elf64_xword_t off;
          } ptr;
        };
      } val;
    };

   protected:
    elf64_strtab_section& strtab; // associated string table
    std::vector<symbol> syms_local, syms_global;
    std::string data;

    int next_sym_id;

   public:
    inline const auto& get_local_symbols () const { return this->syms_local; }
    inline const auto& get_global_symbols () const { return this->syms_global; }
    inline size_t get_count () const { return this->syms_local.size () + this->syms_global.size () + 1; }

    inline elf64_strtab_section& get_strtab () { return this->strtab; }
    inline const elf64_strtab_section& get_strtab () const { return this->strtab; }

   public:
    elf64_symtab_section (elf64_object_file& obj, elf64_strtab_section& strtab);

   public:
    //! \brief Inserts a new symbol to the end of the table.
    int add_symbol (const std::string& name, elf64_symbol_type type,
                    elf64_symbol_binding bind, int sect_id,
                    elf64_addr_t value, elf64_xword_t size = 0);

    /*!
       \brief Inserts a new symbol to the end of the table.

       The value of the symbol will be set to the specified section's virtual
       address summed together with the given offset.
     */
    int add_symbol_ptr (const std::string& name, elf64_symbol_type type,
                        elf64_symbol_binding bind, int sect_id,
                        elf64_addr_t offset = 0, elf64_xword_t size = 0);

    //! \brief Searches the table for the ID of a symbol that has the specified
    //!        name.
    int find_symbol_id (const std::string& name) const;

    //! \brief Returns the symbol has the specified ID.
    const symbol& get_symbol (int id) const;

   private:
    void write_symbol (std::ostream& strm, symbol& sym);

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_dynsym_section
     \brief Dynamic symbol table section.
   */
  class elf64_dynsym_section: public elf64_symtab_section
  {
   public:
    elf64_dynsym_section (elf64_object_file& obj, elf64_strtab_section& strtab);
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_progbits_section
     \brief Section containing information defined by the program.
   */
  class elf64_progbits_section: public elf64_section
  {
    std::vector<unsigned char> data;

   public:
    elf64_progbits_section (elf64_object_file& obj);
    elf64_progbits_section (elf64_object_file& obj, const std::vector<unsigned char>& data);
    elf64_progbits_section (elf64_object_file& obj, std::vector<unsigned char>&& data);
    elf64_progbits_section (elf64_object_file& obj, const unsigned char *data, size_t len);

   private:
    void init ();

   public:
    unsigned char* get_data () { return this->data.data (); }
    void set_data (const unsigned char *data, size_t len);

   public:
    virtual void bake () { }

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_interp_section
     \brief Holds the program interpreter's path.
   */
  class elf64_interp_section: public elf64_section
  {
    std::string interp;

   public:
    inline const std::string& get_interp () const { return this->interp; }
    inline void set_interp (const std::string& val) { this->interp = val; }

   public:
    elf64_interp_section (elf64_object_file& obj, const std::string& interp);

   public:
    virtual void bake () { }

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->interp.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override { }
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_dynamic_section
     \brief Dynamic linking table.
   */
  class elf64_dynamic_section: public elf64_section
  {
    struct entry
    {
      elf64_dynamic_tag tag;
      bool is_ptr;
      union
      {
        elf64_xword_t val;
        struct
        {
          elf64_section *sect;
          elf64_xword_t off;
        } ptr;
      };
    };

   private:
    elf64_strtab_section& strtab;
    std::vector<entry> entries;
    std::string data;

   public:
    inline elf64_strtab_section& get_strtab () { return this->strtab; }
    inline const elf64_strtab_section& get_strtab () const { return this->strtab; }

    inline const auto& get_entries () const { return this->entries; }

   public:
    elf64_dynamic_section (elf64_object_file& obj, elf64_strtab_section& strtab);

   public:
    //! \brief Inserts an entry to the end of the table.
    void add (elf64_dynamic_tag tag, elf64_xword_t val);

    /*!
       \brief Inserts an entry to the end of the table.
       The value of the entry being the sum of the specified section's
       virtual address and the given offset.
     */
    void add (elf64_dynamic_tag tag, elf64_section& sect, elf64_xword_t off = 0);

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_rela_section
     \brief Relocations section.
   */
  class elf64_rela_section: public elf64_section
  {
    struct entry
    {
      elf64_relocation_type type;
      elf64_section *sect;
      elf64_off_t offset;
      int sym_id;
      elf64_sxword_t add;
    };

   private:
    elf64_section& sect;
    elf64_symtab_section& symtab;
    std::vector<entry> entries;
    std::string data;

   public:
    elf64_rela_section (elf64_object_file& obj, elf64_section& sect,
                        elf64_symtab_section& symtab);

   public:
    //! \brief Inserts a relocation entry.
    void add (elf64_relocation_type type, elf64_section& sect,
              elf64_off_t offset, int sym_id, elf64_sxword_t add);

    //! \brief Inserts a zero-based relocation entry.
    void add (elf64_relocation_type type, elf64_off_t offset,
              int sym_id, elf64_sxword_t add);

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override { }
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_verdef_section
     \brief Version definition section.
   */
  class elf64_verdef_section: public elf64_section
  {
   public:
    struct entry
    {
      elf64_half_t index; // version index
      elf64_half_t flags;
      std::vector<std::string> names;
      elf64_word_t hash;

     public:
      entry&
      add_predecessor (const std::string& name)
      { this->names.push_back (name); return *this; }
    };

   private:
    elf64_strtab_section& strtab;
    std::vector<entry> entries;
    std::string data;

   public:
    inline elf64_strtab_section& get_strtab () { return this->strtab; }
    inline const elf64_strtab_section& get_strtab () const { return this->strtab; }

    inline const auto& get_entries () const { return this->entries; }

   public:
    elf64_verdef_section (elf64_object_file& obj, elf64_strtab_section& strtab);

   public:
    entry& add (elf64_half_t index, elf64_half_t flags, const std::string& name);

    entry& get_version (elf64_half_t index);

    const std::string& get_version_name (elf64_half_t index);

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };



//------------------------------------------------------------------------------

  /*!
     \class elf64_versym_section
     \brief Symbol versions section.
   */
  class elf64_versym_section: public elf64_section
  {
    elf64_dynsym_section& dynsym;
    std::vector<elf64_half_t> entries;
    std::string data;

   public:
    elf64_versym_section (elf64_object_file& obj,
                          elf64_dynsym_section& dynsym);

   public:
    void set_entry (int index, elf64_half_t val);

    elf64_half_t get_entry (int index) const;

   public:
    virtual void bake () override;

    virtual size_t compute_size () override;

    virtual const char* get_data () const override
    { return (char *)this->data.data (); }

    virtual void load_raw (const unsigned char *raw, unsigned int len) override;
  };
}

#endif //_JCC__LINKER__TRANSLATORS__ELF64__SECTION__H_
