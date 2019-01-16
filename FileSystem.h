#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "DataBlock.h"
#include "DataFile.h"
#include "NodeEntry.h"
#include "SuperBlock.h"

class FileSystem
{
private:
  int nodeEntriesQuantity;
  DataFile *dataFile;

  SuperBlock *superBlock;

public:
  NodeEntry *currentNodeEntry;

  FileSystem();
  ~FileSystem();

  void createDisk(char *path, const int nodeEntriesQuantity);
  void mountDisk(char *path);
  void makeDirectory(char *name);
  void changeDirectory(char *name);
  void changeToPreviousDirectory();
  void list();
};

#endif