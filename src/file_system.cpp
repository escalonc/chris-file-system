#include <string.h>
#include "file_system.h"
#include "constants.h"

FileSystem::FileSystem() { this->superBlock = new SuperBlock(); }

FileSystem::~FileSystem() {}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;
  this->dataFile = new DataFile(path);
  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;

  int position = 0;
  position += sizeof(SuperBlock);
  this->superBlock->firstNodeEntry = position;
  position += nodeEntriesQuantity;
  this->superBlock->firstDataBlock = position;

  this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
                        sizeof(SuperBlock));

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    NodeEntry *nodeEntry = new NodeEntry();

    char testName[30] = "Dir ";
    strcat(testName, reinterpret_cast<char *>(&i));

    strcpy(nodeEntry->name, testName);

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry),
                          sizeof(NodeEntry));
    delete nodeEntry;
  }

  int dataBlockPosition = 0;

  for (unsigned int i = 0; i < dataBlocksQuantity; i++)
  {
    dataBlockPosition += this->dataFile->tellp();
    DataBlock *dataBlock = new DataBlock();
    dataBlock->next = dataBlockPosition + sizeof(DataBlock);
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
  unsigned int position = this->superBlock->firstNodeEntry;

  NodeEntry *nodeEntry = new NodeEntry();

  for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
  {

    nodeEntry = reinterpret_cast<NodeEntry *>(
        this->dataFile->readKnownSize(position, sizeof(NodeEntry)));

    if (nodeEntry->isFree)
    {
      strcpy(nodeEntry->name, name);
      this->dataFile->write(reinterpret_cast<char *>(nodeEntry), position,
                            sizeof(nodeEntry));
    }
  }
}

void FileSystem::changeDirectory(char *name) {}

void FileSystem::changeToPreviousDirectory() {}

void FileSystem::list() {}
