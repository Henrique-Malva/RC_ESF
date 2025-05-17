#ifndef STRUCTS_API_H
#define STRUCTS_API_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h> 
#include <string.h>

#include "sqlite3.h"

#define LOCAL_DB "bin/local.db"

// Struct pre declaration
struct engineer;
struct challenge;
struct organization;

// Vector declaration

typedef struct str {
    int size;
    char* text;
} str;

// Engineer struct declaration

typedef struct engineer {
    int id;
    char* name;
    int number;
    char* engineeringSpecialty;
    char* employmentInstitution;
    bool studentStatus;
    char* areasOfExpertise;
    char* email;
    char* phoneNumber;
    char* password;
    int status; // registration status
    char* chal; // challenges applied to
} engineer;

// End of Engineer struct declaration

// Beginning of organization declaration with dependencies

// Beggining of challenge declaration

typedef struct challenge {
    int id;
    char* name;
    char* description;
    char* engineerType;
    int hours;
    int organizationId;
    int status;
    char* applicants;
} challenge;

// Ending of challenge declaration

// Beggining of organization struct declaration

typedef struct organization {
    int organizationId;
    char* name;
    int taxIdentificationNumber;
    char* email;
    char* address;
    char* activityDescription;
    char* phoneNumber;
    char* password;
    int status;
} organization;

// Ending of organization declaration

// Beggining of actives struct declaration

typedef struct active {
    int Id;
    char* last_login;
    char* email;
    char* password;
    int role;
    int status;
} active;

// Ending of actives declaration

#define STRING(s, n) strncpy(malloc(n), s, n)

#endif