/*
 * flashimage.ipp
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
 * $Id: flashimage.ipp,v 1.2 2002/06/29 13:07:49 waldi Exp $
 */

#ifndef INLINE
#define inline /* nix */
#endif

inline FlashImage::FlashImageSign FlashImage::FlashImage::sign_image ()
{
  return FlashImageSign ( fs, control );
}

inline FlashImage::FlashImageVerify FlashImage::FlashImage::verify_image ()
{
  return FlashImageVerify ( fs, control );
}

inline std::string FlashImage::FlashImage::get_control_field ( std::string field )
{
  return control[field];
}

#undef inline
