/*
 * evp.hpp
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
 * $Id: evp.hpp,v 1.3 2002/05/30 11:43:18 waldi Exp $
 */

#ifndef __LIBCRYPTO__EVP_HPP
#define __LIBCRYPTO__EVP_HPP 1

#include <string>

#include <libcrypto++/rsa.hpp>

namespace libcrypto
{
  #include <openssl/evp.h>
}

namespace Crypto
{
  namespace evp
  {
    namespace cipher
    {
      class cipher
      {
        public:
          operator libcrypto::EVP_CIPHER * () const { return get (); }
          virtual libcrypto::EVP_CIPHER * get () const = 0;
      };

      class des : public cipher
      {
        public:
          libcrypto::EVP_CIPHER * get () const { return libcrypto::EVP_des_cbc (); }
      };

      class des3 : public cipher
      {
        public:
          libcrypto::EVP_CIPHER * get () const { return libcrypto::EVP_des_ede3_cbc (); }
      };

      class blowfish : public cipher
      {
        public:
          libcrypto::EVP_CIPHER * get () const { return libcrypto::EVP_bf_cbc (); }
      };
    };

    namespace md
    {
      class md
      {
        public:
          operator libcrypto::EVP_MD * () const { return get (); }
          virtual libcrypto::EVP_MD * get () const = 0;
      };

      class md5 : public md
      {
        public:
          libcrypto::EVP_MD * get () const { return libcrypto::EVP_md5 (); }
      };

      class sha1 : public md
      {
        public:
          libcrypto::EVP_MD * get () const { return libcrypto::EVP_sha1 (); }
      };

      class ripemd160 : public md
      {
        public:
          libcrypto::EVP_MD * get () const { return libcrypto::EVP_ripemd160 (); }
      };
    };

    namespace key
    {
      class key
      {
        protected:
          struct item
          {
            item ( libcrypto::EVP_PKEY * key );
            ~item ();

            int count;
            libcrypto::EVP_PKEY * key;
          };

        public:
          key ();
          key ( libcrypto::EVP_PKEY * );
          key ( const key & );
          virtual ~key ();
          key & operator = ( const key & );

          void set ( Crypto::rsa & );
          Crypto::rsa get ();

          operator libcrypto::EVP_PKEY * ();

        protected:
          item * _item;
      };

      class privatekey : public key
      {
        public:
          virtual void read ( std::istream & );
          virtual void read ( std::istream &, const std::string & );
          void write ( std::ostream & );
          void write ( std::ostream &, Crypto::evp::cipher::cipher &, const std::string & );

        protected:
          static int callback ( char * buf, int num, int w, void * key );
          std::string passphrase;
      };
    };

    class decrypt
    {
      public:
        void init ( const cipher::cipher &, const std::string & key ) throw ();
        void update ( std::istream &, std::ostream & );
        void final ( std::ostream & );

      protected:
        libcrypto::EVP_CIPHER_CTX ctx;
    };

    class encrypt
    {
      public:
        void init ( const cipher::cipher &, const std::string & key ) throw ();
        void update ( std::istream &, std::ostream & );
        void final ( std::ostream & );

      protected:
        libcrypto::EVP_CIPHER_CTX ctx;
    };

    class open : public decrypt
    {
      public:
        void final ( std::ostream & );
    };

    class seal : public encrypt
    {
      public:
        void final ( std::ostream & );
    };

    class digest
    {
      public:
        void init ( const md::md & ) throw ();
        void update ( const void *, unsigned int ) throw ();
        void update ( std::istream & );
        std::string final ();

      protected:
        libcrypto::EVP_MD_CTX ctx;
    };

    class sign : public digest
    {
      public:
        std::string process ( const md::md &, std::istream &, Crypto::evp::key::privatekey & );
        std::string final ( Crypto::evp::key::privatekey & );
    };

    class verify : public digest
    {
      public:
        int process ( const md::md &, std::istream &, const std::string & sig, Crypto::evp::key::privatekey & );
        int final ( const std::string & sig, Crypto::evp::key::key & );
    };
  };
};

Crypto::evp::digest & operator >> ( std::string &, Crypto::evp::digest & );

#endif
