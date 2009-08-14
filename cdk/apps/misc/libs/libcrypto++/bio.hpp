/*
 * bio.hpp: c++ wrapper for openssl bio
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
 * $Id: bio.hpp,v 1.3 2002/03/12 19:37:03 waldi Exp $
 */

#ifndef __LIBCRYPTO__BIO_HPP
#define __LIBCRYPTO__BIO_HPP

#include <iostream>
#include <string>

namespace libcrypto
{
  #include <openssl/bio.h>
}

namespace Crypto
{
  namespace bio
  {
    class bio
    {
      public:
        bio ();
        virtual ~bio ();

        virtual int read ( void *, int );
        virtual int write ( const void *, int );
        virtual int gets ( char *, int );
        virtual int puts ( const char * );
        virtual void flush ();

        operator libcrypto::BIO * ();

        libcrypto::BIO * _bio;
    };

    class stream : public bio
    {
      protected:
        static long bio_ctrl ( libcrypto::BIO *, int, long, void *  );
        static int bio_new ( libcrypto::BIO * );
        static int bio_free ( libcrypto::BIO * );
    };

    class istream : virtual public stream
    {
      public:
        istream ( std::istream & );

      protected:
        std::istream & is;

        static libcrypto::BIO_METHOD bio_method;

        static int bio_read ( libcrypto::BIO *, char *, int );
        static int bio_gets ( libcrypto::BIO *, char *, int );
    };

    class ostream : virtual public stream
    {
      public:
        ostream ( std::ostream & );

      protected:
        std::ostream & os;

        static libcrypto::BIO_METHOD bio_method;

        static int bio_write ( libcrypto::BIO *, const char *, int );
        static int bio_puts ( libcrypto::BIO *, const char * );
    };

    class iostream : public istream, public ostream
    {
      public:
        iostream ( std::iostream & );

      protected:
        static libcrypto::BIO_METHOD bio_method;
    };

    class filter : public bio
    {
      public:
        void push ( bio & );
        void pop ();

      protected:
        filter ();
        ~filter ();
        bool used;
    };

    class base64 : public filter
    {
      public:
        base64 ();
        base64 ( bio & );
    };

    class base64NoNewline : public base64
    {
      public:
        base64NoNewline ();
        base64NoNewline ( bio & );
    };
  };
};

std::istream & operator >> ( std::istream &, Crypto::bio::bio & );
std::ostream & operator << ( std::ostream &, Crypto::bio::bio & );

Crypto::bio::bio & operator >> ( Crypto::bio::bio &, std::string & );
Crypto::bio::bio & operator << ( Crypto::bio::bio &, const std::string & );

#endif
