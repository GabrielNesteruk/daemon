#include "directory.h"
#include "dirent.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "string.h"
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <utime.h>
#include <sys/mman.h>
#include <time.h>

void checkForDeletion(const struct ProgramData data)
{
    DIR *dir = opendir(data.destination_path); // otwieramy katalog ze sciezka docelowa
    if (dir == NULL)
        return 1;

    struct dirent *entity;
    entity = readdir(dir); // odczytujemy pierwszy element w katalogu
    while (entity != NULL) // iterujemy dopoki sa elementy do odczytu
    {
        char *source_file_path = concatPaths(data.source_path, entity->d_name);
        char *destination_file_path = concatPaths(data.destination_path, entity->d_name);
        if (access(source_file_path, F_OK) != 0 && entity->d_type == DT_REG)
        {
            // plik nie istnieje w katologu zrodlowym wiec zostaje usuniety z katologu docelowego
            remove(destination_file_path);
            syslog(LOG_INFO, "File %s has been removed.", destination_file_path);
        }
        else if (entity->d_type == DT_DIR && data.recursive_scan)
        {
            if (!(!strcmp(entity->d_name, ".") || !strcmp(entity->d_name, "..")))
            {
                // kopiujemy sciezki do struktury i rekurencyjne przeszukujemy katalogi
                struct ProgramData recursive_data;
                recursive_data.source_path = strdup(source_file_path);
                recursive_data.destination_path = strdup(destination_file_path);
                recursive_data.sleeping_time = data.sleeping_time;
                checkForDeletion(recursive_data);
                if (access(source_file_path, F_OK) != 0) // folder nie istnieje w source
                {
                    rmdir(destination_file_path);
                    syslog(LOG_INFO, "Directory %s has been deleted.", destination_file_path);
                }
                free(recursive_data.source_path);
                free(recursive_data.destination_path);
            }
        }
        entity = readdir(dir);
        free(source_file_path);
        free(destination_file_path);
    }
    closedir(dir);
}

void checkForModificationTime(const struct ProgramData data)
{
    DIR *dir = opendir(data.source_path); // otwieramy katalog ze sciezka zrodlowa
    if (dir == NULL)
        return 1;

    struct dirent *entity;
    entity = readdir(dir); // odczytujemy pierwszy element w katalogu
    while (entity != NULL) // iterujemy dopoki sa elementy do odczytu
    {
        char *destination_file_path = concatPaths(data.destination_path, entity->d_name);
        char *source_file_path = concatPaths(data.source_path, entity->d_name);
        if (entity->d_type == DT_DIR && data.recursive_scan)
        {
            if (!(!strcmp(entity->d_name, ".") || !strcmp(entity->d_name, "..")))
            {
                // kopiujemy sciezki do struktury i rekurencyjne przeszukujemy katalogi
                struct ProgramData recursive_data;
                recursive_data.source_path = strdup(source_file_path);
                recursive_data.destination_path = strdup(destination_file_path);
                recursive_data.sleeping_time = data.sleeping_time;
                if (access(destination_file_path, F_OK) != 0) // folder nie istnieje w dest ale istnieje w source
                {
                    mkdir(destination_file_path, 0777);
                    syslog(LOG_INFO, "Directory %s has been created.", destination_file_path);
                }
                checkForModificationTime(recursive_data);
                free(recursive_data.source_path);
                free(recursive_data.destination_path);
            }
        }
        if (access(destination_file_path, F_OK) == 0 && entity->d_type == DT_REG)
        {
            // znaleziono taki sam plik w katalogu docelowym, teraz nalezy porownac date ich modyfikacji
            if (compareModificationTime(source_file_path, destination_file_path))
            {
                // czas modyfikacji w dest jest pozniejszy niz w source, wiec kopiujemy source -> dest
                if (getFileSize(source_file_path) >= file_size_limit)
                {
                    // clock_t begin = clock();
                    efficientCopyFiles(source_file_path, destination_file_path);
                    // clock_t end = clock();
                    // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                    // syslog(LOG_INFO, "Czas kopiowania pliku to %f [MMAP]", time_spent);
                }
                else
                {
                    // clock_t begin = clock();
                    copyFiles(source_file_path, destination_file_path);
                    // clock_t end = clock();
                    // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                    // syslog(LOG_INFO, "Czas kopiowania pliku to %f [read/write]", time_spent);
                }
            }
        }
        if (access(destination_file_path, F_OK) != 0 && entity->d_type == DT_REG)
        {
            // plik jest w source ale nie ma go w dest wiec trzeba go tam skopiowac
            if (getFileSize(source_file_path) >= file_size_limit)
            {
                // clock_t begin = clock();
                efficientCopyFiles(source_file_path, destination_file_path);
                // clock_t end = clock();
                // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                // syslog(LOG_INFO, "Czas kopiowania pliku to %f [MMAP]", time_spent);
            }
            else
            {
                // clock_t begin = clock();
                copyFiles(source_file_path, destination_file_path);
                // clock_t end = clock();
                // double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
                // syslog(LOG_INFO, "Czas kopiowania pliku to %f [read/write]", time_spent);
            }
        }
        entity = readdir(dir);
        free(source_file_path);
        free(destination_file_path);
    }
    closedir(dir);
}

