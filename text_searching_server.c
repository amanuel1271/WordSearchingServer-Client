#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <strings.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include "WordSearch.h"
#include "helper_string.h"


#define PKT_LEN sizeof(Packet)
#define THREAD_POOL_SIZE 10


pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;






/** custom structs*/

typedef struct packet
{

    uint32_t total_length;
    uint32_t msg_type;


}Packet;


typedef struct s_pool {
    int maxfd; // largest descriptor in sets
    fd_set read_set; // all active read descriptors
    fd_set ready_set; // descriptors ready for reading
    int nready; // return of select()
    int maxi; /* highwater index into client array */
    int clientfd[FD_SETSIZE]; /* set of active descriptors */ 
} pool;


typedef struct enqueue_struct{

    struct enqueue_struct* next;
    int client_fd;
    int index;
    pool* my_pool;

}enqueue_info;



struct thread_info{
    Word* word;
    unsigned long arr_size;
};



enqueue_info* head = NULL;
enqueue_info* tail = NULL;



void ENQUEUE(int client_fd,int index,pool** my_pool){

    enqueue_info* new = calloc(1,sizeof(enqueue_info));
    new->client_fd = client_fd;
    new->index = index;
    new->my_pool = *my_pool;

    if (tail != NULL )
        tail->next = new;
    else
        head = new;
    tail = new;
    
}



enqueue_info* DEQUEUE(){

    if (head == NULL)
        return NULL;

    else{
        enqueue_info* temp = head;
        head = head->next;
        temp->next = NULL;
        if (!head)
            tail = NULL;
        return temp;
    }
}






void* threadtask(void *);

pthread_t threadpool[THREAD_POOL_SIZE]; // to create 10 threads





void initialize_threadpool(struct thread_info *impt_info){
    for (int j = 0; j < THREAD_POOL_SIZE ; j++){
        pthread_create(&threadpool[j],NULL, threadtask,(void *)impt_info);
    }
}


int send_all(int socket, void *buffer,size_t len){
    Packet* ptr  = (Packet *)buffer;
    int count;
    for (;len >0;){
        if ((count = send(socket,ptr,len,0)) == -1){
            return -1;
        }
        ptr = ptr + count;
        len = len - count;
    }
    return 1;
}



int recv_all(int socket, void *buffer,size_t len){
    Packet* ptr  = (Packet *)buffer;
    int count;
    for (;len >0;){
        if ((count = recv(socket,ptr,len,0))  == 0){
            return count;
        }
        ptr = ptr + count;
        len = len - count;
    }
    return 1;
}




int Bootstrap(char *directory,Word** list,unsigned long* arr_size_ptr){

   DIR *dir;
   struct dirent *dent;
   FILE* fp;
   char buf[2048];
   char progname[1024];
   unsigned int ID = 1;

   dir = opendir(directory);
   if (dir == NULL){
       fprintf(stderr,"usage: Couldn't open the directory\n");
       return 0;
   }   
 
    while( (dent=readdir(dir)) != NULL)
    {
        if((StrCompare(dent->d_name,".")==0 || StrCompare(dent->d_name,"..")==0 || (*dent->d_name) == '.' ))
        {
            continue;
        }

        StrCopy(buf,directory);
        StrConcat(buf,"/");
        StrConcat(buf,dent->d_name);
        fp = fopen(buf,"r");

        if (fp == NULL){
            fprintf(stderr,"usage couldn't open the file\n");
            continue;
        }
        manage_words(&fp,list,dent->d_name,arr_size_ptr,&ID);
        Memset_to_zero(buf,2048);
        fclose(fp);
        ID += 1;
    }

    closedir(dir);
    return 1;

}


int create_nonblocking_socket(int port){
    int sockfd,optval;
    struct sockaddr_in serveraddr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
         return -1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(optval)) < 0)
         return -1;
    if (0 > (optval = fcntl(sockfd, F_GETFL)))
        printf("Error\n");

    optval = (optval | O_NONBLOCK);

    if (fcntl(sockfd, F_SETFL, optval))
        printf("Error\n");

    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    if (listen(sockfd, SOMAXCONN) < 0)
        return -1;

    return sockfd;

}





void init_pool(int listenfd, pool *p)
{
    int i;
    p->maxi = -1;
    for (i=0; i< FD_SETSIZE; i++)
        p->clientfd[i] = -1;
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}







