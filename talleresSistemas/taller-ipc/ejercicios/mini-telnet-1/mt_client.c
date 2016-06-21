#include "mt.h"

int main(int argc, char* argv[]) {
    int                 sock_fd;
    struct sockaddr_in  name;
    char                buf[MAX_MSG_LENGTH];

   
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("abriendo socket");
        exit(1);
    }

    name.sin_family = AF_INET;
    if (inet_aton(argv[1], &name.sin_addr) == 0){
    	perror("IP invalida");
    	exit(1);
    }
    
    //name.sin_addr.s_addr = inet_aton;
    name.sin_port = htons(PORT);

    if (connect(sock_fd, (void*) &name, sizeof(name)) == -1) {
        perror("conectandose");
        exit(1);
    }

    while(printf("> "), fgets(buf, MAX_MSG_LENGTH, stdin), !feof(stdin)) {
        if (sendto(sock_fd, buf, strlen(buf), 0, (void *)&name, sizeof(name)) == -1) {
            perror("enviando");
            exit(1);
        }
        if (strncmp(buf, END_STRING, MAX_MSG_LENGTH) == 0)
            break;
    }

   
    close(sock_fd);

    return 0;
}
