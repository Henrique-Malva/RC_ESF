#include "server.h"

// function to read the user input and convert it to an int
int getSelectedOptionInt(int client_fd){
    char buffer[BUF_SIZE];
    size_t nread = read(client_fd, buffer, BUF_SIZE - 1);
    buffer[nread-2] = 0;

    return atoi(buffer);
}

// function to verify if the numeric input is in a valid range
int getSelectedOptionInRange(int client_fd, int min, int max){
    char errStr[200];
    int read;
    
    sprintf(errStr, "\nThe choice you entered is not valid, please enter a valid option (%d-%d): ", min, max);
    while(true){
        read = getSelectedOptionInt(client_fd);

        if(read >= min && read <= max)
            return read;
        
        writeStr(client_fd, errStr);
    }
}

void writeStr(int client_fd, char* str){
    write(client_fd, str, strlen(str));
}

void manageOrganizations(int client_fd, organization* organizations, int size);
void manageEngineers(int client_fd, engineer* engineers, int size);
void manageChallenges(int client_fd, challenge* challenge_list, int size);
void applyChallenges(int client_fd, challenge* challenge_list, int size);

int closeFunction() { return -1; }

// main client handling function where the operation flow is handled
void process_client(int client_fd) {
    int choice = 0;
    // welcome menu
    char* menu0 = "========================================================\n"
                "        Welcome to engineers without borders!\n"
                "========================================================"
                "\nPlease select an option:\n"
                "1. Login\n"
                "2. Organization Registration\n"
                "3. Engineer Registration\n"
                "4. Logout\n"
                "Enter your option (1-4): ";

    char email[100];
    int user_role;
    do{
        
        writeStr(client_fd, menu0);
        choice = getSelectedOptionInRange(client_fd, 1, 4);

        switch (choice)
        {
        case 1: // login
            user_role = authenticate_user(client_fd, email);

            // Based on the user role, send appropriate menu
            switch (user_role) {
            case 0: // engineer logged in
                send_engineer_menu(client_fd, email);
                break;
            case 1: // ONG logged in
                send_organization_menu(client_fd, email);
                break;
            case 2: // admin logged in
                send_admin_menu(client_fd);
                break;
            default: // no such user registered
                writeStr(client_fd, "Closing connection.\n");
                choice=4;
                remove_actives(email);
                close(client_fd);
                break;
            }
            break;
        case 2: // registration of non-profit org
            system("cls");  
            writeStr(client_fd, "==================================================================\n"
                                "                      NON-PROFIT REGISTRATION \n"
                                "         PLEASE ENTER THE DATA OF YOUR NON-PROFIT ORGANIZATION \n"
                                "==================================================================\n\n"
                                " > Non-Profit Organization Registration:\n");
            organizationRegister(client_fd);
            break;
        case 3: // registration of engineer
            system("cls");
            writeStr(client_fd, "==================================================================\n"
                                "                      VOLUNTEER REGISTRATION \n"
                                "               PLEASE ENTER YOUR DATA AND CREDENTIALS \n"
                                "==================================================================\n\n"
                                " > Volunteer Engineer Registration:\n");
            engineerRegister(client_fd);
            break;
        
        default:
            break;
        }
    } while (choice!=4);
}

// engineer handling function
// presents the menu options for an engineer and calls the functions responsible for the option selected by the user
void send_engineer_menu(int client_fd, char *email) {
    int choice = 0;
    char buffer[BUF_SIZE];
    
    // sends the menu
    char* menu = "========================================================\n"
                "                 Welcome Engineer!\n"
                "========================================================"
                "\nPlease select an option:\n"
                    "1. View available challenges\n"
                    "2. Apply for a challenge\n"
                    "3. Update profile\n"
                    "4. Logout\n"
                    "Enter your option (1-4): ";

    engineer* eng;
    sprintf(buffer,"where email='%s'",email);
    get_all_engineers(&eng, buffer);

    do
    {
        write(client_fd, menu, strlen(menu));
        choice = getSelectedOptionInRange(client_fd, 1, 4);

        // add notification section or application results section something like that (F9)
        switch (choice) {
            case 1: // visualize challenges
                challenge* c; int n;
                n = get_all_challenges(&c, "");
                applyChallenges(client_fd, c, n);
                break;
            case 2: // apply for challenges (combine with previous?)
                write(client_fd, "not yet implemented", strlen("not yet implemented"));
                break;
            case 3: // update profile
                // ask for engineer shiz

                update_engineer(eng);
                break;
            default: // logout
                remove_actives(email);
                close(client_fd);
                break;
        }
    } while (choice!=4);
    
    
}

