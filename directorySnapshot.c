#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_DIRS 10

void monitor_directory(const char *path) {
    struct dirent *pDirent;
    DIR *pDir;
    char buff[50];

    pDir = opendir(path);
    if (pDir == NULL) {
        printf("Cannot open directory '%s'", path);
        exit(EXIT_FAILURE);
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, pDirent->d_name);

        struct stat file_stat;
        if (stat(file_path, &file_stat) == 0) {
            int snapfile = open("snapshot.txt", O_RDWR | O_APPEND | O_CREAT, 0644);
            if (snapfile == -1) {
                printf("Error creating/opening snapshot file");
                exit(EXIT_FAILURE);
            }
            snprintf(buff, sizeof(buff), "%s\t%ld\n", pDirent->d_name, file_stat.st_mtime);
            write(snapfile, buff, strlen(buff));
            close(snapfile);
        }
    }
    closedir(pDir);
}

void update_snapshot(const char *path) {
    struct dirent *pDirent;
    DIR *pDir;
    char buff[50];

    pDir = opendir(path);
    if (pDir == NULL) {
        printf("Cannot open directory '%s'", path);
        exit(EXIT_FAILURE);
    }
    while ((pDirent = readdir(pDir)) != NULL) {
        char file_path[PATH_MAX];
        snprintf(file_path, PATH_MAX, "%s/%s", path, pDirent->d_name);

        struct stat file_stat;
        if (stat(file_path, &file_stat) == 0) {
            int snapfile = open("snapshot.txt", O_RDWR | O_APPEND);
            if (snapfile == -1) {
                printf("Error opening snapshot file");
                exit(EXIT_FAILURE);
            }
            char line[100];
            char *name;
            char *mtime_str;
            while (fgets(line, sizeof(line), snapfile)) {
                name = strtok(line, "\t");
                mtime_str = strtok(NULL, "\t");
                if (name != NULL && mtime_str != NULL) {
                    if (strcmp(pDirent->d_name, name) == 0) {
                        long mtime_old = strtol(mtime_str, NULL, 10);
                        if (file_stat.st_mtime != mtime_old) {
                            lseek(snapfile, -strlen(line), SEEK_CUR);
                            snprintf(buff, sizeof(buff), "%s\t%ld\n", pDirent->d_name, file_stat.st_mtime);
                            write(snapfile, buff, strlen(buff));
                        }
                    }
                }
            }
            close(snapfile);
        }
    }
    closedir(pDir);
}

int main(int argc, char *argv[]) {
    if (argc < 2 || argc > MAX_DIRS + 1) {
        perror("Invalid number of arguments.");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++) {
        monitor_directory(argv[i]);
        update_snapshot(argv[i]);
    }
    return 0;
}
