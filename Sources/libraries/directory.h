#pragma once

#include "definitions.h"
#include "stdbool.h"

void checkForDeletion(const struct ProgramData data);
void checkForModificationTime(const struct ProgramData data);

char* concatPaths(const char* path1, const char* path2);
bool compareModificationTime(const char* path1, const char* path2);
void copyFiles(const char* path1, const char* path2);