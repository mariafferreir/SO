#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ncurses.h>


#define UTIL_BUFFER_SIZE    20
#define UTIL_BODY_SIZE      1000
#define UTIL_EMPTY          ""              // "args" vazio

#define VAR_MAX_USERS       "MAXUSERS"
#define VAR_MAX_MSG         "MAXMSG"     
#define VAR_MAX_NOT         "MAXNOT"
#define VAR_DFL_FILENAME    "WORDSNOT"
#define VAR_TIMEOUT         "TIMEOUT"

#define UTIL_TIMEOUT        10              // segundos
#define UTIL_MAX_USERS      10
#define UTIL_MAX_MSG        50
#define UTIL_MAX_NOT        3
#define UTIL_FILENAME       "palavras.txt"

#define SERVER_FILTER       "filter"
#define SERVER_USERS        "users"
#define SERVER_TOPICS       "topics"
#define SERVER_MSG          "msg"
#define SERVER_TOPIC        "topic"
#define SERVER_DEL          "del"
#define SERVER_KICK         "kick"
#define SERVER_SHUTDOWN     "shutdown"
#define SERVER_PRUNE        "prune"
#define SERVER_HELP         "help"
#define SERVER_CHECK        "check"
#define SERVER_INFO         "info"

#define CLIENT_WRITE_MENSAGE       SERVER_MSG
#define CLIENT_TOPICS              SERVER_TOPICS
#define CLIENT_TITLES              "titles"
#define CLIENT_SUBS_TOPIC          "subscribe" 
#define CLIENT_HELP                SERVER_HELP
#define CLIENT_SHUTDOWN            SERVER_SHUTDOWN
#define CLIENT_SHOW_MESSAGE        "showmsg"

#define SERVER_FILTER_ON    "on"
#define SERVER_FILTER_OFF   "off"

#define SERVER_PIPE         "SERVER"
#define CLIENT_PIPE         "CLI%d"

#define USER_KEY_ENTER      10

typedef struct client {
    char username[UTIL_BUFFER_SIZE];
    char pipeName[UTIL_BUFFER_SIZE];
    int id;
} MSGDIST_CLIENT;

typedef struct server {
    char file[UTIL_BUFFER_SIZE];
    int maxUsers;
    int maxMsg;
    int maxNot;
    int timeout;
    int filter; // 1 - ON, 0 - OFF
} MSGDIST_SERVER;

typedef struct msg {
    char topic[UTIL_BUFFER_SIZE];
    char title[UTIL_BUFFER_SIZE];
    char body[UTIL_BODY_SIZE];
    int duration;
    int flag;
    MSGDIST_CLIENT client;
} MSG;

typedef struct pkt {
    MSGDIST_CLIENT client;
    int flag; // se -1 é porque está vazio
    int maxUsers;
} PACKAGE;

typedef struct topics {
    char topic[UTIL_BUFFER_SIZE];
    PACKAGE *clients;
    int msg; //numero de subs
    int flag; //topico ativo
} SUBS_TOPICS;

typedef struct thread {
    PACKAGE *clientsPackage;
    MSG *clientsMSG;
    MSGDIST_SERVER server;
    SUBS_TOPICS *topics;
    int tid;
} THREAD_PACKAGE;

typedef struct senderC {
    MSGDIST_SERVER server;
    MSGDIST_CLIENT client;
    MSG *clientsMSG;
    MSG msg;
    int flag; //1-cliente enviado, 2- mensagem enviada, 3 - todos os topicos ou titulos enviar tds as mensagens
} INFO_CLIENT;

#endif /* UTILS_H */

