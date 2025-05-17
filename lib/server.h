#ifndef SERVER_API_H
#define SERVER_API_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include "structs.h"
#include "engineers.h"
#include "challenge.h"
#include "organization.h"
#include "actives.h"

#define SERVER_PORT 9000
#define BUF_SIZE 1024

int getSelectedOptionInt(int client_fd);
int getSelectedOptionInRange(int client_fd, int min, int max);

void writeStr(int client_fd, char* str);

void deleteFromString(char string[], char substr[]);

void manageOrganizations(int client_fd, organization* organizations, int size);
void manageEngineers(int client_fd, engineer* engineers, int size);
void manageChallenges(int client_fd, challenge* challenge_list, int size);

void process_client(int client_fd);
void error(char* msg);
int authenticate_user(int client_fd, char *email);

void send_engineer_menu(int client_fd, char *email);
void send_organization_menu(int client_fd, char *email);
void send_admin_menu(int client_fd, char* email);

void applyChallenges(int client_fd, challenge* challenge_list, int size, engineer** eng);
void orgChallengeUpdate(int client_fd, challenge** challenge);
void engProfileUpdate(int client_fd, engineer** eng);
void viewApplicationStatus(int client_fd, engineer** eng);
void manageApplications(int client_fd, challenge **c);


#endif