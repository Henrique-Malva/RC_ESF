#ifndef AUTH_API_H
#define AUTH_API_H

enum auth_level {
    UNREGISTERED = -1,
    ENGINEER = 0,
    ORGANIZATION = 1,
    ADMIN = 2
};

struct __client_t {
    char* hash;
    int auth_level;
    UT_hash_handle hh; /* makes this structure hashable */
};

typedef struct __client_t client_t;

#endif