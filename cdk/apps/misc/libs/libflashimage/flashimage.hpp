/*
 * flashimage.hpp
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
 * $Id: flashimage.hpp,v 1.4 2002/06/29 14:31:50 waldi Exp $
 */

#ifndef __LIBFLASHIMAGE__LIBFLASHIMAGE_HPP
#define __LIBFLASHIMAGE__LIBFLASHIMAGE_HPP

#define INLINE

#include <iostream>
#include <map>
#include <string>

#include <libcrypto++/evp.hpp>
#include <libcrypto++/x509.hpp>
#include <libflashimage/flashimagefs.hpp>

namespace FlashImage
{
  class FlashImageSign;
  class FlashImageVerify;

  class NoExceptionFlashImage;

  class FlashImage
  {
    public:
      FlashImage ( FlashImageFS & );

      std::string get_control_field ( std::string );

      int verify_cert ( const std::string & certchain );
      int verify_cert ( const std::string & certchain, std::map < int, std::string > & );
      FlashImageSign sign_image ();
      FlashImageVerify verify_image ();

    protected:
      static int verify_cert_callback ( int ok, libcrypto::X509_STORE_CTX * ctx ) throw ();
      std::map < std::string, std::string > parse_control ( std::istream & stream );

      FlashImageFS & fs;
      std::map < std::string, std::string > control;
      static std::map < int, std::string > errors;

      friend class NoExceptionFlashImage;
  };

  class FlashImageSignature
  {
    public:
      ~FlashImageSignature ();
      int update ();

    protected:
      FlashImageSignature ( FlashImageFS &, std::map < std::string, std::string > &, Crypto::evp::digest & );

      FlashImageFS & fs;
      std::map < std::string, std::string > & control;
      Crypto::evp::digest & digest;
      int block;
      char * buf;
  };

  class FlashImageSign : public FlashImageSignature
  {
    public:
      FlashImageSign ( FlashImageFS &, std::map < std::string, std::string > & );
      void final ( Crypto::evp::key::privatekey & );

    protected:
      Crypto::evp::sign sign;
  };

  class FlashImageVerify : public FlashImageSignature
  {
    public:
      FlashImageVerify ( FlashImageFS &, std::map < std::string, std::string > & );
      int final ();

    protected:
      Crypto::evp::verify verify;
  };
}

#ifdef INLINE
#include <libflashimage/flashimage.ipp>
#endif

#endif
