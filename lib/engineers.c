#include "engineers.h"


typedef struct {
    engineer* engineers;
    int index;
} CallbackData;

int select_callback_engineers(void *data, int argc, char **argv, char **azColName) {
    CallbackData* cbData = (CallbackData*)data;

    if (cbData->index >= MAX_ENGINEERS) {
        return 1; // Stop processing if array is full
    }

    engineer* e = &cbData->engineers[cbData->index];

    e->id = argv[0] ? atoi(argv[0]) : 0;
    e->name = argv[1] ? strdup(argv[1]) : NULL;
    e->number = argv[2] ? atoi(argv[2]) : 0;
    e->engineeringSpecialty = argv[3] ? strdup(argv[3]) : NULL;
    e->employmentInstitution = argv[4] ? strdup(argv[4]) : NULL;
    e->studentStatus = argv[5] ? atoi(argv[5]) : false; // SQLite stores booleans as integers
    e->areasOfExpertise = argv[6] ? strdup(argv[6]) : NULL;
    e->email = argv[7] ? strdup(argv[7]) : NULL;
    e->phoneNumber = argv[8] ? strdup(argv[8]) : NULL;
    e->password = argv[9] ? strdup(argv[9]) : NULL;
    e->status = argv[10] ? atoi(argv[10]) : 0;
    e->chal = argv[11] ? strdup(argv[11]) : NULL;

    cbData->index++;
    return 0;
}

