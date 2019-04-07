/*
  This is simple tcp client. It is demostrate how a tcp client work.
  You should compile this application with gcc -I. client.c libiniparser.a -o client
  Example usage;
  ./client

*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>

/* Socket API headers */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>

/* Definitions */
#include "iniparser.h"
#define DEFAULT_BUFLEN 512
#define LINE_MAX 1024 /* max number of bytes we can get at once */
#define PORT 2724   /* the port client will be connecting to */

 //   #define MAXDATASIZE 100

//#define PORT 1080
#define BUF_LEN  2000
#define STDIN 0
#define SADDR "127.0.0.1"
#define TV_SEC 10

// Ugur AKYOL


//const char *INI_NAME = "server.ini";
const char *INI_NAME = "client.ini";

char *serverip;
char *port;
char *username;
char *userid;
char *userpass;

char *recieverid;
char *message;

char *line = NULL;  /* forces getline to allocate with malloc */
char *title;

        int sockfd, numbytes;
        char buf[LINE_MAX];
        struct hostent *he;
        struct sockaddr_in their_addr;



// functions
void parse_conf(void);


void header(char *title);
void mainmenu(void);
int bottommenu(void);
int confmenu(void);
int answer(void);

void getopfunc(void);

void readini(void);

void setupconnection(void);
void sendtoreceiver(void);
void mailquery(char *query);

//char *stringfunc(char *command, char *string);

void main(void)
{
    system("clear");
    title =" ";
    header(title);

    // Parse configuration from ini file
    parse_conf();


    // Create a socket

    setupconnection();


    mainmenu();

}


void setupconnection(void)
{
 /* connector's address information */



        if ((he=gethostbyname("127.0.0.1")) == NULL)
        {  /* get the host info */
            herror("gethostbyname");
            exit(1);
        }

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("socket");
            exit(1);
        }

        their_addr.sin_family = AF_INET;      /* host byte order */
        their_addr.sin_port = htons(PORT);    /* short, network byte order */
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */

        if (connect(sockfd, (struct sockaddr *)&their_addr,sizeof(struct sockaddr)) == -1)
         {
            perror("connect");
            exit(1);
        }

		if (send(sockfd, "USER 1000 9099\n", 14, 0) == -1)
		{
              perror("send");
		      exit (1);
		}

     //   while (1)
     //   {

        	if ((numbytes=recv(sockfd, buf, LINE_MAX, 0)) == -1)
        	{
            		perror("recv");
            		exit(1);
            }
            buf[numbytes] = '\0';
            printf("\n %s \n", buf);


      //  }
        //close(sockfd);
}

void sendtoreceiver(void)
{
        char  newmessage[LINE_MAX];
        strcpy(newmessage , "SEND ");
        strcat(newmessage,&recieverid);
        strcat(newmessage," ");
        strcat(newmessage,line);
        //printf("\n command : %s",newmessage);
        printf("\n Command : '%s' \n",newmessage);


		if (send(sockfd, newmessage, 14, 0) == -1)
		{
              perror("send");
		      exit (1);
		}
        while((numbytes=recv(sockfd, buf, LINE_MAX, 0)) != -1)
        {
        	if ((numbytes=recv(sockfd, buf, LINE_MAX, 0)) == -1)
        	{
            		perror("recv");
            		exit(1);
            }
            buf[numbytes] = '\0';
            printf("\n Server Responce : %s \n", buf);
            break;
        }

}


void mailquery(char *query)
{
        char  newmessage[LINE_MAX];
        strcpy(newmessage ,query);

       // printf(" %s ",newmessage);
        printf("\n Command : '%s' \n",newmessage);

		if (send(sockfd, newmessage, 14, 0) == -1)
		{
              perror("send");
		      exit (1);
		}
        while((numbytes=recv(sockfd, buf, LINE_MAX, 0)) != -1)
        {
        	if ((numbytes=recv(sockfd, buf, LINE_MAX, 0)) == -1)
        	{
            		perror("recv");
            		exit(1);
            }
            buf[numbytes] = '\0';
            printf("\n %s \n", buf);

            break;
        }
        printf("\n Type R<id> for Reading, D<id> for deleting : ");
}

char *stringfunc(char *command, char *string)
{
      char newcommand[10];
      char newstring[10];
      strcpy(newstring,string);
      printf(" taken string : '%s' \n",newstring);
      strcpy(newcommand,command);
      printf(" taken command : '%s' \n",newcommand);
      strcat(newcommand," ");
      printf(" taken command with space : '%s' \n",newcommand);
      int i=0;

      for(i=0; i<=strlen(string); i++)
      {
            if(i > 0)
            {

                newcommand[3+i]=string[i];

            }

      }
      printf("\n Command : '%s' \n",newcommand);
      return newcommand;

}

