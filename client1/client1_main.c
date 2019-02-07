/* *********************** INFO *****************************
 
            "ITERATIVE FILE TRANSFER TCP SERVER"
            Exercise 2.3 Lab 2
            (client)

            Developer: Antonio Tavera 

************ BRIEF EXPLANATION OF THE ALGORITHM *************

In this exercise I am developing a client that connect to a TCP server, whose address and port number are specified as first and second command line parameter. After having established the connection the client requests the transfer of the files whose names are specified on the command line as third and subsequent parameter, and stores them locally in its working directory.

What the program program does?
First, it gets the TCP server IP address from command-line and converts it from dotted decimal notation to an internet address in network byte order. It proceeds by reading, still from command line, the server port number and converting it in network byte order. Once port and IP adress have been read, the program creates the socket through the Socket() function (using AF_INET for address family, SOCK_STREAM for the type and IPPROTO_TCP for the protocol that will be used). The address structure is prepared and the program proceeds by setting a non blocking socket connect() in order to check for timeout or success during the connect operation. If the connection to the target address complete immediately, without error or timeout, the clientServiceFunction() is invoked, otherwise an error is printed and program stops its execution.

The clientServiceFunction(), that receive as parameters the connected socket, the number and names of file received by command line, starts its execution by entering in a loop until all the file are received and saved locally. First, it checks the correctness of the filename passed by command line (it controls if it contains some invalid charcaters, e.g. if it is a directory), then it prepares the GET command by concatenating the GET_MSG string, the name of the file and the two characters CR and LF. Finally, that message is sent to the server through the sendn() function and a check on the sent bytes is done.
 The clientServiceFunction() now waits a reply from the server. A select structure is defined for every read in order to handle timeout. If a message is received from the server, the first character is read through the readn() function and it is compared to '+' or '-'; if it is '+', the client received a possible OK message and continue reading, if it is '-' the client received a possible ERR message and continue reading, otherwise an unexpected message is received and client stops its execution.
If "+", client continues reading and checks if the rest of the message corresponds to the OK_MSG string expected; if this is true, the file size and timestamp are read and converted in a network byte order. The last things (read with the read() function) are the bytes of the requested file. These bytes are written at the same time within the file created just before. If something of the functions described above fails, an error is printed and clients stops its execution. The clients proceed by printing all the information of the file received and continues loop until all the file requests are satisfied.
If '-' is received, clients continues reading and checks if the rest of the message (read thhrough the readn() function) correspond to the ERR_MSG string expected; if this is true the socket is closed and clients stops its execution.
The last thing that the clientServiceFunction() does is to send to the server the QUIT_MSG command (through the sendn() function) and close connection.
 
The errorHandler() function is used to print an error and close the connected socket.
************************************************************ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include "./../errlib.h"
#include "./../sockwrap.h"

#define RCVBUFFERLENGTH     4098                //receive buffer length
#define SNDBUFFERLENGTH     4097                //send buffer length
#define WAITINGTIME         60                  //waiting time on each read from server
#define CONNECTIONTIME      50                  //waiting time for the connect

static const char OK_MSG[]    =   "OK\r\n";     //Ok message string
static const char ERR_MSG[]   =   "ERR\r\n";    //Error message string
static const char QUIT_MSG[]  =   "QUIT\r\n";   //Quit message string
static const char GET_MSG[]   =   "GET ";       //Get message string

char *prog_name;
void clientServiceFunction(int socket, int nfiles, char **files);
void errorHandler(char *err, int s);



int main(int argc, char **argv){
    
    //defining variables
    int s;                              //socket
    uint16_t tport_n, tport_h;          //server port number (net/host ord)
    struct sockaddr_in saddr;           //server address structure
    struct in_addr sIPaddr;             //server IP address structure
    fd_set rset, wset;                  //set of sockets
    struct timeval tval;                //timeval structure
    socklen_t len;
    int result, flags, n;
    int error;
   
    //assigning program name
    prog_name = argv[0];
    printf("\n");
    
    //getting ip address of server from command line
    result = inet_aton(argv[1], &sIPaddr);
    if(result == 0){
        printf("Invalid address. Stopping execution");
        exit(1);
    }
    
    //getting port number of server from command line
    if(sscanf(argv[2], "%" SCNu16, &tport_h)!=1){
        printf("Invalid port number. Stopping execution");
        exit(1);
    }
    tport_n = htons(tport_h);
    
    //creating the socket
    printf("Creating the socket \t\t\t\t\t");
    s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("-> Done. Socket number: %d\n", s);
    
    //preparing address structure
    bzero(&saddr, sizeof(saddr));
    saddr.sin_family    =   AF_INET;
    saddr.sin_port      =   tport_n;
    saddr.sin_addr      =   sIPaddr;
    
    //setting non blocking connection
    showAddr("Connecting to server address", &saddr);
    if((flags = fcntl(s, F_GETFL, 0)) == -1){
        errorHandler("File descriptor manipulation error. Closing socket\t", s);
        return(0);
    }
    if(fcntl(s, F_SETFL, flags | O_NONBLOCK) == -1){
        errorHandler("File descriptor manipulation error. Closing socket\t", s);
        return(0);
    }
    error = 0;
    
    //connecting to the target address
    if((n = connect(s, (struct sockaddr *)&saddr, sizeof(saddr))) < 0){
        if(errno != EINPROGRESS){
            errorHandler("Error during connect. Closing socket\t\t\t", s);
            return(0);
        }
    }
    //if n==0 connection completed immediatly, no need of setting timeout
    if(n==0)
        goto connected;
    
    //setting select structure
    FD_ZERO(&rset);
    FD_SET(s, &rset);
    wset = rset;
    tval.tv_sec = CONNECTIONTIME;
    tval.tv_usec = 0;
    if((n=Select(s+1, &rset, &wset, NULL, &tval))==0){
        errorHandler("Timeout. Unreachable address. Closing socket\t\t", s);
        return(0);
    }
    
    //checking error during select operation, socket not set
    if (FD_ISSET(s, &rset) || FD_ISSET(s, &wset)) {
        len = sizeof(error);
        if (getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len) < 0){
            errorHandler("Error during connect. Closing socket\t\t\t", s);
            return(0);
        }
    }else{
        errorHandler("Select error. Closing socket\t\t\t", s);
        return(0);
    }

connected:
    //restoring file status flags
    if(fcntl(s, F_SETFL, flags) == -1){
        errorHandler("File descriptor manipulation error. Closing socket\t", s);
        return(0);
    }
    //error check
    if(error){
        errorHandler("Unreachable address. Closing socket\t\t\t", s);
        return(0);
    }
    //connection done. Do client task and finish
    printf("-> Done\n");
    clientServiceFunction(s, argc-3, argv+3);
    return(0);
    
}




void clientServiceFunction(int socket, int nfiles, char **files){
    
    int fileindex;                      //index of the position of the file inside files
    char *filename;                     //used to store the name of the file
    size_t bufsize;                     //size of the sending buffer
    char rcvbuffer[RCVBUFFERLENGTH];    //receiving buffer
    char sndbuffer[SNDBUFFERLENGTH];    //sending buffer
    uint32_t filesize, timestamp;       //file size and timestamp variable
    uint32_t numreceived, receivedsize; //used while reading file
    uint32_t ret;                       //return value of fwrite() function
    FILE *fp;                           //file pointer
    fd_set cset;                        //set of sockets
    struct timeval tval;                //timeval structure
    int n;
    
    
    //enter client loop until all the files requests are sent and a reply is received
    for(fileindex=0; fileindex<nfiles; fileindex++){
        
        //checking if filename is correct or not, if it is a directory or a filename
        filename = strdup(files[fileindex]);
        if(filename[0] == '.' || filename[0] == '~' || (strchr(filename, '/') != NULL)){
            //it is a directory, not a filename
            errorHandler("Error: it is not a filename but a directory. Try again\t", socket);
            return;
        }
        
        //preparing sendbuffer and sending GET message
        printf("\n");
        printf("Sending GET message\t\t\t\t\t");
        bufsize = 0;
        bzero(sndbuffer, SNDBUFFERLENGTH);
        strncpy(sndbuffer, GET_MSG, sizeof(GET_MSG)-1);
        strcat(sndbuffer, filename);
        strcat(sndbuffer, "\r\n");
        bufsize = sizeof(GET_MSG) - 1 + strlen(filename)*sizeof(char) + 2*sizeof(char);
        if(sendn(socket, sndbuffer, bufsize, 0) != bufsize){
            errorHandler("Error while sending GET message. Closing connection\t", socket);
            return;
        }
        printf("-> Message sent\n");
        
        //reading reply (+ or -) message from server
        //if it is "+" -> possible OK MESSAGE
        //if it is "-" -> possible ERR MESSAGE
        //otherwise Unexpected Message, close connection
        FD_ZERO(&cset);
        FD_SET(socket, &cset);
        tval.tv_sec = WAITINGTIME;
        tval.tv_usec = 0;
        if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0) {
            printf("Reading reply from the server\t\t\t\t");
            if(readn(socket, rcvbuffer, 1) != 1){
                errorHandler("Error while reading server reply. Closing connection\t", socket);
                return;
            }
        }else {
            errorHandler("No response received, timeout. Closing connection\t", socket);
            return;
        }
        
        if(strncmp(rcvbuffer, "+", 1)==0){
            
            ////////////////////////////////
            //POSSIBLE OK MESSAGE RECEIVED//
            ////////////////////////////////
            FD_ZERO(&cset);
            FD_SET(socket, &cset);
            tval.tv_sec = WAITINGTIME;
            tval.tv_usec = 0;
            if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0){
                if(readn(socket, rcvbuffer, sizeof(OK_MSG)-1) != (sizeof(OK_MSG)-1)){
                    errorHandler("Reading OK message error. Closing connection\t", socket);
                    return;
                }
            }else{
                errorHandler("No response received, timeout. Closing connection\t", socket);
                return;
            }
            
            //comparing read message with the OK_MSG string expected
            if(strncmp(rcvbuffer, OK_MSG , sizeof(OK_MSG)-1)==0){
                printf("-> Received OK message\n");
                
                //reading file size
                FD_ZERO(&cset);
                FD_SET(socket, &cset);
                tval.tv_sec = WAITINGTIME;
                tval.tv_usec = 0;
                if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0){
                    if(readn(socket, rcvbuffer, sizeof(uint32_t)) != sizeof(uint32_t)){
                        errorHandler("Error while reading file size. Closing connection\t", socket);
                        return;
                    }
                }else{
                    errorHandler("No response received, timeout. Closing connection\t", socket);
                    return;
                }
                filesize = ntohl(*(uint32_t *)rcvbuffer);
              
                //reading file timestamp
                FD_ZERO(&cset);
                FD_SET(socket, &cset);
                tval.tv_sec = WAITINGTIME;
                tval.tv_usec = 0;
                if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0){
                    if(readn(socket, rcvbuffer, sizeof(uint32_t)) != sizeof(uint32_t)){
                        errorHandler("Error while reading file timestamp. Closing connection\t", socket);
                        return;
                    }
                }else{
                    errorHandler("No response received, timeout. Closing connection\t", socket);
                    return;
                }
                timestamp = ntohl(*(uint32_t *)rcvbuffer);
                
                //creating the file in the client directory
                if((fp = fopen(filename, "w")) == NULL){
                    errorHandler("Creating file error. Closing connection\t\t\t", socket);
                    return;
                }
                
                //receiving file from server
                printf("Receiving file from server\t\t\t\t");
                receivedsize = 0;
                while(receivedsize < filesize){
                    
                    FD_ZERO(&cset);
                    FD_SET(socket, &cset);
                    tval.tv_sec = WAITINGTIME;
                    tval.tv_usec = 0;
                    if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0){
                        numreceived = (uint32_t)read(socket, rcvbuffer, RCVBUFFERLENGTH);
                        if(numreceived <= 0){
                            errorHandler("Error while receiving file. Closing connection\t\t", socket);
                            return;
                        }
                    }else{
                        errorHandler("No response received, timeout. Closing connection\t", socket);
                        return;
                    }
                    ret = (uint32_t)fwrite(rcvbuffer, sizeof(char), numreceived, fp);
                    if(ret!=numreceived){
                        errorHandler("Error while writing new file. Closing connection\t", socket);
                        return;
                    }
                    receivedsize += numreceived;
                    
                }
                printf("-> File received\n");
                printf("\t->File name: %s\n", filename);
                printf("\t->File size: %" PRIu32 " byte\n" , filesize);
                printf("\t->File timestamp: %" PRIu32 "\n", timestamp);
                fclose(fp);
            }else{
                errorHandler("Wrong OK message received. Closing connection\t\t", socket);
                return;
            }
            
        }else if(strncmp(rcvbuffer, "-", 1)==0){
            
            /////////////////////////////////
            //POSSIBLE ERR MESSAGE RECEIVED//
            /////////////////////////////////
            FD_ZERO(&cset);
            FD_SET(socket, &cset);
            tval.tv_sec = WAITINGTIME;
            tval.tv_usec = 0;
            if((n = Select(FD_SETSIZE, &cset, NULL, NULL, &tval)) > 0){
                if(readn(socket, rcvbuffer, sizeof(ERR_MSG)-1) != (sizeof(ERR_MSG)-1)){
                    errorHandler("Error while reading error message. Closing connection\t", socket);
                    return;
                }
            }else{
                errorHandler("No response received, timeout. Closing connection\t", socket);
                return;
            }
            
            //comparing message received with the ERR_MSG string expected
            if(strncmp(rcvbuffer, ERR_MSG, sizeof(ERR_MSG)-1) == 0){
                printf("-> Received ERROR message\n");
                printf("Closing connection\t\t\t\t\t");
                Close(socket);
                printf("-> Connection closed\n");
                return;
            }else{
                errorHandler("Wrong ERR message received. Closing connection\t", socket);
                return;
            }
            
        }else{
            
            ///////////////////////////////
            //UNEXPECTED MESSAGE RECEIVED//
            ///////////////////////////////
            errorHandler("Unexpected message received. Closing connection\t", socket);
            return;
            
        }
    }

    //sending QUIT message and close connection
    printf("\n");
    printf("Sending QUIT message\t\t\t\t\t");
    if(sendn(socket, QUIT_MSG, sizeof(QUIT_MSG)-1, 0) != (sizeof(QUIT_MSG)-1)){
        errorHandler("Sending QUIT message error. Closing connection\t", socket);
        return;
    }
    printf("-> Message sent\n");
    printf("Closing connection\t\t\t\t\t");
    Close(socket);
    printf("-> Connection closed\n");
    return;
    
}


//function used to handle error and close socket
void errorHandler(char *err, int s){
    printf("\n");
    printf("%s", err);
    Close(s);
    printf("-> Connection closed\n");
    return;
}