// ONG handling function
// presents the menu options for an ONG and calls the functions responsible for the option selected by the user
void send_organization_menu(int client_fd, char *email) {
    char buffer[BUF_SIZE], name[200];
    int choice;
    size_t nread;
    
    // sends menu
    char* menu = "========================================================\n"
                "          Welcome Non-Profit Organization!\n"
                "========================================================"
                "\nPlease select an option:\n"
                "1. Add a new challenge\n"
                "2. List challenges\n"
                "3. Update challenge\n"
                "4. Delete challenge\n"
                "5. View applications\n"
                "6. Logout\n"
                "Enter your option (1-6): ";
    
    organization* org;
    challenge* c;
    sprintf(buffer,"where email='%s'",email);
    get_all_organizations(&org, buffer);
    //get_organization(&org, email);

    do
    {
        writeStr(client_fd, menu);
        choice = getSelectedOptionInRange(client_fd, 1, 6);

        switch (choice) {
            case 1: // add a new challenge
                addChallengePrompt(client_fd, org);
                break;
            case 2: // list all challenges belonging to this ONG   
                sprintf(buffer,"where organization_id='%d'",org->organizationId);
                int n = get_all_challenges(&c, buffer);
                int count = 0;
                challenge* currentChall = c;
                while(count < n){

                    currentChall = c + count;
            
                    writeStr(client_fd, "\nName: ");
                    writeStr(client_fd, currentChall->name);
                    writeStr(client_fd, "\nDescription: ");
                    writeStr(client_fd, currentChall->description);
                    writeStr(client_fd, "\nEngineer Type: ");
                    writeStr(client_fd, currentChall->engineerType);
                    writeStr(client_fd, "\nHours: ");
                    sprintf(buffer, "%d", currentChall->hours);
                    writeStr(client_fd, buffer);
                    writeStr(client_fd, "\nOrganization ID: ");
                    sprintf(buffer, "%d", currentChall->organizationId);
                    writeStr(client_fd, buffer);
                    switch (currentChall->status){
                        case 0: writeStr(client_fd, "\n\nStatus: Approved\n"); break;
                        case 1: writeStr(client_fd, "\n\nStatus: Pending\n"); break;
                        case 2: writeStr(client_fd, "\n\nStatus: Rejected\n"); break;
                    }

                    count++;
            
                }

                break;
            case 3: // update a challenge (rn it's just removing and readding, need change this)
                writeStr(client_fd, "Challenge Name: ");
                nread = read(client_fd, name, 200 - 1);
                name[nread-2] = 0;
                
                sprintf(buffer,"where organization_id='%d',name='%s' ",org->organizationId,name);
                get_all_challenges(&c, buffer);

                // ask for challenge shiz

                update_challenge(c);

                //remove_challenge(getSelectedOptionInt(client_fd));
                //addChallengePrompt(client_fd, org);
                break;
            case 4: // remove a challenge
                writeStr(client_fd, "Challenge name: ");
                nread = read(client_fd, name, 200 - 1);
                name[nread-2] = 0;
                sprintf(buffer,"where organization_id='%d',name='%s';",org->organizationId,name);
                get_all_challenges(&c, buffer);
                remove_challenge(c->id);
                break;
            case 5: // view and accept applicants
                writeStr(client_fd, "not yet implemented");
                break;
            default: // logout
                remove_actives(email);
                close(client_fd);
                break;
        }
    } while (choice!=6);
    
    
}

