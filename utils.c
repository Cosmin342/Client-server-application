#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <limits.h>
#include <math.h>
#include "utils.h"
#include "helpers.h"

//Functie pentru adaugarea unui nou client in array-ul de clienti activi
clients* add_client(clients* active_clients, char* id, int sockfd, int* length,
    int* size) {
    clients* new_clients;
    /*
    Daca array-ul este plin, i se va dubla dimensiunea si se vor adauga id-ul
    si socket-ul clientului nou. Altfel, datele clientului se vor adauga in
    actualul array
    */
    if (*length == *size) {
        new_clients = (clients*)realloc(active_clients, 2 * (*size) *
            sizeof(clients));
        DIE(new_clients == NULL, "realloc");
        (*size) *= INIT;
        strcpy(new_clients[*length].id, id);
        new_clients[*length].sockfd = sockfd;
        (*length)++;
        return new_clients;
    }
    strcpy(active_clients[*length].id, id);
    active_clients[*length].sockfd = sockfd;
    (*length)++;
    return active_clients;
}

//Functie pentru abonarea unui client la un topic
topics* add_topic_subscriber(topics* subjects, char *topic_name,
    int* length_topics, int* size_topics, int i, char* id, int sf) {
    int position;
    //Se cauta topic-ul aferent
    for (position = 0; position < *length_topics; position++) {
        if (strcmp(subjects[position].topic, topic_name) == 0) {
            break;
        }
    }
    //Daca nu exista, se va adauga in lista
    if (position == *length_topics) {
        topics* new_topics = subjects;
        //Daca array-ul pentru topicuri este plin, i se va dubla capacitatea
        if (*length_topics == *size_topics) {
            new_topics = realloc(subjects, INIT* (*size_topics) *
                sizeof(topics));
            DIE(new_topics == NULL, "realloc");
            (*size_topics) *= INIT;
        }
        /*
        Se adauga datele topicului si datele clientului si se returneaza noul
        array de topic-uri
        */
        strcpy(new_topics[position].topic, topic_name);
        new_topics[position].subscribers_size = INIT;
        new_topics[position].subscribers = calloc(INIT, sizeof(clients));
        DIE(new_topics[position].subscribers == NULL, "calloc");
        (*length_topics)++;
        int number = new_topics[position].subscribers_number;
        new_topics[position].subscribers[number].sf = sf;
        new_topics[position].subscribers[number].sockfd = i;
        strcpy(new_topics[position].subscribers[number].id, id);
        new_topics[position].subscribers_number++;
        return new_topics;
    }
    //Daca topicul exista, se va adauga clientul
    int length = subjects[position].subscribers_number;
    int size_subs = subjects[position].subscribers_size;
    //Daca vectorul de abonati este plin, i se va dubla capacitatea
    if (length == size_subs) {
        subjects[position].subscribers = realloc(subjects[position].subscribers,
            INIT * size_subs * sizeof(clients));
        DIE(subjects[position].subscribers == NULL, "realloc");
        subjects[position].subscribers_size *= INIT;
    }
    subjects[position].subscribers[length].sockfd = i;
    strcpy(subjects[position].subscribers[length].id, id);
    subjects[position].subscribers[length].sf = sf;
    (subjects[position].subscribers_number)++;
    return subjects;
}

//Functie pentru creearea unui nou topic
topics* add_topic(topics* subjects, char *topic_name, int len,
    int* length_topics, int* size_topics) {
    int position;
    char name[MAX_TOPIC_SIZE];
    strncpy(name, topic_name, len + 1);
    //Daca topicul exista deja, functia se opreste
    for (position = 0; position < *length_topics; position++) {
        if (strcmp(subjects[position].topic, name) == 0) {
            return subjects;
        }
    }
    topics* new_topics;
    //Daca este plin array-ul de topicuri, i se mareste capacitatea
    if (*length_topics == *size_topics) {
        new_topics = realloc(subjects, INIT * (*size_topics) * sizeof(topics));
        DIE(new_topics == NULL, "realloc");
        (*size_topics) *= INIT;
        //Se adauga numele topicului si se initializeaza array-ul de clienti
        strcpy(new_topics[*length_topics].topic, name);
        new_topics[*length_topics].subscribers_size = INIT;
        new_topics[*length_topics].subscribers = calloc(INIT, sizeof(clients));
        DIE(new_topics[*length_topics].subscribers == NULL, "calloc");
        (*length_topics)++;
        return new_topics;
    }
    strcpy(subjects[*length_topics].topic, name);
    subjects[*length_topics].subscribers_size = INIT;
    subjects[*length_topics].subscribers = calloc(INIT, sizeof(clients));
    DIE(subjects[*length_topics].subscribers == NULL, "calloc");
    (*length_topics)++;
    return subjects;
}

