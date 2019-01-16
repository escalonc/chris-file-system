#ifndef NODE_ENTRY_H

#include "IndexBlockFirstLevel.h"
#include "IndexBlockSecondLevel.h"
#include "IndexBlockThirdLevel.h"
#include <chrono>

struct NodeEntry
{
  char name[30];
  int size;
  char type;
  char date[30];

  int parent;
  int firstChild;
  int rightBrother;
  int firstBlock;
  bool isFree = true;

  int datablocksLocations[12];
  IndexBlockFirstLevel indexBlockFirstLevel;
  IndexBlockSecondLevel indexBlockSecondLevel;
  IndexBlockThirdLevel indexBlockThirdLevel;
};

#endif