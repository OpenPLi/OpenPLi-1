/*
 * evp.cpp
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
 * $Id: evp.cpp,v 1.4 2002/05/30 11:43:18 waldi Exp $
 */

#include <main.hpp>

#include <evp.hpp>

#include <bio.hpp>
#include <exception.hpp>
#include <lib.hpp>

namespace libcrypto
{
  #include <openssl/pem.h>
}

Crypto::evp::key::key::item::item ( libcrypto::EVP_PKEY * key )
: count ( 1 ), key ( key )
{ }

Crypto::evp::key::key::item::~item ()
{
  if ( key )
    libcrypto::EVP_PKEY_free ( key );
}

Crypto::evp::key::key::key ()
{
  _item = new item ( libcrypto::EVP_PKEY_new () );
}

Crypto::evp::key::key::key ( libcrypto::EVP_PKEY * _key )
: _item ( new item ( _key ) )
{ }

Crypto::evp::key::key::key ( const key & orig )
{
  _item = orig._item;
  _item -> count++;
}

Crypto::evp::key::key::~key ()
{
  _item -> count--;
  if ( ! _item -> count )
    delete _item;
}

Crypto::evp::key::key & Crypto::evp::key::key::operator = ( const key & orig )
{
  TRACE_FUNCTION;

  if ( this != &orig )
  {
    _item -> count--;
    if ( ! _item -> count )
      delete _item;

    _item = orig._item;
    _item -> count++;
  }

  return * this;
}

void Crypto::evp::key::key::set ( Crypto::rsa & rsa )
{
  TRACE_FUNCTION;

  _item -> count--;
  if ( ! _item -> count )
    delete _item;

  libcrypto::EVP_PKEY * temp = libcrypto::EVP_PKEY_new ();

  if ( ! temp )
    throw std::bad_alloc ();

  libcrypto::EVP_PKEY_set1_RSA ( temp, rsa );

  _item = new item ( temp );
}

Crypto::rsa Crypto::evp::key::key::get ()
{
  TRACE_FUNCTION;

  return Crypto::rsa ( libcrypto::EVP_PKEY_get1_RSA ( _item -> key ) );
}

Crypto::evp::key::key::operator libcrypto::EVP_PKEY * ()
{
  TRACE_FUNCTION;

  return _item -> key;
}

void Crypto::evp::key::privatekey::read ( std::istream & stream )
{
  TRACE_FUNCTION;

  _item -> count--;
  if ( ! _item -> count )
    delete _item;

  Crypto::bio::istream bio ( stream );
  libcrypto::EVP_PKEY * temp = libcrypto::PEM_read_bio_PrivateKey ( bio, NULL, NULL, NULL );

  if ( ! temp )
    throw Crypto::exception::evp::bad_decrypt ( Crypto::lib::get_last_error () );

  _item = new item ( temp );
}

void Crypto::evp::key::privatekey::read ( std::istream & stream, const std::string & _passphrase )
{
  TRACE_FUNCTION;

  _item -> count--;
  if ( ! _item -> count )
    delete _item;

  Crypto::bio::istream bio ( stream );
  passphrase = _passphrase;
  libcrypto::EVP_PKEY * temp = libcrypto::PEM_read_bio_PrivateKey ( bio, NULL, &callback, reinterpret_cast < void * > ( this ) );
  //passphrase.clear ();
  passphrase = "";

  if ( ! temp )
    throw Crypto::exception::evp::bad_decrypt ( Crypto::lib::get_error () );

  _item = new item ( temp );
}

void Crypto::evp::key::privatekey::write ( std::ostream & stream )
{
  TRACE_FUNCTION;

  Crypto::bio::ostream bio ( stream );
  libcrypto::PEM_write_bio_PrivateKey ( bio, _item -> key, NULL, NULL, 0, NULL, NULL );
}

void Crypto::evp::key::privatekey::write ( std::ostream & stream, Crypto::evp::cipher::cipher & cipher, const std::string & _passphrase )
{
  TRACE_FUNCTION;

  Crypto::bio::ostream bio ( stream );
  passphrase = _passphrase;
  libcrypto::PEM_write_bio_PrivateKey ( bio, _item -> key, cipher, NULL, 0, &callback, reinterpret_cast < void * > ( this ) );
  //passphrase.clear ();
  passphrase = "";
}

int Crypto::evp::key::privatekey::callback ( char * buf, int num, int, void * _thiz )
{
  Crypto::evp::key::privatekey * thiz = reinterpret_cast < Crypto::evp::key::privatekey * > ( _thiz );
  thiz -> passphrase.copy ( buf, num );
  return thiz -> passphrase.length ();
}

void Crypto::evp::decrypt::init ( const cipher::cipher & cipher, const std::string & key ) throw ()
{
  TRACE_FUNCTION;

  char * buf = new char [ EVP_CIPHER_key_length ( cipher.get () ) ];
  key.copy ( buf, EVP_CIPHER_key_length ( cipher.get () ) );

  libcrypto::EVP_DecryptInit ( &ctx, cipher, reinterpret_cast < unsigned char * > ( buf ), NULL );
}