/*
Functie pentru retinerea unui mesaj pentru un topic la care exista un abonat
cu optiunea sf = 1
*/
sf_msg* add_sf_msg(sf_msg* mesaje, msg mesaj, char* id, int* size_sf,
    int* length_sf) {
    sf_msg* new_messages;
    //Daca array-ul de mesaje este plin, i se dubleaza capacitatea
    if (*length_sf == *size_sf) {
        new_messages = realloc(mesaje, INIT * (*size_sf) * sizeof(sf_msg));
        DIE(new_messages == NULL, "realloc");
        *size_sf = 2 * (*size_sf);
        //Se adauga mesajul si id-ul clientului destinatie
        new_messages[*length_sf].mesaj = mesaj;
        strcpy(new_messages[*length_sf].id, id);
        (*length_sf)++;
        return new_messages;
    }
    mesaje[*length_sf].mesaj = mesaj;
    strcpy(mesaje[*length_sf].id, id);
    (*length_sf)++;
    return mesaje;
}

//Functie pentru trimiterea mesajelor stocate pentru trimitere ulterioara
void send_msg(char* id, int sockfd, sf_msg* messages, int* length_sf) {
    int ret;
    for (int i = 0; i < *length_sf; i++) {
        //Daca mesajul e destinat clientului actual, va fi trimis
        if (strcmp(id, messages[i].id) == 0) {
            ret = send(sockfd, &(messages[i].mesaj), sizeof(messages[i].mesaj),
                0);
            DIE(ret < 0, "send");
            strcpy(messages[i].id, "");
        }
    }
}

//Functie pentru procesarea mesajelor date de un client udp
void process_message(char* content, msg* message) {
    //Se preia tipul mesajului
	int type = content[MAX_TOPIC_SIZE - 1];
	char sign;
	switch (type)
	{
		case 1:
            /*
            Daca mesajul este de tip SHORT_REAL, mesajul e reprezentat de un 
            numar pe doi octeti ce va fi impartit la 100
            */
			strcpy(message->type, "SHORT_REAL");
			float short_n = (float)((uint16_t)(((uint8_t)content[MAX_TOPIC_SIZE]
                << SHIFT) + (uint8_t)content[MAX_TOPIC_SIZE + 1])) / MAX_COMMAND;
			sprintf(message->content,"%.2f", short_n);
			break;
		case 2:
			strcpy(message->type, "FLOAT");
			float float_number;
            /*
            In cazul float se va extrage intai octetul de semn si apoi cei 4
            care reprezinta numarul fara virgula
            */
			sign = content[MAX_TOPIC_SIZE];
			uint32_t module = htonl(*(uint32_t*)(content + MAX_TOPIC_SIZE + 1));
			/*
            Pentru obtinerea numarului cu virgula, se imparte la puterea
            corespunzatoare a lui 10
            */
            int power = pow(MAX_ID - 1, (uint8_t)content[POWER]);
			float_number = ((float) module / power);
			if (sign == 1) {
				float_number *= (-1);
			}
			sprintf(message->content, "%f", float_number);
			break;
		case 3:
            /*
            In cazul string-ului, se va copia direct continutul in campul
            corespunzator
            */
			strcpy(message->type, "STRING");
			memset(message->content, 0, MAX_CONTENT_SIZE);
			strcpy(message->content, content + MAX_TOPIC_SIZE);
			break;
		default:
            /*
            In cazul int, se vor lua octetul de semn si in functie de valoarea
            acestuia se vor inmulti cei 4 octeti extrasi ulterior
            */
			strcpy(message->type, "INT");
			int number = 1;
			sign = content[MAX_TOPIC_SIZE];
			if (sign == 1) {
				number *= (-1);
			}
			uint32_t n = htonl(*(uint32_t*)(content + MAX_TOPIC_SIZE + 1));
			number *= n;
			memset(message->content, 0, MAX_CONTENT_SIZE);
			sprintf(message->content, "%d", number);
			break;
	}
}

//Functie ce verifica daca un client este in lista celor activi
int is_active(char* id, clients* clients, int len) {
	for (int i = 0; i < len; i++) {
		if (strcmp(id, clients[i].id) == 0) {
			return 1;
		}
	}
	return 0;
}