// this funwrite menu
void mainmenu(void)
{

    char *title;
    size_t len = 0;     /* ignored when line = NULL */
    ssize_t read;



    char choise=' ';
    do
    {   char flag=' ';
        while(flag==' ')
        {
        printf("\n   Main Menu\n\n");
        printf("   Please, Choose Your Option\n");
        printf("1) Read/Delete Messages\n");
        printf("2) Write Message to User\n");
        printf("3) Change config parameters\n");
        printf("4) QUIT\n");
        printf("Option >");

        // keyboard selection

        //printf("\nType in a number \n");
        scanf("%c", &choise);
        // this is clear the screen
        system("clear");

        // get the selection

        switch (choise)
        {
        case '1':
            title="     Read/Delete Messages \n";
            header(title);
            printf("\n    Your Messages listed below: \n\n");
            mailquery("LIST");

            char command[5];
            char newcommand[10];

            scanf("%s",&command);
            printf("\n User Command : '%s' \n",command);
            printf("\n User Command : '%c' \n",command[0]);

            switch (command[0])
            {
                case 'r':
                strcpy(newcommand,stringfunc("RET",command));
                printf("\n Query Command : '%s' \n", newcommand);
                mailquery(newcommand);
                break;

                case 'R':
                strcpy(newcommand,stringfunc("RET",command));
                printf("\n Query Command : '%s' \n", newcommand);
                mailquery(newcommand);
                break;

                case 'd':
                strcpy(newcommand,stringfunc("DEL",command));
                printf("\n Query Command : '%s' \n", newcommand);
                mailquery(newcommand);
                break;

                case 'D':
                strcpy(newcommand,stringfunc("DEL",command));
                printf("\n Query Command : '%s' \n", newcommand);
                mailquery(newcommand);
                break;

                default:
                    printf(" Please, Type as shown. \n R<id> for Reading, D<id> for deleting : ");
                break;
            }


            bottommenu();
            break;

        case '2':
            title="     Write Message to User \n";
            header(title);
            printf(" User ID: ");scanf("%s",&recieverid);

            printf("\n Enter your message below. Press [ctrl] + d to quit! \n\n > ");


            while ((read = getline(&line, &len, stdin)) != -1)
            {
                if (read > 1)
                {
                //printf ("\n  read %zd chars from stdin, allocated %zd bytes for line : %s\n", read, len, line);
                printf("\n\n Your message:   %s  \n",line);

                printf(" Would you like to send your message to %s ? (Y/N)\n > ",&recieverid);
                int i = answer();
                if(i == 1)
                {

                    sendtoreceiver();

                    printf(" Your message sent \n");
                }
                else
                {
                     printf("You canceled \n");
                }

                break;
                }


            }

            //free (line);  /* free memory allocated by getline */




            //printf("\n Your message:   %s  ",line);

            bottommenu();
            break;

        case '3':
            title="     Change config parameters \n";
            header(title);
            //parse_conf();
            confmenu();

            //bottommenu();
            break;

        case '4':

            title="    My Chat Server v0.1 Closed... \n";
	    header(title);
            break;

        default:
            header(title);
            printf("Please, enter a number between 1 and 4 \n");
            break;
        }
        scanf("%c", &flag);

        }

    }
    while(choise!='4');

}
int bottommenu(void)
{
    char *title=" ";
    int key;
    do
    {   char f=' ';
        while(f==' ')
        {
        printf("\n1) Back to Main Menu\n");
        printf("2) QUIT\n");
        printf("Option >");

        // keyboard selection

        //printf("\nType in a number \n");
        scanf("%d", &key);
        // this is clear the screen


        // get the selection

        switch (key)
        {
        case 1:
            //main();
            system("clear");
            header(title);
            return 0;
            break;

        case 2:
            printf("    My Chat Server v0.1 Closed. \n\n");
            break;

        default:
            printf("Please, enter a number between 1 and 2 \n");
            break;
        }

        scanf("%c", &f);

        }

    }
    while(key!=2);
    exit(0);
}

