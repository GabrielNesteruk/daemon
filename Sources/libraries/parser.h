#pragma once

#include "definitions.h"
/*
*   Funkcja odpowiedzialna za parsowanie opcji programu
*   @param argc liczba podanych argumentow
*   @param *argv[] wskaznik na tablice argumentow
*   @return ProgramData - struktura z danymi do przetwarzania w programie
*/
struct ProgramData parseCommandLine(int argc, char *argv[]);