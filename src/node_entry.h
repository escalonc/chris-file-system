#ifndef NODE_ENTRY_H

#include "index_block_first_level.h"
#include "index_block_second_level.h"
#include "index_block_third_level.h"

#include "constants.h"

struct NodeEntry
{
  char name[30];
  int size;
  char type;
  char date[30];

  int dataBlockBitmap[NODE_ENTRIES_DATA_BLOCKS];

  int parent;
  int firstChild;
  int rightBrother;
  int firstBlock;
  bool isFree = true;

  unsigned int datablocksLocations[NODE_ENTRIES_DATA_BLOCKS];
  IndexBlockFirstLevel indexBlockFirstLevel;
  IndexBlockSecondLevel indexBlockSecondLevel;
  IndexBlockThirdLevel indexBlockThirdLevel;
};

#endif