#ifndef IO_API_H
#define IO_API_H

#include <stdio.h>
#include <stdlib.h>

void write_to_file(const char* filename, char* data);

char* read_from_file(const char *filename);

#endif