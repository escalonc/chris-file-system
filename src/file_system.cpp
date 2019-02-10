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

  DataBlock *dataBlock = new DataBlock();
  for (size_t i = 0; i < dataBlocksQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(dataBlock),
                          sizeof(DataBlock));
  }

  IndexBlockFirstLevel *indexBlockFirstLevel = new IndexBlockFirstLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockFirstLevel),
                          sizeof(IndexBlockFirstLevel));
  }

  IndexBlockSecondLevel *indexBlockSecondLevel = new IndexBlockSecondLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockSecondLevel),
                          sizeof(IndexBlockFirstLevel));
  }

  IndexBlockThirdLevel *indexBlockThirdLevel = new IndexBlockThirdLevel();
  for (size_t i = 0; i < nodeEntriesQuantity; i++)
  {
    this->dataFile->write(reinterpret_cast<char *>(indexBlockThirdLevel),
                          sizeof(IndexBlockThirdLevel));
  }

  this->dataFile->close();

  this->dataFile->open();
  NodeEntry *rootDirectory = new NodeEntry();
  rootDirectory->firstChild = rootDirectory->rightBrother = rootDirectory->lastChild = -1;
  strcpy(rootDirectory->name, (char *)"/");
  rootDirectory->isFree = false;
  this->dataFile->write(reinterpret_cast<char *>(rootDirectory), this->currentDirectoryInByte, sizeof(NodeEntry));
  this->dataFile->close();

  delete nodeEntry;
  delete dataBlock;
  delete indexBlockFirstLevel;
  delete indexBlockSecondLevel;
  delete indexBlockThirdLevel;
  delete rootDirectory;
}

void FileSystem::mountDisk(char *path) { this->dataFile = new DataFile(path); }

void FileSystem::makeDirectory(char *name)
{
  this->dataFile->open();

  NodeEntry *nodeEntry;
  nodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  if (nodeEntry->isFree)
  {

    strcpy(nodeEntry->name, name);
    nodeEntry->type = 'd';
    nodeEntry->isFree = false;
    nodeEntry->rightBrother = -1;
    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));
  }
  else if (!nodeEntry->isFree && nodeEntry->firstChild == -1)
  {
    long newNodeEntryPosition = nextFreeNodeEntryPosition();

    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, name);
    newNodeEntry->type = 'd';
    newNodeEntry->isFree = false;
    newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
    newNodeEntry->parent = this->currentDirectoryInByte;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
                          sizeof(NodeEntry));

    nodeEntry->firstChild = nodeEntry->lastChild = newNodeEntryPosition;

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));
  }
  else if (!nodeEntry->isFree && nodeEntry->firstChild != -1)
  {
    long newNodeEntryPosition = nextFreeNodeEntryPosition();

    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, name);
    newNodeEntry->type = 'd';
    newNodeEntry->isFree = false;
    newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
    newNodeEntry->parent = this->currentDirectoryInByte;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), newNodeEntryPosition,
                          sizeof(NodeEntry));

    int lastChildPreviousPosition = nodeEntry->lastChild;
    NodeEntry *lastChild = reinterpret_cast<NodeEntry *>(this->dataFile->read(nodeEntry->lastChild, sizeof(NodeEntry)));
    lastChild->rightBrother = newNodeEntryPosition;
    nodeEntry->lastChild = newNodeEntryPosition;

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), this->currentDirectoryInByte,
                          sizeof(NodeEntry));

    this->dataFile->write(reinterpret_cast<char *>(lastChild), lastChildPreviousPosition, sizeof(NodeEntry));
  }

  this->dataFile->close();
  delete nodeEntry;
}

void FileSystem::changeDirectory(char *name)
{
  this->dataFile->open();
  NodeEntry *currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  NodeEntry *childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->firstChild, sizeof(NodeEntry)));
  do
  {
    if (strcmp(childNodeEntry->name, name) == 0)
    {
      this->currentDirectoryInByte = this->dataFile->readPosition() - sizeof(NodeEntry);
      break;
    }
    childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->rightBrother, sizeof(NodeEntry)));

  } while (childNodeEntry->rightBrother != -1);

  this->dataFile->close();
  delete currentDirectory;
  delete childNodeEntry;
}

void FileSystem::changeToPreviousDirectory()
{
  this->dataFile->open();
  NodeEntry *currentDirectory;
  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  this->currentDirectoryInByte = currentDirectory->parent;

  this->dataFile->close();
  delete currentDirectory;
}

void FileSystem::list()
{
  this->dataFile->open();
  NodeEntry *currentDirectory;
  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  if (currentDirectory->isFree || currentDirectory->firstChild == -1)
  {
    std::cout << "Empty directory!" << std::endl;
    return;
  }

  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->firstChild, sizeof(NodeEntry)));
  std::cout << currentDirectory->name << std::endl;

  while (currentDirectory->rightBrother != -1)
  {
    currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentDirectory->rightBrother, sizeof(NodeEntry)));
    std::cout << currentDirectory->name << std::endl;
  }

  this->dataFile->close();
  delete currentDirectory;
}

long FileSystem::nextFreeNodeEntryPosition()
{
  unsigned int initialPosition = this->superBlock->firstNodeEntry;

  for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
  {
    NodeEntry *currentNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(initialPosition, sizeof(NodeEntry)));
    if (currentNodeEntry->isFree && this->dataFile->readPosition() != this->currentDirectoryInByte)
    {
      return this->dataFile->readPosition() - sizeof(NodeEntry);
    }
    initialPosition += sizeof(NodeEntry);
  }

  std::cout << "No space in disk" << std::endl;
  return -1;
}

void FileSystem::removeNodeEntry(char *name)
{
  this->dataFile->open();
  this->dataFile->close();
}
