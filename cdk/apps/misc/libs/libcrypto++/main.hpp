/*
 * main.hpp
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
 * $Id: main.hpp,v 1.1 2002/03/02 21:53:26 waldi Exp $
 */

#ifndef __LIBCRYPTO__MAIN_HPP
#define __LIBCRYPTO__MAIN_HPP 1

//#define TRACE

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef TRACE
#include <iostream>
#define TRACE_FUNCTION std::cerr << __PRETTY_FUNCTION__ << std::endl
#else
#define TRACE_FUNCTION /* nix */
#endif

#endif
