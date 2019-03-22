#ifndef BIT_VECTOR_H
#define BIT_VECTOR_H

#include "constants.h"

struct BitVector
{
  char *dataBlockVector;
  char *indexBlockFirstLevelVector;
  char *indexBlockSecondLevelVector;
  char *indexBlockThirdLevelVector;

  int dataBlockVectorSize;
  int indexBlockFirstLevelVectorSize;
  int indexBlockSecondLevelVectorSize;
  int indexBlockThirdLevelVectorSize;
};

#endif