/*
Functie ce inchide conexiunea cu un client dat si il elimina din array-ul celor
activi
*/
void close_client(int* length, int sockfd, fd_set* read_fds,
	clients* active_clients) {
	for (int j = 0; j < *length; j++) {
		if (active_clients[j].sockfd == sockfd) {
			printf("Client %s disconnected.\n", active_clients[j].id);
            /*
            Pentru a nu lasa un loc gol in array, datele ultimului client se
            vor muta pe pozitia celui deconectat.
            */
			if ((*length) != 1) {
				active_clients[j].sockfd = active_clients[(*length) - 1].sockfd;
				strcpy(active_clients[j].id, active_clients[(*length) - 1].id);
			}
			(*length)--;
			break;
		}
	}
	
    //Se inchide socketul
	close(sockfd);
	
    //Se elimina socketul din mutimea de descriptori
	FD_CLR(sockfd, read_fds);
}

//Functie pentru dezabonarea unui client de la un topic
void unsubscribe(topics* subjects, msg message, int length_topics, int sock) {
	int position;
    //Se cauta topicul si socketul clientului
	for (position = 0; position < length_topics; position++) {
		if (strcmp(subjects[position].topic, message.topic) == 0) {
			break;
		}
	}
	for (int j = 0; j < subjects[position].subscribers_number; j++) {
		if (subjects[position].subscribers[j].sockfd == sock) {
			int number = subjects[position].subscribers_number;
            //Se muta pe pozitia actuala datele clientului de pe ultima pozitie
			if (number != 1) {
				subjects[position].subscribers[j].sockfd =
					subjects[position].subscribers[number - 1].sockfd;
				strcpy(subjects[position].subscribers[j].id,
					subjects[position].subscribers[number - 1].id);
				subjects[position].subscribers[j].sf =
					subjects[position].subscribers[number - 1].sf;
			}
			(subjects[position].subscribers_number)--;
			break;
		}
	}
}

//Functie pentru actualizarea topicurilor la subscribe/unsubscribe
void update_topics(clients* active_clients, int length, int sockfd,
	msg message, int* length_topics, int* size_topics, topics* subjects) {
	int j;
    //Se cauta clientul dupa socket
	for (j = 0; j < length; j++) {
		if (active_clients[j].sockfd == sockfd) {
			break;	
		}
	}
    //Daca mesajul are tipul 1, clientul doreste sa dea subscribe la un topic
	if (strcmp(message.type, "1") == 0) {
		int sf = atoi(message.content);
		subjects = add_topic_subscriber(subjects, message.topic, length_topics,
			size_topics, sockfd, active_clients[j].id, sf);
	}
    //Daca este 2, doreste sa dea unsubscribe
	if (strcmp(message.type, "2") == 0) {
		unsubscribe(subjects, message, *length_topics, sockfd);
	}
}

//Functie care verifica daca server-ul primeste exit de la tastatura
int check_exit(clients* active_clients, int length, fd_set* read_fds) {
	char comanda[EXIT];
	fgets(comanda, EXIT, stdin);
    /*
    Daca s-a primit exit, server-ul va transmite exit si clientilor si va
    inchide conexiunile cu acestia
    */
	if (strcmp(comanda, "exit") == 0) {
		for (int i = 0; i < length; i++) {
			msg message;
			strcpy(message.content, "exit");
			send(active_clients[i].sockfd, &message, sizeof(message), 0);
			close(active_clients[i].sockfd);
			FD_CLR(active_clients[i].sockfd, read_fds);
		}
		return 1;
	}
	return 0;
}

/*
Functie ce verifica daca se poate trimite un mesaj la clienti sau daca trebuie
sa il retina in cazul in care exista clienti care s-au abonat la topic
cu sf = 1
*/
sf_msg* check_and_send(int length_topics, topics* subjects, msg message,
	clients* active_clients, int length, int* size_sf, int* length_sf,
	sf_msg* messages) {
	for (int i = 0; i < length_topics; i++) {
		if (strcmp(subjects[i].topic, message.topic) == 0) {
			if (subjects[i].subscribers_number != 0) {
				for (int j = 0; j < subjects[i].subscribers_number; j++) {
                    /*
                    Daca actualul client este inactiv si este abonat cu
                    sf = 1, se stocheaza mesajul pentru a fi trimis ulterior
                    */
					if ((subjects[i].subscribers[j].sf == 1) &&
						(is_active(subjects[i].subscribers[j].id,
							active_clients, length) == 0)) {
						messages = add_sf_msg(messages, message,
							subjects[i].subscribers[j].id, size_sf, length_sf);
						continue;
					}
                    /*
                    Daca actualul client este doar inactiv, se trece la
                    urmatorul
                    */
					if (is_active(subjects[i].subscribers[j].id,
						active_clients, length) == 0) {
						continue;
					}
                    //Altfel, se incearca trimiterea mesajului
					int ret = send(subjects[i].subscribers[j].sockfd, &message,
						sizeof(message), 0);
					DIE(ret < 0, "send");
				}
			}
			break;
		}
	}
	return messages;
}

