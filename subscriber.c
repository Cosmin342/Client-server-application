#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <limits.h>
#include "utils.h"
#include "helpers.h"

//Functie pentru procesarea comenzilor
void process_command(char* command, int sockfd) {
    int ret;
    msg message;
    char* token = strtok(command, " ");
    //Se verifica tipul comenzii
    if (strcmp(token, "subscribe") == 0) {
        //Tipul mesajului de subscribe va fi 1, iar pentru unsubscribe 0
        strcpy(message.type, "1");
        token = strtok(NULL, " ");
        //Daca un token este null, comanda este invalida
        if (token == NULL) {
            printf("Unknown command.\n");
            return;
        }
        strcpy(message.topic, token);
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Unknown command.\n");
            return;
        }
        strcpy(message.content, token);
        //Se trimite mesajul si se printeaza faptul ca s-a abonat la topic
        ret = send(sockfd, &message, sizeof(message), 0);
        DIE(ret < 0, "send");
        printf("Subscribed to topic.\n");
    }
    else if (strcmp(token, "unsubscribe") == 0) {
        strcpy(message.type, "2");
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("Unknown command.\n");
            return;
        }
        strcpy(message.topic, token);
        /*
        Daca s-a trimis cu succes, se printeaza faptul ca subscriber-ul s-a
        dezabonat de la topic
        */
        ret = send(sockfd, &message, sizeof(message), 0);
        DIE(ret < 0, "send");
        printf("Unsubscribed from topic.\n");
    }
    //Alte comenzi in afara celor doua nu sunt cunoscute
    else {
        printf("Unknown command.\n");
    }
}

int main(int argc, char**argv) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int sockfd, fdmax, flag = 1, ret;
	struct sockaddr_in serv_addr;
    fd_set read, temp;

	FD_ZERO(&read);

    //Se deschide socket-ul
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "socket");
    
    /*
    Se completeaza campurile structurii serv_addr pentru a face posibila
    conexiunea la server
    */
    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);

    //Se conecteaza la server
    ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "connect");

    //Se dezactiveaza algoritmul lui Neagle
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

    /*
    Se seteaza fdmax si se pune in multimea de descriptori socket-ul si
    descriptorul pt stdin
    */
	fdmax = sockfd;
    FD_SET(STDIN_FILENO, &read);
	FD_SET(sockfd, &read);

    //Se trimite la server un mesaj cu id-ul subscriber-ului
    msg message_id;
    strcpy(message_id.content, argv[1]);
    ret = send(sockfd, &message_id, sizeof(message_id), 0);
    DIE(ret < 0, "send");

    while (1) {
  		temp = read;
        msg message;
		select(fdmax + 1, &temp, NULL, NULL, NULL);
        //Se verifica daca a venit un mesaj de la server		
		if (FD_ISSET(sockfd, &temp)) {
			int received = recv(sockfd, &message, sizeof(msg), 0);
            DIE(received < 0, "recv");

			if (received != 0) {
                //Daca mesajul este exit, clientul se va inchide
				if (strcmp(message.content, "exit") == 0) {
                    break;
                }
                //Altfel, se printeaza campurile mesajului
                else {
                    printf("%s:%d - %s - %s - %s\n", message.ip_udp,
                        message.port, message.topic, message.type,
                            message.content);
                }
			}
		}
        //Se verifica daca clientul a dat o comanda de la tastatura
		else if (FD_ISSET(STDIN_FILENO, &temp)) {
			char command[MAX_COMMAND];
            fgets(command, MAX_COMMAND, stdin);
            //Daca s-a primit comanda exit, clientul se inchide
            if (strstr(command, "exit") > 0) {
                break;
            }
            //Altfel, se elimina \n de la final si se proceseaza noua comanda
            command[strlen(command) - 1] = '\0';
            process_command(command, sockfd);
		}
    }
    close(sockfd);
    return 0;
}