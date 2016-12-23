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

#ifndef _JCC__ASSEMBLER__X86_64__ASSEMBLER__H_
#define _JCC__ASSEMBLER__X86_64__ASSEMBLER__H_

#include "assembler/x86_64/instruction.hpp"
#include "assembler/relocation.hpp"
#include <cstdint>
#include <vector>
#include <cstddef>


namespace jcc {

  namespace x86_64 {

    /*!
       \class invalid_instruction_error
       \brief Thrown by the assembler when an attmept to emit a malformed
              instruction is made.
     */
    class invalid_instruction_error: public std::runtime_error
    {
     public:
      invalid_instruction_error (const std::string& str)
          : std::runtime_error (str)
      { }
    };

    
    /*!
       \class assembler 
       \brief The x86-64 assembler.
     */
    class assembler
    {
      struct label_use
      {
        lbl_t lbl;
        size_t pos;
        int add;
      };

     private:
      std::vector<unsigned char> data;
      size_t pos;
      size_t sz;

      // relocations:
      std::vector<relocation> relocs;

      // labels:
      label_id next_lbl_id;
      std::vector<label_use> lbl_uses;
      std::unordered_map<label_id, size_t> lbl_fixes;

     public:
      inline const unsigned char* get_data () const { return this->data.data (); }
      inline size_t get_size () const { return this->sz; }

      inline const std::vector<relocation>& get_relocations () const { return this->relocs; }
      
     public:
      assembler ();

     public:
      //! \brief Creates and returns a new label.
      label_id make_label ();

      //! \brief Sets the position of the specified label to the current position.
      void mark_label (label_id id);

      //! \brief Calls make_label() and mark_label() in succession.
      label_id make_and_mark_label ();

      //! \brief Fixes label references in the generated code.
      void fix_labels ();

     private:
      void put_u8 (uint8_t v);
      void put_u16 (uint16_t v);
      void put_u32 (uint32_t v);
      void put_u64 (uint64_t v);
      
     private:
      void emit_prefixes (const instruction& ins);
      void emit_rex_prefix (const instruction& ins);
      
      void emit_opcode (const instruction& ins);

      void check_operands (const instruction& ins);
      void emit_operands (const instruction& ins);
      void emit_modrm_and_sib (const instruction& ins);
    
     public:
#define DEF_INS_NP(NAME, OPCODE, FLAGS)             \
  void                                              \
  emit_##NAME ()                                    \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.enc = OP_EN_NP;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }
    
#define DEF_INS_RR(NAME, OPCODE, FLAGS)             \
  void                                              \
  emit_##NAME (const reg_t& dest, const reg_t& src) \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.reg = dest;                            \
    ins.opr2.reg = src;                             \
    ins.enc = OP_EN_RR;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }
  
#define DEF_INS_MR(NAME, OPCODE, FLAGS)             \
  void                                              \
  emit_##NAME (const mem_t& dest, const reg_t& src) \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.mem = dest;                            \
    ins.opr2.reg = src;                             \
    ins.enc = OP_EN_MR;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }
  
#define DEF_INS_RM(NAME, OPCODE, FLAGS)             \
  void                                              \
  emit_##NAME (const reg_t& dest, const mem_t& src) \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.reg = dest;                            \
    ins.opr2.mem = src;                             \
    ins.enc = OP_EN_RM;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_OI(NAME, OPCODE, FLAGS)             \
  void                                              \
  emit_##NAME (const reg_t& dest, const imm_t& src) \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.reg = dest;                            \
    ins.opr2.imm = src;                             \
    ins.enc = OP_EN_OI;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_MI2(NAME, OPCODE, OPCODE2, FLAGS)   \
  void                                              \
  emit_##NAME (const mem_t& dest, const imm_t& src) \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opcode2 = (OPCODE2);                        \
    ins.opr1.mem = dest;                            \
    ins.opr2.imm = src;                             \
    ins.enc = OP_EN_MI;                             \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_MI(NAME, OPCODE, FLAGS)             \
  DEF_INS_MI2(NAME, (OPCODE), 0, (FLAGS))

#define DEF_INS_RI3(NAME, OPCODE, OPCODE2, OPCODE3, FLAGS) \
  void                                                     \
  emit_##NAME (const reg_t& dest, const imm_t& src)        \
  {                                                        \
    instruction ins;                                       \
    ins.opcode = (OPCODE);                                 \
    ins.opcode2 = (OPCODE2);                               \
    ins.opcode3 = (OPCODE3);                               \
    ins.opr1.reg = dest;                                   \
    ins.opr2.imm = src;                                    \
    ins.enc = OP_EN_RI;                                    \
    ins.flags = (FLAGS);                                   \
    this->emit (ins);                                      \
  }
  
#define DEF_INS_RI2(NAME, OPCODE, OPCODE2, FLAGS)          \
  DEF_INS_RI3(NAME, (OPCODE), (OPCODE2), 0, (FLAGS))

#define DEF_INS_RI(NAME, OPCODE, FLAGS)        \
  DEF_INS_RI3(NAME, (OPCODE), 0, 0, (FLAGS))

#define DEF_INS_X(NAME, OPCODE, FLAGS)              \
  void                                              \
  emit_##NAME (const rel_t& opr)                    \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.rel = opr;                             \
    ins.enc = OP_EN_X;                              \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_M(NAME, OPCODE, FLAGS)              \
  void                                              \
  emit_##NAME (const mem_t& opr)                    \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opr1.mem = opr;                             \
    ins.enc = OP_EN_M;                              \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_L(NAME, OPCODE, OPCODE2, FLAGS)     \
  void                                              \
  emit_##NAME (const lbl_t& opr)                    \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opcode2 = (OPCODE2);                        \
    ins.opr1.lbl = opr;                             \
    ins.enc = OP_EN_L;                              \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }

