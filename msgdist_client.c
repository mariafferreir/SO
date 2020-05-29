#include "msgdist_utils.h"
//------------------------------------------------------------------ sinal de encerramento do cliente

void serverOffline() {
    char pipeName[20];

    snprintf(pipeName, UTIL_BUFFER_SIZE, CLIENT_PIPE, getpid());
    printf("MSGDIST: (CLIENT) Server is offline. Exiting now...\n");

    unlink(pipeName); //Remover fifo do cliente
    exit(0);
}

//------------------------------------------------------------------ sinal de kick do cliente

void kickedFromServer() {
    char pipeName[20];

    snprintf(pipeName, UTIL_BUFFER_SIZE, CLIENT_PIPE, getpid());
    printf("MSGDIST: (CLIENT) Server kicked you. Exiting now...\n");

    unlink(pipeName); //Remover fifo do cliente
    exit(0);
}

//------------------------------------------------------------------ configuração do cliente

MSGDIST_CLIENT initClient(char username[UTIL_BUFFER_SIZE]) {
    INFO_CLIENT sendMSG;
    int fd_server, n, fd_client;
    char pipeName[UTIL_BUFFER_SIZE];
    MSGDIST_CLIENT client;

    // Username
    strncpy(sendMSG.client.username, username, UTIL_BUFFER_SIZE);

    // ID
    sendMSG.client.id = getpid();

    // Nome do pipe
    snprintf(pipeName, UTIL_BUFFER_SIZE, CLIENT_PIPE, getpid());

    strncpy(sendMSG.client.pipeName, pipeName, UTIL_BUFFER_SIZE);

    sendMSG.flag = 1;

    printf("MSGDIST: (CLIENT) Confirming username, sending to server...\n");

    // Criar fifo do cliente
    mkfifo(pipeName, 0600);

    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    n = write(fd_server, &sendMSG, sizeof (INFO_CLIENT)); //enviar para servidor

    // Recebe resposta
    fd_client = open(pipeName, O_RDONLY);
    n = read(fd_client, &client, sizeof (MSGDIST_CLIENT));
    close(fd_client);
    close(fd_server);


    if (strcmp(client.username, "maxUser") == 0) {
        printf("\nMSGDIST: (ERROR) Maximum users in server was reached\n");
        sleep(5);
        exit(EXIT_FAILURE);
    } else if (strcmp(client.username, username) != 0) {
        printf("\nMSGDIST: (CLIENT) Username already in use.\n");

    }

    printf("\nMSGDIST: (CLIENT) Username received: %s\n", client.username);

    sendMSG.flag = 0;
    return client;
}

//------------------------------------------------------------------ inserção de um caracter

MSG * insert_ch(char c, int index, int line, MSG * msg) {
    int i;
    char * phrase = (char*) malloc(sizeof (char)*UTIL_BUFFER_SIZE);

    index -= 4;

    switch (line) {
        case 1:
            strncpy(phrase, msg->topic, UTIL_BUFFER_SIZE);
            break;
        case 3:
            strncpy(phrase, msg->title, UTIL_BUFFER_SIZE);
            break;
        case 5:
        default:
            strncpy(phrase, msg->body, UTIL_BODY_SIZE);
            break;
    }

    for (i = UTIL_BUFFER_SIZE; i >= index; i--) {
        phrase[i] = phrase[i - 1];
    }

    phrase[index] = c;

    switch (line) {
        case 1:
            strncpy(msg->topic, phrase, UTIL_BUFFER_SIZE);
            break;
        case 3:
            strncpy(msg->title, phrase, UTIL_BUFFER_SIZE);
            break;
        case 5:
        default:
            strncpy(msg->body, phrase, UTIL_BODY_SIZE);
            break;
    }

    free(phrase);

    return msg;
}

//------------------------------------------------------------------ remoção de um caracter

