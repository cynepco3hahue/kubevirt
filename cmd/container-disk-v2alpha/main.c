#include <ctype.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50
#define READINESS_PROBE_FILE "/healthy"

int main(int argc, char **argv) 
{ 
    char *copy_path = NULL;
    char *copy_path_dir;
    bool health_check = false;

    int c;
    while ((c = getopt (argc, argv, "c:p")) != -1) {
        switch (c)
        {
        case 'c':
            copy_path = optarg;
            copy_path_dir = strdup(copy_path);
            dirname(copy_path_dir);
            break;

        case 'p':
            health_check = true;
            break;

        case '?':
            if (optopt == 'c')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
            exit(1);
        default:
            abort();
        }
    }

    struct stat st = {0};
    if (health_check) {
        if (stat(READINESS_PROBE_FILE, &st) == -1) {
            printf("readiness probe %s does not exist\n", READINESS_PROBE_FILE);
            exit(1);
        } else {
            exit(0);
        }
    }

    if (stat(copy_path_dir, &st) == -1) {
        if (mkdir(copy_path_dir, 0777) != 0) {
            printf("failed to create disk directory %s\n", copy_path_dir);
            exit(1);
        }
    }

    struct sockaddr_un address;
    /*
    * For portability clear the whole structure, since some
    * implementations have additional (nonstandard) fields in
    * the structure.
    */
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strcat(copy_path, ".sock");
    strcpy(address.sun_path, copy_path);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        printf("failed to create socket on %s\n", copy_path);
        exit(1);
    }

    if (bind(fd, (struct sockaddr*)(&address), sizeof(struct sockaddr_un)) == -1) {
        printf("failed to bind socket %s\n", copy_path);
        exit(1);
    }

    if (listen(fd, LISTEN_BACKLOG) == -1) {
        printf("failed to listen socket %s\n", copy_path);
        exit(1);
    }

    // Create readiness probe
    FILE *probe;
    probe = fopen(READINESS_PROBE_FILE, "w");
    if (probe == NULL) {
        printf("failed to create readiness probe\n");
        exit(1);
    }
    fclose(probe);

    int data_socket;
    for (;;) {
        data_socket = accept(fd, NULL, NULL);
        if (data_socket == -1) {
            if (stat(copy_path, &st) == -1) {
                printf("socket %s does not exist anymore\n", copy_path);
                exit(0);
            }
            printf("failed to accept connection\n");
            unlink(copy_path);
            exit(1);
        }
    }
}
