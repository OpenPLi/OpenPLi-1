/*
  $Id: libucodes.c,v 1.20 2002/09/18 07:44:07 obi Exp $
  
  $Log: libucodes.c,v $
  Revision 1.20  2002/09/18 07:44:07  obi
  some more ucodes

  Revision 1.19  2002/09/03 22:52:01  obi
  another avia600 ucode

  Revision 1.18  2002/09/03 18:05:11  obi
  two new ucodes

  Revision 1.17  2002/08/30 09:03:08  obi
  added some missing microcodes

  Revision 1.16  2002/08/21 09:18:58  obi
  no more compile warnings

  Revision 1.15  2002/05/16 11:53:23  derget
  erst lesen dann commiten :)

  Revision 1.14  2002/05/16 11:46:50  derget
  01.02.105D
  01.02.105E
  eingebaut

  Revision 1.13  2002/04/26 08:04:21  field
  bn 2.01

  Revision 1.12  2001/10/22 01:57:08  derget
  01_02_002F hinzugefügt

  Revision 1.11  2001/10/22 01:42:54  fnbrd
  Small comment for support ;)

  Revision 1.10  2001/10/09 20:39:26  fnbrd
  Added Id and Log

  Alle vorherigen Versionen sind von McClean, d.h. er ist der Ansprechpartner ;)

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libmd5sum/libmd5sum.h"


struct Sucode
{
	char md5[35];
	char name[20];
};

static struct Sucode ucodes[] = 
{
	{"965d834c7bb743df6841f4fda5b4e790","avia500v083"},
	{"108fa2fa0ee84451ea7ea2db9adaa44b","avia500v090"},
	{"fece1d3324e0917b921d814490d8a824","avia500v093"},
	{"7373f3934263b3c3ea1d0f500f0044a5","avia500v110"},
	{"6fd45384d705289a2fc8f212b2855f5c","avia600vb016"},
	{"da492146ba7e177883feadaa0cf89aa5","avia600vb017"},
	{"c31dc570cf941afb6fc4813f561aa378","avia600vb018"},
	{"6a748fb28000738cafeb9e27443ac623","avia600vb022"},
	{"f8577c6a7056590ca5847520d81339fc","avia600vb028"},
	{"53c5bc4081dfadab9935ad256e4d6239","ucode_0000"},
	{"936b13b9018b2541036a7f20e76648ff","ucode_0001"},
	{"0f610f3e0f1f2b56fc2bb4007700836b","ucode_000C"},
	{"f0418c350547e98f2c5d7f4df00f9261","ucode_0010"},
	{"66627c5ddf269a1f3a9f9f3c22fbd41b","ucode_0013"},
	{"6582a89e7e13e410c366e47b4ef9d38e","ucode_0014"},
	{"e8fb834466d34cb75bb0e64ef08b5544","ucode_001A"},
	{"36a05f39caa0fb375df85c99e66e406d","ucode_B102"},
	{"d4c12df0d4ce8ba9eb858509d832df65","ucode_B107"},
	{"4e080873126f156a0b489af176520620","ucode_B121"},
	{"ff6fafbd2aa1f29afe232a72fac57870","cam_01_01_001D"},
	{"c42d6753794dd95146eac31f2a65b516","cam_01_01_004D"},
	{"be4b0f38557c416ce04e7fa3fa634f95","cam_01_01_005D"},
	{"1905390636e70c966574a3298a1b89c3","cam_01_02_002D"},
	{"f27eda698c202d17af7b9bab83973e8c","cam_01_02_105D"},
	{"7b9b72786623e3750335c79af044e718","cam_01_01_001E"},
	{"be7f1beb1bb437b7f7c99f8e5a968882","cam_01_01_002E"},
	{"a8689d88e0d2df12a15732c98f865b11","cam_01_01_003E"},
	{"4336e7d3fed43c9e063210bcdc95d23e","cam_01_01_004E"},
	{"997b1f858f1efee525e6842558edbe3c","cam_01_01_005E"},
	{"7f56e693a916b39a6e2734dc9b5aab7a","cam_01_02_002E"},
	{"bac1970b0e865c00015b3d78a209b5bf","cam_01_02_105E"},
	{"c734207ddea7b8ceafa2505f1360f3bf","cam_01_01_004F"},
	{"a5984825ff554ea530efc4733ffd7473","cam_01_01_005F"},
	{"704cb8d9965babbdc7d4e7cae6e5584e","cam_01_02_002F"},
	{"c51074b2edf6c14e0bb99ee3ed8b9c47","cam_01_02_105F"},
	{"16c5e1eba0cfe63f2bbc648e1644c883","cam_NOKIA_PRODTEST2"},
	{"ff441ce2f3feab815d360e3032d23888","cam_ProdTestImage_2"},
	{"420baf447bfd529a794bb36ef80f1652","cam_STREAMHACK"}
};


int checkFile(char* filename, char* result)
{
	int		count;
	int		anz;
	unsigned char	md5buffer[16];
	char		md5string[40]="";

	//get the file-md5sum and convert to string..
	if( md5_file(filename, 1, (unsigned char*) &md5buffer))
 	{
		strcpy(result, "not found");
		return -1;
        }

	for(count=0;count<16;count++)
	{
		char tmp[6];
		sprintf((char*) &tmp, "%02x", md5buffer[count] );
		strcat(md5string, tmp);
	}


	anz = sizeof(ucodes) / sizeof(*ucodes); 

	for(count=0;count<anz;count++)
	{
		if( strcmp(ucodes[count].md5, md5string)==0)
		{
			strcpy(result, ucodes[count].name);
			return 1;
		}
	}

	strcpy(result, "unknown");
	return 0;
}
