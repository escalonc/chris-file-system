#include <string.h>
#include "file_system.h"
#include "constants.h"
#include <iostream>

FileSystem::FileSystem()
{
  this->superBlock = new SuperBlock();
  this->bitVector = new BitVector();
}

FileSystem::~FileSystem()
{
  delete this->superBlock;
  delete this->dataFile;
  delete this->bitVector;
}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
  this->dataFile = new DataFile(path);
  this->dataFile->open(std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;

  this->bitVector->dataBlockVector = new char[dataBlocksQuantity / 8];
  this->bitVector->indexBlockFirstLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockSecondLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockThirdLevelVector = new char[nodeEntriesQuantity / 8];

  this->currentDirectoryInByte = sizeof(SuperBlock) + sizeof(this->bitVector);

  this->superBlock->firstNodeEntry = sizeof(SuperBlock) + sizeof(this->bitVector);
  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
  this->superBlock->firstDataBlock = this->currentDirectoryInByte;

  this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
                        sizeof(SuperBlock));

  this->dataFile->write(reinterpret_cast<char *>(this->bitVector), sizeof(this->bitVector));

  NodeEntry *nodeEntry = new NodeEntry();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    nodeEntry->isFree = true;
    nodeEntry->lastChild = nodeEntry->firstChild = -1;
    nodeEntry->rightBrother = -1;
    strcpy(nodeEntry->name, (char *)"00000");

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry),
                          sizeof(NodeEntry));
  }
  delete nodeEntry;

  DataBlock *dataBlock = new DataBlock();
  for (size_t i = 0; i < dataBlocksQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(dataBlock),
                          sizeof(DataBlock));
  }
  delete dataBlock;

  IndexBlockFirstLevel *indexBlockFirstLevel = new IndexBlockFirstLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockFirstLevel),
                          sizeof(IndexBlockFirstLevel));
  }
  delete indexBlockFirstLevel;

  IndexBlockSecondLevel *indexBlockSecondLevel = new IndexBlockSecondLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockSecondLevel),
                          sizeof(IndexBlockFirstLevel));
  }
  delete indexBlockSecondLevel;

  IndexBlockThirdLevel *indexBlockThirdLevel = new IndexBlockThirdLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockThirdLevel),
                          sizeof(IndexBlockThirdLevel));
  }
  delete indexBlockThirdLevel;

  this->dataFile->close();
}

void FileSystem::mountDisk(char *path) { this->dataFile = new DataFile(path); }

void FileSystem::makeDirectory(char *name)
{
  this->dataFile->open();

  NodeEntry *nodeEntry;
  nodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  this->dataFile->close();

  if (nodeEntry->isFree)
  {
    this->dataFile->open();

    strcpy(nodeEntry->name, name);
    nodeEntry->type = 'd';
    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));

    this->dataFile->close();
  }
  else if (!nodeEntry->isFree && nodeEntry->firstChild == 1)
  {
    long newNodeEntryPosition = nextFreeNodeEntryPosition();

    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

    this->dataFile->open();

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, name);
    newNodeEntry->type = 'd';
    newNodeEntry->parent = this->currentDirectoryInByte;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
                          sizeof(NodeEntry));

    nodeEntry->lastChild = nodeEntry->firstChild = newNodeEntryPosition;

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));

    this->dataFile->close();
  }
  else if (!nodeEntry->isFree && nodeEntry->lastChild)
  {
    long newNodeEntryPosition = nextFreeNodeEntryPosition();

    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

    NodeEntry *lastChildNodeEntry;
    this->dataFile->open();
    lastChildNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(nodeEntry->lastChild, sizeof(NodeEntry)));

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, name);
    newNodeEntry->type = 'd';
    newNodeEntry->parent = this->currentDirectoryInByte;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
                          sizeof(NodeEntry));

    lastChildNodeEntry->rightBrother = newNodeEntryPosition;
    nodeEntry->lastChild = newNodeEntryPosition;

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));

    this->dataFile->close();
  }

  for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity - 1; i++)
  {
    if (nodeEntry->isFree)
    {
      strcpy(nodeEntry->name, name);
      nodeEntry->type = 'd';
      this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                            sizeof(NodeEntry));
    }
  }

  delete nodeEntry;
  this->dataFile->close();
}

void FileSystem::changeDirectory(char *name) {}

void FileSystem::changeToPreviousDirectory()
{
  NodeEntry *currentDirectory;
  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  this->currentDirectoryInByte = currentDirectory->parent;
  delete currentDirectory;
}

void FileSystem::list()
{
  this->dataFile->open();
  NodeEntry *currentDirectory, *childNodeEntry;
  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  if (currentDirectory->firstChild == -1)
  {
    std::cout << "Empty directory!" << std::endl;
    return;
  }

  childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->firstChild, sizeof(NodeEntry)));

  while (childNodeEntry->rightBrother != -1)
  {
    std::cout << childNodeEntry->name << std::endl;
    childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->rightBrother, sizeof(NodeEntry)));
  }

  delete currentDirectory;
  delete childNodeEntry;

  this->dataFile->close();
}

long FileSystem::nextFreeNodeEntryPosition()
{
  this->dataFile->open();

  unsigned int initialPosition = this->superBlock->firstNodeEntry;

  for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
  {
    NodeEntry *currentNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(initialPosition, sizeof(NodeEntry)));
    if (currentNodeEntry->isFree)
    {
      return this->dataFile->readPosition() - sizeof(NodeEntry);
    }
  }

  std::cout << "No space in disk" << std::endl;
  return -1;

  this->dataFile->close();
}
