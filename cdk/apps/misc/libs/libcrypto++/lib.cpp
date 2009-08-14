/*
 * lib.cpp
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
 * $Id: lib.cpp,v 1.1 2002/03/02 21:53:26 waldi Exp $
 */

#include <main.hpp>

#include <lib.hpp>

namespace libcrypto
{
  #include <openssl/err.h>
  #include <openssl/rand.h>
  #include <openssl/ssl.h>
}

void Crypto::lib::init ()
{
  if ( ! _init )
  {
    libcrypto::ERR_load_crypto_strings ();
    libcrypto::OpenSSL_add_all_algorithms ();
  }
}

void Crypto::lib::cleanup ()
{
  if ( _init )
  {
    libcrypto::RAND_cleanup ();
    libcrypto::ERR_free_strings ();
  }
}

std::string Crypto::lib::get_error ()
{
  init ();

  unsigned long error = libcrypto::ERR_get_error ();

  if ( error )
  {
    char * buf = new char[256];
    libcrypto::ERR_error_string_n ( error, buf, 256 );
    std::string ret ( buf );
    delete [] buf;
    return ret;
  }
  else
    return std::string ();
}

std::string Crypto::lib::get_last_error ()
{
  init ();

  char * buf = new char[256];
  unsigned long error = libcrypto::ERR_get_error ();
  std::string ret;

  while ( error )
  {
    libcrypto::ERR_error_string_n ( error, buf, 256 );
    ret = buf;
    error = libcrypto::ERR_get_error ();
  }

  delete [] buf;
  return ret;
}

void Crypto::lib::clear_error ()
{
  init ();

  libcrypto::ERR_clear_error ();
}

bool Crypto::lib::_init = 0;

