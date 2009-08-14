/*
 * flashimagefs.hpp
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
 * $Id: flashimagefs.hpp,v 1.4 2002/06/29 14:31:50 waldi Exp $
 */

#ifndef __LIBFLASHIMAGE__LIBFLASHIMAGEFS_HPP
#define __LIBFLASHIMAGE__LIBFLASHIMAGEFS_HPP

#include <iostream>
#include <stdexcept>
#include <string>

namespace FlashImage
{
  class FlashImageFS
  {
    public:
      virtual void get_file ( const std::string &, std::ostream & ) = 0;
      virtual int get_size () = 0;
      virtual int get_size_block () = 0;
      virtual void get_signature ( std::ostream & ) = 0;
      virtual void set_signature ( std::istream & ) { throw std::runtime_error ( "unsupported" ); }
      virtual void get_block ( unsigned int, char * ) = 0;
  };
}

#endif
