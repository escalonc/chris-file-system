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

  this->bitVector->dataBlockVector = new char[dataBlocksQuantity / 8];
  this->bitVector->indexBlockFirstLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockSecondLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockThirdLevelVector = new char[nodeEntriesQuantity / 8];

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

void FileSystem::makeDirectory(char *name)
{
  NodeEntry *nodeEntry;
  nodeEntry = reinterpret_cast<NodeEntry *>(
      this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));

  if (!nodeEntry->isFree && nodeEntry->firstChild == -1)
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

  delete nodeEntry;
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

void FileSystem::importFile(const char *name) {

  this->fileManager = new DataFile(name);
  this->fileManager->open(std::ios::in | std::ios::out | std::ios::binary);
  
  long size = this->fileManager->size() / 4096;

  if (size < 1) {
    size = 1; 
  }
  

  const int firstDataBlockPosition = this->superBlock->firstNodeEntry;

  for(size_t i = 0; i < size; i++)
  {
    int currentBitLocation = 0;

    if (i < NODE_ENTRIES_DATA_BLOCKS) {
      
    }
    else if (i > NODE_ENTRIES_DATA_BLOCKS && i < INDEX_BLOCKS_FIRST_LEVEL) {

    }
    
    
    
  }
  

}