#include "msgdist_utils.h"

pthread_mutex_t mflag;

// -------------------------------------------------------------- verificação das variáveis de ambiente 
// MAXUSERS

int getVarMaxUsers() {
    char * env_var = getenv(VAR_MAX_USERS);

    if (env_var == NULL) // se não estiver definida no ambiente
        return UTIL_MAX_USERS; // fica com o valor da variável definida

    return atoi(env_var); // senão, fica com a variável definida no ambiente
}

// MAXMSG

int getVarMaxMsg() {
    char * env_var = getenv(VAR_MAX_MSG);

    if (env_var == NULL || atoi(env_var) > UTIL_MAX_MSG)
        return UTIL_MAX_MSG;
    return atoi(env_var);
}

// MAXNOT

int getVarMaxNot() {
    char * env_var = getenv(VAR_MAX_NOT);

    if (env_var == NULL)
        return UTIL_MAX_NOT;

    return atoi(env_var);
}

// TIMEOUT

int getVarTimeout() {
    char * env_var = getenv(VAR_TIMEOUT);

    if (env_var == NULL)
        return UTIL_TIMEOUT;

    return atoi(env_var);
}

// FILE

char * getVarFile() {
    char * env_var = getenv(VAR_DFL_FILENAME);

    if (env_var == NULL)
        return UTIL_FILENAME;

    return env_var;
}

// -------------------------------------------------------------- rotina de comandos

// SERVER_FILTER

MSGDIST_SERVER cmdRoutineFilter(char * arg, MSGDIST_SERVER server) {
    if (strcmp(arg, SERVER_FILTER_ON) == 0) {
        server.filter = 1;
        printf("MSGDIST: (SERVER) Message filter is ON\n");
        return server;
    } else if (strcmp(arg, SERVER_FILTER_OFF) == 0) {
        server.filter = 0;
        printf("MSGDIST: (SERVER) Message filter is OFF\n");
        return server;
    }
    printf("MSGDIST: (ERROR) Invalid argument. Try 'help'\n");
}

// SERVER_USERS

void cmdRoutineUsers(PACKAGE * clients, MSGDIST_SERVER server) {
    int i, count = 0;
    for (i = 0; i < server.maxUsers; i++) {
        if (clients[i].flag != -1) {
            printf("User '%s'\t\t[%d] [%s]\n", clients[i].client.username, clients[i].client.id, clients[i].client.pipeName);
            count++;
        }
    }
    if (count == 0) {
        printf("MSGDIST: (SERVER) No users online\n");
    }
}

// SERVER_TOPICS

void cmdRoutineTopics(MSG *allmsg) {
    MSG topics[UTIL_MAX_MSG];
    int i, j = 1, k, flag = 0;


    for (i = 0; i < UTIL_MAX_MSG; i++) {
        strncpy(topics[i].topic, allmsg[i].topic, UTIL_BUFFER_SIZE);
        topics[i].flag = -1;
    }
    printf("\nMSGDIST: (CLIENT) All topics: \n");
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allmsg[i].flag != -1) {
            for (k = i - 1; k >= 0; k--) {
                if (strcmp(topics[i].topic, allmsg[k].topic) == 0) {
                    flag = 1;
                }
            }

            if (flag == 0) {
                printf("\n %d. Topic: '%s' \n", j++, topics[i].topic);
            }
            flag = 0;
        }
    }
}

// SERVER_MSG 

void cmdRoutineMsg(MSG *allmsg) {
    int i, j = 1;

    printf("\nMSGDIST: (SERVER) All messages: \n");
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allmsg[i].flag != -1) {
            printf("%d. Message:", j);
            printf("\n\tTopic: '%s'\n", allmsg[i].topic);
            printf("\tTitle: '%s'\n", allmsg[i].title);
            printf("\tMessage: '%s'\n\n", allmsg[i].body);
            j++;
        }
    }
}

// SERVER_TOPIC

