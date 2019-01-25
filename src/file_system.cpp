#include <string.h>
#include "file_system.h"
#include "constants.h"
#include <iostream>

FileSystem::FileSystem() { this->superBlock = new SuperBlock(); }

FileSystem::~FileSystem() {}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;
  this->dataFile = new DataFile(path);
  this->currentDirectoryInByte += sizeof(SuperBlock);

  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
  this->superBlock->firstDataBlock = this->currentDirectoryInByte;

  this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
                        sizeof(SuperBlock));

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    NodeEntry *nodeEntry = new NodeEntry();
    nodeEntry->isFree = true;
    strcpy(nodeEntry->name, (char *)"00000");

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry),
                          sizeof(NodeEntry));
    delete nodeEntry;
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

  NodeEntry *nodeEntry = new NodeEntry();

  nodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->readKnownSize(this->currentDirectoryInByte, sizeof(NodeEntry)));

  if (nodeEntry->isFree)
  {
    strcpy(nodeEntry->name, name);
    nodeEntry->parent
        nodeEntry->type = 'd';
    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(nodeEntry));
  }
}

void FileSystem::changeDirectory(char *name) {}

void FileSystem::changeToPreviousDirectory() {}

void FileSystem::list()
{
  NodeEntry *currentDirectory;

  while (currentDirectory)
  {
    std::cout << currentDirectory->name << endl;
    long position = this->currentDirectoryInByte += sizeof(NodeEntry);
    this->dataFile->readKnownSize(position, sizeof(NodeEntry));
  }
}
