/*
 * flashimage.cpp
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
 * $Id: flashimage.cpp,v 1.3 2002/06/29 13:07:49 waldi Exp $
 */

#include <flashimage.hpp>

#include <sstream>
#include <stdexcept>

#include <libcrypto++/bio.hpp>
#include <libcrypto++/lib.hpp>
#include <libcrypto++/x509.hpp>

std::map < int, std::string > FlashImage::FlashImage::errors;

FlashImage::FlashImage::FlashImage ( FlashImageFS & fs )
: fs ( fs )
{
  Crypto::lib::init ();

  std::stringstream stream;
  fs.get_file ( "control", stream );
  control = parse_control ( stream );
  stream.clear ();

  if ( control["Format"] == "" )
    throw std::runtime_error ( std::string ( "Format: field empty" ) );
  if ( control["Format"] < "1.0" || control["Format"] > "1.0" )
    throw std::runtime_error ( std::string ( "Format: unknown: " ) + control["Format"] );

  if ( control["Date"] == "" )
    throw std::runtime_error ( std::string ( "Date: field empty" ) );

  if ( control["Version"] == "" )
    throw std::runtime_error ( std::string ( "Version: field empty" ) );

  if ( control["Status"] == "" )
    throw std::runtime_error ( std::string ( "Status: field empty" ) );
  if ( ! ( control["Status"] == "Unofficial" || control["Status"] == "Official" ) )
    throw std::runtime_error ( std::string ( "Status: unknown:" ) + control["Status"] );

  if ( control["Maintainer"] == "" )
    throw std::runtime_error ( std::string ( "Maintainer: field empty" ) );

  if ( control["Digest"] == "" )
    throw std::runtime_error ( std::string ( "Digest: field empty" ) );
  if ( ! ( control["Digest"] == "MD5" || control["Digest"] == "SHA1" || control["Digest"] == "RIPEMD160" ) )
    throw std::runtime_error ( std::string ( "Digest: unknown:" ) + control["Digest"] );
}

int FlashImage::FlashImage::verify_cert ( const std::string & certchain )
{
  std::map < int, std::string > temp;
  return verify_cert ( certchain, temp );
}

int FlashImage::FlashImage::verify_cert ( const std::string & certchain, std::map < int, std::string > & reterrors )
{
  std::stringstream stream;
  Crypto::x509::cert cert;

  try
  {
    fs.get_file ( "signcert", stream );
    cert.read ( stream );
  }

  catch ( std::runtime_error & )
  {
    throw std::runtime_error ( "verification failed: no certificate" );
  }

  Crypto::x509::store store;

  if ( certchain != "" )
    store.add_file ( certchain );

  errors.clear ();

  int ret = cert.verify ( store, &verify_cert_callback );

  if ( ! errors.size () )
    if ( ret <= 0 )
      return -1;
    else
      return 0;

  for ( std::map < int, std::string > ::iterator it = errors.begin (); it != errors.end (); ++it )
    reterrors.insert ( *it );

  if ( ret <= 0 )
    return -3;
  else
    return -2;
}

std::map < std::string, std::string > FlashImage::FlashImage::parse_control ( std::istream & stream )
{
  std::map < std::string, std::string > fields;
  std::string field;
  std::string data;

  while ( ! stream.fail () )
  {
    std::getline ( stream, field, ':' );

    if ( field == "" )
      break;

    std::getline ( stream, data, ' ' );
    std::getline ( stream, data );

    if ( data == "" )
      break;

    fields.insert ( std::pair < std::string, std::string > ( field, data ) );
  }

  return fields;
}

int FlashImage::FlashImage::verify_cert_callback ( int ok, libcrypto::X509_STORE_CTX * ctx ) throw ()
{
  if ( ! ok )
  {
    std::cerr << "error " << ctx -> error << " at " << ctx -> error_depth
      << " depth lookup: " << libcrypto::X509_verify_cert_error_string ( ctx -> error ) << std::endl;

    switch ( ctx -> error )
    {
      case X509_V_ERR_CERT_NOT_YET_VALID:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "not yet valid" ) );
        return 1;
      case X509_V_ERR_CERT_HAS_EXPIRED:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "cert expired" ) );
        return 1;
      case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "self signed cert" ) );
        return 1;
      case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "unable to get issuer cert" ) );
        return 1;
      case X509_V_ERR_CERT_CHAIN_TOO_LONG:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "cert chain too long" ) );
        return 1;
      case X509_V_ERR_CERT_REVOKED:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "cert revoked" ) );
        return 1;
      case X509_V_ERR_INVALID_CA:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "invalid CA" ) );
        return 1;
      case X509_V_ERR_INVALID_PURPOSE:
        errors.insert ( std::pair < int, std::string > ( ctx -> error, "invalid purpose" ) );
        return 1;
    }
  }

  return ok;
}

FlashImage::FlashImageSignature::FlashImageSignature ( FlashImageFS & fs, std::map < std::string, std::string > & control, Crypto::evp::digest & digest )
: fs ( fs ), control ( control ), digest ( digest ), block ( 0 ), buf ( NULL )
{
  if ( fs.get_size_block () <= 0 )
    throw std::length_error ( "verify: no blocks" );

  if ( control["Digest"] == "MD5" )
    digest.init ( Crypto::evp::md::md5 () );
  else if ( control["Digest"] == "SHA1" )
    digest.init ( Crypto::evp::md::sha1 () );
  else if ( control["Digest"] == "RIPEMD160" )
    digest.init ( Crypto::evp::md::ripemd160 () );
  else
    throw std::runtime_error ( std::string ( "Digest: unknown: " ) + control["Digest"] );

  buf = new char[4096];
}

FlashImage::FlashImageSignature::~FlashImageSignature ()
{
  delete buf;
}

int FlashImage::FlashImageSignature::update ()
{
  if ( block >= fs.get_size_block () - 1 )
    std::length_error ( "verify: behind end of image" );

  fs.get_block ( block, buf );
  block++;
  digest.update ( buf, 4096 );
  return fs.get_size_block () - block;
}

FlashImage::FlashImageSign::FlashImageSign ( FlashImageFS & fs, std::map < std::string, std::string > & control )
: FlashImageSignature ( fs, control, sign )
{ }

FlashImage::FlashImageVerify::FlashImageVerify ( FlashImageFS & fs, std::map < std::string, std::string > & control )
: FlashImageSignature ( fs, control, verify )
{ }

void FlashImage::FlashImageSign::final ( Crypto::evp::key::privatekey & key )
{
  while ( block < fs.get_size_block () )
  {
    fs.get_block ( block, buf );
    block++;
    digest.update ( buf, 4096 );
  }

  std::stringstream sigstream;
  sigstream.str ( sign.final ( key ) );
  fs.set_signature ( sigstream );
}

int FlashImage::FlashImageVerify::final ()
{
  while ( block < fs.get_size_block () )
  {
    fs.get_block ( block, buf );
    block++;
    digest.update ( buf, 4096 );
  }

  std::stringstream stream;
  Crypto::x509::cert cert;

  try
  {
    fs.get_file ( "signcert", stream );
    cert.read ( stream );
  }

  catch ( std::runtime_error & )
  {
    throw std::runtime_error ( "verification failed: no certificate" );
  }

  Crypto::evp::key::key key = cert.get_publickey ();

  stream.str ( "" );

  fs.get_signature ( stream );

  return verify.final ( stream.str (), key );
}

#ifndef INLINE
#include <flashimage.ipp>
#endif

