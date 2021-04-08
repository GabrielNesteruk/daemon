#include "directory.h"
#include "dirent.h"
#include "string.h"
#include <unistd.h>
#include <syslog.h>

void checkForDeletion(const struct ProgramData data)
{
    DIR* dir = opendir(data.destination_path);  // otwieramy katalog ze sciezka docelowa
    if(dir == NULL)
        return 1;

    struct dirent* entity;
    entity = readdir(dir); // odczytujemy pierwszy element w katalogu
    while(entity != NULL)   // iterujemy dopoki sa elementy do odczytu
    {
        char* source_file_path = concatPaths(data.source_path, entity->d_name);
        if(access(source_file_path, F_OK) != 0)
        {
            // plik nie istnieje w katologu zrodlowym wiec zostaje usuniety z katologu docelowego
            char* destination_file_path = concatPaths(data.destination_path, entity->d_name);
            remove(destination_file_path);
            syslog(LOG_INFO, "File %s has been removed.", destination_file_path);
            free(destination_file_path);
        }
        entity = readdir(dir);
        free(source_file_path);
    }
    closedir(dir);
}

char* concatPaths(const char* path1, const char* path2)
{
    char* concatenate_path = malloc(strlen(path1) + strlen(path2) + 3); // + 2 bo strlen nie dodaje do dlugosci '\0'
    char* concatenate_origin_pointer = concatenate_path;
    strcat(concatenate_path, path1);
    strcat(concatenate_path, "/");
    strcat(concatenate_path, path2);

    return concatenate_origin_pointer;
}