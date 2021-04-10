#include "directory.h"
#include "dirent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "string.h"
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <utime.h>

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
        if(access(source_file_path, F_OK) != 0 && entity->d_type == DT_REG)
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

void checkForModificationTime(const struct ProgramData data)
{
    DIR* dir = opendir(data.source_path);  // otwieramy katalog ze sciezka zrodlowa
    if(dir == NULL)
        return 1;
    
    struct dirent* entity;
    entity = readdir(dir); // odczytujemy pierwszy element w katalogu
    while(entity != NULL)   // iterujemy dopoki sa elementy do odczytu
    {
        char* destination_file_path = concatPaths(data.destination_path, entity->d_name);
        if(access(destination_file_path, F_OK) == 0 && entity->d_type == DT_REG)
        {
            syslog(LOG_INFO, "%s\n", entity->d_name);
            // znaleziono taki sam plik, teraz nalezy porownac date ich modyfikacji
            char* source_file_path = concatPaths(data.source_path, entity->d_name);
            if(compareModificationTime(source_file_path, destination_file_path))
            {
                // czas modyfikacji w dest jest pozniejszy niz w source, wiec kopiujemy source -> dest
                copyFiles(source_file_path, destination_file_path);   
            }
            free(source_file_path);
        }
        entity = readdir(dir);
        free(destination_file_path);
    }
    closedir(dir);
}

void copyFiles(const char* path1, const char* path2)
{
    int source_fd;
    int dest_fd;
    char buffer[256] = {0};

    struct stat source_file;

    stat(path1, &source_file);

    if((source_fd = open(path1, O_RDONLY, 0666)) == -1 ) // w source otwieramy plik tylko do odczytu
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }
     if((dest_fd = open(path2, O_RDWR | O_TRUNC, 0666)) == -1 ) // w dest otwieramy plik z dostepem rw-rw-rw i jednoczesnie czyscimy jego zawartosc
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }

    struct utimbuf date;
    date.actime = 0;
    date.modtime = source_file.st_mtime;

    if(utime(path2, &date) != 0) // proba zmiany czasu modyfikacji pliku
    {
        syslog(LOG_ERR, "Error has occured while trying to modify modification time of file %s", path2);
    }
    else
        syslog(LOG_INFO, "File %s had been modified and copied.", path2);

    if(source_fd > 0) // jest cokolwiek do odczytania z source
    {
        ssize_t num_read;
        while(num_read = read(source_fd, buffer, sizeof(buffer)))
            write(dest_fd, buffer, num_read);
    }

    close(source_fd);
    close(dest_fd);
}

char* concatPaths(const char* path1, const char* path2)
{
    char* concatenate_path = (char*)malloc(strlen(path1) + strlen(path2) + 2); // + 2 bo strlen nie dodaje do dlugosci '\0'
    char* concatenate_origin_pointer = concatenate_path;
    strcpy(concatenate_path, path1);
    strcat(concatenate_path, "/");
    strcat(concatenate_path, path2);

    return concatenate_origin_pointer;
}

bool compareModificationTime(const char* path1, const char* path2)
{
    struct stat source_file;
    struct stat dest_file;

    stat(path1, &source_file);
    stat(path2, &dest_file);

    if((source_file.st_mtime - dest_file.st_mtime) > 0) // czas modyfikacji w source jest pozniejszy
    {
        return true;
    }
    else
        return false;
}
