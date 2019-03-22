#include "data_file.h"

DataFile::DataFile() { this->file = new std::fstream(); }

DataFile::DataFile(const char *path)
{
  this->path = path;
  this->file = new std::fstream();
}

DataFile::~DataFile()
{
  delete this->file;
  delete[] this->path;
}

void DataFile::open()
{
  this->file->open(this->path, std::ios::in | std::ios::out | std::ios::binary);
}

void DataFile::open(std::ios_base::openmode mode)
{
  this->file->open(this->path, mode);
}

void DataFile::close() { this->file->close(); }

void DataFile::write(char *data, unsigned int position, unsigned int size)
{
  this->file->seekp(position, std::ios::beg);

  this->file->write(data, size);
  this->file->flush();
}

void DataFile::write(char *data, unsigned int size)
{
  this->file->write(data, size);
  this->file->flush();
}

char *DataFile::read(unsigned int position, unsigned int size)
{
  char *element = new char[size];

  this->file->seekg(position, std::ios::beg);
  if (this->file->read(element, size))
  {
    return element;
  }
}

char *DataFile::read(unsigned int size)
{
  char *element = new char[size];

  if (this->file->read(element, size))
  {
    return element;
  }
}

long DataFile::writePosition() { return this->file->tellp(); }

long DataFile::readPosition()
{
  return this->file->tellg();
}

void DataFile::readPosition(int position)
{
  this->file->seekg(position, std::ios::beg);
}

long DataFile::size()
{
  long previousPosition = this->readPosition();
  this->file->seekg(0, std::ios::end);
  int newPosition = this->readPosition();
  this->readPosition(previousPosition);
  return newPosition;
}