void cmdRoutineTopic(MSG *allmsg) {
    int i, j = 1;
    char arg[UTIL_BUFFER_SIZE];

    printf("\nTopic of the message: ");
    scanf(" %20[^\n]", arg);

    printf("\nMSGDIST: (SERVER) Messages from the topic '%s': \n", arg);
    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allmsg[i].flag != -1 && strcmp(allmsg[i].topic, arg) == 0) {
            printf("%d. Message:", j);
            printf("\n\tTopic: '%s'\n", allmsg[i].topic);
            printf("\tTitle: '%s'\n", allmsg[i].title);
            printf("\tMessage: '%s'\n\n", allmsg[i].body);
            j++;
        }
    }

}

// -------------------------------------------------------------- remove uma mensagem de um tópico

void removeMsgFromTopic(MSGDIST_SERVER server, SUBS_TOPICS *topics, MSG msg) {
    int i;

    for (i = 0; i < server.maxMsg; i++) {
        if (strcmp(msg.topic, topics[i].topic) == 0 && topics[i].flag != -1) {
            if (topics[i].msg != 0) {
                topics[i].msg -= 1;
            }
        }
    }
}

// SERVER_DEL  

void cmdRoutineDel(MSG *allmsg, SUBS_TOPICS *topics, MSGDIST_SERVER server) {
    int i;
    char arg[UTIL_BUFFER_SIZE];

    printf("\nDelete msg with the title : ");
    scanf(" %20[^\n]", arg);


    for (i = 0; i < UTIL_MAX_MSG; i++) {
        if (allmsg[i].flag != -1 && strcmp(allmsg[i].title, arg) == 0) {
            removeMsgFromTopic(server, topics, allmsg[i]);
            allmsg[i].flag = -1;
        }
    }
}

// SERVER_KICK  

void cmdRoutineKick(char * arg, PACKAGE * clients) {
    printf("MSGDIST: (SERVER) kick command\n");
    int i;
    union sigval value;

    for (i = 0; i < clients->maxUsers; i++) {
        if (strcmp(clients[i].client.username, arg) == 0) {
            value.sival_int = i;
            sigqueue(clients[i].client.id, SIGUSR2, value);
            clients[i].flag = -1;
            return;
        }
    }
}

// SERVER_PRUNE 

void cmdRoutinePrune(MSG *allmsg, SUBS_TOPICS *topics, MSGDIST_SERVER server) {
    int i;
    
    for(i=0;i<server.maxMsg;i++){
        if(topics[i].msg==0 && topics[i].flag!=-1){
            topics[i].flag=-1;
             printf("\nMSGDIST: (SERVER) Topic '%s' was deleted.\n", topics[i].topic);
        }
    }
    
}

// SHUTDOWN DOS CLIENTES

void shutdownClients(PACKAGE * clients) {
    int i;
    union sigval value;

    for (i = 0; i < clients->maxUsers; i++) {
        if (clients[i].flag != -1) {
            value.sival_int = i;
            sigqueue(clients[i].client.id, SIGUSR1, value);
        }
    }
}

// SERVER_SHUTDOWN

void cmdRoutineShutdown(PACKAGE * clients, int tid) {
    printf("MSGDIST: (SERVER) shutting down...\n");
            
    // Desconectar os clientes
    shutdownClients(clients);
    unlink(SERVER_PIPE);
    exit(0);
}

// SERVER_HELP

void cmdRoutineHelp() {
    printf("MSGDIST: (SERVER) help:\n"
            "filter <on/off>   \t bad words filter\n"
            "users             \t list all users\n"
            "topics            \t list all topics\n"
            "msg               \t list all messages\n"
            "topic             \t list messages from a topic\n"
            "del               \t delete a message\n"
            "kick   <username> \t kick an user from the system\n"
            "shutdown          \t shutdown the server (this will also kick all users)\n"
            "prune             \t delete all empty topics\n"
            "info              \t information about the system\n");
}

// SERVER_INFO

