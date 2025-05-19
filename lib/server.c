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

            for (int j = i; j < string_length; j++)
            {
                string[j] = string[j + substr_length];
            }
            
        }
        else i++;
    }
    string[i] = '\0';
}

void kill_challenge(challenge** c){
    char apps[6*MAX_ENGINEERS], *indiviEng, auxStr[20], *auxPnt;
    int eng_ID, index; //, status;
    engineer* e;

    strcpy(apps,(*c)->applicants);
    indiviEng = strtok(apps,":");
    while (indiviEng != NULL) // go to every engineer that applied
    {
        eng_ID=atoi(indiviEng); // get its ID
        sprintf(auxStr,"where id=%d",eng_ID);
        get_all_engineers(&e,auxStr); // and the engineer itself

        indiviEng = strtok (NULL, ":"); // gets the next applicant on indiviEng

        // if a challenge is removed it makes sense that its applicants now that their application wasn't rejected
        // but that the challenge itself was removed
        // so a new challenge application status to reflect that
        sprintf(auxStr,"%d:",(*c)->id);
        auxPnt = strstr(e->chal,auxStr);
        index = auxPnt - e->chal; // this index corresponds to the first char of chalID: on the applied to list
        // go to the status index
        index=index+strlen(auxStr); // example 452:(status) at index 4. has size 4. if the 4 is at index 4 then (status) is at index 8. index(4) + size(4) = stat_index(8)
        // replace
        (e->chal)[index] = '3';

        update_engineer(e);
    }

    remove_challenge((*c)->id);
}

// function to remove a organization from the database
void kill_organization(organization** o){
    challenge* c;
    char cond[BUF_SIZE];
    int chal_nr, count=0;
    sprintf(cond,"where organization_id='%d'",(*o)->organizationId);
    chal_nr=get_all_challenges(&c,cond);
    
    // to remove a org, all of its associated challenges also have to be removed
    while (count<chal_nr)
    {
        kill_challenge(&c);
        count++;
    }

    remove_actives((*o)->email);
    remove_organization((*o)->email);
    
}

// function to remove a engineer and every trace of it from the database
// in the future if an engineer that is accepted to a challenge deletes his account the organization should be able to know that he exited the platform
// (basically repeat )
void kill_engineer(engineer** e){
    char chals[6*MAX_CHALLENGES], *indiviChal, auxStr[20], *auxPnt;
    int chal_ID, status, nr, index;
    strcpy(chals,(*e)->chal);
    challenge *c;


    indiviChal = strtok(chals,":");
    
    while (indiviChal != NULL) // goes to every challenge this engi applied to
    {
        chal_ID=atoi(indiviChal);
        sprintf(auxStr,"where id=%d",chal_ID);
        nr = get_all_challenges(&c,auxStr);

        indiviChal = strtok(NULL, ",");
        status=atoi(indiviChal);
        indiviChal = strtok (NULL, ":");

        // remove the application from the challenge list, only necessary if not rejected and the challenge still exists
        // for the future: if the status is accepted instead of removing just changing the status value on the applicant list and make code to show the org that that status value indicates that an accepted candidate removed its profile
        if (status!=2 && nr!=0)
        {
            // find the application on the applicants list of the challenge
            sprintf(auxStr,"%d:%d",(*e)->id,status);
            auxPnt = strstr(c->applicants,auxStr);
            index = auxPnt - c->applicants; // this index is the first one of engId:status

            
            // verification of the position of the application to be removed in the applicants list to format correctly the substring to be removed
            if ( (strlen(c->applicants)-1) == (index + strlen(auxStr)-1)) // if it is the last piece of the string
            // if the last index of the applicants list is equal to the status index on engID:status, the applicant is in the last position
            {
                if (index==0) // and also the first one -> no commas to worry about
                {
                    deleteFromString(c->applicants,auxStr);
                }else{ // not the first -> have to remove the comma that is behind
                    sprintf(auxStr,",%d:%d",(*e)->id,status);
                    deleteFromString(c->applicants,auxStr);
                }
            }else{ // if it is somewhere in the middle -> remove the comma that is in front
                sprintf(auxStr,"%d:%d,",(*e)->id,status);
                deleteFromString(c->applicants,auxStr);
            }

            update_challenge(c);

        }    

    }

    remove_actives((*e)->email);
    remove_engineer((*e)->email);
}