// admin handling function
// presents the menu options for an admin and calls the functions responsible for the option selected by the user
void send_admin_menu(int client_fd) {
    int state = 0;

    // sends menu
    char* menu0 = "========================================================"
                "                       Welcome Admin!\n"
                "========================================================"
                "\nPlease select an option:\n"
                "1. Manage engineers\n"
                "2. Manage organizations\n"
                "3. Manage challenges\n"
                "4. Logout\n"
                "Enter your option (1-4): ";
     

    do{
        write(client_fd, menu0, strlen(menu0));
        state = getSelectedOptionInRange(client_fd, 1, 4);  

        switch (state){
            case 1: // manage engineers

                engineer* e;
                int n = get_all_engineers(&e, "");
                manageEngineers(client_fd, e, n);
                break;

            case 2: // manage ONGs
            
                organization* o;
                n = get_all_organizations(&o, "");
                manageOrganizations(client_fd, o, n);
                break;

            case 3: // manage challenges(?)

                challenge* c;
                n = get_all_challenges(&c, "");
                manageChallenges(client_fd, c, n);

                break;

            case 4: // logout
                close(client_fd);
                break;
        }
    }while(state!=4);
}

// user login function
int authenticate_user(int client_fd, char *email) {

    char* login_prompt = "\nEnter your login email: ";
    char* password_prompt = "Enter your password: ";
    char* authentication_error_prompt = "User not registered\n";
    writeStr(client_fd, login_prompt);

    char buffer[BUF_SIZE];
    char email_cond[BUF_SIZE];
    int nread = read(client_fd, buffer, BUF_SIZE - 1);
    buffer[nread - 2] = '\0';
    strcpy(email,buffer);

    char pass[200];
    engineer* eng;
    organization* org;
    sprintf(email_cond,"where email='%s'",email);
    active* active;


    if (strcmp(email, "admin789") == 0) {
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, buffer, BUF_SIZE - 1);
        buffer[nread - 2] = '\0';
        if (strcmp(buffer, "adminpass") == 0) {
            return 2; // Admin
        }
        // in the admin login there is no feedback about wrong password, so that a normal user doesn't find a admin username
    } else if (get_all_engineers(&eng, email_cond)) {
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';
        
        if (strcmp(pass,eng->password) == 0)
        {
            if (eng->status == 0) // registration accepted
            {
                sprintf(buffer,"where email='%s",email);
                add_actives("date",email,1);
                return 0; // Engineer
            }else if (eng->status == 1){ // status pending
                writeStr(client_fd, "Registration pending\n");
            }else { // status rejected
                writeStr(client_fd, "Registration rejected. Please re-register or contact support\n");
                remove_engineer(email);
            }
            return -1;
            
        }else{
            writeStr(client_fd, "Wrong password\n");
            return -1;
        }
        
    } else if (get_all_organizations(&org, email_cond)) {
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';

        if (strcmp(pass,org->password) == 0)
        {
            if (org->status == 0) // registration accepted
            {
                add_actives("date",email,1);
                return 1; // Organization
            }
            else if (org->status == 1){ // status pending
                writeStr(client_fd, "Registration pending\n");
            }else { // status rejected
                writeStr(client_fd, "Registration rejected. Please re-register or contact support\n");
                remove_organization(email);
            }

            return -1;
        } else{
            writeStr(client_fd, "Wrong password\n");
            return -1;
        }
        
    }
    else
        writeStr(client_fd, authentication_error_prompt);
    return -1; // Authentication failed
}

