#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <dirent.h>

void monitor_directory(const char *path) {
        struct dirent *pDirent;
        DIR *pDir;
        char buff[50];
  pDir = opendir (path);
        if (pDir == NULL) {
            printf ("Cannot open directory '%s'",path);
            exit(EXIT_FAILURE);
        }
        while ((pDirent = readdir(pDir)) != NULL) {
          struct stat file_stat;
    int check = stat(path, &file_stat);
    if (check != 0) {
            int snapfile =  open("snapshot.txt", O_RDWR | O_APPEND | O_CREAT);   
            if(!snapfile){
            printf ("error creating snapshot file");
            exit(EXIT_FAILURE);
            }
            buff=file_stat.st_mtim;
        write(snapfile,buff,50);
        close(snapfile);
    }
        }
        closedir (pDir);
}


int main(int argc, char* argv){
    if (argc<2){
        perror("Not enough arguments.");
        exit(EXIT_FAILURE);
    }
    for(int i =2;i<=argc;i++){
        monitor_directory(argv[i]);
    }
    return 0;
}
