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

int main(int argc, char**argv) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int sock_udp, sock_tcp, fdmax, length = 0, size_clients = INIT, ret;
	int size_topics = INIT, length_topics = 0, size_sf = INIT, length_sf = 0;
	fd_set read_fds, tmp_fds;
	msg message;

	/*
	Se initializeaza vectorii de clienti, topicuri si cel pentru mesajele
	de pe topicuri cu sf activat
	*/
	clients* active_clients = calloc(INIT, sizeof(clients));
    DIE(active_clients == NULL, "calloc");
	topics* subjects = calloc(INIT, sizeof(topics));
    DIE(subjects == NULL, "calloc");
	sf_msg* messages = calloc(INIT, sizeof(sf_msg));
    DIE(messages == NULL, "calloc");

	/*
	Se deschid socketii tcp si udp, se initializaeaza multimea read_fds si
	fdmax
	*/
	start(&read_fds, &sock_udp, &sock_tcp, argv[1], &fdmax);

	while (1) {
		tmp_fds = read_fds;
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) < 0;
    	DIE(ret < 0, "select");

		//Se verifica daca server-ul primeste exit de la tastatura
		if (FD_ISSET(STDIN_FILENO, &tmp_fds)) {
			if (check_exit(active_clients, length, &read_fds) == 1) {
				break;
			}
		}
		/*
		Se verifica daca se primeste vreun mesaj de la udp si il transmite,
		daca exista persoane abonate la topic-ul respectiv
		*/
		if (FD_ISSET(sock_udp, &tmp_fds)) {
			subjects = receive_message(&message, sock_udp, subjects,
				&length_topics, &size_topics);
			messages = check_and_send(length_topics, subjects, message,
				active_clients,	length, &size_sf, &length_sf, messages);
			
		}
		for (int i = 1; i <= fdmax; i++) {
			//Se verifica daca se primeste vreun mesaj pe un socket
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sock_udp) {
					continue;
				}
				/*
				Daca se primeaste pe socketul tcp, inseamna ca un nou client
				doreste sa se conecteze
				*/
				if (i == sock_tcp) {
					int newsockfd;
					/*
					Daca este un client deja conectat cu acelasi ip, se va
					respinge conexiunea noului client
					*/
					if (connect_new_client(sock_tcp, active_clients, length,
						&read_fds, &fdmax, &message, &newsockfd) == 0) {
							continue;
					}					
					
					active_clients = add_client(active_clients, message.content,
						newsockfd, &length, &size_clients);
					/*
					Daca exista mesaje stocate, se verifica daca sunt pentru
					clientul nou conectat si se trimit
					*/
					if (length_sf != 0) {
						send_msg(message.content, newsockfd, messages,
							&length_sf);
					}
				}
				else {
					int n = recv(i, &message, sizeof(message), 0);
    				DIE(n < 0, "recv");

					//Daca n este 0, clientul s-a deconectat
					if (n == 0) {
						close_client(&length, i, &read_fds, active_clients);
					}
					//Altfel, se aboneaza/dezaboneaza de la un topic
					else {
						update_topics(active_clients, length, i, message,
							&length_topics, &size_topics, subjects);
					}
				}
			}
		}
	}
	//Se elibereaza structurile folosite si se inchid socketii tcp si udp
	free(messages);
	free(active_clients);
	for (int i = 0; i < length_topics; i++) {
		free(subjects[i].subscribers);
	}
	free(subjects);
	close(sock_udp);
	close(sock_tcp);
    return 0;
}