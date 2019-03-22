#include <string.h>
#include "file_system.h"
#include "constants.h"
#include <iostream>
#include <cmath>
#include <bitset>

FileSystem::FileSystem()
{
  this->superBlock = new SuperBlock();
  this->bitVector = new BitVector();
}

FileSystem::~FileSystem()
{
  this->dataFile->close();
  delete this->superBlock;
  delete this->dataFile;
  delete this->bitVector;
}

void FileSystem::createDisk(char *path, const int nodeEntriesQuantity)
{
  this->dataFile = new DataFile(path);
  this->dataFile->open(std::ios::in | std::ios::out | std::ios::app | std::ios::binary);

  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;

  this->bitVector->dataBlockVectorSize = dataBlocksQuantity / 8;
  this->bitVector->indexBlockFirstLevelVectorSize = nodeEntriesQuantity / 8;
  this->bitVector->indexBlockSecondLevelVectorSize = nodeEntriesQuantity / 8;
  this->bitVector->indexBlockThirdLevelVectorSize = nodeEntriesQuantity / 8;

  this->bitVector->dataBlockVector = new char[this->bitVector->dataBlockVectorSize];
  this->bitVector->indexBlockFirstLevelVector = new char[this->bitVector->indexBlockFirstLevelVectorSize];
  this->bitVector->indexBlockSecondLevelVector = new char[this->bitVector->indexBlockSecondLevelVectorSize];
  this->bitVector->indexBlockThirdLevelVector = new char[this->bitVector->indexBlockThirdLevelVectorSize];

  this->currentDirectoryInByte = sizeof(SuperBlock) + sizeof(this->bitVector);

  this->superBlock->firstNodeEntry = sizeof(SuperBlock) + sizeof(this->bitVector);
  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
  this->superBlock->firstDataBlock = this->superBlock->firstNodeEntry + (sizeof(NodeEntry) * nodeEntriesQuantity);

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

  delete nodeEntry;
  delete dataBlock;
  delete indexBlockFirstLevel;
  delete indexBlockSecondLevel;
  delete indexBlockThirdLevel;
  delete rootDirectory;
}

void FileSystem::mountDisk(char *path) { this->dataFile = new DataFile(path); }

int FileSystem::createNodeEntry(NodeEntry *newNodeEntry)
{
  NodeEntry *nodeEntry;
  nodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  long newNodeEntryPosition = nextFreeNodeEntryPosition();
  if (!nodeEntry->isFree && nodeEntry->firstChild == -1)
  {
    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

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
    if (newNodeEntryPosition == -1)
    {
      std::cout << "No space" << std::endl;
      return;
    }

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

  delete nodeEntry;
  return newNodeEntryPosition;
}

void FileSystem::makeDirectory(char *name)
{
  NodeEntry *newNodeEntry = new NodeEntry();
  strcpy(newNodeEntry->name, name);
  newNodeEntry->type = 'd';
  newNodeEntry->isFree = false;
  createNodeEntry(newNodeEntry);
}

void FileSystem::changeDirectory(char *name)
{
  NodeEntry *currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  int position = currentDirectory->firstChild;
  while (position != -1)
  {
    NodeEntry *childNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
    if (strcmp(childNodeEntry->name, name) == 0)
    {
      this->currentDirectoryInByte = this->dataFile->readPosition() - sizeof(NodeEntry);
      break;
    }
    position = childNodeEntry->rightBrother;
  }

  delete currentDirectory;
}

void FileSystem::changeToPreviousDirectory()
{
  NodeEntry *currentDirectory;
  currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  this->currentDirectoryInByte = currentDirectory->parent;

  delete currentDirectory;
}

void FileSystem::list()
{
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
  unsigned int initialPosition = this->superBlock->firstNodeEntry;
  NodeEntry *currentNodeEntry;

  for (size_t i = 0; i < this->superBlock->nodeEntriesQuantity; i++)
  {
    currentNodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(initialPosition, sizeof(NodeEntry)));
    if (strcmp(currentNodeEntry->name, name) == 0)
    {
      break;
    }
    initialPosition += sizeof(NodeEntry);
  }

  if (strcmp(currentNodeEntry->name, name) != 0)
  {
    std::cout << "Not found!" << std::endl;
  }

  NodeEntry *parent = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentNodeEntry->parent, sizeof(NodeEntry)));

  int parentPosition = currentNodeEntry->parent;
  int brotherPosition = currentNodeEntry->rightBrother;
  int firstChildPosition = currentNodeEntry->firstChild;

  if (currentNodeEntry->firstChild != -1)
  {
    NodeEntry *child = reinterpret_cast<NodeEntry *>(this->dataFile->read(currentNodeEntry->firstChild, sizeof(NodeEntry)));
    removeNodeEntry(child, currentNodeEntry->firstChild);
  }

  if (initialPosition == parent->firstChild)
  {
    parent->firstChild = brotherPosition;
    this->dataFile->write(reinterpret_cast<char *>(parent), parentPosition, sizeof(NodeEntry));
  }
  else if (initialPosition == parent->lastChild)
  {
    NodeEntry *nodeEntry;
    int position = parent->firstChild;
    int location = 0;
    do
    {
      nodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
      location = this->dataFile->readPosition() - sizeof(NodeEntry);
    } while (nodeEntry->rightBrother != initialPosition);

    nodeEntry->rightBrother = -1;

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, (char *)"00000");
    newNodeEntry->isFree = true;
    newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
    newNodeEntry->parent = -1;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), initialPosition,
                          sizeof(NodeEntry));

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), location,
                          sizeof(NodeEntry));
  }
  else
  {
    NodeEntry *nodeEntry;
    int position = parent->firstChild;
    int location = 0;
    do
    {
      nodeEntry = reinterpret_cast<NodeEntry *>(this->dataFile->read(position, sizeof(NodeEntry)));
      position = nodeEntry->rightBrother;
      location = this->dataFile->readPosition() - sizeof(NodeEntry);
    } while (nodeEntry->rightBrother != initialPosition);

    nodeEntry->rightBrother = brotherPosition;

    NodeEntry *newNodeEntry = new NodeEntry();
    strcpy(newNodeEntry->name, (char *)"00000");
    newNodeEntry->isFree = true;
    newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
    newNodeEntry->parent = -1;
    this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), initialPosition,
                          sizeof(NodeEntry));

    this->dataFile->write(reinterpret_cast<char *>(nodeEntry), location,
                          sizeof(NodeEntry));
  }
}

