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

#ifndef _JCC__JTAC__TRASLATE__X86_64__X86_64_TRANSLATOR__H_
#define _JCC__JTAC__TRASLATE__X86_64__X86_64_TRANSLATOR__H_

#include "jtac/program.hpp"
#include "jtac/translate/x86_64/procedure.hpp"
#include "jtac/control_flow.hpp"
#include "jtac/allocation/allocator.hpp"
#include <memory>


namespace jcc {
namespace jtac {

  /*!
     \class x86_64_translator
     \brief JTAC to x86-64 code translator.
   */
  class x86_64_translator
  {
    std::unique_ptr<control_flow_graph> cfg;
    std::unique_ptr<register_allocation> reg_res;

   public:
    x86_64_translator ();

   public:
    /*!
       \brief Translates the specified procedure into x86-64.
     */
    x86_64_procedure translate_procedure (const procedure& proc);
  };
}
}

#endif //_JCC__JTAC__TRASLATE__X86_64__X86_64_TRANSLATOR__H_