/* task that thread does*/
void handle_client(enqueue_info* info,Word* word_list,unsigned long size){
    int i = info->index;
    pool* p = info->my_pool;
    int client_fd = (info->client_fd);
    Packet* recv_pkt = malloc(PKT_LEN);
	
    if (recv_all(client_fd,recv_pkt,PKT_LEN) == 0){
        free(recv_pkt);
        close(client_fd);
        return;
    }
    int len = ntohl(recv_pkt->total_length) - 8;
    char* recv_buf = malloc(len * sizeof(char));

    if (recv_all(client_fd,recv_buf,len) == 0){
        free(recv_buf);
        free(recv_pkt);
        close(client_fd);
        return;
    }

    char *word = recv_buf + 7;

    char *search_buffer = malloc(40000 * sizeof(char)); // I am going to free it so no problem allocating a large value

    
    Search_array(word_list,size,word,search_buffer);
    Packet* send_pkt = malloc(PKT_LEN);

    if (!StrLength(search_buffer)){
        send_pkt->total_length = htonl(8);
        send_pkt->msg_type = htonl(0x00000020); //error
    }
    else{
        send_pkt->total_length = htonl(8 + (StrLength(search_buffer) + 1));
        send_pkt->msg_type = htonl(0x00000011);
    }

    if (send_all(client_fd,send_pkt,PKT_LEN) == -1){
        fprintf(stderr,"Send is failing  %s\n",word);
    }
       

    if (StrLength(search_buffer) ){ // If there is something in the search buffer
        send_all(client_fd,search_buffer,StrLength(search_buffer) + 1); //search result
    }

    free(send_pkt);
    free(recv_pkt);
    free(recv_buf);
    free(search_buffer);

    close(client_fd);
}



void* threadtask(void *arg){

    struct thread_info * info_for_thread = (struct thread_info*)arg;
    Word *word = info_for_thread->word;
    unsigned long arr_size = info_for_thread->arr_size;



    while (1){

        enqueue_info* info;
        pthread_mutex_lock(&mutex1);
        info = DEQUEUE();
        pthread_mutex_unlock(&mutex1);

        if (info != NULL){
            handle_client(info,word,arr_size);
        }

    }
    return NULL;
}


int main(int argc, char *argv[]){

   Word* Word_list = calloc(DATA_SIZE,sizeof(struct word));  
   unsigned long size_of_array = DATA_SIZE;
   char directory_name[300];
   socklen_t clientlen = sizeof(struct sockaddr_in);
   struct sockaddr_in clientaddr;
   pool my_pool;



   StrCopy(directory_name,argv[1]);
   if (!Bootstrap(directory_name,&Word_list,&size_of_array)){
       fprintf(stderr,"Couldn't properly finish the bootstrapping process\n");
       return 1;
   }

   struct thread_info* my_thread = malloc(sizeof(struct thread_info));
   my_thread->arr_size = size_of_array;
   my_thread->word = Word_list;
   initialize_threadpool(my_thread);

   int port = atoi(argv[2]);
   int sockfd = create_nonblocking_socket(port),connfd;
   int ready = 0;

   init_pool(sockfd,&my_pool);

   while (1) {
        my_pool.ready_set = my_pool.read_set;
        my_pool.nready = select(my_pool.maxfd+1, &my_pool.ready_set,NULL, NULL, NULL);
        ready = my_pool.nready;
        for (int sock=0; (sock<= my_pool.maxfd) && (ready > 0);sock++){

            if (FD_ISSET(sock, &my_pool.ready_set)) {

                ready--;

                if (sock == sockfd){
                    while ( (connfd = accept(sockfd,(struct sockaddr *)&clientaddr,&clientlen)) ){

                        if (connfd > 0){
                            FD_SET(connfd,&my_pool.read_set);
                            if (connfd > my_pool.maxfd)
                                my_pool.maxfd = connfd;

                        }
                        else if (connfd < 0)
                            break;

                    }

                }
                else{
                    pool* p = &my_pool;
                    pthread_mutex_lock(&mutex1); // lock to avoid contention
                    ENQUEUE(sock,sock,&p);
                    pthread_mutex_unlock(&mutex1); // unlock
                    FD_CLR(sock,&my_pool.read_set);
                }
            }  
        } 
    }
    return 0;
}
