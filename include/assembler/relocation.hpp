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

#ifndef _JCC__ASSEMBLER__RELOCATION__H_
#define _JCC__ASSEMBLER__RELOCATION__H_

#include <cstddef>
#include <string>
#include <vector>
#include <unordered_map>


namespace jcc {
  
  /*!
     \enum relocation_type
     \brief Enumeration of possible relocation types.
     
     A - The addend used to compute the value of the relocatable field.
     B - The base address at which a shared object has been loaded into memory
         during execution.
     G - Represents the offset into the global offset table.
     GOT - Represents the address of the global offset table.
     L - Represents the place (section offset or address) of the Procedural
         Linkage Table entry for a symbol.
     P - Represents the place (section offset or address) of the storage unit
         being relocated.
     S - Represents the value of the symbol.
     Z - Represents the size of the symbol.
   */
  enum relocation_type
  {
    R_NONE,
    R_PC32,   //! \brief Calculation: S + A - P
  };



  // forward decs:
  class relocation_symbol_store;

  /*!
     \brief Identifies a value that a relocation should take upon itself.
   */
  using relocation_symbol_id = int;

  /*!
     \struct relocation_symbol
     \brief Stores an identifier value and a pointer to the store that
            generated the value.
   */
  struct relocation_symbol
  {
    relocation_symbol_store *store;
    relocation_symbol_id id;
  };
  


  /*!
     \struct relocation
     \brief Stores a single relocation.
   */
  struct relocation
  {
    relocation_type type;
    relocation_symbol sym;
    size_t offset;
    unsigned int size;
    int add;
  };
  
  
  
  /*!
     \class relocation_symbol_store
     \brief Manages relocation symbols.
   */
  class relocation_symbol_store
  {
    std::vector<std::string> names;
    std::unordered_map<std::string, int> index_map;

   public:
    relocation_symbol_store ();

   public:
    //! \brief Returns a relocation symbol for the specified name.
    relocation_symbol get (const std::string& name);

    //! \brief Returns the name associated with the specified symbol ID.
    const std::string& get_name (relocation_symbol_id id) const;
  };  
}

#endif