void cmdRoutineInfo(MSGDIST_SERVER sv) {
    printf("MSGDIST: (SERVER) server info:\n"
            "MAX_USERS \t %d\n"
            "MAX_MSG   \t %d\n"
            "MAX_NOT   \t %d\n"
            "TIMEOUT   \t %d\n"
            "FILENAME  \t '%s'\n"
            "FILTER    \t %d\n", sv.maxUsers, sv.maxMsg, sv.maxNot, sv.timeout, sv.file, sv.filter);
}

// CHAMADA DE FUNÇÕES

MSGDIST_SERVER cmdRoutine(MSGDIST_SERVER sv, PACKAGE * clients, char * cmd, char * arg, MSG *allmsg, SUBS_TOPICS *topics, int tid) {

    if (strcmp(cmd, SERVER_FILTER) == 0) {
        if (strcmp(arg, UTIL_EMPTY) == 0) {
            printf("MSGDIST: (ERROR) This command needs an argument. Try 'help'\n");
        } else {
            sv = cmdRoutineFilter(arg, sv);
        }
    } else if (strcmp(cmd, SERVER_USERS) == 0) {
        cmdRoutineUsers(clients, sv);
    } else if (strcmp(cmd, SERVER_TOPICS) == 0) {
        cmdRoutineTopics(allmsg);
    } else if (strcmp(cmd, SERVER_MSG) == 0) {
        cmdRoutineMsg(allmsg);
    } else if (strcmp(cmd, SERVER_TOPIC) == 0) {
        cmdRoutineTopic(allmsg);
    } else if (strcmp(cmd, SERVER_DEL) == 0) {
        cmdRoutineDel(allmsg, topics, sv);
    } else if (strcmp(cmd, SERVER_KICK) == 0) {
        if (strcmp(arg, UTIL_EMPTY) == 0) {
            printf("MSGDIST: (ERROR) This command needs an argument. Try 'help'\n");
        } else {
            cmdRoutineKick(arg, clients);
        }
    } else if (strcmp(cmd, SERVER_PRUNE) == 0) {
        cmdRoutinePrune(allmsg, topics, sv);
    } else if (strcmp(cmd, SERVER_SHUTDOWN) == 0) {
        cmdRoutineShutdown(clients, tid);
    } else if (strcmp(cmd, SERVER_HELP) == 0) {
        cmdRoutineHelp();
    } else if (strcmp(cmd, SERVER_INFO) == 0) {
        cmdRoutineInfo(sv);
    } else
        printf("MSGDIST: (ERROR) Command not found.\n");
    putchar('\n');
    return sv;
}

// -------------------------------------------------------------- verificador de mensagens

int verifier(char * words, char * file) {
    int p[2], pr[2], res, state;
    char message[UTIL_BODY_SIZE];
    char result[30];

    strncpy(message, words, UTIL_BODY_SIZE);

    pipe(p); //pipe de escrita 
    pipe(pr); //pipe de leitura

    res = fork();
    // caminho do filho
    if (res == 0) {

        //PIPE ANONIMO DE ESCRITA
        close(0); //liberta a posicao de stdin    
        dup(p[0]); //duplica a extremidade de leitura do pipe para a pos 0
        close(p[0]); //fecha a extremidade pq já foi duplicada     
        close(p[1]); //fecha a extremidade de escrita do pipe pq não vai ser utilizada


        //PIPE ANONIMO DE LEITURA

        close(1); //liberta a posicao de stdout    
        dup(pr[1]); //duplica a extremidade de escrita do pipe para a pos 1
        close(pr[1]); //fecha a extremidade pq já foi duplicada  
        close(pr[0]); //fecha a extremidade de leitura do pipe pq não vai ser utilizada


        execl("verificador", "verificador", file, NULL);
        printf("MSGDIST: (ERROR) Failure in opening Verifier. Check name of files\n");
        exit(1);
    }

    // caminho do pai
    close(p[0]); //fecha a extremidade de leitura do pipe de escrita
    close(pr[1]); //fecha a extremidade de escrita do pipe de leitura


    write(p[1], message, strlen(message));
    write(p[1], "\n", 1);
    strncpy(message, "##MSGEND##", UTIL_BUFFER_SIZE);
    write(p[1], message, strlen(message));
    write(p[1], "\n", 1);
    read(pr[0], result, 20);

    close(p[1]); //fecha a extremidade de escrita do pipe de escrita

    close(pr[0]); //fecha a extremidade de leitura do pipe de leitura
    wait(&state); //espera que o processo filho termine


    return atoi(result);
}


