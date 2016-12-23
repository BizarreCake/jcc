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

#ifndef _JCC__COMMON__BINARY__H_
#define _JCC__COMMON__BINARY__H_

#include <ostream>
#include <istream>
#include <cstdint>


namespace jcc {

  namespace bin {

    inline void
    write_u8 (unsigned char *buf, uint8_t val)
    { *buf = val; }

    inline void
    write_i8 (unsigned char *buf, int8_t val)
    { write_u8 (buf, (uint8_t)val ); }

    inline void
    write_u16_le (unsigned char *buf, uint16_t val)
    {
      buf[0] = (unsigned char)(val & 0xFF);
      buf[1] = (unsigned char)(val >> 8);
    }

    inline void
    write_i16_le (unsigned char *buf, int16_t val)
    { write_u16_le (buf, (uint16_t)val); }

    inline void
    write_u32_le (unsigned char *buf, uint32_t val)
    {
      buf[0] = (unsigned char)(val & 0xFF);
      buf[1] = (unsigned char)((val >> 8) & 0xFF);
      buf[2] = (unsigned char)((val >> 16) & 0xFF);
      buf[3] = (unsigned char)(val >> 24);
    }

    inline void
    write_i32_le (unsigned char *buf, int32_t val)
    { write_u32_le (buf, (uint32_t)val); }

    inline void
    write_u64_le (unsigned char *buf, uint64_t val)
    {
      buf[0] = (unsigned char)(val & 0xFF);
      buf[1] = (unsigned char)((val >> 8) & 0xFF);
      buf[2] = (unsigned char)((val >> 16) & 0xFF);
      buf[3] = (unsigned char)((val >> 24) & 0xFF);
      buf[4] = (unsigned char)((val >> 32) & 0xFF);
      buf[5] = (unsigned char)((val >> 40) & 0xFF);
      buf[6] = (unsigned char)((val >> 48) & 0xFF);
      buf[7] = (unsigned char)(val >> 56);
    }

    inline void
    write_i64_le (unsigned char *buf, int64_t val)
    { write_u64_le (buf, (uint64_t)val); }



    inline void
    write_u8 (std::ostream& strm, uint8_t val)
    {
      unsigned char data[1];
      write_u8 (data, val);
      strm.write ((char *)data, 1);
    }

    inline void
    write_i8 (std::ostream& strm, int8_t  val)
    { write_u8 (strm, (uint8_t)val); }

    inline void
    write_u16_le (std::ostream& strm, uint16_t val)
    {
      unsigned char data[2];
      write_u16_le (data, val);
      strm.write ((char *)data, 2);
    }

    inline void
    write_i16_le (std::ostream& strm, int16_t val)
    { write_u16_le (strm, (uint16_t)val); }

    inline void
    write_u32_le (std::ostream& strm, uint32_t val)
    {
      unsigned char data[4];
      write_u32_le (data, val);
      strm.write ((char *)data, 4);
    }

    inline void
    write_i32_le (std::ostream& strm, int32_t val)
    { write_u32_le (strm, (uint32_t)val); }

    inline void
    write_u64_le (std::ostream& strm, uint64_t val)
    {
      unsigned char data[8];
      write_u64_le (data, val);
      strm.write ((char *)data, 8);
    }

    inline void
    write_i64_le (std::ostream& strm, int64_t val)
    { write_u64_le (strm, (uint64_t)val); }


    inline void
    write_zeroes (std::ostream& strm, int count)
    {
      char buf[4] = { 0 };
      while (count >= 4)
        { strm.write (buf, 4); count -= 4; }
      while (count > 0)
        { strm.write (buf, 1); -- count; }
    }



    inline uint16_t
    read_u16_le (const unsigned char *buf)
    {
      return (uint16_t)buf[0]
          | ((uint16_t)buf[1] << 8);
    }

    inline int16_t
    read_i16_le (const unsigned char *buf)
    { return (int16_t)read_u16_le (buf); }

    inline uint32_t
    read_u32_le (const unsigned char *buf)
    {
      return (uint32_t)buf[0]
          | ((uint32_t)buf[1] << 8)
          | ((uint32_t)buf[2] << 16)
          | ((uint32_t)buf[3] << 24);
    }

    inline int32_t
    read_i32_le (const unsigned char *buf)
    { return (int32_t)read_u32_le (buf); }

    inline uint64_t
    read_u64_le (const unsigned char *buf)
    {
      return (uint64_t)buf[0]
          | ((uint64_t)buf[1] << 8)
          | ((uint64_t)buf[2] << 16)
          | ((uint64_t)buf[3] << 24)
          | ((uint64_t)buf[4] << 32)
          | ((uint64_t)buf[5] << 40)
          | ((uint64_t)buf[6] << 48)
          | ((uint64_t)buf[7] << 56);
    }

    inline int64_t
    read_i64_le (const unsigned char *buf)
    { return (int64_t)read_u64_le (buf); }



    inline uint8_t
    read_u8 (std::istream& strm)
    {
      unsigned char buf[1];
      strm.read ((char *)buf, 1);
      return buf[0];
    }

    inline int8_t
    read_i8 (std::istream& strm)
    { return (int8_t)read_u8 (strm); }

    inline uint16_t
    read_u16_le (std::istream& strm)
    {
      unsigned char buf[2];
      strm.read ((char *)buf, 2);
      return read_u16_le (buf);
    }

    inline int16_t
    read_i16_le (std::istream& strm)
    { return (int16_t)read_u16_le (strm); }

    inline uint32_t
    read_u32_le (std::istream& strm)
    {
      unsigned char buf[4];
      strm.read ((char *)buf, 4);
      return read_u32_le (buf);
    }

    inline int32_t
    read_i32_le (std::istream& strm)
    { return (int32_t)read_u32_le (strm); }

    inline uint64_t
    read_u64_le (std::istream& strm)
    {
      unsigned char buf[8];
      strm.read ((char *)buf, 8);
      return read_u64_le (buf);
    }

    inline int64_t
    read_i64_le (std::istream& strm)
    { return (int64_t)read_u64_le (strm); }
  }
}

#endif //_JCC__COMMON__BINARY__H_
