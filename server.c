
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

//      gcc s.c - o - lpthread
//      ./s 8080

struct client_info
{
    int sockno;
    char username[30];
    char ip[INET_ADDRSTRLEN];
};
struct client_file
{
    int fd;
};
struct cli
{
    char username[30];
};
int clients[100];
struct client_file logs[100];
struct cli c[10];
int n = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void sendToAll(char *msg, int curr)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < n; i++)
    {
        if (clients[i] != curr)
        {
            if (send(clients[i], msg, strlen(msg), 0) < 0)
            {
                perror("sending failure...");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void sendToMe(char *msg, int curr)
{
    int i;
    pthread_mutex_lock(&mutex);
    for (i = 0; i < n; i++)
    {
        if (clients[i] == curr)
        {
            if (send(clients[i], msg, strlen(msg), 0) < 0)
            {
                perror("sending failure...");
                continue;
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

void *recvMsg(void *sock)
{
    struct client_info cl = *((struct client_info *)sock);
    char msg[500];
    int len;
    int i;
    int j;
    char join_msg[100];
    join_msg[100] = '\0';

    time_t current_time = time(NULL);
    struct tm *local_time = localtime(&current_time);
    char time_str[100];
    strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S] ", local_time);

    sprintf(join_msg, "\n%s %s joined the chat...\n\n", time_str, cl.username);
    sendToAll(join_msg, cl.sockno);
    memset(msg, '\0', sizeof(join_msg));

    while (1)
    {
        if ((len = recv(cl.sockno, msg, 500, 0)) > 0)
        {
            msg[len] = '\0';

            if (strncmp(msg, "/send ", 6) == 0)
            {
                char recipient_name[100];
                sscanf(msg + 6, "%s", recipient_name);
                int recipient_socket = -1;
                for (int i = 0; i < n; i++)
                {
                    if (strcmp(c[i].username, recipient_name) == 0)
                    {
                        recipient_socket = clients[i];
                        break;
                    }
                }
                if (recipient_socket != -1)
                {
                    pthread_mutex_lock(&mutex);
                    for (i = 0; i < n; i++)
                    {
                        if (clients[i] == recipient_socket)
                        {
                            char join_msg[600];
                            sprintf(join_msg, "Private msg from %s: %s", cl.username, msg + strlen(recipient_name) + 7);
                            if (send(recipient_socket, join_msg, strlen(join_msg), 0) < 0)
                            {
                                perror("sending failure...");
                                continue;
                            }
                            memset(join_msg, '\0', sizeof(join_msg));
                        }
                    }
                    pthread_mutex_unlock(&mutex);
                }
                else
                {

                    join_msg[100] = '\0';
                    sprintf(join_msg, "\nUsername not found...\n\n");
                    sendToMe(join_msg, cl.sockno);
                    memset(msg, '\0', sizeof(join_msg));
                }
            }
            else if (strcmp(msg, "online\n") == 0)
            {
                join_msg[100] = '\0';
                char usernames[1000] = "";
                for (i = 0; i < n; i++)
                {
                    strcat(usernames, c[i].username);
                    strcat(usernames, "\n");
                }
                sprintf(join_msg, "\n%sNumber of online clients: %d\n\n", usernames, n);
                sendToMe(join_msg, cl.sockno);
                memset(msg, '\0', sizeof(join_msg));
            }
            else
            {
                sendToAll(msg, cl.sockno);
            }
            memset(msg, '\0', sizeof(msg));
        }
        else
        {
            join_msg[100] = '\0';
            time_t current_time = time(NULL);
            struct tm *local_time = localtime(&current_time);
            char time_str[100];
            strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S] ", local_time);

            sprintf(join_msg, "\n%s %s left the chat...\n\n", time_str, cl.username);
            sendToAll(join_msg, cl.sockno);
            memset(msg, '\0', sizeof(join_msg));
            break;
        }
    }

    pthread_mutex_lock(&mutex);
    printf("%s with ip %s disconnected...\n", cl.username, cl.ip);
    for (i = 0; i < n; i++)
    {
        if (clients[i] == cl.sockno)
        {
            j = i;
            while (j < (n - 1))
            {
                clients[j] = clients[j + 1];
                strcpy(c[j].username, c[j + 1].username);
                j++;
            }
        }
    }
    n--;
    pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in my_addr, their_addr;
    struct client_info cl;
    socklen_t their_addr_size;
    pthread_t recvt;
    int my_sock;
    int their_sock;
    int portno;
    int len;
    char msg[500];
    char str[100];
    char ip[INET_ADDRSTRLEN];

    if (argc != 2)
    {
        perror("Give correct arguments...");
        exit(1);
    }

    portno = atoi(argv[1]);
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
    {
        perror("Creating Socket unsuccessful...");
        exit(1);
    }
    memset(my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portno);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    their_addr_size = sizeof(their_addr);
    if (bind(my_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) != 0)
    {
        perror("Error while binding...");
        exit(1);
    }
    if (listen(my_sock, 5) != 0)
    {
        perror("Error while listening...");
        exit(1);
    }
    while (1)
    {
        if ((their_sock = accept(my_sock, (struct sockaddr *)&their_addr, &their_addr_size)) < 0)
        {
            perror("Error while accepting...");
            exit(1);
        }
        pthread_mutex_lock(&mutex);
        inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);

        cl.sockno = their_sock;
        strcpy(cl.ip, ip);

        if (read(their_sock, cl.username, 30) <= 0)
        {
            perror("failure while Receiving Message...");
            exit(1);
        }
        strcpy(c[n].username, cl.username);
        printf("%s joined the chat with ip %s connected...\n", c[n].username, cl.ip);
        clients[n] = their_sock;
        n++;

        pthread_create(&recvt, NULL, recvMsg, &cl);
        pthread_mutex_unlock(&mutex);
    }
    pthread_join(recvt, NULL);
    close(my_sock);
}