MSG * delete_ch(int index, int line, MSG * msg) {
    int i;
    char *phrase = (char*) malloc(sizeof (char)*UTIL_BUFFER_SIZE);
    index -= 4;

    switch (line) {
        case 1:
            strncpy(phrase, msg->topic, UTIL_BUFFER_SIZE);
            break;
        case 3:
            strncpy(phrase, msg->title, UTIL_BUFFER_SIZE);
            break;
        case 5:
        default:
            strncpy(phrase, msg->body, UTIL_BODY_SIZE);
            break;
    }


    for (i = index; i <= UTIL_BUFFER_SIZE; i++)
        phrase[i] = phrase[i + 1];

    phrase[index] = ' ';

    switch (line) {
        case 1:
            strncpy(msg->topic, phrase, UTIL_BUFFER_SIZE);
            break;
        case 3:
            strncpy(msg->title, phrase, UTIL_BUFFER_SIZE);
            break;
        case 5:
        default:
            strncpy(msg->body, phrase, UTIL_BODY_SIZE);
            break;
    }

    free(phrase);

    return msg;
}

//--------------------------------------------------------------------------------- envio de uma mensagem

void sendMessage(MSGDIST_CLIENT * client, MSG * msg) {
    INFO_CLIENT sendMSG;
    int fd_server, n, fd_client;

    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    strncpy(sendMSG.msg.topic, msg->topic, UTIL_BUFFER_SIZE);
    strncpy(sendMSG.msg.title, msg->title, UTIL_BUFFER_SIZE);
    strncpy(sendMSG.msg.body, msg->body, UTIL_BODY_SIZE);
    strncpy(sendMSG.msg.client.pipeName, client->pipeName, UTIL_BUFFER_SIZE);

    sendMSG.msg.client.id = client->id;
    strncpy(sendMSG.msg.client.username, client->username, UTIL_BUFFER_SIZE);

    sendMSG.msg.duration = 150;
    sendMSG.flag = 2;

    n = write(fd_server, &sendMSG, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);

    int response;
    // Recebe resposta
    fd_client = open(client->pipeName, O_RDONLY);
    n = read(fd_client, &response, sizeof (int));
    close(fd_client);

    if (response == 0)
        printf("\nMSGDIST: (CLIENT) Your message was received in the server.\n");
    else if (response == 1)
        printf("\nMSGDIST: (CLIENT) Maximum number of messages reached in the server.\n");
    else if (response == 2)
        printf("\nMSGDIST: (CLIENT) Message rejected. Be polite!\n");

}

//--------------------------------------------------------------------------------- escreve mensagem

void writeMessage(MSGDIST_CLIENT * client) {
    WINDOW *win;
    int nrow, ncol, posx, posy, oposx, oposy;
    int ch, flag = 0;

    MSG * msg = (MSG*) malloc(sizeof (MSG));
    if (!msg) {
        printf("\nMSGDIST: (ERROR) Memory Allocation in 'cmdRoutineWriteMessage'.\n");
        return;
    }

    initscr();
    clear();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, nrow, ncol);
    win = newwin(24, 80, 0, 0);
    posy = 1;
    posx = 4;

    refresh();

    mvprintw(0, 0, "TOPIC: \n");
    mvprintw(1, 0, ">> \n");
    mvprintw(2, 0, "TITLE: \n");
    mvprintw(3, 0, ">> \n");
    mvprintw(4, 0, "MESSAGE: \n");
    mvprintw(5, 0, ">> \n");
    move(posy, posx);
    do {
        ch = getch();
        oposy = posy;
        oposx = posx;
        switch (ch) {
            case KEY_UP:
                if (posy == 3 || posy == 5) {
                    posy -= 2;
                    posx = 4;
                } else if (posy > 1) {
                    posy--;
                    posx = 4;
                }

                break;
            case KEY_DOWN:
                if (posy == 1 || posy == 3) {
                    posy += 2;
                    posx = 4;
                } else if (posy < (nrow - 1)) {
                    posy++;
                    posx = 4;
                }
                break;
            case KEY_LEFT:
                posx = (posx > 4) ? posx - 1 : posx;
                break;
            case KEY_RIGHT:
                posx = (posx < (ncol - 1)) ? posx + 1 : posx;
                break;
            case KEY_BACKSPACE:
                if (posx > 4) {
                    posx = posx - 1;
                    mvdelch(posy, posx);

                    msg = delete_ch(posx, posy, msg);
                }
                break;
            case USER_KEY_ENTER:
                flag = 1;
                break;
        }
        if (ch != KEY_UP && ch != KEY_DOWN && ch != KEY_LEFT && ch != KEY_RIGHT && ch != USER_KEY_ENTER && ch != KEY_BACKSPACE) {
            if (posx <= ncol - 1) {
                mvinsch(posy, posx, ch); //insert the character before cursor in a curses window
                msg = insert_ch(ch, posx, posy, msg);

                if (posx < ncol - 1) {
                    posx = posx + 1;
                }
            }
        }
        move(posy, posx);
        refresh();
    } while (flag == 0);
    delwin(win);
    endwin();

    sendMessage(client, msg);
    free(msg);
}

