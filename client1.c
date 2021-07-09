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

struct thread_info{
    char* word_to_search;
    int num_of_req;
    char* ip;
    int port;
    struct sockaddr_in * server_addrp;
};


int connect_to_server_fd(struct sockaddr_in * server_addrp){

        int clientfd;

        if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1; /* Check errno for cause of error */

        if (connect(clientfd, (struct sockaddr *) server_addrp, sizeof(struct sockaddr)) < 0)
            return -1;
        return clientfd;
}

int send_all(int socket, void *buffer,size_t len){
    Packet* ptr  = (Packet *)buffer;
    int count;
	
    for ( ;len >0; ){    
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

void interact_with_server(int client_fd,char* word, char* ip_addr){
        char* m = malloc(4096 * sizeof(char));
        sprintf(m,"search %s",word);
        Packet* send_pkt = malloc(PKT_LEN);
        Packet* recv_pkt = malloc(PKT_LEN);
        int len;
        char* resp_buf;

        send_pkt->total_length = htonl(8 + StrLength(m) + 1);
        send_pkt->msg_type = htonl(0x00000010);

        if (send_all(client_fd,send_pkt,PKT_LEN) == -1){
            fprintf(stderr,"Send failed\n");
            fflush(stderr);
            return;
        }

        if (send_all(client_fd,m,StrLength(m) + 1) == -1){
            fprintf(stderr,"Send failed\n");
            fflush(stderr);
            return;
        }

        

        if (recv_all(client_fd,recv_pkt,PKT_LEN) == -1){
            fprintf(stderr,"Recv is failing\n");
            fflush(stderr);
            return;
        }

        if (ntohl(recv_pkt->msg_type) == 0x00000011){

            len = ntohl(recv_pkt->total_length) - 8; // len of the 
            resp_buf = malloc(len * sizeof(char));
            recv_all(client_fd,resp_buf,len);

             free(resp_buf);
        }
        free(m);
        free(send_pkt);
        free(recv_pkt);


}



void* talk_to_server(void* arg){

    struct thread_info* impt_info = (struct thread_info*)arg;

    int req_per_thread = impt_info->num_of_req;

    char* ip_addr = malloc(StrLength(impt_info->ip) + 1);
    StrCopy(ip_addr,impt_info->ip);


    char* word = malloc(StrLength(impt_info->word_to_search) + 1);
    StrCopy(word,impt_info->word_to_search);

    for (int i = 0;i < req_per_thread;i++){

        int client_fd = connect_to_server_fd(impt_info->server_addrp);
        interact_with_server(client_fd,word,ip_addr);
        close(client_fd);

    }
    




    free(word);
    free(ip_addr);
    return NULL;

}



int main(int arc,char* argv[]){

    char* IP = argv[1],*PORT = argv[2],*word_to_search = argv[5];
    int num_of_threads = atoi(argv[3]);
    int num_of_requests = atoi(argv[4]);

    pthread_t thread_id[num_of_threads];
    struct thread_info* info = malloc(sizeof(struct thread_info));

    struct hostent *hp;
    struct sockaddr_in* serveraddr = malloc(sizeof(struct sockaddr_in));

    if ((hp = gethostbyname(IP)) == NULL)
        return -2; /* Check h_errno for cause of error */
    bzero((char *)serveraddr, sizeof(struct sockaddr_in));
    serveraddr->sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
    (char *)&serveraddr->sin_addr.s_addr, hp->h_length);
    serveraddr->sin_port = htons(atoi(PORT));

    ////

    info->num_of_req = num_of_requests;
    info->word_to_search = malloc(StrLength(word_to_search) + 1);
    StrCopy(info->word_to_search,word_to_search);
    info->ip = malloc(StrLength(IP) + 1);
    StrCopy(info->ip,IP);
    info->port = atoi(PORT);
    info->server_addrp  = serveraddr;


    for (int j = 0; j < num_of_threads;j++){
        pthread_create(&thread_id[j],NULL, talk_to_server,(void *)info);
    }

    for (int j = 0; j < num_of_threads;j++){
        pthread_join(thread_id[j],NULL);
    }

    free(info->word_to_search);
    free(info->ip);
    free(info);
    return 0;

}
