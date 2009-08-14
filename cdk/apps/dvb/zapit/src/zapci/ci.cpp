/*
 * $Id: ci.cpp,v 1.9 2002/10/12 20:19:44 obi Exp $
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

#include <zapit/ci.h>

unsigned int write_length_field (unsigned char * buffer, unsigned int length)
{
        if (length < 128)
        {
                buffer[0] = length;
                return 1;
        }
        else
        {
                unsigned int pos = 0;
                unsigned int shiftby = 8;
                unsigned char length_field_size = 1;

                while ((length >> shiftby) != 0)
                {
                        length_field_size++;
                        shiftby += 8;
                }

                buffer[pos++] = ((1 << 7) | length_field_size);

                while (shiftby != 0)
                {
			shiftby -= 8;
                        buffer[pos++] = length >> shiftby;
                }
                return pos;
        }

}

/*
 * conditional access descriptors
 */
CCaDescriptor::CCaDescriptor (unsigned char * buffer)
{
	descriptor_tag = buffer[0];
	descriptor_length = buffer[1];
	CA_system_ID = (buffer[2] << 8) | buffer[3];
	reserved1 = buffer[4] >> 5;
	CA_PID = ((buffer[4] & 0x1F) << 8) | buffer[5];

	private_data_byte = std::vector<unsigned char>(&(buffer[6]), &(buffer[descriptor_length + 2]));
}

unsigned int CCaDescriptor::writeToBuffer (unsigned char * buffer) // returns number of bytes written
{
	buffer[0] = descriptor_tag;
	buffer[1] = descriptor_length;
	buffer[2] = CA_system_ID >> 8;
	buffer[3] = CA_system_ID;
	buffer[4] = (reserved1 << 5) | (CA_PID >> 8);
	buffer[5] = CA_PID;

	std::copy(private_data_byte.begin(), private_data_byte.end(), &(buffer[6]));

	return descriptor_length + 2;
}


/*
 * generic table containing conditional access descriptors
 */
void CCaTable::addCaDescriptor (unsigned char * buffer)
{
	CCaDescriptor* dummy = new CCaDescriptor(buffer);
	ca_descriptor.push_back(dummy);
	if (info_length == 0)
	    info_length = 1;
	info_length += dummy->getLength();
}

unsigned int CCaTable::writeToBuffer (unsigned char * buffer) // returns number of bytes written
{
	buffer[0] = (reserved2 << 4) | (info_length >> 8);
	buffer[1] = info_length;

	if (info_length == 0)
		return 2;

	buffer[2] = 1;                  // ca_pmt_cmd_id: ok_descrambling= 1;
	unsigned int pos = 3;

	for (unsigned int i = 0; i < ca_descriptor.size(); i++)
		pos += ca_descriptor[i]->writeToBuffer(&(buffer[pos]));
	return pos;
}

CCaTable::~CCaTable ()
{
	for (unsigned int i = 0; i < ca_descriptor.size(); i++)
		delete ca_descriptor[i];
}


/*
 * elementary stream information
 */
unsigned int CEsInfo::writeToBuffer (unsigned char * buffer) // returns number of bytes written
{
	buffer[0] = stream_type;
	buffer[1] = (reserved1 << 5) | (elementary_PID >> 8);
	buffer[2] = elementary_PID;
	return 3 + CCaTable::writeToBuffer(&(buffer[3]));
}


/*
 * contitional access program map table
 */
CCaPmt::~CCaPmt ()
{
	for (unsigned int i = 0; i < es_info.size(); i++)
		delete es_info[i];
}

unsigned int CCaPmt::writeToBuffer (unsigned char * buffer) // returns number of bytes written
{
	unsigned int pos = 0;
	unsigned int i;

	buffer[pos++] = 0x9F;    // ca_pmt_tag
	buffer[pos++] = 0x80;    // ca_pmt_tag
	buffer[pos++] = 0x32;    // ca_pmt_tag

	pos += write_length_field(&(buffer[pos]), getLength());

	buffer[pos++] = ca_pmt_list_management;
	buffer[pos++] = program_number >> 8;
	buffer[pos++] = program_number;
	buffer[pos++] = (reserved1 << 6) | (version_number << 1) | current_next_indicator;

	pos += CCaTable::writeToBuffer(&(buffer[pos]));

	for (i = 0; i < es_info.size(); i++)
		pos += es_info[i]->writeToBuffer(&(buffer[pos]));

	return pos;
}

unsigned int CCaPmt::getLength ()  // the (3 + length_field()) initial bytes are not counted !
{
	unsigned int size = 4 + CCaTable::getLength();

	for (unsigned int i = 0; i < es_info.size(); i++)
		size += es_info[i]->getLength();

	return size;
}

