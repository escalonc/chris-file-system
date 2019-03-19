#ifndef CONSTANTS_H
#define CONSTANTS_H

const int NODE_ENTRIES_DATA_BLOCKS = 12;
const int INDEX_BLOCKS_FIRST_LEVEL = 16;
const int INDEX_BLOCKS_SECOND_LEVEL = 32;
const int INDEX_BLOCKS_THIRD_LEVEL = 64;

const int TOTAL_DATA_BLOCKS_IN_NODE_ENTRY = NODE_ENTRIES_DATA_BLOCKS + INDEX_BLOCKS_FIRST_LEVEL * (1 + (1 + INDEX_BLOCKS_THIRD_LEVEL) * INDEX_BLOCKS_SECOND_LEVEL);

#endif