void Crypto::evp::decrypt::update ( std::istream & istream, std::ostream & ostream )
{
  TRACE_FUNCTION;

  char * bufin = new char[1024];
  char * bufout = new char[1152];
  int size;

  while ( ! istream.eof () )
  {
    istream.read ( bufin, 1024 );
    libcrypto::EVP_DecryptUpdate ( &ctx, reinterpret_cast < unsigned char * > ( bufout ), &size,  reinterpret_cast < unsigned char * > ( bufin ), istream.gcount () );
    ostream << std::string ( bufout, size );
  }

  delete [] bufin;
  delete [] bufout;

  istream.clear ();
}

void Crypto::evp::decrypt::final ( std::ostream & ostream )
{
  TRACE_FUNCTION;

  char * buf = new char[128];
  int size;

  libcrypto::EVP_DecryptFinal ( &ctx, reinterpret_cast < unsigned char * > ( buf ), &size );
  ostream << std::string ( buf, size );

  delete [] buf;
}

void Crypto::evp::encrypt::init ( const cipher::cipher & cipher, const std::string & key ) throw ()
{
  TRACE_FUNCTION;

  char * buf = new char [ EVP_CIPHER_key_length ( cipher.get () ) ];
  key.copy ( buf, EVP_CIPHER_key_length ( cipher.get () ) );

  libcrypto::EVP_EncryptInit ( &ctx, cipher, reinterpret_cast < unsigned char * > ( buf ), NULL );
}

void Crypto::evp::encrypt::update ( std::istream & istream, std::ostream & ostream )
{
  TRACE_FUNCTION;

  char * bufin = new char[1024];
  char * bufout = new char[1152];
  int size;

  while ( ! istream.eof () )
  {
    istream.read ( bufin, 1024 );
    libcrypto::EVP_EncryptUpdate ( &ctx, reinterpret_cast < unsigned char * > ( bufout ), &size,  reinterpret_cast < unsigned char * > ( bufin ), istream.gcount () );
    ostream << std::string ( bufout, size );
  }

  delete [] bufin;
  delete [] bufout;

  istream.clear ();
}

void Crypto::evp::encrypt::final ( std::ostream & ostream )
{
  TRACE_FUNCTION;

  char * buf = new char[128];
  int size;

  libcrypto::EVP_EncryptFinal ( &ctx, reinterpret_cast < unsigned char * > ( buf ), &size );
  ostream << std::string ( buf, size );

  delete [] buf;
}

void Crypto::evp::digest::init ( const md::md & digest ) throw ()
{
  TRACE_FUNCTION;

  libcrypto::EVP_DigestInit ( &ctx, digest );
}

void Crypto::evp::digest::update ( const void * buf, unsigned int len ) throw ()
{
  TRACE_FUNCTION;

  libcrypto::EVP_DigestUpdate ( &ctx, buf, len );
}

void Crypto::evp::digest::update ( std::istream & stream )
{
  TRACE_FUNCTION;

  char * buf = new char[1024];

  while ( ! stream.eof () )
  {
    stream.read ( buf, 1024 );
    libcrypto::EVP_DigestUpdate ( &ctx, buf, stream.gcount () );
  }

  delete [] buf;

  stream.clear ();
}

std::string Crypto::evp::digest::final ()
{
  TRACE_FUNCTION;

  std::string str;
  unsigned char buf [ EVP_MAX_MD_SIZE ];
  unsigned int s;

  libcrypto::EVP_DigestFinal ( &ctx, buf, &s );

  return std::string ( reinterpret_cast < char * > ( buf ), s );
}

std::string Crypto::evp::sign::final ( Crypto::evp::key::privatekey & key )
{
  TRACE_FUNCTION;

  unsigned char buf [ libcrypto::EVP_PKEY_size ( key ) ];
  unsigned int s;
  int ret;

  ret = libcrypto::EVP_SignFinal ( &ctx, buf, &s, key );

  if ( ! ret )
    throw std::runtime_error ( Crypto::lib::get_error () );

  return std::string ( reinterpret_cast < char * > ( buf ), s );
}

int Crypto::evp::verify::final ( const std::string & sig, Crypto::evp::key::key & key )
{
  TRACE_FUNCTION;

  unsigned char * buf  = new unsigned char [ sig.length () ];
  int ret;
  sig.copy ( reinterpret_cast < char * > ( buf ), sig.length () );

  ret = libcrypto::EVP_VerifyFinal ( &ctx, buf, sig.length (), key );

  delete [] buf;

  return ret;
}

Crypto::evp::digest & operator >> ( std::string & string, Crypto::evp::digest & digest )
{
  digest.update ( string.data (), string.length () );
  return digest;
}