//--------------------------------------------------------------------------------- rotina de comandos

// CLIENT_TOPICS

void cmdRoutineShowTopics(MSGDIST_CLIENT *client) {
    MSG allMSG[UTIL_MAX_MSG], topics[UTIL_MAX_MSG];
    INFO_CLIENT sendMessage;
    int fd_server, n, fd_client, i, j = 1, k, flag = 0;

    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    strncpy(sendMessage.client.pipeName, client->pipeName, UTIL_BUFFER_SIZE);
    sendMessage.client.id = client->id;
    strncpy(sendMessage.client.username, client->username, UTIL_BUFFER_SIZE);
    sendMessage.flag = 3;

    n = write(fd_server, &sendMessage, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);


    // Recebe resposta
    fd_client = open(client->pipeName, O_RDONLY);
    n = read(fd_client, &allMSG, sizeof (MSG) * UTIL_MAX_MSG);
    close(fd_client);

    if (n != sizeof (allMSG)) {
        printf("\nMSGDIST: (CLIENT) Error while receiving message from the server.\n");
        return;
    }


    for (i = 0; i < UTIL_MAX_MSG; i++) {
        strncpy(topics[i].topic, allMSG[i].topic, UTIL_BUFFER_SIZE);
        topics[i].flag = -1;
    }
    printf("\nMSGDIST: (CLIENT) All topics: \n");
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allMSG[i].flag != -1) {
            for (k = i - 1; k >= 0; k--) {
                if (strcmp(topics[i].topic, allMSG[k].topic) == 0) {
                    flag = 1;
                }
            }

            if (flag == 0) {
                printf("\n %d. Topic: '%s' \n", j++, topics[i].topic);
            }
            flag = 0;
        }
    }

    return;
}

// CLIENT_TITLES

void cmdRoutineTitles(char * arg, MSGDIST_CLIENT *client) {
    MSG allMSG[UTIL_MAX_MSG];
    INFO_CLIENT sendMessage;
    int fd_server, n, fd_client, i, j = 1;

    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    strncpy(sendMessage.client.pipeName, client->pipeName, UTIL_BUFFER_SIZE);
    sendMessage.client.id = client->id;
    strncpy(sendMessage.client.username, client->username, UTIL_BUFFER_SIZE);
    sendMessage.flag = 3;

    n = write(fd_server, &sendMessage, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);


    // Recebe resposta
    fd_client = open(client->pipeName, O_RDONLY);
    n = read(fd_client, &allMSG, sizeof (MSG) * UTIL_MAX_MSG);
    close(fd_client);

    if (n != sizeof (allMSG)) {
        printf("\nMSGDIST: (CLIENT) Error while receiving message from the server.\n");
        return;
    }


    printf("\nMSGDIST: (CLIENT) All titles from the topic '%s': \n", arg);
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allMSG[i].flag != -1 && strcmp(arg, allMSG[i].topic) == 0) {
            printf("\n %d. Title: '%s' \n", j, allMSG[i].title);
            j++;
        }
    }

    return;
}

