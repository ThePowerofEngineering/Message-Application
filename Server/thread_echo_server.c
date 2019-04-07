/* echo_server_thread.c
 *
 * Copyright (c) 2000 Sean Walton and Macmillan Publishers.  Use may be in
 * whole or in part in accordance to the General Public License (GPL).
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
*/

/*****************************************************************************/
/*** echo_server_thread.c                                                  ***/
/***                                                                       ***/
/*** An echo server using threads.                                         ***/
/***                                                                       ***/
/*** Compile : gcc echo_server_thread.c -o echo_server_thread -lpthread    ***/
/*****************************************************************************/
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include "iniparser.h"

/* Definations */
#define DEFAULT_BUFLEN 1024

int port;
char *rootDir;
char *greeting;
const char *INI_NAME = "server.ini";

void parse_conf();
char *parse_user(char *userID, char *pass);
char *executeCommand(char *user, char *line);

void PANIC(char *msg);
#define PANIC(msg)   \
    {                \
        perror(msg); \
        exit(-1);    \
    }

//*******************************************************************************
// Definition of function Child(). While client is connected this function will *
// get client messages and call other functions to process the messages.        *
//*******************************************************************************

void *Child(void *arg)
{
    char line[DEFAULT_BUFLEN];
    int bytes_read;
    int client = *(int *)arg;
    char user[10], pass[10];
    int equal = 1;

    // Print greeting message
    if ((bytes_read = send(client, greeting, strlen(greeting), 0)) < 0)
        printf("Send failed\n");

    // Authenticate user
    do
    {
        bytes_read = recv(client, line, DEFAULT_BUFLEN, 0);
        if (bytes_read > 0)
        {
            printf("Client: %s", line);

            char *temp = strtok(line, " ");
            temp = strtok(NULL, " ");
            strcpy(user, temp);
            temp = strtok(NULL, "\n");
            strcpy(pass, temp);

            temp = parse_user(user, pass);
            equal = strcmp(temp, "+OK UserID and password okay go ahead.\n");

            if ((bytes_read = send(client, temp, strlen(temp), 0)) < 0)
            {
                printf("Send failed\n");
            }
        }
        else if (bytes_read == 0)
        {
            printf("Connection closed by client\n");
        }
        else
        {
            printf("Connection has problem\n");
        }
    } while (equal != 0);

    // Get commands from client
    do
    {
        char *respExeCmd;
        memset(line, '\0', DEFAULT_BUFLEN);

        bytes_read = recv(client, line, sizeof(line), 0);
        if (bytes_read > 0)
        {
            printf("Client: %s", line);
            respExeCmd = executeCommand(user, line);

            if (strstr(respExeCmd, "+OK Good Bye") != NULL)
            {
                if ((bytes_read = send(client, respExeCmd, strlen(respExeCmd), 0)) < 0)
                    printf("Send failed\n");
                break;
            }
            else
            {
                if ((bytes_read = send(client, respExeCmd, strlen(respExeCmd), 0)) < 0)
                    printf("Send failed\n");
            }
        }
        else if (bytes_read == 0)
        {
            printf("Connection closed by client\n");
            break;
        }
        else
        {
            printf("Connection has problem\n");
            break;
        }
    } while (bytes_read > 0);

    close(client);
    return arg;
}

//*************************************************************************
// Definition of function main(). Setups server and waits for connections *
// For each child separate thread will be created. After terminated child *
// no need to clean up.                                                   *
//*************************************************************************

