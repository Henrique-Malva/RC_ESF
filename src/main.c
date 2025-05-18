#include <stdio.h>
#include <signal.h>

#include "server.h"

void error(char* msg) {
    printf("Error: %s\n", msg);
    exit(-1);
}

void logouts(int sig){
    active* a;
    int n = get_all_actives(&a, "");
    for (int i = 0; i < n; i++) {
        a[i].status=0;
        update_active(&a[i]);
    }
    exit(1);
}

int main() {
    int fd, client;
    struct sockaddr_in addr, client_addr;
    int client_addr_size;
    bzero((void*)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);


    /* replaces the handles for ctrl+C and ctrl+\ to guarantee that every client is logged out when the server is terminated*/
    signal(SIGINT, logouts);
    signal(SIGKILL, logouts);



    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("in function socket");
    }

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        error("in function bind");
    }

    if (listen(fd, 5) < 0) {
        error("in function listen");
    }
    
    // Diagnostics
    engineer* e;
    int n = get_all_engineers(&e, "");
    for (int i = 0; i < n; i++) {
        printf("%d: %s\n", i, e[i].email);
    }
    challenge* c;
    n = get_all_challenges(&c, "");
    for (int i = 0; i < n; i++) {
        printf("%d: %s\n", i, c[i].name);
    }
    organization* o;
    n = get_all_organizations(&o, "");
    for (int i = 0; i < n; i++) {
        printf("%d: %s\n", i, o[i].email);
    }
    active* a;
    n = get_all_actives(&a, "");
    for (int i = 0; i < n; i++) {
        printf("%d: %s\n", i, a[i].email);
    }


    while (1) {
        client_addr_size = sizeof(client_addr);
        client = accept(fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_size);

        if (client > 0) {
            if (fork() == 0) {
                close(fd);
                process_client(client);
                exit(0);
            }
            close(client);
        }
    }
    return 0;
}
