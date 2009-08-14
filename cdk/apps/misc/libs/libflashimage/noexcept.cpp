/*
 * noexcept.cpp
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
 * $Id: noexcept.cpp,v 1.1 2002/06/29 14:31:38 waldi Exp $
 */

#include <libflashimage/noexcept.hpp>

FlashImage::CramFSNoExcept::CramFSNoExcept ( std::iostream & image ) throw ()
: image ( image ), item ( NULL ), item_fs ( NULL ), item_verify ( NULL )
{
  try
  {
    item_fs = new FlashImageCramFS ( image );
    item = new FlashImage ( *item_fs );
  }
  catch ( std::runtime_error & except )
  {
    error = std::string ( except.what () );
    delete item;
    delete item_fs;
  }
}

const std::string & FlashImage::CramFSNoExcept::get_error () throw ()
{
  return error;
}

int FlashImage::CramFSNoExcept::get_size () throw ()
{
  if ( item )
    return item_fs -> get_size ();
  else
    return -255;
}

int FlashImage::CramFSNoExcept::verify_init () throw ()
{
  if ( item_verify )
    return -1;

  try
  {
    item_verify = new verify ( item -> verify_image () );
    return 0;
  }
  catch ( ... )
  {
  }

  return -255;
}

int FlashImage::CramFSNoExcept::verify_update () throw ()
{
  if ( item_verify )
  {
    try
    {
      return item_verify -> item.update ();
    }
    catch ( ... )
    {
    }
  }

  return -255;
}

int FlashImage::CramFSNoExcept::verify_final () throw ()
{
  if ( item_verify )
  {
    try
    {
      return item_verify -> item.final ();
    }
    catch ( ... )
    {
    }
  }

  return -255;
}