int closeFunction() { return -1; }

// main client handling function where the operation flow is handled
void process_client(int client_fd) {
    int choice = 0;
    // welcome menu
    char* menu0="\n\n========================================================\n"
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
            writeStr(client_fd, "\n\n==================================================================\n"
                                "                      NON-PROFIT REGISTRATION \n"
                                "         PLEASE ENTER THE DATA OF YOUR NON-PROFIT ORGANIZATION \n"
                                "==================================================================\n\n"
                                " > Non-Profit Organization Registration:\n");
            organizationRegister(client_fd);
            break;
        case 3: // registration of engineer
            system("cls");
            writeStr(client_fd, "\n\n==================================================================\n"
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
    char* menu= "\n\n========================================================\n"
                "                 Welcome Engineer!\n"
                "========================================================\n"
                "Please select an option:\n"
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
                kill_engineer(&eng);
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
    char buffer[BUF_SIZE];
    int choice;
    
    // sends menu
    char* menu= "\n\n========================================================\n"
                "          Welcome Non-Profit Organization!\n"
                "========================================================\n"
                "Please select an option:\n"
                "1. Add a new challenge\n"
                "2. List and operate on challenges\n"
                "3. Logout\n"
                "4. Delete account\n"
                "Enter your option (1-4): ";
    
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
        choice = getSelectedOptionInRange(client_fd, 1, 4);

        switch (choice) {
            case 1: // add a new challenge
                addChallengePrompt(client_fd, org);
                break;
            case 2: // list all challenges belonging to this ONG, one at a time and ask the user to choose what to do next (view next challenge or operate on the current one)
                char* option_prompt = "\n\nPlease select one of the options below\n"
                    "1. Update challenge\n"
                    "2. View applicants\n"
                    "3. Delete challenge\n"
                    "4. Next challenge\n"
                    "5. Previous menu\n"
                    "Your choice(1-5): ";

                sprintf(buffer,"where organization_id='%d'",org->organizationId);
                n = get_all_challenges(&c, buffer);
                count = 0;
                int chal_choice, goback=0;
                challenge* currentChall = c;

                if (n==0)
                {
                    writeStr(client_fd,"\nYou have no registered challenges\n");
                }

                while(count < n){

                    currentChall = c + count;
                    
                    // print challenge info
                    printChal(client_fd, currentChall, 1);

                    writeStr(client_fd, option_prompt);
                    chal_choice = getSelectedOptionInRange(client_fd, 1, 5);

                    switch(chal_choice){
                        case 1: 
                            orgChallengeUpdate(client_fd, &currentChall);
                            update_challenge(currentChall);
                            break;
                        case 2: 
                            int app_again;
                            do
                            {
                                manageApplications(client_fd, &currentChall);
                                writeStr(client_fd, "\nDo you want to see the APPLICATION list from the top (1) or go back to the previous menu (other)? Your choice: ");
                                app_again = getSelectedOptionInt(client_fd);
                                if (app_again!=1)
                                {
                                    app_again=0;
                                }
                                
                            } while (app_again);

                            break;
                        case 3:
                            kill_challenge(&currentChall);
                            //remove_challenge(currentChall->id);
                            break;
                        case 5: goback=1; break;

                    }
                    if (goback)
                    {
                        break;
                    }
                    
                    count++;
                    
                    if(count == n){
                        writeStr(client_fd, "\nDo you want to see the CHALLENGE list from the top (1) or go back to the previous menu (other)? Your choice: ");
                        chal_choice = getSelectedOptionInt(client_fd);
                        if(chal_choice == 1){
                            count = 0;
                        }
                        else break;
                    }
                }

                break;
            case 3: // logout
                // guarantee that the db table is updated to mirror a logout
                a->status=0;
                update_active(a);
                close(client_fd);
                break;
            case 4: // delete account
                
                kill_organization(&org);

                close(client_fd);
            default: 
                break;
                
        }
    } while (choice!=4 && choice!=5); // repeats unless the user logged out or deleted account
    
    
}

