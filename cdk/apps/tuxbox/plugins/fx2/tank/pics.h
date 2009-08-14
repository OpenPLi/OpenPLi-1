#ifndef PICS_H
#define PICS_H

#define _	AIR
#define	D	DACH
#define	W	WHITE
#define	B	BLACK
#define	R	RED
#define	O	ORANGE
#define	Y	YELLOW

static unsigned char house_pic[] = {
_,_,_,_,_,_,_,D,D,_,_,_,_,_,_,_,
_,_,_,_,_,_,D,D,D,D,_,B,B,_,_,_,
_,_,_,_,_,D,D,D,D,D,D,B,B,_,_,_,
_,_,_,_,D,D,D,D,D,D,D,B,B,_,_,_,
_,_,_,D,D,D,D,D,D,D,D,D,B,_,_,_,
_,_,D,D,D,D,D,D,D,D,D,D,D,D,_,_,
_,D,D,D,D,D,D,D,D,D,D,D,D,D,D,_,
_,W,W,W,W,W,W,W,W,W,W,W,W,W,W,_,
_,W,B,B,B,W,W,W,W,W,W,B,B,B,W,_,
_,W,B,B,B,W,W,W,W,W,W,B,B,B,W,_,
_,W,B,B,B,W,W,B,B,B,W,B,B,B,W,_,
_,W,W,W,W,W,W,B,B,B,W,W,W,W,W,_,
_,W,W,W,W,W,W,B,B,B,W,W,W,W,W,_,
_,W,W,W,W,W,W,B,B,B,W,W,W,W,W,_,
_,W,W,W,W,W,W,B,B,B,W,W,W,W,W,_,
_,W,W,W,W,W,W,B,B,B,W,W,W,W,W,_ };

static unsigned char flame1_pic[] = {
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,Y,R,R,Y,_,_,_,_,_,
_,_,_,_,_,_,Y,Y,Y,Y,O,Y,_,_,_,_,
_,_,_,_,_,_,Y,Y,Y,O,O,Y,_,_,_,_,
_,_,_,O,O,Y,Y,R,Y,O,Y,Y,Y,_,_,_,
_,_,O,O,Y,Y,Y,R,R,Y,Y,Y,R,Y,_,_,
_,O,Y,Y,Y,R,Y,R,R,Y,R,R,R,Y,Y,_,
_,Y,Y,Y,O,R,R,R,Y,Y,R,R,Y,Y,Y,_,
_,R,Y,Y,O,O,Y,Y,Y,Y,Y,R,R,Y,Y,_,
_,R,Y,O,O,Y,Y,Y,Y,Y,Y,O,R,R,Y,_,
_,_,R,Y,Y,R,Y,Y,R,R,R,O,Y,_,_,_,
_,_,_,Y,Y,Y,R,R,R,Y,Y,Y,Y,O,_,_,
_,_,_,Y,Y,Y,R,R,Y,Y,Y,O,O,O,_,_,
_,_,_,Y,Y,Y,R,R,O,Y,Y,Y,O,_,_,_,
_,_,_,_,R,Y,Y,Y,Y,Y,Y,R,Y,_,_,_,
_,_,_,_,R,R,Y,Y,Y,Y,R,R,_,_,_,_ };

static unsigned char flame2_pic[] = {
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,R,Y,Y,Y,_,_,_,_,_,
_,_,_,_,Y,Y,R,R,O,Y,Y,Y,_,_,_,_,
_,_,_,_,Y,Y,Y,Y,O,Y,R,Y,_,_,_,_,
_,_,_,Y,Y,Y,O,Y,O,Y,R,R,R,_,_,_,
_,Y,Y,R,R,R,Y,Y,Y,Y,Y,Y,R,Y,_,_,
_,Y,R,R,Y,Y,Y,Y,R,Y,Y,Y,Y,Y,Y,_,
_,Y,R,Y,Y,Y,Y,R,R,R,O,Y,Y,Y,O,_,
_,Y,Y,Y,O,Y,Y,Y,Y,R,Y,Y,Y,O,O,_,
_,Y,Y,Y,O,O,R,Y,Y,Y,Y,Y,O,O,O,_,
_,_,Y,Y,O,O,R,R,Y,Y,Y,Y,O,_,_,_,
_,_,_,Y,Y,Y,Y,R,R,R,Y,O,O,Y,_,_,
_,_,_,Y,Y,Y,O,Y,R,R,Y,Y,Y,Y,_,_,
_,_,_,Y,R,Y,Y,O,O,Y,Y,Y,Y,_,_,_,
_,_,_,_,R,R,R,Y,Y,Y,Y,R,R,_,_,_,
_,_,_,_,Y,Y,R,R,Y,Y,Y,R,_,_,_,_ };

static unsigned char flame3_pic[] = {
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_,
_,_,_,_,_,_,O,O,O,O,_,_,_,_,_,_,
_,_,_,_,Y,O,O,Y,Y,O,O,_,_,_,_,_,
_,_,_,Y,Y,Y,Y,R,R,Y,Y,Y,_,_,_,_,
_,_,Y,Y,Y,Y,Y,Y,R,R,R,Y,Y,_,_,_,
_,Y,Y,R,R,R,Y,Y,Y,R,R,R,Y,Y,_,_,
_,Y,R,R,Y,Y,Y,Y,Y,Y,R,R,Y,Y,_,_,
_,Y,Y,Y,Y,Y,R,Y,Y,O,O,Y,O,O,_,_,
_,Y,Y,Y,Y,Y,R,R,R,Y,O,O,Y,O,Y,_,
_,_,Y,Y,Y,O,Y,R,R,Y,Y,O,Y,Y,_,_,
_,_,Y,O,R,O,O,Y,Y,Y,Y,R,Y,Y,_,_,
_,_,Y,O,R,R,O,Y,R,Y,Y,R,R,Y,_,_,
_,_,Y,O,O,R,R,Y,O,Y,R,R,R,Y,_,_,
_,_,Y,Y,O,O,Y,Y,Y,Y,R,R,Y,Y,_,_,
_,_,_,Y,Y,Y,Y,Y,Y,Y,Y,Y,Y,_,_,_ };

#undef	_
#undef	D
#undef	W
#undef	B
#undef	R
#undef	O
#undef	Y

#endif
