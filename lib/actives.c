#include "organization.h"

#define MAX_ACTIVES 200  // Define a maximum or manage dynamic resizing

typedef struct {
    active* clients;
    int index;
} CallbackData;

int select_callback_actives(void *data, int argc, char **argv, char **azColName) {
    CallbackData* cbData = (CallbackData*)data;

    if (cbData->index >= MAX_ACTIVES) {
        return 1; // Stop processing if array is full
    }

    active* e = &cbData->clients[cbData->index];

    e->Id = argv[0] ? atoi(argv[0]) : 0;
    e->last_login = argv[1] ? strdup(argv[1]) : NULL;
    e->email = argv[2] ? atoi(argv[2]) : 0;
    e->status = argv[3] ? atoi(argv[3]) : 0;

    cbData->index++;
    return 0;
}

static int callback_actives(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i < argc; i++) {
       printf("%30s = %30s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
 }

int add_actives(char* last_login, char* email, int status) {
    sqlite3* db;
    char sql[512];
    char* err = 0;

    /* Open database */  
    if(sqlite3_open(LOCAL_DB, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sprintf(sql, "INSERT INTO clients (last_login,email,status) VALUES ('%s','%s',%d); ",
        last_login, email, status);

    /* Execute SQL statement */    
    if(sqlite3_exec(db, sql, callback_actives, 0, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Records created successfully\n");
    }

    sqlite3_close(db);

    return 1;
}

int remove_actives(char* email) {
    sqlite3* db;
    char sql[512];
    char* err = 0;

    /* Open database */  
    if(sqlite3_open(LOCAL_DB, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create merged SQL statement */
    sprintf(sql, "DELETE from clients where email='%s'; SELECT * from clients", email);

    /* Execute SQL statement */
    if(sqlite3_exec(db, sql, callback_actives, 0, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }

    sqlite3_close(db);
    return 1;
}


int get_all_actives(active** actives, char* condition) {
    sqlite3* db;
    char sql[512];
    char* err = 0;

    *actives = malloc(MAX_ACTIVES* sizeof(active));

    CallbackData cbData = { .clients = *actives, .index = 0 };

    /* Open database */  
    if(sqlite3_open(LOCAL_DB, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sprintf(sql, "SELECT * from clients %s", condition);

    /* Execute SQL statement */    
    if(sqlite3_exec(db, sql, select_callback_actives, &cbData, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Records selected successfully\n");
    }

    sqlite3_close(db);

    return cbData.index;
}