// admin handling function
// presents the menu options for an admin and calls the functions responsible for the option selected by the user
void send_admin_menu(int client_fd, char* email) {
    int state = 0;
    char buffer[BUF_SIZE];

    // sends menu
    char* menu0="\n\n========================================================\n"
                "                       Welcome Admin!\n"
                "========================================================\n"
                "Please select an option:\n"
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

    char* login_prompt = "\n\nEnter your login email: ";
    char* password_prompt = "Enter your password: ";
    char* authentication_error_prompt = "User not registered\n";
    writeStr(client_fd, login_prompt);

    char buffer[BUF_SIZE];
    char email_cond[BUF_SIZE];
    int nread = read(client_fd, buffer, BUF_SIZE - 1);
    buffer[nread - 2] = '\0';
    strcpy(email,buffer);

    char pass[200];
    unsigned char hash[SHA256_DIGEST_LENGTH];
    char hexstr[SHA256_DIGEST_LENGTH*2+1];
    engineer* eng;
    organization* org;
    sprintf(email_cond,"where email='%s'",email);
    active* actives;

    // obtaining current date
    time_t now;
    struct tm *info_time;

    time(&now);
    info_time = localtime(&now);

    // first it will check the table of users with accepted registration
    if (get_all_actives(&actives, email_cond)) {
        // if the email is correct, asks for password
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';

        if (actives->role!=2)
        {
            SHA256((unsigned char*)pass, strlen(pass), hash);
            // Convert binary hash to hex string
            for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                sprintf(hexstr + (i * 2), "%02x", hash[i]);
            }
            hexstr[SHA256_DIGEST_LENGTH * 2] = '\0';
        }else{
            strcpy(hexstr,pass); // admin passes are not hashed
        }
        

        // if the password is correct
        if (strcmp(hexstr,actives->password)==0)
        {
            // checks if that client is already logged in

            if (actives->status==1)
            {
                writeStr(client_fd, "\nClient already logged in\n");
                return -1;
            }else{
                // if it isn't, updates the status and the last_login
                actives->status=1;
                // format the date
                sprintf(buffer,"%02d/%02d/%04d", info_time->tm_mday, info_time->tm_mon + 1, info_time->tm_year + 1900);
                strcpy(actives->last_login,buffer);
                update_active(actives);

                return actives->role;
            }
            
        }else{ // if the password is incorrect
            
            if (actives->role!=2) // informs the user of incorrect password only if it isn't an admin
            // to prevent common users from finding out an admin login
            {
                writeStr(client_fd, "\nWrong password\n");
            }else{
                printf("\nAttempted admin login. Wrong password\n");
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

        SHA256((unsigned char*)pass, strlen(pass), hash);
        // Convert binary hash to hex string
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(hexstr + (i * 2), "%02x", hash[i]);
        }
        hexstr[SHA256_DIGEST_LENGTH * 2] = '\0';
        
        if (strcmp(hexstr,eng->password) == 0)
        {
            if (eng->status == 1){ // status pending
                writeStr(client_fd, "\nRegistration pending\n");
            }else { // status rejected
                writeStr(client_fd, "\nRegistration rejected. Please re-register or contact support\n");
                kill_engineer(&eng);
            }
            return -1;
            
        }else{
            writeStr(client_fd, "\nWrong password\n");
            return -1;
        }
    }else if(get_all_organizations(&org, email_cond)){
        writeStr(client_fd, password_prompt);
        nread = read(client_fd, pass, 200 - 1);
        pass[nread - 2] = '\0';

        SHA256((unsigned char*)pass, strlen(pass), hash);
        // Convert binary hash to hex string
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(hexstr + (i * 2), "%02x", hash[i]);
        }
        hexstr[SHA256_DIGEST_LENGTH * 2] = '\0';

        if (strcmp(hexstr,org->password) == 0)
        {
            if (org->status == 1){ // status pending
                writeStr(client_fd, "\nRegistration pending\n");
            }else { // status rejected
                writeStr(client_fd, "\nRegistration rejected. Please re-register or contact support\n");
                // even though this organization removal is from a rejected registration
                // since the admin can change the registration from accepted to rejected
                // theres a possibility the organization interacted with the system, so full removal is required
                kill_organization(&org);
                //remove_organization(email);
            }

            return -1;
        } else{
            writeStr(client_fd, "\nWrong password\n");
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
    
    if (size==0)
    {
        writeStr(client_fd,"\nThere are no organizations registered\n");
    }

    // shows every organization in the database, one at a time and prompts the admin for the action to take
    while(count < size){

        currentOrg = organizations+count;

        // show ONG info in the screen
        printOrg(client_fd,currentOrg);
        
        // prompt admin for action
        write(client_fd, option_prompt, strlen(option_prompt));
        choice = getSelectedOptionInRange(client_fd, 1, 6);

        switch(choice){

            case 1: 
                if (currentOrg->status != 0) // prevents adding to clients table if it's an already accepted registration
                {
                    currentOrg->status = 0; 
                    // no login ever, date is just date
                    add_actives("date",currentOrg->email,currentOrg->password,1,0); 
                }
                break; // change registration status to approved
            case 2: currentOrg->status = 1; remove_actives(currentOrg->email); break; // change registration status to pending and removes from accepted clients
            case 3: currentOrg->status = 2; remove_actives(currentOrg->email); break; // change registration status to rejected and removes from accepted clients
            case 4: kill_organization(&currentOrg); break; // delete organization
            case 6: return; // previous menu
            default: break; // next org

        }

        if (choice!=4)
        {
            update_organization(currentOrg);
        }
        
        count++;

        // at the end of the ONG list, aks the admin if he wants to see the list again or go back to the previous menu
        if (count == size) {
            writeStr(client_fd, "\nDo you want to see ORGANIZATION list from the top (1) or go back to the previous menu (other)? Your choice: ");
            choice = getSelectedOptionInt(client_fd);
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
    int choice;
    int count = 0;

    
    char* option_prompt = "\n\nPlease select one of the options below\n"
                          "1. Change the status to approved\n"
                          "2. Change the status to pending\n"
                          "3. Change the status to rejected\n"
                          "4. Delete engineer\n"
                          "5. Next engineer\n"
                          "6. Previous menu\n"
                          "Your choice(1-6): ";

    if (size==0)
    {
        writeStr(client_fd,"\nThere are no engineers registered\n");
    }

    while(count < size){

        currentEng = engineers + count;

        printEng(client_fd,currentEng,1);
        
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
            case 4: kill_engineer(&currentEng); break; // delete engineer
            case 6: return;
            default: break; // next eng
        }

        if (choice!=4)
        {
            update_engineer(currentEng);
        }

        
        count++;

        if(count == size){
            writeStr(client_fd, "\nDo you want to see the ENGINEER list from the top (1) or go back to the previous menu (other)? Your choice: ");
            choice = getSelectedOptionInt(client_fd);
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
    int choice;
    int count = 0;

    char* option_prompt = "\n\nPlease select one of the options below\n"
                          "1. Change the status to approved\n"
                          "2. Change the status to pending\n"
                          "3. Change the status to rejected\n"
                          "4. Next challenge\n"
                          "5. Previous menu\n"
                          "Your choice(1-5): ";

    if (size==0)
    {
        writeStr(client_fd,"\nThere are no challenges\n");
    }

    while(count < size){

        currentChall = challenge_list + count;

        printChal(client_fd, currentChall, 1);

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
            writeStr(client_fd, "\nDo you want to see the CHALLENGE list from the top (1) or go back to the previous menu (other)? Your choice: ");
            choice = getSelectedOptionInt(client_fd);
            if(choice == 1){
                count = 0;
            }
            else return;
        }

    }
}

// function that allows engineers to visualize the challenges available and apply to them
///////////////////prevent duplicate applications
void applyChallenges(int client_fd, challenge* challenge_list, int size, engineer** eng){
    challenge* currentChall = challenge_list;
    char auxStr[6*MAX_CHALLENGES], *chal=NULL;
    int choice;
    int count = 0;

    char* option_prompt = "\nPlease select one of the options below\n"
                          "1. Apply\n"
                          "2. Next challenge\n"
                          "3. Previous menu\n"
                          "Your choice(1-3): ";
    if (size==0) // problem: if there are only pending challenges
    {
        writeStr(client_fd,"\nThere are no challenges at the moment\n");
    }
    
    while(count < size){

        currentChall = challenge_list + count;

        if (currentChall->status==0) // if the challenge is accepted by the admin
        {
            printChal(client_fd,currentChall,0);
        
            writeStr(client_fd, option_prompt);
            choice = getSelectedOptionInRange(client_fd, 1, 3);

            switch(choice){

                case 1: 
                    // first searches the challenges applied to list to make sure the engineer hasn't already applied to this challenge
                    sprintf(auxStr,"%d:",currentChall->id);
                    chal = strstr((*eng)->chal,auxStr); // if the result is null it means this challenge is still not apply to by this engineer

                    if (chal==NULL)
                    {
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
                        strcpy((*eng)->chal, auxStr);
                        update_engineer((*eng));
                    }else{
                        writeStr(client_fd,"\nAlready applied to this challenge\n");
                    }
                    

                    break;
                case 3: return;

            }

            
        }

        count++;

        if(count == size){
            writeStr(client_fd, "\nDo you want to see the CHALLENGE list from the top (1) or go back to the previous menu (other)? Your choice: ");
            choice = getSelectedOptionInt(client_fd);
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
    char* chUpPrompt = "\n\n1: Name\n2: Description\n3: Engineer Type\n4: Hours\nOther: None\n"
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
    char* chUpPrompt = "\n\n1: Name\n2: Email\n3: Specialty\n4: Institution\n5: Areas Of Expertise\n6: Phone\n7: Password\n8: Student Status\nOther: None\n"
                    "\nSelect field to update: ";
    int choice, nread, check=0;
    uint8_t leave=0;
    char buffer[BUF_SIZE], cond[BUF_SIZE+15];
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
                    writeStr(client_fd, "\nThis email is already in use\n\n");
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
    char chals[6*MAX_CHALLENGES], *indiviChal, auxStr[20];
    int chal_ID, status, chal_nr=0, removals=0;
    strcpy(chals,(*eng)->chal);
    challenge *c;


    indiviChal = strtok(chals,":");

    if (indiviChal==NULL)
    {
        writeStr(client_fd,"\nYou have not applied to any challenges\n");
    }
    
    while (indiviChal != NULL)
    {
        chal_nr++;
        chal_ID=atoi(indiviChal);
        sprintf(auxStr,"where id=%d",chal_ID);
        get_all_challenges(&c,auxStr);

        

        indiviChal = strtok(NULL, ",");
        status=atoi(indiviChal);
        indiviChal = strtok (NULL, ":");

        if (status!=3)
        {
            printChal(client_fd,c,0);
        }

        switch (status)
        {
        case 0: // pending
            writeStr(client_fd,"\nApplication status: Pending\n\n");
            break;
        case 1: // accepted
            writeStr(client_fd,"\nApplication status: Approved\n\n");
            break;
        case 2: // rejected
        case 3: // challenge removed
            if (status==2)
            {
                writeStr(client_fd,"\nApplication status: Rejected\n\n");
            }else{
                writeStr(client_fd,"\nThis Challenge was Removed\n\n");
            }
            
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

// funtion that allows organizations to visualize who applied to a given challenge, and to decide on each applications approval status
void manageApplications(int client_fd, challenge **c){

    char* option_prompt = "\n\nPlease select one of the options below\n"
                          "1. Change the status to pending\n"
                          "2. Change the status to approved\n"
                          "3. Change the status to rejected\n"
                          "4. Next applicant\n"
                          //"5. Previous menu\n"
                          "Your choice(1-4): ";

    char app[6*MAX_ENGINEERS], *indiviEng, auxStr[20], *auxPnt;
    int eng_ID, status, eng_nr=0, changes=0, choice, new_status, index;
    strcpy(app,(*c)->applicants);
    engineer *e;


    indiviEng = strtok(app,":");

    if (indiviEng==NULL)
    {
        writeStr(client_fd,"\nThere are no applicants for this challenge\n");
    }

    while (indiviEng != NULL) // for every engineer in the applicant list of the given challenge
    {
        eng_nr++;
        eng_ID=atoi(indiviEng);
        sprintf(auxStr,"where id=%d",eng_ID);
        get_all_engineers(&e,auxStr);

        // prints the engineer info
        printEng(client_fd,e,0);
        

        // finds and prints the status of its application
        indiviEng = strtok(NULL, ",");
        status=atoi(indiviEng);
        indiviEng = strtok (NULL, ":");
        switch (status)
        {
        case 0: // pending
            writeStr(client_fd,"\nApplication status: Pending\n\n");
            break;
        case 1: // accepted
            writeStr(client_fd,"\nApplication status: Approved\n\n");
            break;
        default:
            break;
        }

        // prompts the user for the new approval status
        writeStr(client_fd, option_prompt);
        choice = getSelectedOptionInRange(client_fd, 1, 4);

        switch(choice){

            case 1: new_status = 0; break; // pending
            case 2: new_status = 1; break; // accepted
            case 3: new_status = 2; break; // rejected
            //case 5: break;
        }

        // and now updates the applicants list for the challenge and the challenges applied to list for the engineer

        // changes on the applicants list
        switch (new_status)
        {
        case 0: // pending
        case 1: // accepted
            if (new_status!=status)
            {   
                // find index of the engID:status on the applicant list
                sprintf(auxStr,"%d:%d",eng_ID,status);
                auxPnt = strstr((*c)->applicants,auxStr);
                index = auxPnt - (*c)->applicants; // this index corresponds to the first char of engID:status on the applicant list
                // go to the status index
                index=index+strlen(auxStr)-1; // example 452:0 at index 4. has size 5. if the 4 is at index 4 then 0 is at index 8. index(4) + size(5) -1 = stat_index(8)
                // replace
                ((*c)->applicants)[index] = new_status+48;
                if (!changes)
                {
                    changes=1;
                }
            }
            break;
        case 2: // rejected
            // since when the status of an applicant is changed to rejected is application is immediately removed from the list
            // it will never happen that the old status is rejected
            if (!changes)
            {
                changes=1;
            }
            
            // verification of the position of the applicants to be removed in the applicant list to format correctly the substring to be removed
            if (indiviEng == NULL) // if it is the last piece of the string
            {
                if (eng_nr==1) // and also the first one -> no commas to worry about
                {
                    sprintf(auxStr,"%d:%d",eng_ID,status);
                    deleteFromString((*c)->applicants,auxStr);
                }else{ // not the first -> have to remove the comma that is behind
                    sprintf(auxStr,",%d:%d",eng_ID,status);
                    deleteFromString((*c)->applicants,auxStr);
                }
            }else{ // if it is somewhere in the middle -> remove the comma that is in front
                sprintf(auxStr,"%d:%d,",eng_ID,status);
                deleteFromString((*c)->applicants,auxStr);
            }
            break;
        default:
            break;
        }
    
        // changes on the challenges list for the engineer
        if (new_status!=status) // only has to change on the engineers if there was a change on the status
        {
            // find index of the cahlID:status on the challenges list
            sprintf(auxStr,"%d:%d",(*c)->id,status);
            auxPnt = strstr(e->chal,auxStr);
            index = auxPnt - e->chal; // this index corresponds to the first char of engID:status on the applicant list
            // go to the status index
            index=index+strlen(auxStr)-1; // example 452:0 at index 4. has size 5. if the 4 is at index 4 then 0 is at index 8. index(4) + size(5) -1 = stat_index(8)
            // replace
            (e->chal)[index] = new_status+48;
            if (!changes)
            {
                changes=1;
            }
            update_engineer(e);
        }
        
    }

    // updates the applicant list on the challenge only if there were changes made
    if (changes)
    {
        update_challenge((*c));
    } 

}