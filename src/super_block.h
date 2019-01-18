#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

struct SuperBlock
{
  char *name;
  int nodeEntriesQuantity;
  int firstNodeEntry;
  int firstFreeDataBlock;
  int firstDataBlock;
};

#endif