//Functie pentru conectarea unui nou client
int connect_new_client(int sock_tcp, clients* active_clients, int length,
	fd_set* read_fds, int* fdmax, msg* message, int* newsockfd) {
	struct sockaddr_in client_addr;
    int flag = 1;

	socklen_t client_size = sizeof(client_addr);
	//Se accepta conexiunea si se adauga socketul in multimea de descriptori
    *newsockfd = accept(sock_tcp, (struct sockaddr *) &client_addr,
        &client_size);
	DIE(*newsockfd < 0, "accept");
	
    //Se dezactiveaza algoritmul lui Neagle
    setsockopt(*newsockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

	FD_SET(*newsockfd, read_fds);
    //Eventual, se actualizeaza si fdmax
	if (*newsockfd > *fdmax) { 
		*fdmax = *newsockfd;
	}

    //Se receptioneaza id-ul clientului intr-un mesaj
	int ret = recv(*newsockfd, message, sizeof(*message), 0);
	DIE(ret < 0, "recv");
	
	int duplicat = 0;
	for (int j = 0; j < length; j++) {
        /*
        Daca exista deja un client conectat cu acelasi id, se inchide
        clientul actual
        */
		if (strcmp(message->content, active_clients[j].id) == 0) {
			duplicat = 1;
			printf("Client %s already connected.\n", message->content);
			msg message;
			strcpy(message.content, "exit");
			send(*newsockfd, &message, sizeof(message), 0);
			close(*newsockfd);
			FD_CLR(*newsockfd, read_fds);
			break;
		}
	}
	
	if (duplicat == 1) {
		return 0;
	}

    //Se afiseaza datele noului client conectat
	printf("New client %s connected from %s:%d.\n",
			message->content, inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));

	return 1;
}

//Functie pentru primirea mesajelor de la un client udp
topics* receive_message(msg* message, int sock_udp, topics* subjects,
	int* length_topics, int* size_topics) {
	char content[MAX_CONTENT_SIZE];
	memset(content, 0, MAX_CONTENT_SIZE);
	struct sockaddr_in client_addr;
	unsigned int size = sizeof(client_addr);

	int ret = recvfrom(sock_udp, content, MAX_CONTENT_SIZE, 0,
        (struct sockaddr*)&client_addr, &size);
	DIE(ret < 0, "recvfrom");
	
    /*
    Se va crea noul mesaj destinat abonatilor ce va contine port-ul clientului
    udp, ip-ul, numele topicului, tipul si continutul, care va fi prelucrat
    in functie de tipul de date aparent
    */
	message->port = ntohs(client_addr.sin_port);
	strcpy(message->ip_udp, inet_ntoa(client_addr.sin_addr));
	char* apar = strchr(content, '\0');
    //Se calculeaza lungimea topicului
	int len = apar - content;
    //Se creeaza topicul, daca acesta nu exista deja
	subjects = add_topic(subjects, content, len, length_topics, size_topics);
	strncpy(message->topic, content, len + 1);
	process_message(content, message);
	return subjects;
}

/*
Functie pentru initializarea socket-ilor udp si tcp si a multimii de
descriptori
*/
void start(fd_set* read_fds, int* sock_udp, int* sock_tcp, char* port,
	int* fdmax) {
	int ret;
	FD_ZERO(read_fds);
    struct sockaddr_in my_sockaddr;

    //Se deschid socketii si se completeaza structura de tip sockaddr_in
	*sock_udp = socket(PF_INET, SOCK_DGRAM, 0);
    DIE(*sock_udp < 0, "socket");
	*sock_tcp = socket(AF_INET, SOCK_STREAM, 0);
    DIE(*sock_tcp < 0, "socket");

	my_sockaddr.sin_family = AF_INET;
	my_sockaddr.sin_port = htons(atoi(port));
	my_sockaddr.sin_addr.s_addr = INADDR_ANY;
  
    /*
    Se face bind pe socketi, iar pe tcp se face listen pentru a astepta
    conexiuni
    */
	ret = bind(*sock_tcp, (struct sockaddr*) &my_sockaddr, sizeof(my_sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(*sock_tcp, INT_MAX);
    DIE(ret < 0, "listen");

    ret = bind(*sock_udp, (struct sockaddr*) &my_sockaddr, sizeof(my_sockaddr));
    DIE(ret < 0, "bind");

    /*
    Se seteaza fdmax si se adauga in multimea de descriptori socketii si
    descriptorul pentru stdin
    */
	*fdmax = *sock_tcp;
	if (*sock_udp > *fdmax) {
		*fdmax = *sock_udp;
	} 
	FD_SET(*sock_udp, read_fds);
	FD_SET(*sock_tcp, read_fds);
	FD_SET(0, read_fds);

}