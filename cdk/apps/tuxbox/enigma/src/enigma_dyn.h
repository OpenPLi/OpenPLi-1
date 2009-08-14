/*
 * $Id: enigma_dyn.h,v 1.80 2007/10/04 10:32:27 digi_casi Exp $
 *
 * (C) 2005,2007 by digi_casi <digi_casi@tuxbox.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#ifndef __enigma_dyn_h
#define __enigma_dyn_h

#ifdef ENABLE_EXPERT_WEBIF
#define WEBIFVERSION "6.0.4-Expert"
#else
#define WEBIFVERSION "6.0.4"
#endif

#define ZAPMODETV 0
#define ZAPMODERADIO 1
#define ZAPMODEDATA 2
#define ZAPMODERECORDINGS 3
#define ZAPMODEROOT 4
#define ZAPMODESTREAMING 5

#define ZAPSUBMODENAME 0
#define ZAPSUBMODECATEGORY 1
#define ZAPSUBMODESATELLITES 2
#define ZAPSUBMODEPROVIDERS 3
#define ZAPSUBMODEBOUQUETS 4
#define ZAPSUBMODEALLSERVICES 5

class eHTTPDynPathResolver;

void ezapInitializeDyn(eHTTPDynPathResolver *dyn_resolver);
eString getDiskInfo(int html);

#endif /* __enigma_dyn_h */
