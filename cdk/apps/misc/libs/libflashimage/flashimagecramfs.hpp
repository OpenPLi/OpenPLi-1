/*
 * flashimagecramfs.hpp
 *
 * Copyright (C) 2002 Bastian Blank <waldi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: flashimagecramfs.hpp,v 1.3 2002/06/29 14:31:50 waldi Exp $
 */

#ifndef __LIBFLASHIMAGE__LIBFLASHIMAGECRAMFS_HPP
#define __LIBFLASHIMAGE__LIBFLASHIMAGECRAMFS_HPP

#include <libflashimage/flashimagefs.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <string>

namespace FlashImage
{
  class FlashImageCramFS : public FlashImageFS
  {
    private:
      struct signatureinfo
      {
        unsigned long magic;
        unsigned long version;
        unsigned long size;
        unsigned long reserved;
      };

    public:
      FlashImageCramFS ( std::iostream & );
      virtual ~FlashImageCramFS () throw ();

      void get_file ( const std::string &, std::ostream & );
      int get_size () throw ();
      int get_size_block () throw ();
      void get_signature ( std::ostream & );
      void set_signature ( std::istream & );
      void get_block ( unsigned int, char * );

      static const unsigned long magic = 0x9ad13486;

    private:
      unsigned long readdir ( std::istream &, unsigned long, unsigned long, std::istream & );
      void decompress ( std::istream &, std::ostream &, unsigned int );

      std::iostream & stream;
      unsigned int cramfssize, cramfsblocks, imagesize;
      char * const buf;
  };
}

#endif
