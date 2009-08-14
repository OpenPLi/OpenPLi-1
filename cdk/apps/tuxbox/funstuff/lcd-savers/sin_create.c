#include <stdio.h>
#include <math.h>

#define SIN_SIZE 1024
#define SIN_MUL 65536

int main() {
	int x, sin_x;
	
	printf("/* sinus.h autocreated by sin_create (C) 2001 by Ge0rG */\n\n");

	printf("#define SIN_SIZE %i\n", SIN_SIZE);
	printf("#define SIN_MUL %i\n", SIN_MUL);

	printf("static int Sin[SIN_SIZE] = {\n	");
	for (x=0; x<SIN_SIZE; x++) {
		sin_x = lrint(SIN_MUL*sin(x*M_PI/SIN_SIZE));
		printf("0x%04x", sin_x);
		if ((x & 7)==7) {
			if (x<SIN_SIZE-1) printf(",\n	");
			else printf("};\n");
		} else printf(", ");

	}
	printf("int isin(int x) {\n	x = x & (2*SIN_SIZE-1);\n	if (x>=SIN_SIZE) return -Sin[x-SIN_SIZE]; else return Sin[x];\n}\n\n");

	return 0;
}
