#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_DIRS 10
#define PATH_MAX 256

// Monitor a directory and its subdirectories for changes
void monitor_directory(const char *path) {
    struct dirent *pDirent;
    DIR *pDir;
    char buff[50];

    // Open the directory
    pDir = opendir(path);
    if (pDir == NULL) {
        perror("Cannot open directory");
        exit(EXIT_FAILURE);
    }

    // Read each entry in the directory
    while ((pDirent = readdir(pDir))!= NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, pDirent->d_name);

        // Get metadata for the entry
        struct stat file_stat;
        if (stat(file_path, &file_stat) == 0) {
            int snapfile = open("snapshot.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
            if (snapfile == -1) {
                perror("Error creating/opening snapshot file");
                exit(EXIT_FAILURE);
            }

            // Write metadata for the entry to the snapshot file
            snprintf(buff, sizeof(buff), "%s\t%ld\n", pDirent->d_name, file_stat.st_mtime);
            write(snapfile, buff, strlen(buff));
            close(snapfile);
        }
    }

    // Close the directory
    closedir(pDir);
}

// Update the snapshot file for a directory
void update_snapshot(const char *path, const char *output_dir) {
    // Open the directory
    DIR *pDir = opendir(path);
    if (pDir == NULL) {
        perror("Cannot open directory");
        exit(EXIT_FAILURE);
    }

    // Open the snapshot file
    char snapshot_file[PATH_MAX];
    snprintf(snapshot_file, PATH_MAX, "%s/snapshot.txt", output_dir);
    int snapfile = open(snapshot_file, O_RDWR | O_APPEND);
    if (snapfile == -1) {
        perror("Error opening snapshot file");
        exit(EXIT_FAILURE);
    }

    // Read each entry in the directory
    struct dirent *pDirent;
    while ((pDirent = readdir(pDir))!= NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, pDirent->d_name);

        // Get metadata for the entry
        struct stat file_stat;
        if (stat(file_path, &file_stat) == 0) {
            // Read each line in the snapshot file
            char line[100];
            char *name;
            char *mtime_str;
            while (fgets(line, sizeof(line), snapfile)) {
                name = strtok(line, "\t");
                mtime_str = strtok(NULL, "\t");
                if (name!= NULL && mtime_str!= NULL) {
                    if (strcmp(pDirent->d_name, name) == 0) {
                        long mtime_old = strtol(mtime_str, NULL, 10);
                        if (file_stat.st_mtime!= mtime_old) {
                            // Update the metadata for the entry in the snapshot file
                            lseek(snapfile, -strlen(line), SEEK_CUR);
                            char buff[50];
                            snprintf(buff, sizeof(buff), "%s\t%ld\n", pDirent->d_name, file_stat.st_mtime);
                            write(snapfile, buff, strlen(buff));
                        }
                    }
                }
            }
        }
    }

    // Close the snapshot file
    close(snapfile);

    // Close the directory
    closedir(pDir);
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 2 || argc > MAX_DIRS + 1) {
        perror("Invalid number of arguments");
        exit(EXIT_FAILURE);
    }

    // Get the output directory
    char output_dir[PATH_MAX];
    snprintf(output_dir, PATH_MAX, "%s", argv[1]);

    // Monitor and update the snapshot file for each directory
    for (int i = 2; i < argc; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            monitor_directory(argv[i]);
            update_snapshot(argv[i], output_dir);
            exit(0);
        } else if (pid < 0) {
            // Error forking
            perror("Error forking");
            exit(EXIT_FAILURE);
        }
    }

    // Parent process
    int status;
    pid_t terminated_pid;
    while ((terminated_pid = wait(&status)) > 0) {
        printf("Child Process %d terminated with PID %d and exit code %d.\n", terminated_pid, WEXITSTATUS(status));
    }

    return 0;
}