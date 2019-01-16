#include "DataFile.h"

DataFile::DataFile() { this->file = new fstream(); }

DataFile::~DataFile()
{
  delete this->file;
  delete[] this->path;
}

DataFile::DataFile(char *path)
{
  this->path = path;

  this->file = new fstream();
  this->file->open(this->path, ios::in | ios::out | ios::app | ios::binary);
}

DataFile::DataFile(char *path, ios_base::openmode mode)
{
  this->path = path;

  this->file = new fstream();
  this->file->open(this->path, mode);
}

void DataFile::open()
{
  this->file->open(this->path, ios::in | ios::out | ios::app | ios::binary);
}

void DataFile::close() { this->file->close(); }

void DataFile::write(char *data, unsigned int position, unsigned int size)
{
  this->file->seekp(0, ios::beg);
  this->file->seekp(position * size);

  this->file->write(data, size);
  // this->file->flush();
}

void DataFile::write(char *data, unsigned int size)
{
  this->file->write(data, size);
  this->file->flush();
}

char *DataFile::read(unsigned int position, unsigned int size)
{
  char *element = new char[size];

  this->file->seekg(position * size);

  this->file->read(element, size);

  return element;
}

char *DataFile::readKnownSize(unsigned int position, unsigned int size)
{
  char *element = new char[size];

  this->file->seekg(position);

  this->file->read(element, size);

  return element;
}

long DataFile::tellp() { return this->file->tellp(); }

long DataFile::tellg()
{
  return this->file->tellg();
  ;
}
