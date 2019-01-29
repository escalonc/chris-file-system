#include "data_file.h"

DataFile::DataFile() { this->file = new fstream(); }

DataFile::DataFile(char *path)
{
  this->path = path;
  this->file = new fstream();
}

DataFile::~DataFile()
{
  delete this->file;
  delete[] this->path;
}

void DataFile::open()
{
  this->file->open(this->path, ios::in | ios::out | ios::binary);
}

void DataFile::open(ios_base::openmode mode)
{
  this->file->open(this->path, mode);
}

void DataFile::close() { this->file->close(); }

void DataFile::write(char *data, unsigned int position, unsigned int size)
{
  this->file->seekp(0, ios::beg);
  this->file->seekp(position);

  this->file->write(data, size);
}

void DataFile::write(char *data, unsigned int size)
{
  this->file->write(data, size);
}

char *DataFile::read(unsigned int position, unsigned int size)
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
}
