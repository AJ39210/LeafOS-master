#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>
#include <libgen.h>

#define SHELL_BUFSIZE 1024
#define SHELL_LS_BUFSIZE 64

/* Get current working directory for prompt */
static char cwd[512] = "/";

/* Update CWD */
void update_cwd(void) {
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        strcpy(cwd, "/");
    }
}

/* Simple tokenizer */
char **shell_split_line(char *line) {
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "Allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n");
    }

    tokens[position] = NULL;
    return tokens;
}

/* Built-in: cd */
int shell_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME") ?: "/") != 0) {
            perror("cd");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    update_cwd();
    return 1;
}

/* Built-in: echo */
int shell_echo(char **args) {
    for (int i = 1; args[i]; i++) {
        printf("%s", args[i]);
        if (args[i + 1]) printf(" ");
    }
    printf("\n");
    return 1;
}

/* Built-in: exit */
int shell_exit(char **args) {
    return 0;
}

/* Built-in: export */
int shell_export(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "export: missing argument\n");
        return 1;
    }

    if (putenv(args[1]) != 0) {
        perror("export");
    }
    return 1;
}

/* Built-in: set */
int shell_set(char **args) {
    if (args[1] == NULL) {
        /* List all environment variables */
        extern char **environ;
        for (int i = 0; environ[i]; i++) {
            printf("%s\n", environ[i]);
        }
    } else {
        /* Set variable */
        if (putenv(args[1]) != 0) {
            perror("set");
        }
    }
    return 1;
}

/* Check if command is a built-in */
int shell_execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    /* Built-in commands */
    if (strcmp(args[0], "cd") == 0) {
        return shell_cd(args);
    }
    if (strcmp(args[0], "echo") == 0) {
        return shell_echo(args);
    }
    if (strcmp(args[0], "exit") == 0) {
        return shell_exit(args);
    }
    if (strcmp(args[0], "export") == 0) {
        return shell_export(args);
    }
    if (strcmp(args[0], "set") == 0) {
        return shell_set(args);
    }

    /* External command - fork and exec */
    pid_t pid = fork();
    int status;

    if (pid == 0) {
        /* Child process */
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        /* Error forking */
        perror("fork");
    } else {
        /* Parent process - wait for child */
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/* Main shell loop */
int main(int argc, char *argv[]) {
    char *line = NULL;
    char **args;
    int should_run = 1;

    /* Get initial CWD */
    update_cwd();

    /* Print welcome message */
    printf("\n========================================\n");
    printf("    LeafOS Simple Shell v0.1\n");
    printf("========================================\n\n");

    while (should_run) {
        /* Print prompt */
        printf("user@leafos:%s# ", cwd);
        fflush(stdout);

        /* Read line */
        size_t bufsize = 0;
        if (getline(&line, &bufsize, stdin) == -1) {
            if (feof(stdin)) {
                printf("\n");
                break;
            }
            perror("getline");
            continue;
        }

        /* Skip empty lines */
        if (line[0] == '\n') {
            continue;
        }

        /* Parse and execute */
        args = shell_split_line(line);
        should_run = shell_execute(args);
        free(args);
    }

    if (line) {
        free(line);
    }

    printf("Exiting LeafOS Shell.\n");
    return EXIT_SUCCESS;
}
