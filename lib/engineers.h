#ifndef ENGINEERS_API_H
#define ENGINEERS_API_H

#include "structs.h"

#define BUF_SIZE 1024

int add_engineer(char* name, int number, char* specialty, char* institution, bool student, char* areas_of_expertise, char* email, char* phone, char* password, int status, char* chal);
int update_engineer(engineer* engineers);
int remove_engineer(char* email);

int get_all_engineers(engineer** engineers, char* condition);
void engineerRegister(int client_fd);
void printEng(int client_fd, engineer* e, int adm);

#endif