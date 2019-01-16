struct SuperBlock
{
  char *name;
  int nodeEntriesQuantity;
  int firstNodeEntry;
  int firstFreeDataBlock;
  int firstDataBlock;
};