#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

struct DataBlock
{
  char data[4092];
  int next;
};

#endif