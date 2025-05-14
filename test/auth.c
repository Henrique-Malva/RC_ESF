#include "auth.h"
#include "uthash/uthash.h"

client_t* clients = NULL;

void add_client(client_t client) {
    client_t* cp = memcpy(malloc(sizeof(client_t)), &client, sizeof(client_t));
    HASH_ADD_STR(clients, hash, cp);
}

client_t* find_client(char* hash) {
    client_t* cp;
    HASH_FIND_STR(clients, hash, cp);
    return cp;
}

void delete_client(client_t* client) {
    HASH_DEL(clients, client);
}

char* gethash(char* username, char* password) {
    size_t n1 = strlen(username);
    size_t n2 = strlen(password);
    char* hash = malloc(n1 + n2);
    strncpy(hash, username, n1);
    strncpy(hash + n1, password, n2);
    return hash;
}

int auth_client(char* username, char* password) {
    char* hash = gethash(username, password);

    return 0;
}