//compile with "gcc main.c -lreadline -o terminal.exe"

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_ARGS 10

void versionInfo() {
    printf("Shell by Costean Robert\n");
}

void handle_sigint(int sig) {
    printf("\n");  // Print a newline to separate the shell prompt from the interrupted command
}

void helpCommand() {
    printf("List of available commands:\n");
    printf("dir\n");
    printf("rm, with the following options: -i, -r (-R), -v\n");
    printf("uniq, with the following possible options: -i, -d, -u\n");
    printf("version\n");
    printf("help\n");
    printf("External commands\n");
}


void uniq(char *inputFile, char *outputFile, bool nonCaseSensitive, bool justDuplicates, bool uniqueLines) {
    char currentLine[1000];
    char endLine[1000] = "";
    int specialPrint;
    bool linePrint = true;
    int alreadyPrinted = 0;

    FILE *input = fopen(inputFile, "r");
    if (!input) {
        fprintf(stderr, "Error: input file %s does not exist.\n", inputFile);
        return;
    }

    FILE *output = NULL;
    if (outputFile != NULL) {
        output = fopen(outputFile, "w");
        if (!output) {
            fprintf(stderr, "Error: Can't open output file %s for writing.\n", outputFile);
            fclose(input);
            return;
        }
    }

    while (fgets(currentLine, 1000, input)) {
        linePrint = false;
        if (nonCaseSensitive) {
            if (strcasecmp(endLine, currentLine) == 0) {
                alreadyPrinted++;
            } else { alreadyPrinted = 0; }
        } else {
            if (strcmp(endLine, currentLine) == 0) {
                alreadyPrinted++;
            } else {
                alreadyPrinted = 0;
            }
        }
        if (alreadyPrinted == 1 && justDuplicates && uniqueLines == 0) {
            if (outputFile != NULL) {
                fprintf(output, "%s", endLine);
            } else {
                printf("%s", endLine);
            }
        }
        if (nonCaseSensitive) {
            if (strcasecmp(currentLine, endLine) != 0 && justDuplicates == false &&
                uniqueLines == false) { linePrint = true; }
        } else if (strcmp(currentLine, endLine) != 0 && justDuplicates == false && uniqueLines == false) {
            linePrint = true;
        }

        if (nonCaseSensitive) {
            if (uniqueLines && strcasecmp(currentLine, endLine) != 0 && specialPrint == 1 && justDuplicates == 0) {
                if (outputFile != NULL) {
                    fprintf(output, "%s", endLine);
                } else {
                    printf("%s", endLine);
                }
            }
        } else if (uniqueLines && strcmp(currentLine, endLine) != 0 && specialPrint == 1 && justDuplicates == 0) {
            if (outputFile != NULL) {
                fprintf(output, "%s", endLine);
            } else {
                printf("%s", endLine);
            }
        }

        if (linePrint) {
            if (outputFile != NULL) {
                fprintf(output, "%s", endLine);
            } else {
                printf("%s", currentLine);
            }
        }
        specialPrint = 0;
        if (nonCaseSensitive) {
            if (uniqueLines && strcasecmp(currentLine, endLine) != 0) {
                specialPrint = 1;
            }
        } else if (uniqueLines && strcmp(currentLine, endLine) != 0) {
            specialPrint = 1;
        }
        strcpy(endLine, currentLine);
    }
    if (feof(input) && specialPrint == 1 && justDuplicates == 0) {
        if (outputFile != NULL) {
            fprintf(output, "%s", currentLine);
        } else {
            printf("%s", currentLine);
        }
    }
    fclose(input);
    if (outputFile != NULL) {
        fclose(output);
    }
}