void copyFiles(const char *path1, const char *path2)
{
    int source_fd;
    int dest_fd;
    char buffer[256] = {0};

    struct stat source_file;

    stat(path1, &source_file);

    if ((source_fd = open(path1, O_RDONLY)) == -1) // w source otwieramy plik z flaga do odczytu
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }
    if ((dest_fd = open(path2, O_RDWR | O_TRUNC | O_CREAT, 0666)) == -1) // w dest otwieramy plik z dostepem rw-rw-rw i jednoczesnie czyscimy jego zawartosc
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }

    struct utimbuf date;
    date.actime = 0;
    date.modtime = source_file.st_mtime;

    if (utime(path2, &date) != 0) // proba zmiany czasu modyfikacji pliku
    {
        syslog(LOG_ERR, "Error has occured while trying to modify modification time of file %s", path2);
    }

    if (source_fd > 0) // jest cokolwiek do odczytania z source
    {
        ssize_t num_read;
        while (num_read = read(source_fd, buffer, sizeof(buffer)))
            write(dest_fd, buffer, num_read);
        syslog(LOG_INFO, "File %s had been modified and copied.", path2);
    }

    close(source_fd);
    close(dest_fd);
}

void efficientCopyFiles(const char *path1, const char *path2)
{
    int source_fd;
    int dest_fd;
    char buffer[256] = {0};

    struct stat source_file;

    stat(path1, &source_file);

    if ((source_fd = open(path1, O_RDONLY)) == -1) // w source otwieramy plik z flaga do odczytu
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }
    if ((dest_fd = open(path2, O_RDWR | O_TRUNC | O_CREAT, 0666)) == -1) // w dest otwieramy plik z dostepem rw-rw-rw i jednoczesnie czyscimy jego zawartosc
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s.", path1, path2);
        return;
    }

    char *source_map = (char *)mmap(0, source_file.st_size, PROT_READ, MAP_SHARED | MAP_FILE, source_fd, 0);
    if (source_map != MAP_FAILED)
    {
        write(dest_fd, source_map, source_file.st_size);
        syslog(LOG_INFO, "File %s had been modified and copied. [MMAP copy]", path2);
    }
    else
    {
        syslog(LOG_ERR, "Error has occured while trying to copy %s to %s. [MMAP FAILURE]", path1, path2);
    }

    munmap(source_map, source_file.st_size);

    close(source_fd);
    close(dest_fd);
}

char *concatPaths(const char *path1, const char *path2)
{
    char *concatenate_path = (char *)malloc(strlen(path1) + strlen(path2) + 2); // + 2 bo strlen nie dodaje do dlugosci '\0'
    char *concatenate_origin_pointer = concatenate_path;
    strcpy(concatenate_path, path1);
    strcat(concatenate_path, "/");
    strcat(concatenate_path, path2);

    return concatenate_origin_pointer;
}

bool compareModificationTime(const char *path1, const char *path2)
{
    struct stat source_file;
    struct stat dest_file;

    stat(path1, &source_file);
    stat(path2, &dest_file);

    if ((source_file.st_mtime - dest_file.st_mtime) > 0) // czas modyfikacji w source jest pozniejszy
    {
        return true;
    }
    else
        return false;
}

long getFileSize(const char *path)
{
    struct stat file;
    stat(path, &file);

    return (long)file.st_size;
}