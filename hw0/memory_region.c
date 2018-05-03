struct MemoryRegion {
  void *startAddr;
  void *endAddr;
  int isReadable;
  int isWriteable;
  int isExecutable;
};

void incrementSize(struct MemoryRegion *m, int *readOnlySize,
                   int *readWriteSize) {
  if (m->isReadable) {
    int size = m->endAddr - m->startAddr;
    if (m->isWriteable) {
      *readWriteSize += size;
    } else {
      *readOnlySize += size;
    }
  }
}

void prettyPrintMemoryRegion(struct MemoryRegion *m) {
  printf("%p to %p: ", m->startAddr, m->endAddr);
  printf("%s readable, %s writeable, %s executable\n",
         m->isReadable ? "" : "not", m->isWriteable ? "" : "not",
         m->isExecutable ? "" : "not");
}

void prettyPrintRegionSizes(int readOnlySize, int readWriteSize) {
  printf("read-only region size: %d bytes\nread-write region size: %d bytes\n",
         readOnlySize, readWriteSize);
}