int dirRemove(const char *dirPath, bool interactive, bool verbose) {
    DIR *deer = opendir(dirPath);
    size_t path_len = strlen(dirPath);
    int rem = -1;
    char userFeedback;
    if (deer) {
        struct dirent *u;
        rem = 0;
        while (!rem && (u = readdir(deer))) {
            int r2 = -1;
            char *buffer;
            size_t len;
            /* Skip "." and ".." as we don't need to consider them */
            if (!strcmp(u->d_name, ".") || !strcmp(u->d_name, "..")) {
                continue;
            }
            len = path_len + strlen(u->d_name) + 2;
            buffer = (char *) malloc(len);
            if (buffer) {
                struct stat statbuf;
                snprintf(buffer, len, "%s/%s", dirPath, u->d_name);
                if (!stat(buffer, &statbuf)) {
                    if (S_ISDIR(statbuf.st_mode)) {
                        r2 = dirRemove(buffer, interactive, verbose);
                    } else {
                        if (interactive) {
                            printf("remove regular file '%s'?", buffer);
                            scanf(" %c", &userFeedback);
                            if (userFeedback == 'y') {
                                r2 = unlink(buffer);
                                if (verbose) {
                                    printf("removed '%s'\n", buffer);
                                }
                            }
                        } else {
                            r2 = unlink(buffer);
                            if (verbose) {
                                printf("removed '%s'\n", buffer);
                            }
                        }
                    }
                }
                free(buffer);
            }
            rem = r2;
        }
        closedir(deer);
    }
    if (!rem) {
        if (interactive) {
            printf("remove directory '%s'?", dirPath);
            scanf(" %c", &userFeedback);
            if (userFeedback == 'y') {
                rem = rmdir(dirPath);
                if (verbose) {
                    printf("removed '%s'\n", dirPath);
                }
            }
        } else {
            rem = rmdir(dirPath);
            if (verbose) {
                printf("removed '%s'\n", dirPath);
            }
        }
    }
    return rem;
}

void rm(char *removable, bool recursive, bool interactive, bool verbose) {
    struct stat st;
    if (lstat(removable, &st) != 0) {
        fprintf(stderr, "Error: %s\n", strerror(errno));
        return;
    }

    if (recursive && S_ISDIR(st.st_mode)) {
        if (dirRemove(removable, interactive, verbose) == -1) {
            fprintf(stderr, "Error: %s\n", strerror(errno));
        }
    } else {
        if (interactive) {
            char userFeedback;
            printf("remove regular file '%s'?", removable);
            scanf(" %c", &userFeedback);
            if (userFeedback == 'y') {
                if (S_ISDIR(st.st_mode)) {
                        fprintf(stderr, "Error, cannot remove directories without -r or -R options\n");
                    }
                else {
                    if (remove(removable) != 0) {
                        fprintf(stderr, "Error: %s\n", strerror(errno));
                    }
                    if (verbose) {
                        printf("removed '%s'\n", removable);
                    }
                }
            }
        } else {
            if (S_ISDIR(st.st_mode)) {
                    fprintf(stderr, "Error, cannot remove directories without -r or -R options\n");
                }
            else {
                if (remove(removable) != 0) {
                    fprintf(stderr, "Error: %s\n", strerror(errno));
                }
                if (verbose) {
                    printf("removed '%s'\n", removable);
                }
            }
        }
    }
}


void dir(char *directoryName) {
    DIR *directoryOpener;
    struct dirent *directorySlice;
    directoryOpener = opendir(directoryName);
    if (directoryOpener) {
        while ((directorySlice = readdir(directoryOpener)) != NULL) {
            // skipping . and .. directories
            if (strcmp(directorySlice->d_name, ".") == 0 || strcmp(directorySlice->d_name, "..") == 0) {
                continue;
            }
            struct stat sb;
            char path[1000];
            snprintf(path, sizeof(path), "%s/%s", directoryName, directorySlice->d_name);
            stat(path, &sb);
            //printings
            if (S_ISREG(sb.st_mode)) {
                printf("%s (Regular file)\n", directorySlice->d_name);
            } else if (S_ISDIR(sb.st_mode)) {
                printf("%s (Directory)\n", directorySlice->d_name);
            } else {
                printf("%s (Other)\n", directorySlice->d_name);
            }
        }
        closedir(directoryOpener);
    }
}

