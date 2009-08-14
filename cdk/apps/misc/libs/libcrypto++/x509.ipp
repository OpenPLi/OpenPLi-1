/*
 * x509.ipp: c++ wrapper for openssl x509
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
 * $Id: x509.ipp,v 1.1 2002/06/29 13:04:29 waldi Exp $
 */

#ifndef INLINE
#define inline /* nix */
#endif

inline Crypto::x509::cert::cert ( libcrypto::X509 * _cert ) throw ()
: _cert ( _cert )
{ }

inline Crypto::x509::cert::~cert () throw ()
{
  libcrypto::X509_free ( _cert );
}

inline long Crypto::x509::cert::get_version () const throw ()
{
  return libcrypto::X509_get_version ( _cert );
}

inline void Crypto::x509::cert::set_version ( long version ) throw ()
{
  libcrypto::X509_set_version ( _cert, version );
}

inline long Crypto::x509::cert::get_serialNumber () const throw ()
{
  return libcrypto::ASN1_INTEGER_get ( libcrypto::X509_get_serialNumber ( _cert ) );
}

inline void Crypto::x509::cert::set_serialNumber ( long serial ) throw ()
{
  libcrypto::ASN1_INTEGER_set ( libcrypto::X509_get_serialNumber ( _cert ), serial );
}

inline Crypto::x509::name Crypto::x509::cert::get_issuer_name () const throw ()
{
  return name ( libcrypto::X509_NAME_dup ( libcrypto::X509_get_issuer_name ( _cert ) ) );
}

inline void Crypto::x509::cert::set_issuer_name ( Crypto::x509::name & name ) throw ()
{
  X509_set_issuer_name ( _cert, name );
}

inline Crypto::x509::name Crypto::x509::cert::get_subject_name () const throw ()
{
  return name ( X509_NAME_dup ( X509_get_subject_name ( _cert ) ) );
}

inline void Crypto::x509::cert::set_subject_name ( Crypto::x509::name & name ) throw ()
{
  X509_set_subject_name ( _cert, name );
}

inline void Crypto::x509::cert::set_notBefore ( const long time ) throw ()
{
  libcrypto::X509_gmtime_adj ( _cert -> cert_info -> validity -> notBefore, time );
}

inline void Crypto::x509::cert::set_notAfter ( const long time ) throw ()
{
  libcrypto::X509_gmtime_adj ( _cert -> cert_info -> validity -> notAfter, time );
}

inline Crypto::evp::key::key Crypto::x509::cert::get_publickey () const throw ( std::bad_alloc )
{
  return Crypto::evp::key::key ( libcrypto::X509_get_pubkey ( _cert ) );
}

inline void Crypto::x509::cert::set_publickey ( Crypto::evp::key::key & key ) throw ()
{
  libcrypto::X509_set_pubkey ( _cert, key );
}

inline void Crypto::x509::cert::add_extension ( extension & extension ) throw ()
{
  libcrypto::X509_add_ext ( _cert, extension, 0 );
}

inline void Crypto::x509::cert::sign ( Crypto::evp::key::privatekey & key, Crypto::evp::md::md & md ) throw ()
{
  libcrypto::X509_sign ( _cert, key, md );
}

inline Crypto::x509::cert::operator libcrypto::X509 * () throw ()
{
  return _cert;
}

inline Crypto::x509::crl::operator libcrypto::X509_CRL * () throw ()
{
  return _crl;
}
  
inline Crypto::x509::ctx::ctx ( cert & issuer ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, issuer, NULL, NULL, NULL, 0 );
}

inline Crypto::x509::ctx::ctx ( cert & issuer, cert & subject ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, issuer, subject, NULL, NULL, 0 );
}

inline Crypto::x509::ctx::ctx ( cert & issuer, cert & subject, req & req ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, issuer, subject, req, NULL, 0 );
}

inline Crypto::x509::ctx::ctx ( req & req ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, NULL, NULL, req, NULL, 0 );
}

inline Crypto::x509::ctx::ctx ( crl & crl ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, NULL, NULL, NULL, crl, 0 );
}

inline Crypto::x509::ctx::ctx ( cert & issuer, crl & crl ) throw ()
{
  libcrypto::X509V3_set_ctx ( &_ctx, issuer, NULL, NULL, crl, 0 );
}

inline Crypto::x509::ctx::operator libcrypto::X509V3_CTX * () throw ()
{
  return &_ctx;
}

inline Crypto::x509::extension::~extension () throw ()
{
  libcrypto::X509_EXTENSION_free ( _extension );
}

inline Crypto::x509::extension::operator libcrypto::X509_EXTENSION * () throw ()
{
  return _extension;
}

inline Crypto::x509::name::name ( libcrypto::X509_NAME * _name ) throw ()
: _name ( _name )
{ }

inline Crypto::x509::name::~name () throw ()
{
  libcrypto::X509_NAME_free ( _name );
}

inline void Crypto::x509::name::add ( const std::string & text, const std::string & entry ) throw ()
{
  add ( text2nid ( text ), entry );
}

inline void Crypto::x509::name::add ( int nid, const std::string & entry ) throw ()
{
  libcrypto::X509_NAME_add_entry_by_NID ( _name, nid, MBSTRING_ASC, ( unsigned char * ) entry.c_str (), -1, -1, 0 );
}

inline std::string Crypto::x509::name::get ( const std::string & text ) throw ()
{
  return get ( text2nid ( text ) );
}

inline Crypto::x509::name::operator libcrypto::X509_NAME * () throw ()
{
  return _name;
}

inline Crypto::x509::req::operator libcrypto::X509_REQ * () throw ()
{
  return _req;
}
  
inline Crypto::x509::store::store ( libcrypto::X509_STORE * _store ) throw ()
: _store ( _store )
{ }

inline Crypto::x509::store::~store () throw ()
{
  libcrypto::X509_STORE_free ( _store );
}

inline void Crypto::x509::store::add ( cert & cert ) throw ()
{
  libcrypto::X509_STORE_add_cert ( _store, cert );
}

inline Crypto::x509::store::operator libcrypto::X509_STORE * () throw ()
{
  return _store;
}

#undef inline
