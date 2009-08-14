/*
 * noexcept.hpp
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
 * $Id: noexcept.hpp,v 1.1 2002/06/29 14:31:38 waldi Exp $
 */

#ifndef __LIBFLASHIMAGE__NOEXCEPT_HPP
#define __LIBFLASHIMAGE__NOEXCEPT_HPP

#include <libflashimage/flashimage.hpp>
#include <libflashimage/flashimagecramfs.hpp>

namespace FlashImage
{
  class CramFSNoExcept
  {
    public:
      CramFSNoExcept ( std::iostream & ) throw ();
      ~CramFSNoExcept () throw ();

      const std::string & get_error () throw ();
      int get_size () throw ();
      int verify_init () throw ();
      int verify_update () throw ();
      int verify_final () throw ();

    private:
      struct verify
      {
        verify ( FlashImageVerify item )
        : item ( item )
        { }
        FlashImageVerify item;
      };

      std::iostream & image;
      FlashImage * item;
      FlashImageCramFS * item_fs;
      verify * item_verify;

      std::string error;
  };
}

#endif