// -------------------------------------------------------------- leitura de comandos

void readRoutineCommands(THREAD_PACKAGE * threadPackage) {
    char cmd[UTIL_BUFFER_SIZE];
    char * argument;

    do {
        fflush(stdout);
        fflush(stdin);

        printf("MSGDIST@SERVER >> ");
        scanf(" %19[^\n]s", cmd);

        // Separação do pedido recebido em comando e argumento
        strtok_r(cmd, " ", &argument);

        threadPackage->server = cmdRoutine(threadPackage->server, threadPackage->clientsPackage, cmd, argument, threadPackage->clientsMSG, threadPackage->topics, threadPackage->tid);
    } while (1);
}

//--------------------------------------------------------------- DEBUGGER THREAD_PACKAGE

void debuggerThreadPackage(THREAD_PACKAGE * threadPKG) {
    int i = 0;

    printf("\n________________________________________DEBUGGER_________________________________________________\n");

    printf("Server: %s , MaxUsers: %d , MaxMsg: %d, MaxNot: %d, Timeout: %d \n\n", threadPKG->server.file, threadPKG->server.maxUsers,
            threadPKG->server.maxMsg, threadPKG->server.maxNot, threadPKG->server.timeout);

    printf("\nActive clients:\n");
    for (i = 0; i < threadPKG->server.maxMsg; i++) {
        if (threadPKG->clientsPackage[i].flag != -1) {
            printf("Client: %s , Pipename: %s , Id: %d \n", threadPKG->clientsPackage[i].client.username, threadPKG->clientsPackage[i].client.pipeName,
                    threadPKG->clientsPackage[i].client.id);
        }
    }

    printf("\nMessages: \n");
    for (i = 0; i < threadPKG->server.maxMsg; i++) {
        if (threadPKG->clientsMSG[i].flag != -1) {
            printf("Message:  Topic: %s , Title: %s , Body: %s \n", threadPKG->clientsMSG[i].topic, threadPKG->clientsMSG[i].title, threadPKG->clientsMSG[i].body);
        }
    }

    printf("\n________________________________________DEBUGGER_________________________________________________\n");

}

// -------------------------------------------------------------- Inicialização dos packages dos clientes

PACKAGE * pkgInitialize(MSGDIST_SERVER server) {
    PACKAGE * pkg;
    int i = 0;

    pkg = malloc(sizeof (PACKAGE) * (server.maxUsers));

    if (pkg == NULL) {
        printf("MSGDIST: (ERRO) Memory Allocation \n");
        exit(0);
    }

    for (i = 0; i < server.maxUsers; i++) {
        pkg[i].flag = -1;
        pkg[i].maxUsers = server.maxUsers;
    }


    return pkg;
}

// -------------------------------------------------------------- Inicialização Tópicos subscritos

SUBS_TOPICS * topicsInitialize(MSGDIST_SERVER serverPackage) {
    SUBS_TOPICS *topics;
    int i = 0;

    topics = malloc(sizeof (SUBS_TOPICS) * (serverPackage.maxMsg));

    if (topics == NULL) {
        printf("MSGDIST: (ERRO) Memory Allocation \n");
        exit(0);
    }

    for (i = 0; i < serverPackage.maxMsg; i++) {
        topics[i].flag = -1;
        topics[i].clients = pkgInitialize(serverPackage);
        topics[i].msg = 0;
    }
    return topics;
}


// -------------------------------------------------------------- Inicialização dos packages do servidor

MSGDIST_SERVER serverInitialize() {
    MSGDIST_SERVER server;

    server.maxUsers = getVarMaxUsers();
    server.maxMsg = getVarMaxMsg();
    server.maxNot = getVarMaxNot();
    server.timeout = getVarTimeout();
    server.filter = 1;
    strncpy(server.file, getVarFile(), UTIL_BUFFER_SIZE);

    return server;
}

