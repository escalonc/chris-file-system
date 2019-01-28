#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

struct SuperBlock
{
  char name[30];
  int nodeEntriesQuantity;
  int firstNodeEntry;
  int firstDataBlock;
  int firstFreeNodeEntry;
  int firstFreeDataBlock;
};

#endif