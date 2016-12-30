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

#ifndef _JCC__JTAC__LIVE__H_
#define _JCC__JTAC__LIVE__H_

#include "control_flow.hpp"
#include <unordered_map>


namespace jcc {

  /*!
     \class live_variable_analysis
     \brief Stores the result of a live-variable analysis.
   */
  class live_variable_analysis
  {

  };


  /*!
     \class live_variable_analyzer
     \brief JTAC Analyzer that performs live-variable analysis.
   */
  class live_variable_analyzer
  {
    class basic_block_info
    {

    };

   private:
    std::unordered_map<basic_block_id, basic_block_info> infos;

   public:
    /*!
       \brief Performs live-variable on the specified control flow graph.
       \param root Root basic block of the control flow graph.
     */
    live_variable_analysis analyze (const basic_block& root);
  };
}

#endif //_JCC__JTAC__LIVE__H_
