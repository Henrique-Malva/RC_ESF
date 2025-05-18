#ifndef ORGANIZATION_API_H
#define ORGANIZATION_API_H

#include "structs.h"

#define BUF_SIZE 1024

int add_organization(char* name, int tax_id, char* email, char* address, char* description, char* phone, char* password, int status);
int update_organization(organization* organizations);
int remove_organization(char* email);

int get_all_organizations(organization** organizations, char* condition);
void organizationRegister(int client_fd);
void printOrg(int client_fd, organization* org);

#endif