#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

// Программе передаются два аргумента: имя файла и строка для поиска.
// Необходимо найти все вхождения строки в текстовом файле,
// используя отображение на память с помощью системного вызова mmap.
// На стандартный поток вывода вывести список всех позиций (с нуля),
// где встречается искомая строка, по одной на строку.

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search_string>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *search_str = argv[2];
    size_t search_len = strlen(search_str);

    if (search_len == 0) {
        return 0;
    }

    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0 || (size_t)st.st_size < search_len) {
        close(fd);
        return 0;
    }

    char *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    for (size_t i = 0; i <= (size_t)st.st_size - search_len; ++i) {
        if (memcmp(map + i, search_str, search_len) == 0) {
            printf("%zu\n", i);
        }
    }

    if (munmap(map, st.st_size) < 0) {
        perror("munmap");
    }
    close(fd);

    return 0;
}
