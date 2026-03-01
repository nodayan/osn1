#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>     // voor stat()
#include <sys/wait.h>     // voor: wait()

void print_prompt() {
    char cwd[1024];                        
    if (getcwd(cwd, sizeof(cwd)) == NULL) { 
        strcpy(cwd, "couldn't get current directory");                
    }

    printf("%s$ ", cwd);                   
    fflush(stdout);                        
}

bool read_input(char *userinput) {   
    print_prompt();
    if (fgets(userinput, 1024, stdin) == NULL) {
        printf("\n");
        return false; 
    }
    return true;
}

int tokenized(char *userinput, char *tokens[]) {
    int teller = 0;                          

    char *tok = strtok(userinput, " \t\r\n");

    while (tok != NULL && teller < 511) {      
        tokens[teller] = tok;                 
        teller++;                          
        tok = strtok(NULL, " \t\r\n");        
    }

    tokens[teller] = NULL;                   
    return teller;                         
}


// =======================
// NEW FUNCTION (TASK 2)
// =======================

char* path_command(char* command) {

    // voor commands met slash gelijk return
    if (strchr(command, '/') != NULL) {
        return strdup(command);
    }

    char *path_env = getenv("PATH");
    if (path_env == NULL) return NULL;

    // copie maken
    char *path_copy = strdup(path_env);
    if (path_copy == NULL) return NULL;

    char *dir = strtok(path_copy, ":");

    while (dir != NULL) {

        char test_path[1024];

        // full path voor bv: /usr/bin/ls
        snprintf(test_path, sizeof(test_path), "%s/%s", dir, command);

        struct stat st;

        // file exist check
        if (stat(test_path, &st) == 0) {
            free(path_copy);
            return strdup(test_path);
        }

        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

// maakt een functie mogelijk (beetje van mijn vorig jaar project gepakt)
void spawn(char *fullpath, char *tokens[], int in_fd, int out_fd) {

    pid_t pid = fork();

    if (pid == 0) {  // CHILD

        // If input is not standard input, redirect
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        // If output is not standard output, redirect
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        execv(fullpath, tokens);

        perror("execv failed");
        exit(1);
    }
}


int main() {
    char userinput[1024];

    while (true) {
        if (!read_input(userinput))
        	break;

        char *commands[100];
        int ncommands = 0;

        char *cmd = strtok(userinput, "|");
        //commands in array zetten en opsplitsen
        while (cmd != NULL && ncommands < 100) {

            // spaties weghalen, weet niet of dit helemaal klopt
            while (*cmd == ' ') cmd++;

            // Lege commandos
            if (*cmd == '\0') {
                printf("Invalid pipeline\n");
                ncommands = 0;
                break;
            }

            commands[ncommands++] = cmd;
            cmd = strtok(NULL, "|");
        }
    
    if (ncommands == 0) continue;
    if (ncommands == 1) {

        char *tokens[512];
        int ntokens = tokenized(commands[0], tokens);
        if (ntokens == 0) continue;

        if (strcmp(tokens[0], "exit") == 0) break;

        if (strcmp(tokens[0], "cd") == 0) {
            char *target = tokens[1];
            if (target == NULL) target = getenv("HOME");
            if (target == NULL) target = "/";
            if (chdir(target) != 0) perror("cd");
            continue;
        }

        char *fullpath = path_command(tokens[0]);
        if (fullpath == NULL) {
            printf("Command not found\n");
            continue;
        }

        spawn(fullpath, tokens, STDIN_FILENO, STDOUT_FILENO);
        wait(NULL);
        free(fullpath);
    }
    //meerdere pipelines implementatie MOET NOG
    else {

    }
    



    }
}








    /* WEGECOMMENT WAT EERST MAIN WAS HAAL DEZE REGEL WEG VOOR WERKENDE MAIN 
	char userinput[1024];

    while (true) {
        if (!read_input(userinput))
        	break;

        char *tokens[512];				
        int ntokens = tokenized(userinput, tokens);
        if (ntokens == 0) continue;

        if (strcmp(tokens[0], "exit") == 0) {
            break;
        }

        if (strcmp(tokens[0], "cd") == 0) {
            char *target = tokens[1];
            if (target == NULL) target = getenv("HOME");
            if (target == NULL) target = "/";
            if (chdir(target) != 0) perror("cd");
            continue;
        }

        // ============================
        // NEW test voor task 2
        // ============================
        
        char *fullpath = path_command(tokens[0]);

        if (fullpath == NULL) {
            printf("Command not found\n");
            continue;
        }

        // ============================
        // NEW: child process met fork geprobeerd
        // ============================
        
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            free(fullpath);
            continue;
        }

        // ============================
        // CHILD PROCESS
        // ============================
        if (pid == 0) {

            // Execute the command
            execv(fullpath, tokens);

            // If execv returns, something failed
            perror("execv failed");
            exit(1);
        }

        // ============================
        // PARENT PROCESS new
        // ============================
        else {
            wait(NULL);  // wait for child to finish
        }

        free(fullpath);  // prevent memory leak
    }

    return 0;
}