int main() {
    while (1) {
        signal(SIGINT, handle_sigint);
        long unsigned int commandInitialLength;
        char *inputCommand = readline("# ");
        char *commandLineCopy = malloc(strlen(inputCommand) + 1);
        strcpy(commandLineCopy,inputCommand);
        commandInitialLength = strlen(inputCommand);
        if (!inputCommand)
            break;
        if (strcmp(inputCommand, "exit") == 0)
            break;
        if (!isspace(*inputCommand))
            add_history(inputCommand);
        char *slice = strtok(inputCommand, " ");

        // help function
        if (slice && strcmp(slice, "help") == 0) {
            helpCommand();
        }

            // version function
        else if (slice && strcmp(slice, "version") == 0) {
            versionInfo();
        } else if ((commandInitialLength == 3) && (strcmp(slice, "dir") == 0)) {
            char workingDirectory[1000];
            if (getcwd(workingDirectory, sizeof(workingDirectory)) != NULL) {
                printf("Current working directory:\n");
                dir(workingDirectory);
            } else {
                fprintf(stderr, "Error: Unable to get working directory: %s\n", strerror(errno));
            }
        }

            // dir function
        else if (slice && strcmp(slice, "dir") == 0) {
            int errorprint = 0;
            char workingDirectory[1000];
            char *directoryName = NULL;

            while ((slice = strtok(NULL, " "))) {

                directoryName = slice;
                if (directoryName[0] == '-') {
                    printf("Error: dir command does not accept options\n");
                    errorprint = 1;
                } else {
                    DIR *directory = opendir(directoryName);
                    if (directory) {
                        closedir(directory);
                        printf("%s:", directoryName);
                        printf("\n");
                        dir(directoryName);
                    } else {
                        if (errorprint == 0) {
                            printf("Error: non-existent directory '%s'\n", directoryName);
                        }
                    }
                }
            }
        }

            // rm function
        else if (slice && strcmp(slice, "rm") == 0) {
            int errorPrint = 0;
            bool recursive = false;
            bool interactive = false;
            bool verbose = false;
            char *filenames[100];
            int numFiles = 0;
            while ((slice = strtok(NULL, " "))) {
                if (strncmp(slice, "-", 1) == 0) {
                    if (((strncmp(&slice[1], "i", 1) != 0 && strncmp(&slice[1], "r", 1) != 0 &&
                          strncmp(&slice[1], "R", 1) != 0 && strncmp(&slice[1], "v", 1) != 0)) || strlen(slice) > 2) {
                        errorPrint = 1;
                        fprintf(stderr,
                                "Error: rm command does not accept '%s' as an option, it only accepts -i, -r, -R, and -v as options.\n",
                                slice);
                        break;
                    }
                    if (strcmp(slice, "-i") == 0) {
                        interactive = true;
                    } else if (strcmp(slice, "-r") == 0 || strcmp(slice, "-R") == 0) {
                        recursive = true;
                    } else if (strcmp(slice, "-v") == 0) {
                        verbose = true;
                    }
                } else {
                    filenames[numFiles++] = slice;
                }
            }
            if (errorPrint == 0 && numFiles == 0)
                fprintf(stderr, "Error: No file name(s) for rm command\n");
            for (int i = 0; i < numFiles; i++) {
                rm(filenames[i], recursive, interactive, verbose);
            }
        }


            // uniq function
        else if (slice && strcmp(slice, "uniq") == 0) {
            int errorPrint = 0;
            bool uniqueLines = false;
            bool nonCaseSensitive = false;
            bool justDuplicates = false;
            char *inputFile = NULL;
            char *outputFile = NULL;
            while ((slice = strtok(NULL, " "))) {
                if (strncmp(&slice[0], "-", 1) == 0) {
                    if ((strncmp(&slice[1], "i", 1) == 0)) {
                        nonCaseSensitive = true;
                    } else if (strncmp(&slice[1], "d", 1) == 0) {
                        justDuplicates = true;
                    } else if (strncmp(&slice[1], "u", 1) == 0) {
                        uniqueLines = true;
                    } else {
                        errorPrint = 1;
                        fprintf(stderr,
                                "Error: uniq command does not accept '%s' as an option, it only accepts -i, -d, and -u as options.\n",
                                slice);
                        break;
                    }
                } else if (!inputFile) {
                    inputFile = slice;
                } else if (!outputFile) {
                    outputFile = slice;
                } else {
                    fprintf(stderr, "Error: invalid command format, too many arguments\n");
                    break;
                }
            }
            if (inputFile) {
                uniq(inputFile, outputFile, nonCaseSensitive, justDuplicates, uniqueLines);
            } else {
                fprintf(stderr, "Error: No input file.\n");
            }
            if (errorPrint == 1) {
                break;
            }
        } else {
            pid_t processID = fork();
            if (processID == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            } else if (processID == 0) {
                int i = 0;
                char *args[10];
                char* token = strtok(commandLineCopy, " ");
                while (token != NULL) {
                    args[i] = token;
                    i++;
                    token = strtok(NULL, " ");
                }
                args[i] = NULL;
                execvp(args[0], args);
                perror("exec");
                exit(EXIT_FAILURE);
            } else {
                int status;
                waitpid(processID, &status, 0);
                if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                    printf("error code %d\n", WEXITSTATUS(status));
                }
            }
        }
            free(inputCommand);
            free(commandLineCopy);
    }
    return 0;
}