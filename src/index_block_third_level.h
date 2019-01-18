#ifndef INDEX_BLOCK_THIRD_LEVEL_H
#define INDEX_BLOCK_THIRD_LEVEL_H

#include "index_block_second_level.h"

struct IndexBlockThirdLevel
{
  int dataBlockBitmap[INDEX_BLOCKS_THIRD_LEVEL];
  IndexBlockSecondLevel indexBlockSecondLevel[INDEX_BLOCKS_THIRD_LEVEL];
};

#endif