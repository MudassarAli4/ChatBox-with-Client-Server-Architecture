
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

//      gcc client.c -o c6 -lpthread
//      ./c6 Masood 8080 Magenta

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

struct sockaddr_in their_addr;
struct client
{
	char username[100];
};
struct client_file
{
	int fd;
};
struct client_file logs[20];
struct client c[10];
int n = 0;
pthread_t recvt;
int my_sock;
int their_sock;
int their_addr_size;
int portno;
int len;
char msg[500];
char stime[20];
int u = 0;
char color[10];
char res[600];
char ip[INET_ADDRSTRLEN];
char color_code[10];
char filename[30];

void *recvMsg(void *sock)
{
	int their_sock = *((int *)sock);
	char msg[500];
	int len;
	sprintf(filename, "%s_chat.txt", c[n].username);
	FILE *file = fopen(filename, "a");

	if (file == NULL)
	{
		printf("Error opening file while receiving.\n");
		exit(1);
	}
	while ((len = recv(their_sock, msg, 500, 0)) > 0)
	{
		msg[len] = '\0';
		strcat(msg, color_code);
		fputs(msg, stdout);
		fputs(msg, file);
		strcat(msg, ANSI_COLOR_RESET);
		memset(msg, '\0', sizeof(msg));
	}
	fclose(file);
}

void send_to_client(char *client_name, char *msg)
{
	time_t current_time = time(NULL);
	struct tm *local_time = localtime(&current_time);
	char time_str[100];
	strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S] ", local_time);

	FILE *file = fopen(filename, "a");
	if (file == NULL)
	{
		printf("Error opening file while sending private msg.\n");
		exit(1);
	}
	sprintf(res, "/send %s %s %s", client_name, time_str, msg);
	fputs(res, file);
	fclose(file);
	send(my_sock, res, strlen(res), 0);
}
int main(int argc, char *argv[])
{

	if (argc > 4 || argc < 3)
	{
		printf("Give correct arguments...");
		exit(1);
	}
	portno = atoi(argv[2]);
	strcpy(c[n].username, argv[1]);
	if (argc == 4)
	{
		strcpy(color, argv[3]);
	}

	my_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (my_sock < 0)
	{
		perror("Creating Socket unsuccessful...");
		exit(1);
	}
	memset(their_addr.sin_zero, '\0', sizeof(their_addr.sin_zero));
	their_addr.sin_family = AF_INET;
	their_addr.sin_port = htons(portno);
	their_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(my_sock, (struct sockaddr *)&their_addr, sizeof(their_addr)) < 0)
	{
		perror("Connection not esatablished...");
		exit(1);
	}
	inet_ntop(AF_INET, (struct sockaddr *)&their_addr, ip, INET_ADDRSTRLEN);

	printf("connected to %s, start chatting\n", ip);
	printf("To send private msg...\t'/send username message'\n");
	printf("To check Online Clients, Type 'online'\n");
	printf("To read all the backup chat, Type 'export'\n");
	pthread_create(&recvt, NULL, recvMsg, &my_sock);

	write(my_sock, c[n].username, strlen(c[n].username));

	while (1)
	{
		sprintf(filename, "%s_chat.txt", c[n].username);
		FILE *file = fopen(filename, "a");
		if (file == NULL)
		{
			printf("Error opening file while sending msg.\n");
			exit(1);
		}
		fgets(msg, 500, stdin);

		time_t current_time = time(NULL);
		struct tm *local_time = localtime(&current_time);
		char time_str[100];
		strftime(time_str, sizeof(time_str), "[%Y-%m-%d %H:%M:%S] ", local_time);

		if (strcmp(color, "red") == 0)
			strcpy(color_code, ANSI_COLOR_RED);
		else if (strcmp(color, "green") == 0)
			strcpy(color_code, ANSI_COLOR_GREEN);
		else if (strcmp(color, "yellow") == 0)
			strcpy(color_code, ANSI_COLOR_YELLOW);
		else if (strcmp(color, "blue") == 0)
			strcpy(color_code, ANSI_COLOR_BLUE);
		else if (strcmp(color, "magenta") == 0)
			strcpy(color_code, ANSI_COLOR_MAGENTA);
		else
			strcpy(color_code, ANSI_COLOR_CYAN);

		if (strncmp(msg, "/send ", 6) == 0)
		{
			char recipient_name[100];
			sscanf(msg + 6, "%s", recipient_name);
			send_to_client(recipient_name, msg + strlen(recipient_name) + 7);
		}
		else if (strcmp(msg, "exit\n") == 0)
		{
			printf("Disconnecting from server...\n");
			exit(1);
		}
		else if (strcmp(msg, "online\n") == 0)
		{
			send(my_sock, msg, strlen(msg), 0);
		}
		else if (strcmp(msg, "export\n") == 0)
		{
			fclose(file);
			FILE *file = fopen(filename, "r");
			if (file == NULL)
			{
				printf("Error opening file while reading...\n");
			}
			char line[500];
			printf("\t\t--->>>\tEXPORTED CHAT START\t<<<---\n\n");
			while (fgets(line, sizeof(line), file))
			{
				printf("%s", line);
			}
			printf("\t\t--->>>\tEXPORTED CHAT FINISHED\t<<<---\n\n");
		}
		else
		{
			fputs(msg, file);
			fclose(file);
			strcpy(res, c[n].username);
			strcat(res, ": ");
			strcat(res, color_code);
			strcat(res, time_str);
			strcat(res, msg);
			strcat(res, ANSI_COLOR_RESET);

			len = send(my_sock, res, strlen(res), 0);
			if (len < 0)
			{
				perror("message not sent...");
				exit(1);
			}
		}
		memset(msg, '\0', sizeof(msg));
		memset(res, '\0', sizeof(res));
	}
	pthread_join(recvt, NULL);
	close(my_sock);
}
