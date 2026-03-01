int main() {
	char userinput[1024];

    while (true) {
        if (!read_input(userinput))
        	break;

        char *commands[100];
        int ncommands = 0;

        char *cmd = strtok(userinput, "|");

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

        if (ncommands == 0) continue; //dit is als je een command hebt, is normaal
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

        // meerdere pipe
        else {

            int in_fd = STDIN_FILENO;

            for (int i = 0; i < ncommands; i++) {

                char *tokens[512];
                tokenized(commands[i], tokens);

                char *fullpath = path_command(tokens[0]);
                if (fullpath == NULL) {
                    printf("Command not found\n");
                    break;
                }

                // pipe maken
                if (i < ncommands - 1) {

                    int fd[2];
                    pipe(fd);

                    spawn(fullpath, tokens, in_fd, fd[1]);

                    close(fd[1]);       // parent closes write end
                    if (in_fd != STDIN_FILENO)
                        close(in_fd);  // close previous read end

                    in_fd = fd[0];     // next command reads from here
                }

                // laatste command
                else {

                    spawn(fullpath, tokens, in_fd, STDOUT_FILENO);

                    if (in_fd != STDIN_FILENO)
                        close(in_fd);
                }

                free(fullpath);
            }

            // BELANGRIJK wachten op alle proccessen
            while (wait(NULL) > 0);
        }
    }

    return 0;
}
