#ifndef INDEX_BLOCK_THIRD_LEVEL_H
#define INDEX_BLOCK_THIRD_LEVEL_H

#include "IndexBlockSecondLevel.h"

struct IndexBlockThirdLevel
{
  IndexBlockSecondLevel indexBlockSecondLevel[64];
};

#endif