// -------------------------------------------------------------- Inicialização dos packages dos clientes

MSG * msgInitialize(MSGDIST_SERVER serverPackage) {
    MSG *clientsMSG;
    int i = 0;

    clientsMSG = malloc(sizeof (MSG) * (serverPackage.maxMsg));

    if (clientsMSG == NULL) {
        printf("MSGDIST: (ERRO) Memory Allocation \n");
        exit(0);
    }

    for (i = 0; i < serverPackage.maxMsg; i++) {
        clientsMSG[i].flag = -1;
    }

// -------------------------------------------------------------- remove uma mensagem de um tópico

void removeMsgFromTopic(MSGDIST_SERVER server, SUBS_TOPICS *topics, MSG msg) {
    int i;

    for (i = 0; i < server.maxMsg; i++) {
        if (strcmp(msg.topic, topics[i].topic) == 0 && topics[i].flag != -1) {
            if (topics[i].msg != 0) {
                topics[i].msg -= 1;
            }
        }
    }
}
    return clientsMSG;
}

// -------------------------------------------------------------- Inicialização da package da thread

THREAD_PACKAGE * threadpkgInitialize() {
    THREAD_PACKAGE *threadPKG;

    threadPKG = (THREAD_PACKAGE*) malloc(sizeof (THREAD_PACKAGE));

    if (threadPKG == NULL) {
        printf("MSGDIST: (ERRO) Memory Allocation \n");
        exit(0);
    }

    threadPKG->server = serverInitialize();

    threadPKG->topics = topicsInitialize(threadPKG->server);

    threadPKG->clientsMSG = msgInitialize(threadPKG->server);

    threadPKG->clientsPackage = pkgInitialize(threadPKG->server);

    return threadPKG;

}

// -------------------------------------------------------------- debugger função auxiliar

void debugger(PACKAGE * pkg) {
    int i;

    printf("=================================Debug==================================");
    for (i = 0; i < 10; i++) {
        printf("\nMSGDIST: New client [%s]\n \tPID: %d \tPipe Name: %s\n\n", pkg[i].client.username, pkg[i].client.id, pkg[i].client.pipeName);
        printf("Debug: Pkg.flag= %d\n", pkg[i].flag);
    }
    printf("========================================================================");
}

// -------------------------------------------------------------- subscrição de um cliente a um tópico

int clientSubscribeToTopic(MSG receivedMSG, MSGDIST_CLIENT client, MSGDIST_SERVER server, SUBS_TOPICS *topics) {
    int i, j;
    int flag = -1;
    
    for (i = 0; i <  server.maxMsg; i++) {
        if (strcmp(receivedMSG.topic, topics[i].topic) == 0 && topics[i].flag != -1) {    //se o topico existe
            for (j = 0; j < server.maxUsers; j++) {
                if ((topics[i].clients[j].client.id==client.id) && (topics[i].clients[j].flag != -1)) {
                    flag = 0;
                    topics[i].clients[j].flag = -1;
                    return -1;
                }
            }
            if (flag == -1) {
                for (j = 0; j < server.maxUsers; j++) {
                    if (topics[i].clients[j].flag != -1) {
                        topics[i].clients[j].client.id = client.id;
                        strncpy(topics[i].clients[j].client.pipeName, client.pipeName, UTIL_BUFFER_SIZE);
                        strncpy(topics[i].clients[j].client.username, client.username, UTIL_BUFFER_SIZE);
                        topics[i].clients[j].flag = 1;
                        return 0;
                    }
                }
            }
        }
    }
    return 1;
}

// -------------------------------------------------------------- verificação do username

