/*
 * bio.cpp: c++ wrapper for openssl bio
 *          bio method for std::iostream
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
 * $Id: bio.cpp,v 1.4 2002/05/24 16:33:33 waldi Exp $
 */

#include <main.hpp>

#include <bio.hpp>

namespace libcrypto
{
  #include <openssl/evp.h>
}

Crypto::bio::bio::bio ()
: _bio ( NULL )
{
  TRACE_FUNCTION;
}

Crypto::bio::bio::~bio ()
{
  TRACE_FUNCTION;
  if ( _bio )
  {
    flush ();
    libcrypto::BIO_free ( _bio );
  }
}

int Crypto::bio::bio::read ( void * buf, int size )
{
  TRACE_FUNCTION;
  if ( _bio )
    return libcrypto::BIO_read ( _bio, buf, size );
  return -1;
}

int Crypto::bio::bio::write ( const void * buf, int size )
{
  TRACE_FUNCTION;
  if ( _bio )
    return libcrypto::BIO_write ( _bio, buf, size );
  return -1;
}

int Crypto::bio::bio::gets ( char * buf, int size )
{
  TRACE_FUNCTION;
  if ( _bio )
    return libcrypto::BIO_gets ( _bio, buf, size );
  return -1;
}

int Crypto::bio::bio::puts ( const char * buf )
{
  TRACE_FUNCTION;
  if ( _bio )
    return libcrypto::BIO_puts ( _bio, buf );
  return -1;
}

void Crypto::bio::bio::flush ()
{
  TRACE_FUNCTION;
  if ( _bio )
    libcrypto::BIO_ctrl ( _bio, BIO_CTRL_FLUSH, 0, NULL );
}

Crypto::bio::bio::operator libcrypto::BIO * ()
{
  return _bio;
}

libcrypto::BIO_METHOD Crypto::bio::istream::bio_method =
{
  BIO_TYPE_FILE,
  "istream",
  NULL,
  bio_read,
  NULL,
  bio_gets,
  bio_ctrl,
  bio_new,
  bio_free,
  NULL,
};

libcrypto::BIO_METHOD Crypto::bio::ostream::bio_method =
{
  BIO_TYPE_FILE,
  "ostream",
  bio_write,
  NULL,
  bio_puts,
  NULL,
  bio_ctrl,
  bio_new,
  bio_free,
  NULL,
};

libcrypto::BIO_METHOD Crypto::bio::iostream::bio_method =
{
  BIO_TYPE_FILE,
  "iostream",
  bio_write,
  bio_read,
  bio_puts,
  bio_gets,
  bio_ctrl,
  bio_new,
  bio_free,
  NULL,
};

Crypto::bio::istream::istream ( std::istream & stream )
: is ( stream )
{
  TRACE_FUNCTION;
  _bio = libcrypto::BIO_new ( &bio_method );
  _bio -> ptr = reinterpret_cast < void * > ( this );
}

Crypto::bio::ostream::ostream ( std::ostream & stream )
: os ( stream )
{
  TRACE_FUNCTION;
  _bio = libcrypto::BIO_new ( &bio_method );
  _bio -> ptr = reinterpret_cast < void * > ( this );
}

Crypto::bio::iostream::iostream ( std::iostream & stream )
: Crypto::bio::istream ( stream ), Crypto::bio::ostream ( stream )
{
  TRACE_FUNCTION;
  _bio = libcrypto::BIO_new ( &bio_method );
  _bio -> ptr = reinterpret_cast < void * > ( this );
}

int Crypto::bio::ostream::bio_write ( libcrypto::BIO * b, const char * buf, int size )
{
  TRACE_FUNCTION;
  Crypto::bio::ostream * thiz = reinterpret_cast < Crypto::bio::ostream * > ( b -> ptr );
  thiz -> os.write ( buf, size );
  return size;
}

int Crypto::bio::istream::bio_read ( libcrypto::BIO * b, char * buf, int size )
{
  TRACE_FUNCTION;
  Crypto::bio::istream * thiz = reinterpret_cast < Crypto::bio::istream * > ( b -> ptr );
  thiz -> is.read ( buf, size );
  return thiz -> is.gcount ();
}