int callback_engineers(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for(i = 0; i < argc; i++) {
       printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

// adds an engineer to the database with the information passed as arguments
int add_engineer(char* name, int number, char* specialty, char* institution, bool student, char* areas_of_expertise, char* email, char* phone, char* password, int status, char* chal) {
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
    sprintf(sql, "INSERT INTO engineers (name,number,specialty,institution,student,areas_of_expertise,email,phone,password,status,challenges) VALUES ('%s',%d,'%s','%s',%d,'%s','%s','%s','%s',%d,'%s'); ",
        name, number, specialty, institution, student, areas_of_expertise, email, phone, password, status, chal);

    /* Execute SQL statement */    
    if(sqlite3_exec(db, sql, callback_engineers, 0, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Records created successfully\n");
    }

    sqlite3_close(db);

    return 1;
}

// updates the info on the database about the engineer passed as argument
int update_engineer(engineer* engineers) {
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
    sprintf(sql, "UPDATE engineers SET name='%s',number='%d',specialty='%s',institution='%s',student='%d',areas_of_expertise='%s',email='%s',phone='%s',password='%s',status='%d',challenges='%s' WHERE id='%d'; ",
        engineers->name, engineers->number, engineers->engineeringSpecialty, engineers->employmentInstitution, engineers->studentStatus, engineers->areasOfExpertise, engineers->email, engineers->phoneNumber, engineers->password, engineers->status, engineers->chal, engineers->id);

    /* Execute SQL statement */
    if(sqlite3_exec(db, sql, callback_engineers, 0, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }

    sqlite3_close(db);
    return 1; 
}

// removes the engineer with the email passed in the first argument from the database
int remove_engineer(char* email) {
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
    sprintf(sql, "DELETE from engineers where email='%s'; SELECT * from engineers", email);

    /* Execute SQL statement */
    if(sqlite3_exec(db, sql, callback_engineers, 0, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Operation done successfully\n");
    }

    sqlite3_close(db);
    return 1; 
}

// get_all as in get_all that satisfy the SQL query condition in the second argument
// if condition is empty it gets all of them and stores them into dynamic memory, returning the pointer to that memory address in the 1st argument
int get_all_engineers(engineer** engineers, char* condition) { 
    sqlite3* db;
    char sql[512];
    char* err = 0;

    *engineers = malloc(MAX_ENGINEERS * sizeof(engineer));

    CallbackData cbData = { .engineers = *engineers, .index = 0 };

    /* Open database */  
    if(sqlite3_open(LOCAL_DB, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0;
    } else {
        fprintf(stderr, "Opened database successfully\n");
    }

    /* Create SQL statement */
    sprintf(sql, "SELECT * from engineers %s", condition);

    /* Execute SQL statement */    
    if(sqlite3_exec(db, sql, select_callback_engineers, &cbData, &err) != SQLITE_OK){
        fprintf(stderr, "SQL error: %s\n", err);
        sqlite3_free(err);
        return 0;
    } else {
        fprintf(stdout, "Records selected successfully\n");
    }

    sqlite3_close(db);

    return cbData.index;
}

// function used to register engineers, asks for all of the necessary information
// verifies the correctedness of the given information and avoides duplicates where they can't exist
// and calls the add function
void engineerRegister(int client_fd) {
	int oeNumber;
    int studentStatus;
    char fullName[100], specialty[50], institution[100];
    char areasOfExpertise[200], email[100], phone[20], pass[20];
    char chal[6*MAX_ENGINEERS]; // max id length is 3 + 2 for approval status and separation from status and id, max 100 challenges = 500 chars + 99(100-1) commas separating each challenge id + \0 = 600
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char hexstr[SHA256_DIGEST_LENGTH*2+1];

    char buf[10], buffer[BUF_SIZE];
    engineer* eng; organization* org;

    int check=0, nread;

    strcpy(chal,"");
    
    // Get engineer's details
    write(client_fd, "Enter your full name: ", strlen("Enter your full name: "));
    nread = read(client_fd, fullName, 100 - 1);
    fullName[nread - 2] = '\0';

    do // OE number can't have duplicates
    {
        if (check)
        {
            write(client_fd, "This OE number is already in use\n\n", strlen("This OE number is already in use\n\n"));
        }
        check=1;
        
        write(client_fd, "Enter your OE number: ", strlen("Enter your OE number: "));
        nread = read(client_fd, buf, 10 - 1);
        buf[nread - 2] = '\0';
        oeNumber = atoi(buf);
        sprintf(buffer,"where number='%d'",oeNumber);
    } while (get_all_engineers(&eng,buffer));
    check=0;

    write(client_fd, "Enter a password: ", strlen("Enter a password: "));
    nread = read(client_fd, pass, 20 - 1);
    pass[nread - 2] = '\0';

    SHA256((unsigned char*)pass, strlen(pass), hash);
    // Convert binary hash to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hexstr + (i * 2), "%02x", hash[i]);
    }
    hexstr[SHA256_DIGEST_LENGTH * 2] = '\0';


    write(client_fd, "Enter your engineering specialty: ", strlen("Enter your engineering specialty: "));
    nread = read(client_fd, specialty, 50 - 1);
    specialty[nread - 2] = '\0';

    write(client_fd, "Enter your institution of employment: ", strlen("Enter your institution of employment: "));
    nread = read(client_fd, institution, 100 - 1);
    institution[nread - 2] = '\0';

    write(client_fd, "Are you still a student? (1/0): ", strlen("Are you still a student? (1/0): "));
    nread = read(client_fd, buf, 10 - 1);
    buf[nread - 2] = '\0';
    studentStatus=atoi(buf);

    write(client_fd, "Enter your areas of expertise: ", strlen("Enter your areas of expertise: "));
    nread = read(client_fd, areasOfExpertise, 200 - 1);
    areasOfExpertise[nread - 2] = '\0';

    do // email can't be duplicate
    {
        if (check)
        {
            write(client_fd, "This email is already in use\n\n", strlen("This email is already in use\n\n"));
        }
        check=1;

        write(client_fd, "Enter your email address: ", strlen("Enter your email address: "));
        nread = read(client_fd, email, 100 - 1);
        email[nread - 2] = '\0';

        sprintf(buffer,"where email='%s'", email);
    } while (get_all_engineers(&eng,buffer) || get_all_organizations(&org,buffer));
    check=0;

    write(client_fd, "Enter your mobile phone number (optional): ", strlen(" Enter your mobile phone number (optional): "));
    nread = read(client_fd, phone, 20 - 1);
    phone[nread - 2] = '\0';

    write(client_fd, "\nRegistration successful!\n", strlen("\nRegistration successful!\n"));
    
    add_engineer(fullName,oeNumber,specialty,institution,studentStatus,areasOfExpertise,email,phone,hexstr,1,chal);
}

void printEng(int client_fd, engineer* e, int adm){
    char buffer[BUF_SIZE];

    write(client_fd, "\nEngineer name: ", 16);
    write(client_fd, e->name,strlen(e->name));
    write(client_fd, "\nOE Number: ",12);
    sprintf(buffer, "%d", e->number);
    write(client_fd, buffer,strlen(buffer));
    write(client_fd, "\nEngineering Specialty: ",24);
    write(client_fd, e->engineeringSpecialty,strlen(e->engineeringSpecialty));
    write(client_fd, "\nStudent Status: ",17);
    if(e->studentStatus){
        write(client_fd, "Studying",8);
    }
    else write(client_fd, "Not studying",12);
    write(client_fd, "\nAreas of Expertise: ",21);
    write(client_fd, e->areasOfExpertise,strlen(e->areasOfExpertise));
    write(client_fd, "\nEmail: ",8);
    write(client_fd, e->email,strlen(e->email));
    write(client_fd, "\nPhone Number: ",15);
    write(client_fd, e->phoneNumber,strlen(e->phoneNumber));

    if (adm)
    {
        switch (e->status){
            case 0: write(client_fd, "\n\nStatus: Approved",18); break;
            case 1: write(client_fd, "\n\nStatus: Pending",17); break;
            case 2: write(client_fd, "\n\nStatus: Rejected",18); break;
        }
    }
    

}