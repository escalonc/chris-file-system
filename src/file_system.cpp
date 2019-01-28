#include <string.h>
#include "file_system.h"
#include "constants.h"
#include <iostream>

FileSystem::FileSystem()
{
  this->superBlock = new SuperBlock();
}

FileSystem::~FileSystem()
{
}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
  this->dataFile = new DataFile(path);
  this->dataFile->open(ios::in | ios::out | ios::app | ios::binary);

  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;
  this->currentDirectoryInByte += sizeof(SuperBlock);

  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
  this->superBlock->firstDataBlock = this->currentDirectoryInByte;

  this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
                        sizeof(SuperBlock));

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    NodeEntry *newNodeEntry = new NodeEntry();
    newNodeEntry->isFree = true;
    strcpy(newNodeEntry->name, (char *)"00000");

    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry),
                          sizeof(NodeEntry));
    delete newNodeEntry;
  }

  for (unsigned int i = 0; i < dataBlocksQuantity; i++)
  {
    DataBlock *dataBlock = new DataBlock();
    this->dataFile->write(reinterpret_cast<char *>(dataBlock),
                          sizeof(DataBlock));
    delete dataBlock;
  }

  this->dataFile->close();
}

void FileSystem::mountDisk(char *path) { this->dataFile = new DataFile(path); }

void FileSystem::makeDirectory(char *name)
{
  this->dataFile->open();

  NodeEntry *newNodeEntry = new NodeEntry();
  newNodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  for (size_t i = 0; i < 1000; i++)
  {
    if (newNodeEntry->isFree)
    {
      strcpy(newNodeEntry->name, name);
      newNodeEntry->type = 'd';
      this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), this->currentDirectoryInByte,
                            sizeof(NodeEntry));
    }
  }

  delete newNodeEntry;
  this->dataFile->close();
}

void FileSystem::changeDirectory(char *name) {}

void FileSystem::changeToPreviousDirectory() {}

void FileSystem::list()
{
  NodeEntry *currentDirectory;

  while (currentDirectory)
  {
    std::cout << currentDirectory->name << "\n";
    long position = this->currentDirectoryInByte += sizeof(NodeEntry);
    this->dataFile->read(position, sizeof(NodeEntry));
  }
}
