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

#include "jtac/translate/x86_64/x86_64_translator.hpp"
#include "jtac/ssa.hpp"
#include "jtac/allocation/basic/basic.hpp"


namespace jcc {
namespace jtac {

// number of general purpose registers
#define X86_64_NUM_GP_REGISTERS 14 // rax, rbx, rcx, rdx, rsi, rdi
                                   // r8, r9, r10 ,r11, r12, r13, r14, r15

  x86_64_translator::x86_64_translator ()
  {
    this->cfg = nullptr;
  }



  /*!
     \brief Translates the specified procedure into x86-64.
   */
  x86_64_procedure
  x86_64_translator::translate_procedure (const procedure& proc)
  {
    // build control flow graph
    this->cfg.reset (new control_flow_graph (
        std::move (control_flow_analyzer::make_cfg (proc.get_body ()))));

    // transform into SSA form
    ssa_builder ssab;
    ssab.transform (*this->cfg);

    // perform register allocation
    basic_register_allocator reg_alloc;
    this->reg_res.reset (new register_allocation (std::move (
        reg_alloc.allocate (*this->cfg, X86_64_NUM_GP_REGISTERS))));



    x86_64_procedure xproc {};
    return xproc;
  }
}
}