// funtion for ONG managment for use by the admin
void manageOrganizations(int client_fd, organization* organizations, int size){

    organization* currentOrg = organizations;
    char buffer[1024];
    int nread;
    int choice;
    int count = 0;

    // sends the menu to the organization
    char* option_prompt = "\nPlease select one of the options below\n"
                          "1. Change the status to approved\n"
                          "2. Change the status to pending\n"
                          "3. Change the status to rejected\n"
                          "4. Delete organization\n"
                          "5. Next organization\n"
                          "6. Previous menu\n"
                          "Your choice(1-6): ";    

    // shows every organization in the database, one at a time and prompts the admin for the action to take
    while(count < size){

        currentOrg = organizations+count;

        // show ONG info in the screen
        writeStr(client_fd, "\nName: ");
        writeStr(client_fd, currentOrg->name);
        writeStr(client_fd, "\nTax Identification Number: ");
        snprintf(buffer, 10, "%d", currentOrg->taxIdentificationNumber);
        writeStr(client_fd, buffer);
        writeStr(client_fd, "\nEmail: ");
        writeStr(client_fd, currentOrg->email);
        writeStr(client_fd, "\nAddress: ");
        writeStr(client_fd, currentOrg->address);
        writeStr(client_fd, "\nActivity Description: ");
        //writeStr(client_fd, currentOrg->activityDescription); // not working
        writeStr(client_fd, "\nPhone Number: ");
        writeStr(client_fd, currentOrg->phoneNumber);

        switch (currentOrg->status){
            case 0: writeStr(client_fd, "\n\nStatus: Approved"); break;
            case 1: writeStr(client_fd, "\n\nStatus: Pending"); break;
            case 2: writeStr(client_fd, "\n\nStatus: Rejected"); break;
        }
        
        // prompt admin for action
        write(client_fd, option_prompt, strlen(option_prompt));
        choice = getSelectedOptionInRange(client_fd, 1, 6);

        switch(choice){

            case 1: currentOrg->status = 0; break; // change registration status to approved
            case 2: currentOrg->status = 1; break; // change registration status to pending
            case 3: currentOrg->status = 2; break; // change registration status to rejected
            case 4: remove_organization(currentOrg->email); break; // delete organization
            case 6: return; // previous menu
            default: break; // next org

        }

        update_organization(currentOrg);
        count++;

        // at the end of the ONG list, aks the admin if he wants to see the list again or go back to the previous menu
        if (count == size) {
            writeStr(client_fd, "Do you want to see the list from the top (1) or go back to the start menu (other)? Your choice: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            choice = atoi(buffer);
            if(choice == 1){
                count = 0;
            }
            else return;
        }

    }

}


// funtion for engineer managment for use by the admin
// works in the same way of the manageOrganizations
void manageEngineers(int client_fd, engineer* engineers, int size){

    engineer* currentEng = engineers;
    char buffer[1024];
    int nread;
    int choice;
    int count = 0;

    
    char* option_prompt = "\nPlease select one of the options below\n"
                          "1. Change the status to approved\n"
                          "2. Change the status to pending\n"
                          "3. Change the status to rejected\n"
                          "4. Delete engineer\n"
                          "5. Next engineer\n"
                          "6. Previous menu\n"
                          "Your choice(1-6): ";

    
    while(count < size){

        currentEng = engineers + count;

        writeStr(client_fd, "\nName: ");
        writeStr(client_fd, currentEng->name);
        writeStr(client_fd, "\nOE Number: ");
        sprintf(buffer, "%d", currentEng->number);
        writeStr(client_fd, buffer);
        writeStr(client_fd, "\nEngineering Specialty: ");
        writeStr(client_fd, currentEng->engineeringSpecialty);
        writeStr(client_fd, "\nStudent Status: ");
        if(currentEng->studentStatus){
            writeStr(client_fd, "Studying");
        }
        else writeStr(client_fd, "Not studying");
        writeStr(client_fd, "\nAreas of Expertise: ");
        writeStr(client_fd, currentEng->areasOfExpertise);
        writeStr(client_fd, "\nEmail: ");
        writeStr(client_fd, currentEng->email);
        writeStr(client_fd, "\nPhone Number: ");
        writeStr(client_fd, currentEng->phoneNumber);
        
        switch (currentEng->status){
            case 0: writeStr(client_fd, "\n\nStatus: Approved"); break;
            case 1: writeStr(client_fd, "\n\nStatus: Pending"); break;
            case 2: writeStr(client_fd, "\n\nStatus: Rejected"); break;
        }
        
        writeStr(client_fd, option_prompt);
        choice = getSelectedOptionInRange(client_fd, 1, 6);

        switch(choice){

            case 1: currentEng->status = 0; break;
            case 2: currentEng->status = 1; break;
            case 3: currentEng->status = 2; break;
            case 4: remove_engineer(currentEng->email); break; // delete engineer
            case 6: return;

        }

        update_engineer(currentEng);
        count++;

        if(count == size){
            writeStr(client_fd, "Do you want to see the list from the top (1) or go back to the start menu (2)? Your choice: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            choice = atoi(buffer);
            if(choice == 1){
                count = 0;
            }
            else return;
        }

    }

}


// funtion for challenge managment for use by the admin
// works in the same way of the manageChallenges
void manageChallenges(int client_fd, challenge* challenge_list, int size){
    
    challenge* currentChall = challenge_list;
    char buffer[1024];
    int nread;
    int choice;
    int count = 0;

    char* option_prompt = "\nPlease select one of the options below\n"
                          "1. Change the status to approved\n"
                          "2. Change the status to pending\n"
                          "3. Change the status to rejected\n"
                          "4. Next challenge\n"
                          "5. Previous menu\n"
                          "Your choice(1-5): ";

    while(count < size){

        currentChall = challenge_list + count;

        writeStr(client_fd, "\nName: ");
        writeStr(client_fd, currentChall->name);
        writeStr(client_fd, "\nDescription: ");
        writeStr(client_fd, currentChall->description);
        writeStr(client_fd, "\nEngineer Type: ");
        writeStr(client_fd, currentChall->engineerType);
        writeStr(client_fd, "\nHours: ");
        sprintf(buffer, "%d", currentChall->hours);
        writeStr(client_fd, buffer);
        writeStr(client_fd, "\nOrganization ID: ");
        sprintf(buffer, "%d", currentChall->organizationId);
        writeStr(client_fd, buffer);
        switch (currentChall->status){
            case 0: writeStr(client_fd, "\n\nStatus: Approved"); break;
            case 1: writeStr(client_fd, "\n\nStatus: Pending"); break;
            case 2: writeStr(client_fd, "\n\nStatus: Rejected"); break;
        }

        writeStr(client_fd, option_prompt);
        choice = getSelectedOptionInRange(client_fd, 1, 5);

        switch(choice){

            case 1: currentChall->status = 0; break;
            case 2: currentChall->status = 1; break;
            case 3: currentChall->status = 2; break;
            case 5: return;

        }

        update_challenge(currentChall);
        count++;

        if(count == size){
            writeStr(client_fd, "Do you want to see the list from the top (1) or go back to the start menu (2)? Your choice: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            choice = atoi(buffer);
            if(choice == 1){
                count = 0;
            }
            else return;
        }

    }
}


void applyChallenges(int client_fd, challenge* challenge_list, int size){
    challenge* currentChall = challenge_list;
    char buffer[1024];
    int nread;
    int choice;
    int count = 0;

    char* option_prompt = "\nPlease select one of the options below\n"
                          "1. Apply\n"
                          "2. Next challenge\n"
                          "3. Previous menu\n"
                          "Your choice(1-3): ";

    while(count < size){

        currentChall = challenge_list + count;

        writeStr(client_fd, "\nName: ");
        writeStr(client_fd, currentChall->name);
        writeStr(client_fd, "\nDescription: ");
        writeStr(client_fd, currentChall->description);
        writeStr(client_fd, "\nEngineer Type: ");
        writeStr(client_fd, currentChall->engineerType);
        writeStr(client_fd, "\nHours: ");
        sprintf(buffer, "%d", currentChall->hours);
        writeStr(client_fd, buffer);
        writeStr(client_fd, "\nOrganization ID: ");
        sprintf(buffer, "%d", currentChall->organizationId);
        writeStr(client_fd, buffer);

        writeStr(client_fd, option_prompt);
        choice = getSelectedOptionInRange(client_fd, 1, 3);

        switch(choice){

            case 1: /*stuffies*/ break;
            case 3: return;

        }


        if(count == size){
            writeStr(client_fd, "Do you want to see the list from the top (1) or go back to the start menu (2)? Your choice: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            choice = atoi(buffer);
            if(choice == 1){
                count = 0;
            }
            else return;
        }

    }
}