int Crypto::bio::ostream::bio_puts ( libcrypto::BIO * b, const char * buf )
{
  TRACE_FUNCTION;
  Crypto::bio::ostream * thiz = reinterpret_cast < Crypto::bio::ostream * > ( b -> ptr );
  int size = strlen ( buf );
  thiz -> os.write ( buf, size );
  return size;
}

int Crypto::bio::istream::bio_gets ( libcrypto::BIO * b, char * buf, int size )
{
  TRACE_FUNCTION;
  Crypto::bio::istream * thiz = reinterpret_cast < Crypto::bio::istream * > ( b -> ptr );
  thiz -> is.getline ( buf, size );
  return strlen ( buf );
}

long Crypto::bio::stream::bio_ctrl ( libcrypto::BIO *, int, long, void *  )
{
  TRACE_FUNCTION;
  return 1;
}

int Crypto::bio::stream::bio_new ( libcrypto::BIO * b )
{
  TRACE_FUNCTION;
  b -> init = 1;
  b -> num = 0;
  b -> ptr = NULL;
  return 1;
}

int Crypto::bio::stream::bio_free ( libcrypto::BIO * )
{
  TRACE_FUNCTION;
  return 1;
}

Crypto::bio::filter::filter ()
: used ( false )
{ } 

Crypto::bio::filter::~filter ()
{ 
  if ( used )
    libcrypto::BIO_pop ( _bio );
} 

void Crypto::bio::filter::push ( bio & bio )
{
  TRACE_FUNCTION;

  if ( ! used )
  {
    libcrypto::BIO_push ( _bio, bio._bio );
    used = true;
  }
}

void Crypto::bio::filter::pop ()
{
  TRACE_FUNCTION;

  if ( used )
  {
    libcrypto::BIO_pop ( _bio );
    used = false;
  }
}

Crypto::bio::base64::base64 ()
{
  TRACE_FUNCTION;
  _bio = libcrypto::BIO_new ( libcrypto::BIO_f_base64 () );
}

Crypto::bio::base64::base64 ( bio & bio )
{
  TRACE_FUNCTION;
  _bio = libcrypto::BIO_new ( libcrypto::BIO_f_base64 () );
  push ( bio );
}

Crypto::bio::base64NoNewline::base64NoNewline ()
{
  TRACE_FUNCTION;
  BIO_set_flags ( _bio, BIO_FLAGS_BASE64_NO_NL );
}

Crypto::bio::base64NoNewline::base64NoNewline ( bio & bio )
: base64 ( bio )
{
  TRACE_FUNCTION;
  BIO_set_flags ( _bio, BIO_FLAGS_BASE64_NO_NL );
}

std::istream & operator >> ( std::istream & stream, Crypto::bio::bio & bio )
{
  TRACE_FUNCTION;

  char * buf = new char[1024];
  int ret;

  while ( ! stream.eof () );
  {
    stream.read ( buf, 1024 );
    ret = bio.write ( buf, stream.gcount () );
    if ( ret == 0 || ret == -1 || ret == -2 )
    {
      delete [] buf;
      throw;
    }
  }

  bio.flush ();

  delete [] buf;

  return stream;
}

std::ostream & operator << ( std::ostream & stream, Crypto::bio::bio & bio )
{
  TRACE_FUNCTION;

  char buf[1025];
  int ret = 0;

  do
  {
    ret = bio.read ( buf, 1024 );
    if ( ret > 0 )
    {
      buf[ret] = '\0';
      stream << buf;
    }
    else if ( ret == 0 || ret == -1 )
      throw;
    else if ( ret == -2 )
      throw;
  }
  while ( ret > 0 );

  return stream;
}

Crypto::bio::bio & operator >> ( Crypto::bio::bio & bio, std::string & str )
{
  TRACE_FUNCTION;

  char buf[1024];
  int ret;

  do
  {
    ret = bio.read ( buf, 1024 );
    if ( ret > 0 )
    {
      str.append ( buf, ret );
    }
    else if ( ret == 0 )
      break;
    else if ( ret == -1 )
      throw;
    else if ( ret == -2 )
      throw;
  }
  while ( ret > 0 );

  return bio;
}

Crypto::bio::bio & operator << ( Crypto::bio::bio & bio, const std::string & str )
{
  TRACE_FUNCTION;

  bio.write ( str.data (), str.length () );

  bio.flush ();

  return bio;
}