int confmenu(void)
{
    char *title=" ";
    char *tempvalue;
    int key;
    char key2;

    do
    {   char f=' ';
        while(f==' ')
        {
        printf("\n   Would you change any following value?\n");
        printf("1) Target Server : %s\n",serverip);
        printf("2) Target Port : %s\n",port);
        //printf("3) User Name : %s\n",username);
        printf("3) User ID : %s\n",userid);
        printf("4) Password : %s\n",userpass);
        printf("5) Back to Main Menu\n");
        printf("6) QUIT\n");
        printf("Option >");

        // keyboard selection

        //printf("\nType in a number \n");
        scanf("%d", &key);
        // this is clear the screen


        // get the selection

        switch (key)
        {
        case 1:
             printf("Please, give Target Server value \n");
             printf(" Target Server : ");scanf("%s",&tempvalue);
             printf("\n New value : %s\n",&tempvalue);
             printf("\n\n Do you want to use new value ? (Y/N)\n");

             scanf("%s",&key2);
             printf("key : %c",key2);
             if ((key2 == 'y') || (key2 == 'Y'))
             {
                strcpy(serverip,&tempvalue);
             }
	    // else
	    // {printf("it could not change\n");}
             f=' ';
            break;

        case 2:
             printf("Please, give new Target Port value \n");
             printf(" Target Port : ");scanf("%s",&tempvalue);
             printf("\n New value : %s\n",&tempvalue);
             printf("\n\n Do you want to use new value ? (Y/N)\n");scanf("%s", &key2);
             if ((key2 == 'y') || (key2 == 'Y'))
             {
               // strcpy(port,&tempvalue);
               strcpy(port,&tempvalue);

             }
             f=' ';
            break;
        case 3:
             printf("Please, give new User Id value \n");
             printf(" User Id : ");scanf("%s",&tempvalue);
             printf("\n New value : %s\n",&tempvalue);
             printf("\n\n Do you want to use new value ? (Y/N)\n");scanf("%s", &key2);
             if ((key2 == 'y') || (key2 == 'Y'))
             {
                //strcopy(userid,&tempvalue);
                strcpy(userid,&tempvalue);
               // tempvalue =" ";
             }
             f=' ';
            break;
        case 4:
             printf("Please, give new User Password value \n");
             printf(" User Password : ");scanf("%s",&tempvalue);
             printf("\n New value : %s\n",&tempvalue);
             printf("\n\n Do you want to use new value ? (Y/N)\n");scanf("%s", &key2);
             if ((key2 == 'y') || (key2 == 'Y'))
             {
               // strcopy(userpass,&tempvalue);
               strcpy(userpass,&tempvalue);
               // tempvalue =" ";
             }
             f=' ';
            break;
        case 5:
            //main();
            system("clear");
            header(title);
            return 0;
            break;

        case 6:
            printf("    My Chat Server v0.1 Closed. \n\n");
            break;

        default:
            printf(" Please, enter a number between 1 and 6 \n");
            break;
        }

        scanf("%c", &f);

        }

    }
    while(key!=6);
    exit(0);
}

int answer(void)
{
    int key;
    char key2;

    do
    {
        scanf("%s",&key2);

        if ((key2 == 'y') || (key2 == 'Y'))
        {
            printf("\n Your answer : Yes \n");
            key=1;
            return 1;
        }
        else if ((key2 == 'n') || (key2 == 'N'))
        {
            printf("\n Your answer : No \n");
            key=1;
            return 0;
        }
        else
        {
            printf("\n Wrong Choise! Please, just press [Y] for yes or [N] for no. \n");
            key=0;

        }

    }
    while(key!=1);

}


void header(char *title)
{
    time_t rawtime;
    struct tm * timeinfo;
    //title;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    printf("\n~~~~~~~~~~~~~~~~~~  2018/2019  ~~~~~~~~~~~~~~~~~~\n\n");
    printf("            ~ My Chat Server v0.1 Ready. ~         \n\n");
    printf("                 HELLO, UGUR AKYOL \n\n");
    printf("              %s \n\n", asctime (timeinfo) );
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
    printf("   %s  \n",title);

}

void parse_conf(void)
{
    dictionary *dict;

    dict = iniparser_load(INI_NAME);
    if (dict == NULL)
    {
        fprintf(stderr, "Cannot parse file: %s\n", INI_NAME);
        exit(1);
    }

    // Get port number, root directory and greeting message from server.ini
   // port = iniparser_getint(dict, "server:listenport", -1);
   // rootDir = iniparser_getstring(dict, "server:serverroot", NULL);
   // greeting = iniparser_getstring(dict, "server:servermsg", NULL);

    serverip = iniparser_getstring(dict, "client:TargetServer", NULL);
    port = iniparser_getstring(dict, "client:TargetPort", NULL);
    userid = iniparser_getstring(dict, "client:UserID", NULL);
    userpass = iniparser_getstring(dict, "client:Passwd", NULL);


    // Print greeting message
    printf("Ini file readed succesfully \n");
    printf("TargetServer= %s\n", serverip);
    printf("TargetPort= %s\n", port);
    printf("UserID= %s\n", userid);
    printf("Passwd= %s\n", userpass);

    //iniparser_freedict(dict);
}


void getopfunc(void)
{
/*
while ((c = getopt (argc, argv, "p:s:")) != -1)
    switch (c)
    {
        case 's':
            saddr = optarg;
            break;
        case 'p':
            sport = optarg;
            break;
        case '?':
            fprintf(stderr,"Unrecognized option: -%c\n", optopt);
            return 1;
        default:
            abort ();
    }
*/
}
