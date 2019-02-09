#ifndef DATA_FILE_H
#define DATA_FILE_H

#include <fstream>

class DataFile
{
private:
  std::fstream *file;
  char *path;

public:
  DataFile();
  ~DataFile();
  DataFile(char *path);
  void open();
  void open(std::ios_base::openmode mode);
  void close();
  void write(char *data, unsigned int position, unsigned int size);
  void write(char *data, unsigned int size);
  char *read(unsigned int position, unsigned int size);
  long writePosition();
  long readPosition();
};

#endif