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

#ifndef _JCC__ASSEMBLER__X86_64__INSTRUCTION__H_
#define _JCC__ASSEMBLER__X86_64__INSTRUCTION__H_

#include "assembler/relocation.hpp"
#include <cstdint>


namespace jcc {

  namespace x86_64 {
    
    enum reg_code
    {
      REG_NONE    = 0xFF,
    
      REG_AL      = 0x00,
      REG_CL      = 0x01,
      REG_DL      = 0x02,
      REG_BL      = 0x03,
      REG_AH      = 0x04,
      REG_CH      = 0x05,
      REG_DH      = 0x06,
      REG_BH      = 0x07,
      
      REG_AX      = 0x10,
      REG_CX      = 0x11,
      REG_DX      = 0x12,
      REG_BX      = 0x13,
      REG_SP      = 0x14,
      REG_BP      = 0x15,
      REG_SI      = 0x16,
      REG_DI      = 0x17,
      
      REG_EAX     = 0x20,
      REG_ECX     = 0x21,
      REG_EDX     = 0x22,
      REG_EBX     = 0x23,
      REG_ESP     = 0x24,
      REG_EBP     = 0x25,
      REG_ESI     = 0x26,
      REG_EDI     = 0x27,
      
      REG_RAX     = 0x30,
      REG_RCX     = 0x31,
      REG_RDX     = 0x32,
      REG_RBX     = 0x33,
      REG_RSP     = 0x34,
      REG_RBP     = 0x35,
      REG_RSI     = 0x36,
      REG_RDI     = 0x37,
      REG_RIP     = 0x38,
    };
    
    enum size_specifier
    {
      SS_BYTE,
      SS_WORD,
      SS_DWORD,
      SS_QWORD,
    };
    
    
    
    // 
    // Operands:
    // 
    
    struct imm_t
    {
      int size;       // in bytes
      int64_t val;
      size_specifier ss;
      
    public:
      imm_t () : size (1), val (0), ss (SS_DWORD) { }
      imm_t (int64_t val) : size (-1), val (val), ss (SS_DWORD) { }
      imm_t (int size, int64_t val) : size (size), val (val), ss (SS_DWORD) { }
      imm_t (size_specifier ss, int64_t val) : size (-1), val (val), ss (ss) { }
      imm_t (size_specifier ss, int size, int64_t val) : size (size), val (val), ss (ss) { }
    };
    
    struct reg_t
    {
      int code;
      
    public:
      reg_t () : code (0) { }
      reg_t (int code) : code (code) { }
      
    public:
      //! /brief Returns the size of the register in bits.
      int register_size () const;
    };
    
    struct mem_t
    {
      size_specifier ss;
      reg_t base;
      int scale;      // 1, 2, 4 or 8
      reg_t index;
      int disp_size;  // in bytes
      int64_t disp;
      
    public:
      mem_t (size_specifier ss, int base_reg, int scale = 1,
             int index_reg = REG_NONE, int disp_size = 0, int64_t disp = 0)
        : ss (ss), base (base_reg), scale (scale), index (index_reg),
        disp_size (disp_size), disp (disp)
        { }
    };

    struct rel_t
    {
      relocation_symbol sym;

     public:
      rel_t (relocation_symbol sym)
        : sym (sym)
      { }
    };

    using label_id = int;

    struct fixed_t { };

    struct lbl_t
    {
      label_id id;
      size_specifier ss;
      bool fixed;
      long long val;

     public:
      lbl_t (label_id id, size_specifier ss = SS_DWORD)
          : id (id), ss (ss), fixed (false), val (0)
      { }

      lbl_t (fixed_t, long long val, size_specifier ss = SS_DWORD)
          : id (0), ss (ss), fixed (true), val (val)
      { }

      lbl_t ()
          : id (0), ss (SS_DWORD), fixed (false), val (0)
      { }
    };
    
    
    
    union operand_t
    {
      imm_t imm;
      reg_t reg;
      mem_t mem;
      rel_t rel;
      lbl_t lbl;

      operand_t () { }
    };
    
    
    
//------------------------------------------------------------------------------
    
    enum operand_encoding
    {
      //! \brief Instruction does not take any operands.
      OP_EN_NP,
      
      //! \brief Instruction takes two register operands.
      //  Not an actual encoding, but used for convenience.
      OP_EN_RR, 
      
      //! \brief Memory destination operand and register source operand.
      OP_EN_MR,
      
      //! \brief Register destination operand and memory source operand.
      OP_EN_RM,
      
      //! \brief Register destination operand and immediate source operand
      //!        NOTE: No ModR/M.
      OP_EN_OI,
      
      //! \brief Same as OP_EN_OI, but with a ModR/M byte.
      OP_EN_RI,
      
      //! \brief Memory destination operand and immediate source operand.
      OP_EN_MI,

      //! \brief Instruction takes single relocation operand.
      OP_EN_X,

      //! \brief Single memory operand.
      OP_EN_M,

      //! \brief Instruction takes a label operand.
      OP_EN_L,

      //! \brief Instruction takes single immediate operand.
      OP_EN_I,
    };
    
    enum instruction_flags
    {
      //! \brief Subtracts one from opcode in case destination operand is
      //!        8 bits wide.
      INS_FLAG_DEST_8_MINUS_ONE = 1 << 0,
      
      //! \brief Subtracts eight from opcode in case destination operand is
      //!        8 bits wide.
      INS_FLAG_DEST_8_MINUS_EIGHT = 1 << 1,
      
      //! \brief Subtracts one from opcode in case source operand is
      //!        8 bits wide.
      INS_FLAG_SRC_8_MINUS_ONE = 1 << 2,
      
      //! \brief Subtracts eight from opcode in case source operand is
      //!        8 bits wide.
      INS_FLAG_SRC_8_MINUS_EIGHT = 1 << 3,
      
      //! \brief Adds the lower 3 bits of the register operand's code to
      //!        to the opcode.
      INS_FLAG_ADD_REG_TO_OPCODE = 1 << 4,
      
      //! \brief The width of the immediate operand is at most 32 bits.
      INS_FLAG_IMM_MAX_32 = 1 << 5,
      
      //! \brief Uses the instruction's secondary opcode in case the
      //!        destination register is AL/AX/EAX/RAX.
      INS_FLAG_USE_OPCODE2_FOR_AX = 1 << 6,
      
      //! \brief Uses the instruction's third opcode in case the
      //!        source immediate operand is 8 bits wide and smaller than the
      //!        destination operand.
      INS_FLAG_USE_OPCODE3_FOR_IMM8 = 1 << 7,

      //! \brief Places bits 8-10 of the instruction's opcode into the ModR/M's
      //!        reg field.
      INS_FLAG_MODRM_REG_EXTEND = 1 << 8,

      //! \brief Use instruction's second opcode in case the destination
      //!        operand is 8 bits wide.
      INS_FLAG_DEST_8_USE_OPCODE2 = 1 << 9,
    };
    
    
    
    /*!
       \struct instruction
       \brief Stores a single x86-64 instruction.
     */
    struct instruction
    {
      int opcode;
      int opcode2; // secondary opcode (used if special flags are used)
      int opcode3; // ^
      operand_t opr1;
      operand_t opr2;
      operand_encoding enc;
      unsigned int flags;
      
      instruction () { }
    };
  }
}

#endif