int main()
{
    int sd, optval;
    struct sockaddr_in addr;

    // Parse configuration from ini file
    parse_conf();

    // Create a socket
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        PANIC("Socket");

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    optval = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

    // Call bind()
    if (bind(sd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        PANIC("Bind");

    // Call listen()
    if (listen(sd, SOMAXCONN) != 0)
        PANIC("Listen");

    // Accept the client and create a child thread
    while (1)
    {
        int client, addr_size = sizeof(addr);
        pthread_t child;

        client = accept(sd, (struct sockaddr *)&addr, &addr_size);
        printf("Connected: %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        if (pthread_create(&child, NULL, Child, &client) != 0)
            perror("Thread creation");
        else
            pthread_detach(child); /* disassociate from parent */
        pthread_join(child, NULL);
    }
    return 0;
}

//*************************************************************************
// Definition of function executeCommand(). Takes user ID with user input *
// line and executes command according to specified keyword. User may in- *
// put at most three values, they are dividied into parts and assigned to *
// variables for later usage.                                             *
//*************************************************************************

char *executeCommand(char *user, char *line)
{
    char *cmd = NULL, *id = NULL, *msg = NULL;
    line = strtok(line, " \n");
    cmd = line;
    line = strtok(NULL, " \n");
    id = line;
    line = strtok(NULL, "\"");
    msg = line;

    if (strcmp(cmd, "LIST") == 0)
    {
        DIR *d;
        struct dirent *dir;
        char *fileName, *time, *sender, *size;
        char response[DEFAULT_BUFLEN];
        char allMsgs[DEFAULT_BUFLEN];

        memset(response, '\0', DEFAULT_BUFLEN);
        memset(allMsgs, '\0', DEFAULT_BUFLEN);

        int count = 0;
        int totalSize = 0;
        int found = 0;

        d = opendir(user);
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                fileName = dir->d_name;
                if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
                {
                    fileName = strtok(fileName, "_");
                    time = fileName;
                    fileName = strtok(NULL, "_");
                    sender = fileName;
                    fileName = strtok(NULL, ".");
                    size = fileName;
                    count++;
                    if (id != NULL && count == atoi(id))
                    {
                        snprintf(allMsgs, DEFAULT_BUFLEN, "+OK %d %s %s %s\n", count, time, sender, size);
                        found = 1;
                        break;
                    }
                    else
                    {
                        totalSize += atoi(size);
                        snprintf(allMsgs + strlen(allMsgs), DEFAULT_BUFLEN - strlen(allMsgs), "%d %s %s %s\n", count, time, sender, size);
                    }
                }
            }
            if (count == 0)
            {
                snprintf(response, sizeof(response), "-ERR There are no messages in your mailbox\n");
            }
            else
            {
                if (id == NULL)
                {
                    snprintf(response, sizeof(response), "+OK %d messages (%d Octets)\n", count, totalSize);
                    snprintf(response + strlen(response), DEFAULT_BUFLEN - strlen(response), "%s.\n", allMsgs);
                }
                else
                {
                    if (found)
                        strcpy(response, allMsgs);
                    else
                        snprintf(response, sizeof(response), "-ERR no such message, only %d messages in your maildrop\n", count);
                }
            }
        }
        else
        {
            snprintf(response, sizeof(response), "-ERR There are no messages in your mailbox\n");
        }

        closedir(d);
        return response;
    }
    else if (strcmp(cmd, "RET") == 0)
    {
        DIR *d;
        FILE *fileMsg;
        struct dirent *dir;
        char *fileName, *time, *sender, *size;
        char response[DEFAULT_BUFLEN];
        char message[DEFAULT_BUFLEN];
        char filePath[100];

        memset(response, '\0', DEFAULT_BUFLEN);

        int count = 0;
        int found = 0;

        d = opendir(user);
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                fileName = dir->d_name;
                snprintf(filePath, 100, "%s/%s", user, fileName);
                if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
                {
                    fileName = strtok(fileName, "_");
                    time = fileName;
                    fileName = strtok(NULL, "_");
                    sender = fileName;
                    fileName = strtok(NULL, ".");
                    size = fileName;
                    count++;
                    if (id != NULL && count == atoi(id))
                    {
                        fileMsg = fopen(filePath, "r");
                        snprintf(response, DEFAULT_BUFLEN, "+OK %d %s %s %s Octets\n", count, time, sender, size);
                        if (fileMsg != NULL)
                        {
                            while (fgets(message, DEFAULT_BUFLEN, fileMsg) != NULL)
                            {
                                snprintf(response + strlen(response), DEFAULT_BUFLEN - strlen(response), "%s", message);
                            }
                            strcat(response, ".\n");
                        }
                        else
                        {
                            printf("Can't open the file\n");
                        }
                        found = 1;
                        fclose(fileMsg);
                        break;
                    }
                }
            }
            if (found == 0)
            {
                snprintf(response, sizeof(response), "-ERR no such message available\n");
            }
        }
        else
        {
            snprintf(response, sizeof(response), "-ERR no such message available\n");
        }

        return response;
    }
    else if (strcmp(cmd, "DEL") == 0)
    {
        DIR *d;
        FILE *fileMsg;
        struct dirent *dir;
        char *fileName, *time, *sender, *size;
        char response[DEFAULT_BUFLEN];
        char message[DEFAULT_BUFLEN];
        char filePath[100];

        memset(response, '\0', DEFAULT_BUFLEN);

        int count = 0;
        int status = 1;

        d = opendir(user);
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                fileName = dir->d_name;
                snprintf(filePath, 100, "%s/%s", user, fileName);
                if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
                {
                    fileName = strtok(fileName, "_");
                    time = fileName;
                    fileName = strtok(NULL, "_");
                    sender = fileName;
                    fileName = strtok(NULL, ".");
                    size = fileName;
                    count++;
                    if (id != NULL && count == atoi(id))
                    {
                        status = remove(filePath);
                        if (status == 0)
                        {
                            snprintf(response, sizeof(response), "+OK message %s deleted\n", id);
                        }
                        break;
                    }
                }
            }
            if (status == 1)
                snprintf(response, sizeof(response), "-ERR no such a message\n");
        }
        else
        {
            snprintf(response, sizeof(response), "-ERR no such a message\n");
        }

        return response;
    }
    else if (strcmp(cmd, "SEND") == 0)
    {
        char msgName[DEFAULT_BUFLEN];
        int timeStamp = time(NULL);
        char ok[40];
        char err[40];
        FILE *fileMsg;

        mkdir(id, 0700);
        snprintf(msgName, sizeof(msgName), "%s/%d_%s_%d.txt", id, timeStamp, user, strlen(msg) - 1);

        fileMsg = fopen(msgName, "w");
        if (fileMsg != NULL)
        {
            // File is created
            fputs(msg, fileMsg);
            snprintf(ok, sizeof(ok), "%s %s\n", "+OK message is sent to", id);
            fclose(fileMsg);
            return ok;
        }
        else
        {
            // File not created
            snprintf(err, sizeof(err), "%s %s\n", "-ERR message can not be sent to", id);
            return err;
        }
    }
    else if (strcmp(cmd, "QUIT") == 0)
    {
        char bye[30];
        snprintf(bye, sizeof(bye), "%s %s\n", "+OK Good Bye", user);
        return bye;
    }
    else
        return "Please provide correct option\n";
}