THREAD_PACKAGE * verifyUsername(MSGDIST_CLIENT clientReceived, THREAD_PACKAGE * threadPKG) {

    int i, j, cnt = 0;
    int maxUsers = threadPKG->server.maxUsers;
    int fd_cli;
    char name[UTIL_BUFFER_SIZE], identifier[UTIL_BUFFER_SIZE];
    int flag = 0;


    strncpy(name, clientReceived.username, UTIL_BUFFER_SIZE);


    for (i = 0; i < maxUsers; i++) {
        if (threadPKG->clientsPackage[i].flag == -1 && flag == 0) {
            for (j = 0; j < maxUsers; j++) {
                // Se o username já existir nos clientes
                if (strcmp(threadPKG->clientsPackage[j].client.username, clientReceived.username) == 0) {
                    strncpy(clientReceived.username, name, UTIL_BUFFER_SIZE);
                    snprintf(identifier, UTIL_BUFFER_SIZE, "%d", ++cnt);
                    strncat(clientReceived.username, identifier, UTIL_BUFFER_SIZE);
                }
            }
            strncpy(threadPKG->clientsPackage[i].client.username, clientReceived.username, UTIL_BUFFER_SIZE);
            threadPKG->clientsPackage[i].client.id = clientReceived.id;
            strncpy(threadPKG->clientsPackage[i].client.pipeName, clientReceived.pipeName, UTIL_BUFFER_SIZE);

            threadPKG->clientsPackage[i].flag = 1;
            flag = 1;
        }
        if (threadPKG->clientsPackage[i].flag != -1 && flag == 0) {
            threadPKG->clientsPackage[i].flag = 1;
        }
    }



    if (flag == 0) {
        printf("\nMSGDIST: (SERVER) Maximum users reachead.\n");
        strncpy(clientReceived.username, "maxUser", UTIL_BUFFER_SIZE);
    } else {
        printf("\nMSGDIST: (SERVER) New client '%s' joined\n\n", clientReceived.username);
    }

    fd_cli = open(clientReceived.pipeName, O_WRONLY); //ENVIAR RESPOSTA
    write(fd_cli, &clientReceived, sizeof (MSGDIST_CLIENT));
    close(fd_cli);


    return threadPKG;
}

// -------------------------------------------------------------- criação de tópico

void createTopic(MSG msg, SUBS_TOPICS *topics, MSGDIST_SERVER server) {
    int i, flag = -1, flag2=0;
    ;

    for (i = 0; i < server.maxMsg; i++) {
        if (strcmp(msg.topic, topics[i].topic) == 0 && topics[i].flag != -1) {
            flag = 0;
        }
    }

    if (flag == -1) {
        for (i = 0; i < server.maxMsg; i++) {
            if (strcmp(msg.topic, topics[i].topic) != 0 && topics[i].flag == -1 && flag2!=1) {
                strncpy(topics[i].topic, msg.topic, UTIL_BUFFER_SIZE);
                topics[i].clients = pkgInitialize(server);
                topics[i].flag = 1;
                printf("\nMSGDIST: (SERVER) New topic '%s' was created!\n", topics[i].topic);
                flag2=1;
            }
        }
    }

}

// -------------------------------------------------------------- verificação de mensagens

