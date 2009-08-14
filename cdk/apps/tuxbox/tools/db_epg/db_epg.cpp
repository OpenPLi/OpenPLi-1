#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cmain.h"

int main(int argc, char *argv[])
{
    cMain m;
    return m.start(argc, argv);
}
