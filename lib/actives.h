#ifndef ACTIVES_API_H
#define ACTIVES_API_H

#include "structs.h"

int add_actives(char* last_login, char* email, char* password, int role, int status);
int remove_actives(char* email);

int get_all_actives(active** actives, char* condition);
int update_active(active* actives);

#endif