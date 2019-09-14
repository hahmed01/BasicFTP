/******************************************************
Programmer Name: Haya Ahmed
  Program Name: CS 372 Project 2, fserver.c
  Program Description: server side of ftp app, written in c 
  Course Name: CS 372 Intro to Computer Networks 
  Last Modified: August 15
  The following resources were used:
  Notes from 344 on Network Clients, by Benjamin Brewster
  Beej's Guide to Network Programming
  https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
  https://stackoverflow.com/questions/9875735/c-socket-programming-a-client-server-application-to-send-a-file
*******************************************************/
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define BUFFER_SIZE 500

void startUp(int* listenSocketFD, struct sockaddr_in* serverAddress, int *portNumber);
void handleRequest(int* connectionFD, struct sockaddr_in clientAddress, char* hostname, int mainPort);
void sendDirectory(struct sockaddr_in clientAddress, int mainPort, int data_port, char* hostname);
void mySendFile(int* connectionFD, struct sockaddr_in clientAddress, int mainPort, int data_port, char* hostname, char* filename);

int main(int argc, char** argv)
{
    //Check number args
    if(argc < 2)
    {
        printf("Incorrent number of arguments\n");
        exit(1);
    }

    //Initialize variables
    int listenSocketFD, connectionFD, portNumber;
    socklen_t sizeClientInfo;
    struct sockaddr_in serverAddress, clientAddress;
    
    portNumber = atoi(argv[1]);

    //Set up main socket
    startUp(&listenSocketFD, &serverAddress, &portNumber);


    while(1)
    {
        //Accept client connection
        sizeClientInfo = sizeof(clientAddress);
        connectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeClientInfo);

        if(connectionFD < 0)
        {
            close(connectionFD);
            printf("SERVER ERROR: accepting connection\n");
            exit(1);
        }     

        //Get host name
        char host_name[256];
        memset(host_name, 0, sizeof(host_name)); 
        gethostname(host_name, 255);

        //handle request
        handleRequest(&connectionFD, clientAddress, host_name, portNumber);

        //Close main socket
        close(connectionFD); 
    }
    return 0;
}



/*************************************************
Main Description: starts up socket
Pre-conditions: main socket parameters are obtained 
Post-conditions: socket is set up
 * **********************************************/
void startUp(int* listenSocketFD, struct sockaddr_in* serverAddress, int *portNumber)
{
    memset((char*)serverAddress, '\0', sizeof(serverAddress));

    //Create and setsocket
    serverAddress->sin_family = AF_INET;
    serverAddress->sin_port = htons(*portNumber);
    serverAddress->sin_addr.s_addr = INADDR_ANY;


    *listenSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(*listenSocketFD < 0)
    {
        printf("SERVER ERROR: setting socket");
    }
    if(bind(*listenSocketFD, (struct sockaddr *)serverAddress, sizeof(*serverAddress)) < 0)
    {
        printf("SERVER ERROR: binding socket");
    }

    //Listen 
    listen(*listenSocketFD, 5);
}


/*************************************************
Main Description: calls to either senddirectoy or sendfile
Pre-conditions: main socket parameters are obtained 
Post-conditions: breaks command from client into components
 * **********************************************/
void handleRequest(int* connectionFD, struct sockaddr_in clientAddress, char* hostname, int mainPort)
{
    int charsRead;
    int command_type;
    int data_port;
    int dp_digits;
    char *buffer = malloc(BUFFER_SIZE);
    memset(buffer, '\0', BUFFER_SIZE);

    //Receive command
    charsRead = recv(*connectionFD, buffer, BUFFER_SIZE, 0);
    if(charsRead == 0)
        printf("SERVER ERROR: Empty command recieved\n");
    

    char *commandType = malloc(4);
    char *dataPort = malloc(20);
    char *filename = malloc(50);
    char *clientHost = malloc(100);
    char *service = malloc(20);

    //break command into components
    commandType = strtok_r(buffer, "|", &buffer);
    command_type = atoi(commandType);

    dataPort = strtok_r(NULL, "|", &buffer);
    data_port = atoi(dataPort);

    filename = strtok_r(NULL, "|", &buffer);

    if(command_type) //-g command
    {

        mySendFile(connectionFD, clientAddress, mainPort, data_port, hostname, filename);

    }
    else //-l command
    {
        sendDirectory(clientAddress, mainPort, data_port, hostname);
    }
}

/*************************************************
Main Description: copies directory contents
Pre-conditions: none
Post-conditions: directory contents are copied and sent to client
  https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
/*************************************************/

void copyDirectoryContents(char *message)
{
    DIR* currDirectory;
    struct dirent* directoryPtr;
    char* fileNameInDir;
    currDirectory = opendir(".");

    //read directory filenames 
    while( (directoryPtr = readdir(currDirectory)) != NULL)
    {
        if (strcmp(directoryPtr->d_name, "..") == 0 || strcmp(directoryPtr->d_name, ".") == 0) 
        {
            // pass "." and ".." 
        }
        else
        {
            //copy into messgae
            fileNameInDir = directoryPtr->d_name;
            strcat(message, fileNameInDir);
            strcat(message, "\n");
        }
    }
    closedir(currDirectory);
}