#define DEF_INS_I(NAME, OPCODE, OPCODE2, FLAGS)     \
  void                                              \
  emit_##NAME (const imm_t& opr)                    \
  {                                                 \
    instruction ins;                                \
    ins.opcode = (OPCODE);                          \
    ins.opcode2 = (OPCODE2);                        \
    ins.opr1.imm = opr;                             \
    ins.enc = OP_EN_I;                              \
    ins.flags = (FLAGS);                            \
    this->emit (ins);                               \
  }


#define DEF_INS_RR_STANDARD(NAME, OPCODE)           \
  DEF_INS_RR(NAME, OPCODE, INS_FLAG_SRC_8_MINUS_ONE)

#define DEF_INS_MR_STANDARD(NAME, OPCODE)           \
  DEF_INS_MR(NAME, OPCODE, INS_FLAG_SRC_8_MINUS_ONE)

#define DEF_INS_RM_STANDARD(NAME, OPCODE)           \
  DEF_INS_RM(NAME, OPCODE, INS_FLAG_DEST_8_MINUS_ONE)
  
#define DEF_INS_OI_STANDARD(NAME, OPCODE)           \
  DEF_INS_OI(NAME, OPCODE, INS_FLAG_SRC_8_MINUS_EIGHT | INS_FLAG_ADD_REG_TO_OPCODE)
  
#define DEF_INS_MI_STANDARD(NAME, OPCODE)           \
  DEF_INS_MI(NAME, OPCODE, INS_FLAG_SRC_8_MINUS_ONE | INS_FLAG_IMM_MAX_32)
      
      DEF_INS_RI3(add, 0x81, 0x05, 0x83, INS_FLAG_DEST_8_MINUS_ONE
        | INS_FLAG_IMM_MAX_32 | INS_FLAG_USE_OPCODE2_FOR_AX
        | INS_FLAG_USE_OPCODE3_FOR_IMM8)
      DEF_INS_MR(add, 0x01, INS_FLAG_SRC_8_MINUS_ONE)
      DEF_INS_RM(add, 0x03, INS_FLAG_DEST_8_MINUS_ONE)
      DEF_INS_RR_STANDARD(mov, 0x89)
      DEF_INS_MR_STANDARD(mov, 0x89)
      DEF_INS_RM_STANDARD(mov, 0x8B)
      DEF_INS_OI_STANDARD(mov, 0xB8)
      DEF_INS_MI_STANDARD(mov, 0xC7)
      DEF_INS_RR(movzx, 0x0FB7, INS_FLAG_SRC_8_MINUS_ONE)
      DEF_INS_RM(movzx, 0x0FB7, INS_FLAG_SRC_8_MINUS_ONE)
      DEF_INS_NP(syscall, 0x0F05, 0)
      DEF_INS_NP(sysenter, 0x0F34, 0)
      DEF_INS_X(call, 0xE8, 0)
      DEF_INS_M(push, 0x6FF, INS_FLAG_MODRM_REG_EXTEND)
      DEF_INS_I(push, 0x68, 0x6A, INS_FLAG_DEST_8_USE_OPCODE2)
      DEF_INS_M(jmp, 0x4FF, INS_FLAG_MODRM_REG_EXTEND)
      DEF_INS_L(jmp, 0xE9, 0xEB, INS_FLAG_DEST_8_USE_OPCODE2)
      DEF_INS_NP(nop, 0x90, 0)
      
      
      
      /*!
         Emits the specified instruction onto the underlying buffer.
       */
      void emit (const instruction& ins);
    };
  }
}

#endif