THREAD_PACKAGE * verifyMessage(MSG receivedMSG, THREAD_PACKAGE * threadPKG) {
    int i, fd_cli, isMsgOk = 0;
    int words = 0;
    int flag = 0;
    int maxMgs = threadPKG->server.maxMsg;

    if (threadPKG->server.filter == 1) {
        words += verifier(receivedMSG.title, threadPKG->server.file);
        words += verifier(receivedMSG.topic, threadPKG->server.file);
        words += verifier(receivedMSG.body, threadPKG->server.file);
    }

    printf("\nMSGDIST: (SERVER) Verifier detected %d bad words!\n", words);

    if (words >= UTIL_MAX_NOT) {
        printf("\nMSGDIST: (SERVER) Message from '%s' rejected.\n", receivedMSG.client.username);
        isMsgOk = 2;
    } else {
        for (i = 0; i < maxMgs; i++) {
            if (threadPKG->clientsMSG[i].flag == -1 && flag == 0) {
                strncpy(threadPKG->clientsMSG[i].topic, receivedMSG.topic, UTIL_BUFFER_SIZE);
                strncpy(threadPKG->clientsMSG[i].title, receivedMSG.title, UTIL_BUFFER_SIZE);
                strncpy(threadPKG->clientsMSG[i].body, receivedMSG.body, UTIL_BODY_SIZE);
                threadPKG->clientsMSG[i].duration = receivedMSG.duration;
                threadPKG->clientsMSG[i].flag = 1;
                threadPKG->clientsMSG[i].client.id = receivedMSG.client.id;
                strncpy(threadPKG->clientsMSG[i].client.pipeName, receivedMSG.client.pipeName, UTIL_BUFFER_SIZE);
                strncpy(threadPKG->clientsMSG[i].client.username, receivedMSG.client.username, UTIL_BUFFER_SIZE);
                flag = 1;
            }
        }

        if (flag == 0) {
            printf("\nMSGDIST: (SERVER) Maximum messages reachead.\n");
            isMsgOk = 1;
        } else {
            printf("\nMSGDIST: (SERVER) New message from '%s' \n", receivedMSG.client.username);
            createTopic(receivedMSG, threadPKG->topics, threadPKG->server);
        }
    }

    fd_cli = open(receivedMSG.client.pipeName, O_WRONLY); //ENVIAR RESPOSTA
    write(fd_cli, &isMsgOk, sizeof (int));
    close(fd_cli);

    return threadPKG;

}

//--------------------------------------------------------------- Envia todas as mensagens

void sendAllMessages(MSGDIST_CLIENT clientReceived, THREAD_PACKAGE * threadPKG) {
    MSG allMSG[UTIL_MAX_MSG];
    int i;
    int fd_cli;

    for (i = 0; i < threadPKG->server.maxMsg; i++) {
        strncpy(allMSG[i].topic, threadPKG->clientsMSG[i].topic, UTIL_BUFFER_SIZE);
        strncpy(allMSG[i].title, threadPKG->clientsMSG[i].title, UTIL_BUFFER_SIZE);
        strncpy(allMSG[i].body, threadPKG->clientsMSG[i].body, UTIL_BODY_SIZE);
        allMSG[i].duration = threadPKG->clientsMSG[i].duration;
        allMSG[i].flag = threadPKG->clientsMSG[i].flag;
        allMSG[i].client.id = threadPKG->clientsMSG[i].client.id;
        strncpy(allMSG[i].client.pipeName, threadPKG->clientsMSG[i].client.pipeName, UTIL_BUFFER_SIZE);
        strncpy(allMSG[i].client.username, threadPKG->clientsMSG[i].client.username, UTIL_BUFFER_SIZE);
    }

    fd_cli = open(clientReceived.pipeName, O_WRONLY); //ENVIAR RESPOSTA
    write(fd_cli, &allMSG, sizeof (MSG) * UTIL_MAX_MSG);
    close(fd_cli);

    return;
}

//--------------------------------------------------------------- remover um cliente (via shutdown)

void removeClient(MSGDIST_CLIENT * clientReceived, THREAD_PACKAGE * threadPKG) {
    int i;
    for (i = 0; i < threadPKG->server.maxUsers; i++) {
        if (clientReceived->id == threadPKG->clientsPackage[i].client.id) {
            printf("MSGDIST: (SERVER) User '%s' left\n", clientReceived->username);
            threadPKG->clientsPackage[i].flag = -1;
            return;
        }
    }
}

// -------------------------------------------------------------- subscrever a topico

THREAD_PACKAGE * subscribeTopic(MSG msgReceived, MSGDIST_CLIENT client, THREAD_PACKAGE *threadPKG) {
    int r, fd_cli;

    r = clientSubscribeToTopic(msgReceived, client, threadPKG->server, threadPKG->topics);

    if(r=0){
        printf("\nMSGDIST: (CLIENT) Client: %s subscribes now to the topic '%s'. \n", client.username, msgReceived.topic);
    }
    if(r=-1){
        printf("\nMSGDIST: (CLIENT) Client: %s no longer subscribes to the topic '%s'. \n", client.username, msgReceived.topic);
    }
    
    
    
    fd_cli = open(client.pipeName, O_WRONLY); //ENVIAR RESPOSTA
    write(fd_cli, &r, sizeof (int));
    close(fd_cli);

    return threadPKG;

}


