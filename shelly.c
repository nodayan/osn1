#include <stdio.h>
#include <stdbool.h>

void print_prompt() {
    char cwd[1024];                        
    if (getcwd(cwd, sizeof(cwd)) == NULL) { 
        strcpy(cwd, "coldn't get current directory");                
    }

    printf("%s$ ", cwd);                    // directory met $ erachter
    fflush(stdout);                         // laat prompt zien op scherm zonder \n
}

bool read_input(char *userinput) {   //bool van functie gemaakt want makkelijkr zo :)
    print_prompt();
    if (fgets(userinput, 1024, stdin) == NULL) {
        printf("\n");
        return false; 
    }
    return true;
}

int tokenized(char *userinput, char *tokens[]) {
    int teller = 0;                          // token teller

    char *tok = strtok(userinput, " \t\r\n"); // tokenize en split met een spatie tab of nieuwe line

    while (tok != NULL && teller < 511) {      //max aantal tokens op 512
        tokens[teller] = tok;                 
        teller++;                          
        tok = strtok(NULL, " \t\r\n");        
    }

    tokens[teller] = NULL;                   
    return teller;                         
}


int main() {
	char userinput[1024];

    while (true) {
        if (!read_input(userinput))   // ga uit programma als geen line gelezen wordt
        	break;

        char *tokens[512];				
        int ntokens = tokenized(userinput, tokens);
        if (ntokens == 0) continue;		// ga door als enter op een lege line

        if (strcmp(tokens[0], "exit") == 0) {   // ga eruit als eerste woord exit is
            break;
        }

        if (strcmp(tokens[0], "cd") == 0) {	// eerste woord is cd
            char *target = tokens[1];
            if (target == NULL) target = getenv("HOME");	//als er geen dir is gegeven ga automatisch naar home
            if (target == NULL) target = "/";			// als er geen home "bestaat", ga naar de andere wortel dir
            if (chdir(target) != 0) perror("cd");		// returnet een gedetailleerde error van waarom de dir niet gewisseld kan worden, niet 0 is error
            continue;
        }

        printf("TOKENS: ");
        for (int i = 0; tokens[i] != NULL; i++) {
            printf("[%s] ", tokens[i]);
        }
        printf("\n");
    }
    return 0;
}