//******************************************************************
// Definition of function parse_conf(). Takes all necessary values *
// from server.ini file and assigns them to global variables.      *
//******************************************************************

void parse_conf()
{
    dictionary *dict;

    dict = iniparser_load(INI_NAME);
    if (dict == NULL)
    {
        fprintf(stderr, "Cannot parse file: %s\n", INI_NAME);
        exit(1);
    }

    // Get port number, root directory and greeting message from server.ini
    port = iniparser_getint(dict, "server:listenport", -1);
    rootDir = iniparser_getstring(dict, "server:serverroot", NULL);
    greeting = iniparser_getstring(dict, "server:servermsg", NULL);
    sprintf(greeting, "%s is ready\n", greeting);
}

//*********************************************************************
// Definition of function parse_user(). Compares provided user ID and *
// password with values stored in the server.ini. Returns appropriate *
// message after authentication.                                      *
//*********************************************************************

char *parse_user(char *user_key, char *pass)
{
    dictionary *dict;
    const char *str;
    char *msg;
    char users_sec[] = "users:";

    dict = iniparser_load(INI_NAME);
    if (dict == NULL)
    {
        fprintf(stderr, "Cannot parse file: %s\n", INI_NAME);
        return NULL;
    }

    str = iniparser_getstring(dict, strcat(users_sec, user_key), NULL);

    if (strcmp(str, pass) == 0)
    {
        msg = "+OK UserID and password okay go ahead.\n";
    }
    else
    {
        msg = "-ERR Either UserID or Password is wrong.\n";
    }
    iniparser_freedict(dict);
    return msg;
}
