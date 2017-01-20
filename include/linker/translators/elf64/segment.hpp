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

#ifndef _JCC__LINKER__TRANSLATORS__ELF64__SEGMENT__H_
#define _JCC__LINKER__TRANSLATORS__ELF64__SEGMENT__H_

#include <vector>
#include "linker/translators/elf64/elf64.hpp"


namespace jcc {

  class elf64_section;

  /*!
     \class elf64_segment
     \brief Describes an ordered collection of sections to be grouped together
            in executable and shared object files.
   */
  class elf64_segment
  {
    elf64_phdr_t phdr;
    std::vector<elf64_section *> sects;

   public:
    inline elf64_phdr_t& get_header () { return this->phdr; }
    inline const elf64_phdr_t& get_header () const { return this->phdr; }

    inline std::vector<elf64_section *>& get_sections () { return this->sects; }
    inline const std::vector<elf64_section *>& get_sections () const { return this->sects; }

   public:
    elf64_segment ();

   public:
    elf64_segment&
    set_flags (elf64_word_t flags)
    { this->phdr.p_flags = flags; return *this; }

   public:
    //! \brief Inserts the specified section to the end of the segment.
    elf64_segment& add_section (elf64_section& sect);

    //! \brief Checks whether this segment contains the specified section.
    bool has_section (const elf64_section& sect) const;
  };
}

#endif //_JCC__LINKER__TRANSLATORS__ELF64__SEGMENT__H_
