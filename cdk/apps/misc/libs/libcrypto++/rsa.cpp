/*
 * rsa.cpp
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
 * $Id: rsa.cpp,v 1.1 2002/03/02 21:53:26 waldi Exp $
 */

#include <main.hpp>

#include <rsa.hpp>

Crypto::rsa::rsa ( libcrypto::RSA * rsa ) throw ()
: _rsa ( rsa )
{ }

Crypto::rsa::~rsa () throw ()
{
  if ( _rsa )
    libcrypto::RSA_free ( _rsa );
}

void Crypto::rsa::generate ( int bits, unsigned long e )
{
  if ( _rsa )
  {
    libcrypto::RSA_free ( _rsa );
    _rsa = NULL;
  }

  _rsa = libcrypto::RSA_generate_key ( bits, e, NULL, NULL );
  
  if ( ! _rsa )
    throw;
}

Crypto::rsa::operator libcrypto::RSA * ()
{
  if ( ! _rsa )
    throw;

  return _rsa;
}
