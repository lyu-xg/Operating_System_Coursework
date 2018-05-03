#include <stdio.h>
#include "memory_region.c"
 
int main ()
{
    FILE *infile;
    struct MemoryRegion m;
    int readOnlySize = 0, readWriteSize = 0;
     
    infile = fopen ("memory.dat", "r");
    if (infile == NULL) { return 1; }

    while(fread(&m, sizeof(struct MemoryRegion), 1, infile)) {
        prettyPrintMemoryRegion(&m);
        incrementSize(&m, &readOnlySize, &readWriteSize);
    }

    prettyPrintRegionSizes(readOnlySize, readWriteSize);

    return 0;
}