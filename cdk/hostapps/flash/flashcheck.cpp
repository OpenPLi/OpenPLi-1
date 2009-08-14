/*
 * flashcheck.cpp
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
 * $Id: flashcheck.cpp,v 1.2 2002/06/29 13:10:54 waldi Exp $
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include <getopt.h>

#include <libflashimage/flashimage.hpp>
#include <libflashimage/flashimagecramfs.hpp>

#define PROGRAM_NAME "flashcheck"

#define AUTHORS "Bastian Blank"

const char * program_name;

static struct option long_options [] =
{
  { "certchain", required_argument, 0, 'c' },
  { "help", no_argument, 0, 250 },
  { "version", no_argument, 0, 251 },
  { 0, 0, 0, 0 }
};

void usage ( int status )
{
  if ( status != 0 )
    std::cerr << "Try `" << program_name << " --help' for more information.\n";
  else
    std::cout
      << "Usage: " << program_name << " [OPTION]... FILE\n\n"
      << "Check a signed flash image\n\n"
      << "  -c, --certchain=FILE        certchain for checks\n"
      << "      --help                  display this help and exit\n"
      << "      --version               output version information and exit\n";

  exit ( status );
}

int parse_options ( int argc, char ** argv, std::map < std::string, std::string > & options )
{
  unsigned char c;
  int option_index;

  while ( 1 )
  {
    option_index = 0;

    c = getopt_long (argc, argv, "", long_options, &option_index);

    if ( c == 255 )
      break;
    switch ( c )
    {
      case 'c':
        options.insert ( std::pair < std::string, std::string > ( "certchain", optarg ) );
        break;
      case 250:
        usage ( EXIT_SUCCESS );
        break;
      case 251:
        std::cout 
          << PROGRAM_NAME " (" PACKAGE ") " VERSION "\n"
          << "Written by " AUTHORS ".\n\n"
          << "This is free software; see the source for copying conditions.  There is NO\n"
          << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n";
        exit ( EXIT_SUCCESS );
        break;
      default:
        usage (1);
    }
  }

  return optind;
}

int main ( int argc, char ** argv )
{
  program_name = argv[0];

  std::map < std::string, std::string > options;
  int optind = parse_options ( argc, argv, options );

  if ( argc == optind )
  {
    std::cerr << program_name << ": need a image file!\n";
    usage ( 1 );
  }

  if ( options["certchain"] == "" )
    std::cerr << program_name << ": can't really check image without a certchain.\n";

  std::fstream image_in ( argv[optind], std::ios::in );

  try
  {
    image_in.seekg ( 0, std::ios::end );
    int size = image_in.tellg ();

    if ( size % 4096 )
      throw std::runtime_error ( "image not alligned" );

    FlashImage::FlashImageCramFS fs ( image_in );
    FlashImage::FlashImage image ( fs );

    std::cout << "verify image:" << std::endl;
    fs.get_file ( "control", std::cout );
    std::cout << std::endl;

    {
      std::map < int, std::string > errors;
      switch ( image.verify_cert ( options["certchain"], errors ) )
      {
        case 0:
          std::cout << "cert verification successfull" << std::endl;
          break;
        case -1:
          std::cout << "cert verification failed" << std::endl;
          break;
        case -2:
          std::cout << "cert verification probably failed:" << std::endl;
          for ( std::map < int, std::string > ::iterator it = errors.begin (); it != errors.end (); ++it )
            std::cout << it -> second << std::endl;
          break;
        case -3:
          std::cout << "cert verification failed:" << std::endl;
          for ( std::map < int, std::string > ::iterator it = errors.begin (); it != errors.end (); ++it )
            std::cout << it -> second << std::endl;
          break;
      }
    }

    if ( image.verify_image ().final () )
      std::cout << "image verification successfull" << std::endl;
    else
      std::cout << "image verification failed" << std::endl;
  }

  catch ( std::runtime_error & except )
  {
    std::cout << "exception: " << except.what () << std::endl;
  }

  image_in.close ();

  return 0;
}

