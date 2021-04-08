#pragma once

#include "definitions.h"

// Funkcja realizujca inicjalizacje demona
void daemonInit();
// Funkcja odpowiedziala za zakonczenie dzialania demona
void daemonExit();
// Funkcja odpowiedzialna za czynnosci wykonywane przez demona
void daemonService(struct ProgramData data);