// CLIENT_SUBS_TOPIC

void cmdRoutineSubscribe(MSGDIST_CLIENT * client) {
    INFO_CLIENT sendMessage;
    int fd_server, n, fd_client, r, j = 1, k, flag = -1;
    char str[UTIL_BUFFER_SIZE];
    MSG *allMSG;


    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    strncpy(sendMessage.client.pipeName, client->pipeName, UTIL_BUFFER_SIZE);
    sendMessage.client.id = client->id;
    strncpy(sendMessage.client.username, client->username, UTIL_BUFFER_SIZE);
    sendMessage.flag = 5;


    printf("Title of the topic: ");
    scanf(" %20[^\n]", sendMessage.msg.topic);


    n = write(fd_server, &sendMessage, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);


    // Recebe resposta
    fd_client = open(client->pipeName, O_RDONLY);
    n = read(fd_client, &r, sizeof (int));
    close(fd_client);



    if (r == -1) { //Deixou de subscrever
        printf("\nMSGDIST: (CLIENT) The client no longer subscribes to the topic '%s'. \n", sendMessage.msg.topic);
    }
    if (r == 0) { //Começou a subscrever
        printf("\nMSGDIST: (CLIENT) The client now subscribes to the topic '%s'. \n", sendMessage.msg.topic);
    }
    if (r == 1) { //Topico não existe
        printf("\nMSGDIST: (CLIENT) The topic '%s' does not exists. \n", sendMessage.msg.topic);
    }
}

// CLIENT_SHOW_MESSAGE

void cmdRoutineShowMessage(MSGDIST_CLIENT * client) {
    MSG allMSG[UTIL_MAX_MSG];
    INFO_CLIENT sendMessage;
    int fd_server, n, fd_client, i, j = 1;
    char arg[UTIL_BUFFER_SIZE], arg1[UTIL_BUFFER_SIZE];


    // Abrir para escrita pipe do servidor
    fd_server = open(SERVER_PIPE, O_WRONLY);

    strncpy(sendMessage.client.pipeName, client->pipeName, UTIL_BUFFER_SIZE);
    sendMessage.client.id = client->id;
    strncpy(sendMessage.client.username, client->username, UTIL_BUFFER_SIZE);
    sendMessage.flag = 3;

    n = write(fd_server, &sendMessage, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);


    // Recebe resposta
    fd_client = open(client->pipeName, O_RDONLY);
    n = read(fd_client, &allMSG, sizeof (MSG) * UTIL_MAX_MSG);
    close(fd_client);

    if (n != sizeof (allMSG)) {
        printf("\nMSGDIST: (CLIENT) Error while receiving message from the server.\n");
        return;
    }


    printf("\nTopic of the message: ");
    scanf(" %20[^\n]", arg1);

    printf("\nMSGDIST: (CLIENT) Choose one of the titles from the topic '%s': \n", arg1);
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allMSG[i].flag != -1 && strcmp(arg1, allMSG[i].topic) == 0) {
            printf("\n %d. Title: '%s' \n", j, allMSG[i].title);
            j++;
        }
    }
    printf("Title of the message: ");
    scanf(" %20[^\n]", arg);



    j = 0;

    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allMSG[i].flag != -1 && strcmp(arg, allMSG[i].title) == 0) {
            printf("\nMSGDIST: (CLIENT) Message with the title '%s': \n", arg);
            printf("\n\tTopic: '%s'\n", allMSG[i].topic);
            printf("\tTitle: '%s'\n", allMSG[i].title);
            printf("\tMessage: '%s'\n\n", allMSG[i].body);
            j = 1;
        }
    }

    if (j == 0) {
        printf("\nMSGDIST: (CLIENT) Message with the title '%s' does not exists \n", arg);
    }
}

// CLIENT_HELP