void FileSystem::removeNodeEntry(NodeEntry *nodeEntry, int position)
{
  if (position == -1)
  {
    return;
  }

  int childPosition = nodeEntry->firstChild;
  int brotherPosition = nodeEntry->rightBrother;

  NodeEntry *newNodeEntry = new NodeEntry();
  strcpy(newNodeEntry->name, (char *)"00000");
  newNodeEntry->isFree = true;
  newNodeEntry->firstChild = newNodeEntry->lastChild = newNodeEntry->rightBrother = -1;
  newNodeEntry->parent = -1;
  this->dataFile->write(reinterpret_cast<char *>(newNodeEntry), position,
                        sizeof(NodeEntry));

  if (brotherPosition != -1)
  {
    NodeEntry *brother = reinterpret_cast<NodeEntry *>(this->dataFile->read(brotherPosition, sizeof(NodeEntry)));
    removeNodeEntry(brother, brotherPosition);
  }

  if (childPosition != -1)
  {
    NodeEntry *child = reinterpret_cast<NodeEntry *>(this->dataFile->read(childPosition, sizeof(NodeEntry)));
    removeNodeEntry(child, childPosition);
  }
}

void FileSystem::importFile(const char *name)
{
  NodeEntry *newNodeEntry = new NodeEntry();
  strcpy(newNodeEntry->name, name);
  newNodeEntry->type = 'a';
  newNodeEntry->isFree = false;

  std::ifstream *fileReader = new std::ifstream();
  fileReader->open(name, std::ios::in | std::ios::binary);

  fileReader->seekg(0, std::ios::end);
  long sizeInBytes = fileReader->tellg();

  double size = sizeInBytes / (double)4096;
  long remainingSize = sizeInBytes;

  fileReader->seekg(0, std::ios::beg);

  if (size < 1)
  {
    size = 1;
  }

  std::cout << "size in bytes is: " << sizeInBytes;
  int total = ceil(size);
  const int firstDataBlockPosition = this->superBlock->firstDataBlock;

  for (int i = 0; i < total; i++)
  {

    if (i <= NODE_ENTRIES_DATA_BLOCKS)
    {

      DataBlock *dataBlock = new DataBlock();
      int dataBlockPosition = this->nextFreeBlock(this->bitVector->dataBlockVector, this->bitVector->dataBlockVectorSize);

      if (remainingSize < 4096)
      {
        fileReader->read(dataBlock->data, remainingSize);
      }
      else
      {
        fileReader->read(dataBlock->data, 4096);
      }

      this->dataFile->write(reinterpret_cast<char *>(dataBlock), this->superBlock->firstDataBlock + dataBlockPosition, sizeof(DataBlock));
      newNodeEntry->datablocksLocations[i] = this->superBlock->firstDataBlock + dataBlockPosition;
      remainingSize -= 4096;
    }
    else if (i > NODE_ENTRIES_DATA_BLOCKS && i < INDEX_BLOCKS_FIRST_LEVEL)
    {
    }
  }

  createNodeEntry(newNodeEntry);

  fileReader->close();
}

void FileSystem::exportFile(const char *name)
{
  DataFile *exporter = new DataFile(name);
  exporter->open(std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
  int size = 25695;

  std::cout << "Export" << std::endl;
  for (size_t i = 0; i < 7; i++)
  {
    DataBlock *dataBlock = new DataBlock();

    if (size < 4096)
    {
      dataBlock = reinterpret_cast<DataBlock *>(this->dataFile->read(this->superBlock->firstDataBlock + (i * sizeof(DataBlock)), sizeof(DataBlock)));
      exporter->write(dataBlock->data, size);
    }
    else
    {
      dataBlock = reinterpret_cast<DataBlock *>(this->dataFile->read(this->superBlock->firstDataBlock + (i * sizeof(DataBlock)), sizeof(DataBlock)));
      exporter->write(dataBlock->data, 4096);
    }

    size -= 4096;
  }
  exporter->close();
}

void FileSystem::printBitVectorSection(int bitVector)
{
  std::bitset<8> x(bitVector);
  std::cout << x << '\n';
}

int FileSystem::readBit(char bitVector, int position)
{
  return (bitVector >> (7 - position)) & 1U;
}

void FileSystem::setBit(char *bitVector, int position)
{
  *bitVector |= 1UL << (7 - position);
}

int FileSystem::nextFreeBlock(char *bitVector, int size)
{
  for (size_t i = 0; i < size; i++)
  {
    char element = bitVector[i];
    for (size_t j = 0; j < 7; i++)
    {
      if (!this->readBit(element, j))
      {
        return (i * 8) + j;
      }
    }
  }
}