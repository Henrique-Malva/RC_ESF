#ifndef CHALLENGE_API_H
#define CHALLENGE_API_H

#include "structs.h"

#define BUF_SIZE 1024
#define MAX_CHALLENGES 100  // Define a maximum or manage dynamic resizing

int add_challenge(char* name, char* description, char* type, int hours, int organization_id, bool status, char* app);
int update_challenge(challenge* challenges);
int remove_challenge(int id);

int get_all_challenges(challenge** challenges, char* condition);
void addChallengePrompt(int client_fd, organization * org);
void printChal(int client_fd, challenge* c, int adm);

#endif