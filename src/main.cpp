#ifndef CHRIS_FILE_SYSTEM
#define CHRIS_FILE_SYSTEM

#include <fstream>
#include <iostream>
#include <string.h>
#include <cstring>

#include "file_system.h"

void init()
{
  std::cout << "Chris File System [CFS]" << std::endl
            << std::endl;
}

int main()
{
  char token[30] = "", *command, *parameter = nullptr;

  FileSystem *fileSystem = new FileSystem();

  init();

  while (strcmp(token, "exit") != 0)
  {
    std::cout << "$ ";
    std::cin.getline(token, 30);

    if (strlen(token) == 0)
    {
      continue;
    }

    command = strtok(token, " ");
    parameter = strtok(nullptr, " ");

    if (strcmp(command, "create-disk") == 0)
    {
      fileSystem->createDisk(parameter, 10);
    }
    else if (strcmp(command, "mkdir") == 0)
    {
      fileSystem->makeDirectory(parameter);
    }
    else if (strcmp(command, "ls") == 0)
    {
      fileSystem->list();
    }
    else if (strcmp(command, "cd") == 0 && strcmp(parameter, "..") == 0)
    {
      fileSystem->changeToPreviousDirectory();
    }
    else if (strcmp(command, "cd") == 0)
    {
      fileSystem->changeDirectory(parameter);
    }
    else if (strcmp(command, "rm") == 0)
    {
      fileSystem->removeNodeEntry(parameter);
    }
    else if (strcmp(command, "import") == 0)
    {
      fileSystem->importFile(parameter);
    }
    else if (strcmp(command, "export") == 0)
    {
      fileSystem->exportFile(parameter);
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