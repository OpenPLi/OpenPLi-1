/*
 * x509.cpp: c++ wrapper for openssl x509
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
 * $Id: x509.cpp,v 1.5 2002/06/29 13:04:05 waldi Exp $
 */

#include <main.hpp>

#include <x509.hpp>

#include <bio.hpp>
#include <lib.hpp>

namespace libcrypto
{
  #include <openssl/pem.h>
}

Crypto::x509::cert::cert () throw ( std::bad_alloc )
{
  _cert = libcrypto::X509_new ();

  if ( ! _cert )
    throw std::bad_alloc ();
}

Crypto::x509::cert::cert ( const cert & orig ) throw ( std::bad_alloc )
{
  _cert = libcrypto::X509_dup ( orig._cert );

  if ( ! _cert )
    throw std::bad_alloc ();
}

Crypto::x509::cert & Crypto::x509::cert::operator = ( const cert & orig )
{
  if ( this != &orig )
  {
    if ( _cert )
      libcrypto::X509_free ( _cert );

    _cert = libcrypto::X509_dup ( orig._cert );

    if ( ! _cert )
      throw std::bad_alloc ();
  }

  return * this;
}

void Crypto::x509::cert::read ( std::istream & stream )
{
  libcrypto::X509_free ( _cert );
  _cert = NULL;

  Crypto::bio::istream bio ( stream );
  _cert = libcrypto::PEM_read_bio_X509 ( bio, NULL, NULL, NULL );

  if ( ! _cert )
    throw Crypto::exception::undefined_libcrypto_error ();
}

void Crypto::x509::cert::write ( std::ostream & stream ) const
{
  Crypto::bio::ostream bio ( stream );
  libcrypto::PEM_write_bio_X509 ( bio, _cert );
}

void Crypto::x509::cert::print ( std::ostream & stream ) const
{
  Crypto::bio::ostream bio ( stream );
  libcrypto::X509_print ( bio, _cert );
}

int Crypto::x509::cert::verify ( store & store, int ( * callback ) ( int, libcrypto::X509_STORE_CTX * ) ) throw ( std::bad_alloc, Crypto::exception::undefined_libcrypto_error )
{
  libcrypto::X509_STORE * _store = store;
  libcrypto::X509_STORE_CTX * ctx = libcrypto::X509_STORE_CTX_new();

  if ( ! ctx )
    throw std::bad_alloc ();
  
  X509_STORE_set_verify_cb_func ( _store, callback );
  libcrypto::X509_STORE_CTX_init ( ctx, _store, _cert, NULL );
  int ret = libcrypto::X509_verify_cert ( ctx );
  libcrypto::X509_STORE_CTX_free ( ctx );

  if ( ret > 0 )
    return 1;
  if ( ret == -1 )
    throw Crypto::exception::undefined_libcrypto_error ();
  else
    return 0;
}

Crypto::x509::extension::extension ( ctx & ctx, const std::string & name, const std::string & value ) throw ( Crypto::exception::undefined_libcrypto_error )
{
  _extension = libcrypto::X509V3_EXT_conf ( NULL, ctx, ( char * ) name.c_str (), ( char * ) value.c_str () );

  if ( ! _extension )
    throw Crypto::exception::undefined_libcrypto_error ();
}

Crypto::x509::extension::extension ( const std::string & name, const std::string & value ) throw ( Crypto::exception::undefined_libcrypto_error )
{
  _extension = libcrypto::X509V3_EXT_conf ( NULL, NULL, ( char * ) name.c_str (), ( char * ) value.c_str () );

  if ( ! _extension )
    throw Crypto::exception::undefined_libcrypto_error ();
}

Crypto::x509::extension::extension ( ctx & ctx, int nid, const std::string & value ) throw ( Crypto::exception::undefined_libcrypto_error )
{
  _extension = libcrypto::X509V3_EXT_conf_nid ( NULL, ctx, nid, ( char * ) value.c_str () );

  if ( ! _extension )
    throw Crypto::exception::undefined_libcrypto_error ();
}

Crypto::x509::extension::extension ( int nid, const std::string & value ) throw ( Crypto::exception::undefined_libcrypto_error )
{
  _extension = libcrypto::X509V3_EXT_conf_nid ( NULL, NULL, nid, ( char * ) value.c_str () );

  if ( ! _extension )
    throw Crypto::exception::undefined_libcrypto_error ();
}

Crypto::x509::name::name () throw ( std::bad_alloc )
{
  _name = libcrypto::X509_NAME_new ();

  if ( ! _name )
    throw std::bad_alloc ();
}

Crypto::x509::name::name ( const name & orig ) throw ( std::bad_alloc )
{
  _name = libcrypto::X509_NAME_dup ( orig._name );

  if ( ! _name )
    throw std::bad_alloc ();
}

Crypto::x509::name & Crypto::x509::name::operator = ( const name & orig ) throw ( std::bad_alloc )
{
  if ( this != &orig )
  {
    if ( _name )
      libcrypto::X509_NAME_free ( _name );

    _name = libcrypto::X509_NAME_dup ( orig._name );

    if ( ! _name )
    {
      Crypto::lib::get_error ();
      throw std::bad_alloc ();
    }
  }

  return * this;
}

void Crypto::x509::name::print ( std::ostream & stream ) const throw ()
{
  char * buf = libcrypto::X509_NAME_oneline ( _name, NULL, 0 );
  stream << buf;
  free ( buf );
}

std::string Crypto::x509::name::get ( int nid ) throw ()
{
  char * buf = new char[256];
  libcrypto::X509_NAME_get_text_by_NID ( _name, nid, buf, 256 );
  std::string ret ( buf );
  delete [] buf;
  return ret;
}

int Crypto::x509::name::text2nid ( const std::string & text ) throw ()
{
  if ( text == "CN" )
    return NID_commonName;
  else if ( text == "C" )
    return NID_countryName;
  else if ( text == "L" )
    return NID_localityName;
  else if ( text == "ST" )
    return NID_stateOrProvinceName;
  else if ( text == "O" )
    return NID_organizationName;
  else if ( text == "OU" )
    return NID_organizationalUnitName;
  else if ( text == "Email" )
    return NID_pkcs9_emailAddress;
  throw;
}

Crypto::x509::store::store () throw ( std::bad_alloc )
{
  _store = libcrypto::X509_STORE_new ();

  if ( ! _store )
    throw std::bad_alloc ();
}

void Crypto::x509::store::add_file ( const std::string & name )
{
  libcrypto::X509_LOOKUP * lookup = libcrypto::X509_STORE_add_lookup ( _store, libcrypto::X509_LOOKUP_file () );

  if ( ! lookup )
    throw std::bad_alloc ();

  if ( ! libcrypto::X509_LOOKUP_load_file ( lookup, name.c_str (), X509_FILETYPE_PEM ) )
    throw Crypto::exception::undefined_libcrypto_error ();
}

#ifndef INLINE
#include <x509.ipp>
#endif

