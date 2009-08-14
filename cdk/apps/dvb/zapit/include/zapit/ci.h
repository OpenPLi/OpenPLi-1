/*
 * $Id: ci.h,v 1.4 2002/08/27 21:12:36 thegoodguy Exp $
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
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

#ifndef __ci_h__
#define __ci_h__

#include <vector>

unsigned int write_length_field (unsigned char * buffer);

class CCaDescriptor
{
	private:
		unsigned char	descriptor_tag		: 8;
		unsigned char	descriptor_length	: 8;
		unsigned short	CA_system_ID		: 16;
		unsigned char	reserved1		: 3;
		unsigned short	CA_PID			: 13;
		std::vector <unsigned char> private_data_byte;

	public:
		CCaDescriptor (unsigned char * buffer);
		unsigned int    writeToBuffer (unsigned char * buffer);
		unsigned int    getLength()                             { return descriptor_length + 2; };
};

/*
 * children of this class need to delete all
 * CCaDescriptors in their destructors
 */
class CCaTable
{
	public:
		unsigned char	reserved2		: 4;
	private:
		unsigned short	info_length		: 12;
		std::vector <CCaDescriptor *> ca_descriptor;
	protected:
		CCaTable ()                                             { info_length = 0; };
		~CCaTable ();
		unsigned int    getLength()                             { return info_length + 2; };
		unsigned int    writeToBuffer (unsigned char * buffer);
	public:
		void addCaDescriptor (unsigned char * buffer);
};

class CEsInfo : public CCaTable
{
	public:
		unsigned char	stream_type		: 8;
		unsigned char	reserved1		: 3;
		unsigned short	elementary_PID		: 13;
	protected:
		unsigned int    getLength()                             { return CCaTable::getLength() + 3; };
		unsigned int    writeToBuffer (unsigned char * buffer);
	friend class CCaPmt;
};

class CCaPmt : public CCaTable
{
	public:
		~CCaPmt ();
		unsigned int    getLength();
		unsigned int    writeToBuffer (unsigned char * buffer);

		unsigned char	ca_pmt_list_management	: 8;
		unsigned short	program_number		: 16;
		unsigned char	reserved1		: 2;
		unsigned char	version_number		: 5;
		unsigned char	current_next_indicator	: 1;

		std::vector <CEsInfo *> es_info;
};

#endif /* __ci_h__ */
