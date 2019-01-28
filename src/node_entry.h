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
  char date[10];

  int parent;
  int firstChild;
  int rightBrother;
  int firstBlock;
  bool isFree;

  unsigned int datablocksLocations[NODE_ENTRIES_DATA_BLOCKS];
  IndexBlockFirstLevel indexBlockFirstLevel;
  IndexBlockSecondLevel indexBlockSecondLevel;
  IndexBlockThirdLevel indexBlockThirdLevel;
};

#endif