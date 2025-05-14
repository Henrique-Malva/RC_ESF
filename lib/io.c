#include "io.h"

void write_to_file(const char* filename, char* data) {
    FILE* f = fopen(filename, "w");
    if (f) {
        fputs(data, f);
        fclose(f);
    } else {
        perror("Failed to open file for writing.");
    }
}

char* read_from_file(const char *filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open file for reading");
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char* data = (char*)malloc(len + 1);
    fread(data, 1, len, f);
    data[len] = '\0';
    fclose(f);

    return data;
}