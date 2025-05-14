#ifndef ENGINEERS_API_H
#define ENGINEERS_API_H

#include "structs.h"

int add_engineer(char* name, int number, char* specialty, char* institution, bool student, char* areas_of_expertise, char* email, char* phone, char* password, int status);
int update_engineer(engineer* engineers);
int remove_engineer(char* email);

int get_all_engineers(engineer** engineers, char* condition);
void engineerRegister(int client_fd);

#endif