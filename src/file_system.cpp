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
  this->dataFile->open(ios::in | ios::out | ios::app | ios::binary);

  const int dataBlocksQuantity = nodeEntriesQuantity * TOTAL_DATA_BLOCKS_IN_NODE_ENTRY;

  this->bitVector->dataBlockVector = new char[dataBlocksQuantity / 8];
  this->bitVector->indexBlockFirstLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockSecondLevelVector = new char[nodeEntriesQuantity / 8];
  this->bitVector->indexBlockThirdLevelVector = new char[nodeEntriesQuantity / 8];

  this->currentDirectoryInByte += sizeof(SuperBlock) + sizeof(this->bitVector);

  this->superBlock->nodeEntriesQuantity = nodeEntriesQuantity;
  this->superBlock->firstDataBlock = this->currentDirectoryInByte;

  this->dataFile->write(reinterpret_cast<char *>(this->superBlock),
                        sizeof(SuperBlock));

  this->dataFile->write(reinterpret_cast<char *>(this->bitVector), sizeof(this->bitVector));

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

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    IndexBlockFirstLevel *indexBlockFirstLevel = new IndexBlockFirstLevel();
    this->dataFile->write(reinterpret_cast<char *>(indexBlockFirstLevel),
                          sizeof(IndexBlockFirstLevel));
    delete indexBlockFirstLevel;
  }

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    IndexBlockSecondLevel *indexBlockSecondLevel = new IndexBlockSecondLevel();
    this->dataFile->write(reinterpret_cast<char *>(indexBlockSecondLevel),
                          sizeof(IndexBlockFirstLevel));
    delete indexBlockSecondLevel;
  }

  for (unsigned int i = 0; i < nodeEntriesQuantity; i++)
  {
    IndexBlockThirdLevel *indexBlockThirdLevel = new IndexBlockThirdLevel();
    this->dataFile->write(reinterpret_cast<char *>(indexBlockThirdLevel),
                          sizeof(IndexBlockThirdLevel));
    delete indexBlockThirdLevel;
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

  while (currentDirectory)
  {
    std::cout << currentDirectory->name << "\n";
    long position = this->currentDirectoryInByte += sizeof(NodeEntry);
    this->dataFile->read(position, sizeof(NodeEntry));
    currentDirectory = reinterpret_cast<NodeEntry *>(this->dataFile->read(this->currentDirectoryInByte, sizeof(NodeEntry)));
  }

  delete currentDirectory;
}
