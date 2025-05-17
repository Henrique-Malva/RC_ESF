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

// function that deletes a substring from a string
// necessary to remove applicants from challenges and challenges applied to from engineers, from the respective lists (that are strings) in the structure
// credits to Portfolio Courses' at https://www.youtube.com/watch?v=p6uqGop26es&t=342s
void deleteFromString(char string[], char substr[]){
    int i=0;
    int string_length = strlen(string);
    int substr_length = strlen(substr);

    while (i < string_length)
    {
        if (strstr(&string[i], substr) == &string[i])
        {
            string_length -= substr_length;

            for (size_t j = i; j < string_length; j++)
            {
                string[j] = string[j + substr_length];
            }
            
        }
        else i++;
    }
    string[i] = '\0';
}

void manageOrganizations(int client_fd, organization* organizations, int size);
void manageEngineers(int client_fd, engineer* engineers, int size);
void manageChallenges(int client_fd, challenge* challenge_list, int size);

int closeFunction() { return -1; }

// main client handling function where the operation flow is handled
void process_client(int client_fd) {
    int choice = 0;
    char buffer[BUF_SIZE];
    // welcome menu
    char* menu0 = "========================================================\n"
                "        Welcome to engineers without borders!\n"
                "========================================================"
                "\nPlease select an option:\n"
                "1. Login\n"
                "2. Organization Registration\n"
                "3. Engineer Registration\n"
                "4. Exit\n"
                "Enter your option (1-4): ";

    char email[100];
    int user_role;
    do{ // stays here until a correct option is selected
        
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
                send_admin_menu(client_fd, email);
                break;
            default: // no such user registered
                writeStr(client_fd, "Closing connection.\n");
                choice=4;
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
                    "1. View and apply to challenges\n"
                    "2. View application status\n"
                    "3. Update profile\n"
                    "4. Logout\n"
                    "5. Delete account\n"
                    "Enter your option (1-5): ";

    engineer* eng;
    sprintf(buffer,"where email='%s'",email);
    get_all_engineers(&eng, buffer); // gets the eng that has the given email, to have access to everything related to it
    active* a;
    get_all_actives(&a,buffer);
    challenge* c; int n;

    do // stays here until logout, after every operation comes back to this menu
    {
        write(client_fd, menu, strlen(menu));
        choice = getSelectedOptionInRange(client_fd, 1, 5);

        // add notification section or application results section something like that (F9)
        switch (choice) {
            case 1: // visualize and apply to challenges
                n = get_all_challenges(&c, "");
                applyChallenges(client_fd, c, n, &eng);
                break;
            case 2: // view status of each application
                viewApplicationStatus(client_fd, &eng);
                break;
            case 3: // update profile
                engProfileUpdate(client_fd, &eng);
                update_engineer(eng);
                strcpy(email,eng->email);
                break;
            case 4: // logout
                // guarantee that the db table is updated to mirror a logout
                a->status=0;
                update_active(a);
                close(client_fd);
                break;
            case 5: // delete account
                remove_actives(email);
                remove_engineer(email);
                close(client_fd);
                break;
            default:
                // no need for default since getSelectedOptionInRange is blocking until the option is in the correct range
                break;
        }
    } while (choice!=4 && choice!=5); // repeats unless the user logged out or deleted account
    
    
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
                "7. Delete account\n"
                "Enter your option (1-7): ";
    
    organization* org;
    challenge* c;
    sprintf(buffer,"where email='%s'",email);
    get_all_organizations(&org, buffer); // gets the org that has the given email, to have access to everything related to it
    active* a;
    get_all_actives(&a,buffer);

    int n,count;

    do // stays here until logout, after every operation comes back to this menu
    {
        writeStr(client_fd, menu);
        choice = getSelectedOptionInRange(client_fd, 1, 7);

        switch (choice) {
            case 1: // add a new challenge
                addChallengePrompt(client_fd, org);
                break;
            case 2: // list all challenges belonging to this ONG   
                sprintf(buffer,"where organization_id='%d'",org->organizationId);
                n = get_all_challenges(&c, buffer);
                count = 0;
                challenge* currentChall = c;
                while(count < n){

                    currentChall = c + count;
                    
                    // print challenge info
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
            case 3: // update a challenge
                // asks user for the wanted challenge
                writeStr(client_fd, "Challenge Name: ");
                nread = read(client_fd, name, 200 - 1);
                name[nread-2] = 0;
                
                // gets from the db the specific challenge
                sprintf(buffer,"WHERE name='%s' AND organization_id='%d'",name, org->organizationId);
                printf("\n\ncond: %s\n\n",buffer);
                get_all_challenges(&c, buffer);

                orgChallengeUpdate(client_fd,&c);

                update_challenge(c);
                break;
            case 4: // remove a challenge
                writeStr(client_fd, "Challenge name: ");
                nread = read(client_fd, name, 200 - 1);
                name[nread-2] = 0;
                sprintf(buffer,"WHERE name='%s' AND organization_id='%d'",name,org->organizationId);
                get_all_challenges(&c, buffer);
                remove_challenge(c->id);
                break;
            case 5: // view and accept applicants
                writeStr(client_fd, "not yet implemented");
                break;
            case 6: // logout
                // guarantee that the db table is updated to mirror a logout
                a->status=0;
                update_active(a);
                close(client_fd);
                break;
            case 7: // delete account
                remove_actives(email);
                sprintf(buffer,"where organization_id='%d'",org->organizationId);
                n = get_all_challenges(&c, buffer);
                count = 0;
                challenge* curChall = c;
                while (count<n)
                {
                    curChall = c + count;
                    remove_challenge(curChall->id);
                    count++;
                }
                
                remove_organization(email);

                close(client_fd);
            default: 
                break;
                
        }
    } while (choice!=6 && choice!=7); // repeats unless the user logged out or deleted account
    
    
}

// admin handling function
// presents the menu options for an admin and calls the functions responsible for the option selected by the user
void send_admin_menu(int client_fd, char* email) {
    int state = 0;
    char buffer[BUF_SIZE];

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
     

    do{ // stays here until logout, after every operation comes back to this menu
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

            case 3: // manage challenges

                challenge* c;
                n = get_all_challenges(&c, "");
                manageChallenges(client_fd, c, n);

                break;

            case 4: // logout
                // guarantee that the db table is updated to mirror a logout
                active* a;
                sprintf(buffer,"where email='%s'",email);
                get_all_actives(&a,buffer);
                a->status=0;
                update_active(a);
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
    active* actives;

    // first it will check the table of users with accepted registration
    if (get_all_actives(&actives, email_cond)) {
        // if the email is correct, asks for password
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';

        // if the password is correct
        if (strcmp(pass,actives->password)==0)
        {
            // checks if that client is already logged in

            if (actives->status==1)
            {
                writeStr(client_fd, "Client already logged in\n");
                return -1;
            }else{
                // if it isn't, updates the status and the last_login
                actives->status=1;
                strcpy(actives->last_login,"date");
                update_active(actives);

                return actives->role;
            }
            
        }else{ // if the password is incorrect
            
            if (actives->role!=2) // informs the user of incorrect password only if it isn't an admin
            // to prevent common users from finding out an admin login
            {
                writeStr(client_fd, "Wrong password\n");
            }else{
                printf("Attempted admin login. Wrong password\n");
                writeStr(client_fd, authentication_error_prompt);
            }
            
            return -1;
        }

        // if there isn't anything in the actives it will check engineer and organization tables
        // this tables save the registration status
    }else if(get_all_engineers(&eng, email_cond)){
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';
        
        if (strcmp(pass,eng->password) == 0)
        {
            if (eng->status == 1){ // status pending
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
    }else if(get_all_organizations(&org, email_cond)){
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';

        if (strcmp(pass,org->password) == 0)
        {
            if (org->status == 1){ // status pending
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
    char* option_prompt = "\n\nPlease select one of the options below\n"
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
        writeStr(client_fd, currentOrg->activityDescription);
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

            case 1: 
                if (currentOrg->status != 0) // prevents adding to clients table if it's an already accepted registration
                {
                    currentOrg->status = 0; 
                    add_actives("date",currentOrg->email,currentOrg->password,1,0); 
                }
                break; // change registration status to approved
            case 2: currentOrg->status = 1; remove_actives(currentOrg->email); break; // change registration status to pending and removes from accepted clients
            case 3: currentOrg->status = 2; remove_actives(currentOrg->email); break; // change registration status to rejected and removes from accepted clients
            case 4: remove_actives(currentOrg->email); remove_organization(currentOrg->email); break; // delete organization
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

            case 1: 
                if (currentEng->status != 0)
                {
                    currentEng->status = 0; 
                    add_actives("date", currentEng->email, currentEng->password, 0, 0); 
                }
                
            break;
            case 2: currentEng->status = 1; remove_actives(currentEng->email); break;
            case 3: currentEng->status = 2; remove_actives(currentEng->email); break;
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
// works in the same way of the manageOrganizations, except there is no manipulation of the clients table
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

// function that allows engineers to visualize the challenges available and apply to them
void applyChallenges(int client_fd, challenge* challenge_list, int size, engineer** eng){
    challenge* currentChall = challenge_list;
    char buffer[1024], auxStr[600];
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
        
        writeStr(client_fd, option_prompt);
        choice = getSelectedOptionInRange(client_fd, 1, 3);

        switch(choice){

            case 1: 
                // formatting of the applicants string for the challenges
                if (strcmp(currentChall->applicants,"")==0) 
                {
                    sprintf(auxStr,"%d:0",(*eng)->id);
                }else{
                    sprintf(auxStr,"%s,%d:0",currentChall->applicants,(*eng)->id);
                }

                // copies to the challenge struct and updates the db table
                strcpy(currentChall->applicants,auxStr);
                update_challenge(currentChall);

                // formatting of the challenges string for the engineers
                if (strcmp((*eng)->chal,"")==0)
                {
                    sprintf(auxStr,"%d:0",currentChall->id);
                }else{
                    sprintf(auxStr,"%s,%d:0",(*eng)->chal,currentChall->id);
                }
                // copies to the engineer struct and updates the db table
                strcpy((*eng)->chal,auxStr);
                update_engineer((*eng));

                break;
            case 3: return;

        }

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

// function to allow a user-friendly way of updating challenges for organizations
// instead of having to retype every field, the user chooses the field to update and is prompted to do so until satisfied
void orgChallengeUpdate(int client_fd, challenge** chall){
    char* chUpPrompt = "\n1: Name\n2: Description\n3: Engineer Type\n4: Hours\nOther: None\n"
                    "\nSelect field to update: ";
    int choice, nread;
    uint8_t leave=0;
    char buffer[BUF_SIZE];
    do // stays here until the user is satisfied with the updated fields
    {
        writeStr(client_fd, chUpPrompt);
        choice = getSelectedOptionInt(client_fd);

        switch (choice) // after prompting the user for the field to be updates, asks for the new information and modifies the challenge
        // only after returning from this function is the database updated
        {
        case 1:
            writeStr(client_fd, "\nChallenge Name: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*chall)->name,buffer);

            break;
        case 2:
            writeStr(client_fd, "\nChallenge Description: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*chall)->description,buffer);
            break;
        case 3:
            writeStr(client_fd, "\nEngineer Type: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*chall)->engineerType,buffer);
            break;
        case 4:
            writeStr(client_fd, "\nChallenge Hours: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            (*chall)->hours=atoi(buffer);
            break;
        
        default:
            leave=1;
            break;
        }

    } while (leave==0);    

}

// function to allow a user-friendly way of updating the profile for engineers)
// instead of having to retype every field, the user chooses the field to update and is prompted to do so until satisfied
void engProfileUpdate(int client_fd, engineer** eng){
    char* chUpPrompt = "\n1: Name\n2: Email\n3: Specialty\n4: Institution\n5: Areas Of Expertise\n6: Phone\n7: Password\n8: Student Status\nOther: None\n"
                    "\nSelect field to update: ";
    int choice, nread, check=0;
    uint8_t leave=0;
    char buffer[BUF_SIZE], cond[BUF_SIZE];
    engineer* engs;
    active* act;
    sprintf(cond,"where email='%s'",(*eng)->email);
    get_all_actives(&act,cond);

    do // stays here until the user is satisfied with the updated fields
    {
        writeStr(client_fd, chUpPrompt);
        choice = getSelectedOptionInt(client_fd);

        switch (choice) // after prompting the user for the field to be updates, asks for the new information and modifies the challenge
        // only after returning from this function is the database updated
        {
        case 1:
            writeStr(client_fd, "\nName: ");
            nread = read(client_fd, buffer, 100-1);
            buffer[nread-2] = '\0';

            strcpy((*eng)->name,buffer);

            break;
        case 2:
            do
            {
                if (check)
                {
                    write(client_fd, "This email is already in use\n\n", strlen("This email is already in use\n\n"));
                }
                check=1;

                writeStr(client_fd, "\nEmail: ");
                nread = read(client_fd, buffer, 100 - 1);
                buffer[nread - 2] = '\0';

                sprintf(cond,"where email='%s'", buffer);
            } while (get_all_engineers(&engs,cond));
            check=0;
            strcpy(act->email,buffer);
            strcpy((*eng)->email,buffer);
            break;
        case 3:
            writeStr(client_fd, "\nSpeciality: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*eng)->engineeringSpecialty,buffer);
            break;
        case 4:
            writeStr(client_fd, "\nInstitution of employment: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*eng)->employmentInstitution,buffer);
            break;
        case 5:
            writeStr(client_fd, "\nAreas of expertise: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*eng)->areasOfExpertise,buffer);
            break;
        case 6:
            writeStr(client_fd, "\nPhone: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*eng)->phoneNumber,buffer);
            break;
        case 7:
            writeStr(client_fd, "\nPassword: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            strcpy((*eng)->password,buffer);
            strcpy(act->password,buffer);
            break;

        case 8:
            writeStr(client_fd, "\nStudent Status: ");
            nread = read(client_fd, buffer, BUF_SIZE-1);
            buffer[nread-2] = '\0';
            (*eng)->studentStatus=atoi(buffer);
            break;
        
        default:
            leave=1;
            break;
        }

    } while (leave==0);
    // updates the active client referent to the logged in engineer   
    update_active(act);
}

// funtion to allow the engineer to know the status of his application in the challenges he applied to
void viewApplicationStatus(int client_fd, engineer** eng){
    char chals[600], *indiviChal, auxStr[20];
    int chal_ID, status, chal_nr=0, removals=0;
    strcpy(chals,(*eng)->chal);
    challenge *c;


    indiviChal = strtok(chals,":");
    while (indiviChal != NULL)
    {
        chal_nr++;
        chal_ID=atoi(indiviChal);
        sprintf(auxStr,"where id=%d",chal_ID);
        get_all_challenges(&c,auxStr);
        
        writeStr(client_fd, "\nChallenge Name: ");
        writeStr(client_fd, c->name);
        writeStr(client_fd, "\nDescription: ");
        writeStr(client_fd, c->description);
        writeStr(client_fd, "\nEngineer Type: ");
        writeStr(client_fd, c->engineerType);
        writeStr(client_fd, "\nHours: ");
        sprintf(auxStr, "%d", c->hours);
        writeStr(client_fd, auxStr);

        indiviChal = strtok(NULL, ",");
        status=atoi(indiviChal);
        indiviChal = strtok (NULL, ":");
        switch (status)
        {
        case 0: // pending
            writeStr(client_fd,"\nStatus: Pending\n\n");
            break;
        case 1: // accepted
            writeStr(client_fd,"\nStatus: Approved\n\n");
            break;
        case 2: // rejected
            writeStr(client_fd,"\nStatus: Rejected\n\n");
            if (!removals)
            {
                removals=1;
            }

            // verification of the position of the challenge to be removed in the challenge list to format correctly the substring to be removed
            if (indiviChal == NULL) // if it is the last piece of the string
            {
                if (chal_nr==1) // and also the first one -> no commas to worry about
                {
                    sprintf(auxStr,"%d:%d",chal_ID,status);
                    deleteFromString((*eng)->chal,auxStr);
                }else{ // not the first -> have to remove the comma that is behind
                    sprintf(auxStr,",%d:%d",chal_ID,status);
                    deleteFromString((*eng)->chal,auxStr);
                }
            }else{ // if it is somewhere in the middle -> remove the comma that is in front
                sprintf(auxStr,"%d:%d,",chal_ID,status);
                deleteFromString((*eng)->chal,auxStr);
            }
            break;
        default:
            break;
        }
    }

    // updates the challenge list on the engineer only if there were removals
    if (removals)
    {
        update_engineer((*eng));
    } 
}