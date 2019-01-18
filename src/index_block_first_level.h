#ifndef INDEX_BLOCK_FIRST_LEVEL_H
#define INDEX_BLOCK_FIRST_LEVEL_H

#include "data_block.h"
#include "constants.h"

struct IndexBlockFirstLevel
{
  int dataBlockBitmap[INDEX_BLOCKS_FIRST_LEVEL];
  int dataBlockLocation[INDEX_BLOCKS_FIRST_LEVEL];
};

#endif