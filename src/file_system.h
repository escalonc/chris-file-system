#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "data_block.h"
#include "data_file.h"
#include "node_entry.h"
#include "super_block.h"
#include "bit_vector.h"

class FileSystem
{
private:
  DataFile *dataFile;
  SuperBlock *superBlock;
  int currentDirectoryInByte;
  BitVector *bitVector;
  long makeDirectoryInPosition(char *name, unsigned int position);
  long nextFreeNodeEntryPosition();
  void removeNodeEntry(NodeEntry *nodeEntry, int position);

public:
  FileSystem();
  ~FileSystem();

  void createDisk(char *path, const int nodeEntriesQuantity);
  void mountDisk(char *path);
  void makeDirectory(char *name);
  void changeDirectory(char *name);
  void changeToPreviousDirectory();
  void removeNodeEntry(char *name);
  void list();
  void importFile(const char *name);
  void exportFile(const char *name);
  void setBit(int position);
  int readBit(int position);
};

#endif