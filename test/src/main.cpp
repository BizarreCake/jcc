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

#include <iostream>
#include "linker/generic_module.hpp"
#include "linker/translators/translator.hpp"
#include "assembler/x86_64/assembler.hpp"
#include "linker/linker.hpp"
#include <fstream>
#include <cstring>



int
main (int argc, char *argv[])
{
  using namespace jcc;
  using namespace x86_64;

  std::ifstream fs ("/lib/x86_64-linux-gnu/libc.so.6",
                    std::ios_base::in | std::ios_base::binary);

  auto tr = module_translator::create ("elf64");
  auto libc = tr->load (fs);

  using namespace x86_64;
  relocation_symbol_store rstore;
  assembler asem;
  asem.emit_call(rel_t (rstore.get ("exit")));

  generic_module mod (module_type::relocatable, target_architecture::x86_64);

  mod.set_image_base (0x400000);

  code_section cs (".text");
  auto& code = cs.get_code ();
  code.resize (asem.get_size ());
  std::memcpy (code.data (), asem.get_data (), asem.get_size ());
  for (auto& reloc : asem.get_relocations ())
    cs.add_relocation (reloc);
  mod.add_section (std::move (cs));

  mod.set_entry_point (cs);

  {
    linker lnk;
    lnk.add_module (mod);
    lnk.add_module (*libc);

    auto out = lnk.link ();

    std::ofstream fs ("a.out", std::ios_base::out | std::ios_base::binary);
    auto tr = module_translator::create ("elf64");
    tr->save (*out, fs);
  }

  return 0;
}