void cmdRoutineHelp() {
    printf("MSGDIST: (CLIENT) help:\n"
            "topics                  \t list all topics\n"
            "msg                     \t write a message\n"
            "titles       <topic>    \t list titles from a topic\n"
            "subscribe    <topic>    \t subscribe to a topic\n"
            "showmsg                 \t show message from a topic\n");
}

// CLIENT_SHUTDOWN

void cmdRoutineShutdown(char * name) {
    INFO_CLIENT info;
    int fd_server, n;
    char pipeName[20];

    info.flag = 4;
    info.client.id = getpid();
    strncpy(info.client.username, name, UTIL_BODY_SIZE);

    fd_server = open(SERVER_PIPE, O_WRONLY);
    n = write(fd_server, &info, sizeof (INFO_CLIENT)); //enviar para servidor
    close(fd_server);

    printf("\nMSGDIST: (CLIENT) shutting down....\n");
    snprintf(pipeName, UTIL_BUFFER_SIZE, CLIENT_PIPE, getpid());
    unlink(pipeName); //Remover fifo do cliente
    exit(0);

}

// CHAMADA DE FUNÇÕES

void cmdRoutine(MSGDIST_CLIENT * client, char * cmd, char * arg) {

    if (strcmp(cmd, CLIENT_WRITE_MENSAGE) == 0) {
        writeMessage(client);
    } else if (strcmp(cmd, CLIENT_TOPICS) == 0) {
        cmdRoutineShowTopics(client);
    } else if (strcmp(cmd, CLIENT_TITLES) == 0) {
        if (strcmp(arg, UTIL_EMPTY) == 0) {
            printf("MSGDIST: (ERROR) This command needs an argument. Try 'help'\n");
        } else {
            cmdRoutineTitles(arg, client);
        }
    } else if (strcmp(cmd, CLIENT_SUBS_TOPIC) == 0) {
        cmdRoutineSubscribe(client);
    } else if (strcmp(cmd, CLIENT_HELP) == 0) {
        cmdRoutineHelp();
    } else if (strcmp(cmd, CLIENT_SHOW_MESSAGE) == 0) {
        cmdRoutineShowMessage(client);
    } else if (strcmp(cmd, CLIENT_SHUTDOWN) == 0) {
        cmdRoutineShutdown(client->username);
    } else {
        printf("MSGDIST: (ERROR) Command not found.\n");
        putchar('\n');
    }
}

// -------------------------------------------------------------- leitura de comandos

void readRoutineCommands(MSGDIST_CLIENT * client) {
    char cmd[UTIL_BUFFER_SIZE];
    char * argument;

    do {
        fflush(stdout);
        fflush(stdin);

        printf("MSGDIST@CLIENT >> ");
        scanf(" %19[^\n]s", cmd);

        // Separação do pedido recebido em comando e argumento
        strtok_r(cmd, " ", &argument);

        cmdRoutine(client, cmd, argument);
    } while (1);
}

int main(int argc, char* argv[], char * envp[]) {
    MSGDIST_CLIENT client;
    struct sigaction offlineSignal, kickSignal;

    // Leitura dos argumentos na linha de comandos
    if (argc != 2) {
        printf("MSGDIST: (ERROR) Number of agrs!\n");
        exit(EXIT_FAILURE);
    }

    // Verificar se o servidor se encontra ativo
    if (access(SERVER_PIPE, F_OK) != 0) {
        printf("MSGDIST: (ERROR) Server is offline!\n");
        exit(EXIT_FAILURE);
    }

    printf("MSGDIST: (CLIENT) initialized with PID %d\n\n", getpid());

    client = initClient(argv[1]);

    offlineSignal.sa_flags = SA_SIGINFO;
    offlineSignal.sa_sigaction = serverOffline;

    kickSignal.sa_flags = SA_SIGINFO;
    kickSignal.sa_sigaction = kickedFromServer;

    sigaction(SIGUSR1, &offlineSignal, NULL);
    sigaction(SIGUSR2, &kickSignal, NULL);

    readRoutineCommands(&client);
}


