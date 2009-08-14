/* 
TuxTerm v0.2

Written 10/2006 by Seddi
Contact: seddi@ihad.tv / http://www.ihad.tv
*/

#ifndef __RENDER_H__

#define __RENDER_H__

#include "main.h"

FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface);
int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, unsigned char *color);
int GetStringLen(const char *string, int size);
void RenderString(const char *string, int sx, int sy, int maxwidth, int layout, int size, unsigned char *color);
void RenderBox(int sx, int sy, int ex, int ey, int mode, unsigned char *color);
void RenderBoxBB(int sx, int sy, int ex, int ey, int mode, unsigned char *color);

#endif