// -------------------------------------------------------------- Verifica se um cliente é subscritor de um topico topico

bool verifyClientSubscription( MSGDIST_CLIENT client,MSGDIST_SERVER server, SUBS_TOPICS *topics, MSG msg) {

    int i, j;
    int flag = -1;

    for (i = 0; i < (int) VAR_MAX_MSG; i++) {
        if (strcmp(msg.topic, topics[i].topic) == 0 && topics[i].flag != -1) {
            for (j = 0; j < server.maxUsers; j++) {
                if (topics[i].clients[j].client.id == client.id && topics[i].clients[j].flag != -1) {
                    flag = 0;
                    return true;
                }
            }
        }
    }

    return false;

}

// -------------------------------------------------------------- adiciona uma mensagem a um tópico

void addMsgToTopic(MSGDIST_SERVER server, SUBS_TOPICS *topics, MSG msg) {
    int i;

    for (i = 0; i < server.maxMsg; i++) {
        if (strcmp(msg.topic, topics[i].topic) == 0 && topics[i].flag != -1) {
            topics[i].msg += 1;
        }
    }
}

// -------------------------------------------------------------- recepção de dados (mensagens e clientes)

void * receptionOfData(void * param) {
    INFO_CLIENT receivedMessage;
    THREAD_PACKAGE * threadPKG = (THREAD_PACKAGE*) param;

    int fd, buffer;

    do {
        //mkfifo
        mkfifo(SERVER_PIPE, 0600);
        fd = open(SERVER_PIPE, O_RDONLY);

        if (fd == -1) {
            printf("MSGDIST: (ERROR) Server pipe.\n");
            exit(EXIT_FAILURE);
        }

        buffer = 0;

        receivedMessage.flag = 0;

        buffer = read(fd, &receivedMessage, sizeof (INFO_CLIENT));


        if (buffer == sizeof (INFO_CLIENT)) {
            if (receivedMessage.flag == 1) {
                pthread_mutex_lock(&mflag);
                threadPKG = verifyUsername(receivedMessage.client, threadPKG);
                pthread_mutex_unlock(&mflag);
            }

            if (receivedMessage.flag == 2) {
                pthread_mutex_lock(&mflag);
                threadPKG = verifyMessage(receivedMessage.msg, threadPKG);
                pthread_mutex_unlock(&mflag);
            }
            if (receivedMessage.flag == 3) {
                pthread_mutex_lock(&mflag);
                sendAllMessages(receivedMessage.client, threadPKG);
                pthread_mutex_unlock(&mflag);
            }
            if (receivedMessage.flag == 4) {
                pthread_mutex_lock(&mflag);
                removeClient(&receivedMessage.client, threadPKG);
                pthread_mutex_unlock(&mflag);
            }
            if (receivedMessage.flag == 5) {
                pthread_mutex_lock(&mflag);
                threadPKG = subscribeTopic(receivedMessage.msg, receivedMessage.client, threadPKG);
                pthread_mutex_unlock(&mflag);
            }

        }
        close(fd);
    } while (1);


    pthread_exit(0);
    return 0;

}

// -------------------------------------------------------------- main

int main(int argc, char* argv[], char * envp[]) {
    pthread_t *tasks;
    THREAD_PACKAGE *threadPKG;


    if (access(SERVER_PIPE, F_OK) == 0) {
        printf("MSGDIST: (ERROR) Another server already exists!\n");
        exit(EXIT_FAILURE);
    }
        
    //Inicializar packages
    threadPKG = (THREAD_PACKAGE*) threadpkgInitialize();

    
    // Threads
    tasks = (pthread_t*) malloc(sizeof (pthread_t) * 3); // 3 = numero de threads

    
    if (pthread_create(&tasks[0], NULL, receptionOfData, threadPKG) != 0) {
        printf("MSGDIST: (ERROR) Creating thread.\n");
    }
    
    threadPKG->tid = tasks[0];

    readRoutineCommands(threadPKG);
}
