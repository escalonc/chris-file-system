#ifndef INDEX_BLOCK_SECOND_LEVEL_H
#define INDEX_BLOCK_SECOND_LEVEL_H

#include "index_block_first_level.h"

struct IndexBlockSecondLevel
{
  int dataBlockBitmap[INDEX_BLOCKS_SECOND_LEVEL];
  IndexBlockFirstLevel indexBlockSecondLevel[INDEX_BLOCKS_SECOND_LEVEL];
};

#endif