/*************************************************
Main Description: sends directory contents to client
Pre-conditions: main socket connected, parameters needed such as
data port, host name ect are obtained 
Post-conditions: copies and sends over directory contents
 * **********************************************/
void sendDirectory(struct sockaddr_in clientAddress, int mainPort, int data_port, char* hostname)
{
    printf("Listing Directory");

    //Initialize variables
    int dataSocketFD;
    struct sockaddr_in dataAddress;
    struct hostent* dataHostInfo;
    struct dirent* dir_file;
    //
    char* fileNameInDir;

    memset((char*)&dataAddress, '\0', sizeof(dataAddress));

    //Create and set socket
    dataAddress.sin_family = AF_INET;
    dataAddress.sin_port = htons(data_port);
    dataHostInfo = gethostbyname(inet_ntoa(clientAddress.sin_addr));

    if(dataHostInfo == NULL)
    {
        fprintf(stderr, "SERVER ERROR: invalid host\n");
        exit(0);
    }
    memcpy((char*)&dataAddress.sin_addr.s_addr, (char*)dataHostInfo->h_addr, dataHostInfo->h_length);

    dataSocketFD = socket(AF_INET, SOCK_STREAM, 0);

    if(dataSocketFD < 0)
    {
        close(dataSocketFD);
        printf("SERVER ERROR: opening socket");
    }
    if(connect(dataSocketFD, (struct sockaddr*)&dataAddress, sizeof(dataAddress)) < 0)
    {
        printf("SERVER ERROR: connecting");
    }
    
    char * serverMessage = malloc(2000 * sizeof(char));
    memset(serverMessage, '\0', sizeof(serverMessage));
    strcpy(serverMessage, "");

    //copy message
    copyDirectoryContents(serverMessage);


    int charsWritten, serverMessageLength;
    serverMessageLength = strlen(serverMessage);

    //Send message 
    charsWritten = send(dataSocketFD, serverMessage, serverMessageLength, 0);
    if(charsWritten < 0)
    {
        printf("SERVER ERROR: writing to socket\n");
    }


}

/*************************************************
 * Function: mySendFile
Main Description: sends file contents to client
Pre-conditions: main socket connected, parameters needed such as
data port, host name ect are obtained 
Post-conditions: copies and sends over file contents to client
 * **********************************************/
void mySendFile(int* connectionFD, struct sockaddr_in clientAddress, int mainPort, int data_port, char* hostname, char* filename)
{
    char buffer[BUFFER_SIZE] = {0}; 
    memset(buffer, 0, BUFFER_SIZE); 
    
    FILE *file = fopen(filename, "r"); 
    if(file == NULL)
    {
        char* message = "Message from server: File not found";

        int charsWritten, message_length;
        message_length = strlen(message);

        //Send message contents
        charsWritten = send(*connectionFD, message, message_length, 0);
        if(charsWritten < 0)
        {
            printf("SERVER ERROR: writing to socket\n");
        }
    }
    else
    {

        //Initialize variables
        int dataSocketFD;
        struct sockaddr_in dataAddress;
        struct hostent* dataHostInfo;

        //Set data port connection
        memset((char*)&dataAddress, '\0', sizeof(dataAddress));

        //Create and set ddata socket
        dataAddress.sin_family = AF_INET;
        dataAddress.sin_port = htons(data_port);
        dataHostInfo = gethostbyname(inet_ntoa(clientAddress.sin_addr));

        if(dataHostInfo == NULL)
        {
            fprintf(stderr, "SERVER ERROR: invalid host\n");
            exit(0);
        }
        memcpy((char*)&dataAddress.sin_addr.s_addr, (char*)dataHostInfo->h_addr, dataHostInfo->h_length);

        dataSocketFD = socket(AF_INET, SOCK_STREAM, 0);
        if(dataSocketFD < 0)
        {
            close(dataSocketFD);
            printf("SERVER ERROR: opening socket");
        }
        if(connect(dataSocketFD, (struct sockaddr*)&dataAddress, sizeof(dataAddress)) < 0)
        {
            printf("SERVER ERROR: connecting");
        }


        printf("Sending File");

        memset(buffer, 0, BUFFER_SIZE);


        int i = 0;
        char character;
        //read buffer one char at a time
        while ((character = fgetc(file)) != EOF) {       
        // full buffer
            if(i == BUFFER_SIZE - 1) 
            {
                buffer[i + 1] = '\0';
                int charsWritten, serverMessageLength;
                serverMessageLength = strlen(buffer);

                //Send message 
                charsWritten = send(dataSocketFD, buffer, serverMessageLength, 0);
                if(charsWritten < 0)
                {
                    printf("ERROR: writing to socket\n");
                }

                memset(buffer, 0, BUFFER_SIZE); 
                i = 0; //Reset buffer
            }
            
            buffer[i] = character; 
            i++; 
       
        }
        if(i != 0) //if buffer not empty
        {
            //End of file character to file
            buffer[i] = 1;
            buffer[i+1] = '\0';
            int charsWritten, serverMessageLength;
            serverMessageLength = strlen(buffer);

            //Send message 
            charsWritten = send(dataSocketFD, buffer, serverMessageLength, 0);
            if(charsWritten < 0)
            {
                printf("ERROR: writing to socket\n");
            }

        }
        fclose(file); 
        close(dataSocketFD); 
    }
}


