#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INIT                2
#define EXIT			    5
#define SHIFT               8
#define MAX_ID			    11
#define MAX_IP              17
#define MAX_TOPIC_SIZE      51
#define POWER               56
#define MAX_COMMAND         100
#define MAX_CONTENT_SIZE    1501
#define MAX_LEN_MSG		    1600

typedef struct client {
    char id[MAX_ID];
    int sockfd;
    int sf;
} clients;

typedef struct topic {
    char topic[MAX_TOPIC_SIZE];
    int subscribers_number;
    int subscribers_size;
    clients* subscribers;
} topics;

typedef struct msg {
    char topic[MAX_TOPIC_SIZE];
    char type[MAX_ID];
    char content[MAX_CONTENT_SIZE];
    char ip_udp[MAX_IP];
    int port;
} msg;

typedef struct sf_msg{
    msg mesaj;
    char id[MAX_ID];
} sf_msg;

clients* add_client(clients* active_clients, char* id, int sockfd, int* length,
    int* size);

topics* add_topic_subscriber(topics* topicuri, char *nume_topic,
    int* length_topics, int* size_topics, int i, char* id, int sf);

topics* add_topic(topics* topicuri, char *nume_topic, int len, 
    int* length_topics, int* size_topics);

sf_msg* add_sf_msg(sf_msg* mesaje, msg mesaj, char* id, int* size_sf,
    int* length_sf);

void send_msg(char* id, int sockfd, sf_msg* mesaje, int* length_sf);

sf_msg* check_and_send(int length_topics, topics* subjects, msg message,
	clients* active_clients, int length, int* size_sf, int* length_sf,
	sf_msg* messages);

int connect_new_client(int sock_tcp, clients* active_clients, int length,
	fd_set* read_fds, int* fdmax, msg* message, int* newsockfd);

topics* receive_message(msg* message, int sock_udp, topics* subjects,
	int* length_topics, int* size_topics);

void start(fd_set* read_fds, int* sock_udp, int* sock_tcp, char* port,
	int* fdmax);

int check_exit(clients* active_clients, int length, fd_set* read_fds);

void update_topics(clients* active_clients, int length, int sockfd,
	msg message, int* length_topics, int* size_topics, topics* subjects);

void close_client(int* length, int sockfd, fd_set* read_fds,
	clients* active_clients);