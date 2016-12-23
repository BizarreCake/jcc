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

#include "linker/translators/elf64/segment.hpp"
#include "linker/translators/elf64/section.hpp"
#include <cstring>
#include <algorithm>


namespace jcc {

  elf64_segment::elf64_segment ()
  {
    std::memset (&this->phdr, 0, sizeof this->phdr);
    this->phdr.p_align = 16;
    this->phdr.p_flags = PF_R;
  }



  //! \brief Inserts the specified section to the end of the segment.
  elf64_segment&
  elf64_segment::add_section (elf64_section& sect)
  {
    auto& hdr = sect.get_header ();
    hdr.sh_flags |= SHF_ALLOC;

    this->sects.push_back (&sect);
    return *this;
  }

  //! \brief Checks whether this segment contains the specified section.
  bool
  elf64_segment::has_section (const elf64_section& sect) const
  {
    return std::find (this->sects.begin (), this->sects.end (), &sect) != this->sects.end ();
  }
}
