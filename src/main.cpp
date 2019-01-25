#ifndef CHRIS_FILE_SYSTEM
#define CHRIS_FILE_SYSTEM

#include <fstream>
#include <iostream>
#include <string.h>
#include <cstring>

#include "file_system.h"

using namespace std;

void init()
{
  system("clear");
  cout << "Chris File System [CFS]" << endl
       << endl;
}

int main()
{
  char token[30] = "", *command, *parameter = nullptr;

  FileSystem *fileSystem = new FileSystem();

  init();

  while (strcmp(token, "exit") != 0)
  {
    cout << "$ ";
    cin.getline(token, 30);

    if (strlen(token) == 0)
    {
      continue;
    }

    command = strtok(token, " ");
    parameter = strtok(nullptr, " ");

    if (strcmp(command, "create-disk") == 0)
    {
      fileSystem->createDisk(parameter, 20);
    }
    else if (strcmp(command, "mkdir") == 0)
    {
      fileSystem->makeDirectory(parameter);
    }
    else if (strcmp(command, "clear") == 0)
    {
      init();
    }
    else if (strcmp(command, "exit") == 0)
    {
      break;
    }
  }

  return 0;
}

#endif