#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "data_block.h"
#include "data_file.h"
#include "node_entry.h"
#include "super_block.h"

class FileSystem
{
private:
  int nodeEntriesQuantity;
  DataFile *dataFile;
  int currentDirectoryInByte;

  SuperBlock *superBlock;

public:
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