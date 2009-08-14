/*
 * x509.hpp: c++ wrapper for openssl x509
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
 * $Id: x509.hpp,v 1.4 2002/06/29 13:04:05 waldi Exp $
 */

#ifndef __LIBCRYPTO__X509_HPP
#define __LIBCRYPTO__X509_HPP

#define INLINE

#include <iostream>

#include <libcrypto++/evp.hpp>
#include <libcrypto++/exception.hpp>

namespace libcrypto
{
  #include <openssl/x509.h>
  #include <openssl/x509v3.h>
  #include <openssl/x509_vfy.h>
}

namespace Crypto
{
  namespace x509
  {
    class extension;
    class name;
    class req;
    class store;

    class cert
    {
      public:
        cert () throw ( std::bad_alloc );
        cert ( std::istream & );
        cert ( libcrypto::X509 * ) throw ();
        cert ( const cert & ) throw ( std::bad_alloc );
        ~cert () throw ();
        cert & operator = ( const cert & );

        void read ( std::istream & );
        void write ( std::ostream & ) const;
        void print ( std::ostream & ) const;

        long get_version () const throw ();
        void set_version ( long ) throw ();

        long get_serialNumber () const throw ();
        void set_serialNumber ( long ) throw ();

        name get_issuer_name () const throw ();
        void set_issuer_name ( name & ) throw ();
        name get_subject_name () const throw ();
        void set_subject_name ( name & ) throw ();

        std::string get_notBefore () const throw ();
        void set_notBefore ( const std::string & ) throw ();
        void set_notBefore ( const long ) throw ();
        std::string get_notAfter () const throw ();
        void set_notAfter ( const std::string & ) throw ();
        void set_notAfter ( const long ) throw ();

        Crypto::evp::key::key get_publickey () const throw ( std::bad_alloc );
        void set_publickey ( Crypto::evp::key::key & ) throw ();

        void add_extension ( extension & ) throw ();

        void sign ( Crypto::evp::key::privatekey &, Crypto::evp::md::md & ) throw ();
        int verify ( store &, int (*) ( int, libcrypto::X509_STORE_CTX * ) = NULL ) throw ( std::bad_alloc, Crypto::exception::undefined_libcrypto_error );

      protected:
        operator libcrypto::X509 * () throw ();

        libcrypto::X509 * _cert;

        friend class ctx;
        friend class store;
    };

    class crl
    {
      public:
        crl () throw ();
        ~crl () throw ();

        void sign ( Crypto::evp::key::privatekey & ) throw ();
        bool verify ( Crypto::evp::key::key & ) throw ();

      protected:
        operator libcrypto::X509_CRL * () throw ();

        libcrypto::X509_CRL * _crl;

        friend class ctx;
    };

    class ctx
    {
      public:
        ctx ( cert & ) throw ();
        ctx ( cert &, cert & ) throw ();
        ctx ( cert &, cert &, req & ) throw ();
        ctx ( req & ) throw ();
        ctx ( crl & ) throw ();
        ctx ( cert &, crl & ) throw ();

      protected:
        operator libcrypto::X509V3_CTX * () throw ();

        libcrypto::X509V3_CTX _ctx;

        friend class extension;
    };

    class extension
    {
      public:
        extension ( ctx &, const std::string &, const std::string & ) throw ( Crypto::exception::undefined_libcrypto_error );
        extension ( const std::string &, const std::string & ) throw ( Crypto::exception::undefined_libcrypto_error );
        extension ( ctx &, int, const std::string & ) throw ( Crypto::exception::undefined_libcrypto_error );
        extension ( int, const std::string & ) throw ( Crypto::exception::undefined_libcrypto_error );
        ~extension () throw ();

      protected:
        operator libcrypto::X509_EXTENSION * () throw ();

        libcrypto::X509_EXTENSION * _extension;

        friend class cert;
        friend class crl;
        friend class req;
    };

    class name
    {
      public:
        name () throw ( std::bad_alloc );
        name ( libcrypto::X509_NAME * ) throw ();
        name ( const name & ) throw ( std::bad_alloc );
        ~name () throw ();
        name & operator = ( const name & ) throw ( std::bad_alloc );

        void print ( std::ostream & ) const throw ();

        void add ( const std::string &, const std::string & ) throw ();
        void add ( int, const std::string & ) throw ();
        std::string get ( const std::string & ) throw ();
        std::string get ( int ) throw ();

      protected:
        operator libcrypto::X509_NAME * () throw ();
        int text2nid ( const std::string & ) throw ();

        libcrypto::X509_NAME * _name;

        friend class cert;
    };

    class req
    {
      public:
        req () throw ();
        ~req () throw ();

        void sign ( Crypto::evp::key::privatekey & ) throw ();
        bool verify ( Crypto::evp::key::key & ) throw ();

      protected:
        operator libcrypto::X509_REQ * () throw ();

        libcrypto::X509_REQ * _req;

        friend class ctx;
    };

    class revoked
    {
      public:
        revoked () throw ();
        ~revoked () throw ();

      protected:
        operator libcrypto::X509_REVOKED * () throw ();

        libcrypto::X509_REVOKED * _revoked;
    };

    class store
    {
      public:
        store () throw ( std::bad_alloc );
        store ( libcrypto::X509_STORE * ) throw ();
        ~store () throw ();

        void add ( cert & ) throw ();
        void add ( crl & ) throw ();
        void add_file ( const std::string & );

      protected:
        operator libcrypto::X509_STORE * () throw ();

        libcrypto::X509_STORE * _store;

      private:
        store ( const store & );

        friend class cert;
    };
  };
};

#ifdef INLINE
#include <libcrypto++/x509.ipp>
#endif

#endif
