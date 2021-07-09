#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <strings.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <string.h>
#include "WordSearch.h"
#include "helper_string.h"


#define PKT_LEN sizeof(Packet)


typedef struct packet
{

    uint32_t total_length;
    uint32_t msg_type;


}Packet;




int connect_to_server_fd(char* IP,int port){

        int clientfd;
        struct hostent *hp;
        struct sockaddr_in serveraddr;

        if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1; /* Check errno for cause of error */


        if ((hp = gethostbyname(IP)) == NULL)
            return -2; /* Check h_errno for cause of error */
        bzero((char *) &serveraddr, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        bcopy((char *)hp->h_addr_list[0],
        (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
        serveraddr.sin_port = htons(port);


        if (connect(clientfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)
            return -1;
        return clientfd;
}



int send_all(int socket, void *buffer,size_t len){
    Packet* ptr  = (Packet *)buffer;
    int count;
	
    for (;len >0;){
        if ((count = send(socket,ptr,len,0)) == -1)
            return -1;
        
        ptr = ptr + count;
        len = len - count;
    }
    return 1;
}



int recv_all(int socket, void *buffer,size_t len){
    Packet* ptr  = (Packet *)buffer;
    int count;
	
    for (;len >0;){
        if ((count = recv(socket,ptr,len,0))  == 0)
            return count;
        
        ptr = ptr + count;
        len = len - count;
    }
    return 1;
}








int main(int argc, char* argv[]){

    char IP[100];
    StrCopy(IP,argv[1]);
    int port = atoi(argv[2]);
    char progname[1024];
    StrCopy(progname,argv[0]);
    char *search,*word;
    char* m;
    Packet* send_pkt;
    Packet* recv_pkt;
    char* resp_buf;
    int len;

    while (1){

        int client_fd = connect_to_server_fd(IP,port);
        search = malloc(1024 * sizeof(char));
        word = malloc(2048 * sizeof(char));
        m = malloc(4096 * sizeof(char));
        send_pkt = malloc(PKT_LEN);
        recv_pkt = malloc(PKT_LEN);


        printf("%s> ",progname+2);
        fflush(stdout);
        scanf("%s %s",search,word);
        fflush(stdout);
        sprintf(m,"%s %s",search,word);


        if ( (StrCompare(search,"search")) ){
            fprintf(stderr,"usage: search [Word] , where word is the word you want to search in the folder\n");
            fflush(stderr);
            continue;
        }

        send_pkt->total_length = htonl(8 + StrLength(m) + 1);
        send_pkt->msg_type = htonl(0x00000010);

        if (send_all(client_fd,send_pkt,PKT_LEN) == -1)
            continue; // send failed\

        if (send_all(client_fd,m,StrLength(m) + 1) == -1)
            continue;
        

        if (recv_all(client_fd,recv_pkt,PKT_LEN) == -1)
            continue;

        
        resp_buf = malloc(len * sizeof(char));

        if (ntohl(recv_pkt->msg_type) == 0x00000011){

            len = ntohl(recv_pkt->total_length) - 8; // len of the 
            resp_buf = malloc(len * sizeof(char));
            recv_all(client_fd,resp_buf,len);
            printf("%s\n",resp_buf);
        }

        free(resp_buf);
        free(recv_pkt);
        free(send_pkt);
        free(search);
        free(m);
        free(word);
        close(client_fd);

    }


    return 0;

}
