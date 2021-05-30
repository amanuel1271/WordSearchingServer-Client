#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "WordSearch.h"
#include "helper_string.h"




Word *Search_for_word(const char* word, Word* Word_table,unsigned long array_size){
    Word * begin = Word_table;
    int i = 0;
    for ( ; i < (array_size - 1);i++){
      if  (   ( begin->single_word)  == NULL)
            return NULL;

        else if ( StrCompare(begin->single_word,word) == 0 ) // word is found, return the pointer
            return begin;
        begin += 1;
    }
     if ( (begin->single_word) == NULL)
        return NULL;
    else if (StrCompare(begin->single_word,word) == 0 )
        return begin;
    
    return NULL;
}

int check_reallocation(unsigned long arr_size,Word *address){
    Word * begin = address;
    int i = 0;
    
    for (; i < (arr_size - 500); i++){
        if (!((begin)->single_word))
            break;
        begin += 1;
    }
    if (i == (arr_size - 500))
        return 1; // we need realloc
    else
        return 0;   
}

void insert_word_info(struct word *word,char *name,unsigned long num){
    struct single_word_info *ptr = word->info,*copy ;

    while(ptr->next != NULL){
        ptr = ptr->next;
    }

    copy = calloc(1,sizeof(struct single_word_info));
    
    (copy)->filename = calloc(1,StrLength(name)+1);
    StrCopy((copy)->filename,name);

    (copy)->line_num = num;
    (copy)->next = NULL;
     ptr->next = copy;
}

void read_and_extract_words(const char buf[],char** begin,char** moving,unsigned long *line_number,
  unsigned long* arr_size, Word ** addr,char *f_name, unsigned int* ID){
     
     unsigned int len_word;
     char *dummpy_ptr;
     Word *address_of_word[1];

     while (**moving  != '\0'){
         while (!isalpha(**moving) && (**moving != '\0')){
             (*moving)++;
             (*begin)++;
         }

         while(isalpha(**moving) > 0){
                (*moving)++;
         }
         if (**moving == '\0'){
             return;
         }
         len_word = *moving - *begin;
         len_word++; // I added 1 because of null character
         
         dummpy_ptr = malloc(len_word * sizeof(char)); // allocate this much space
         Selectcopy(*begin,dummpy_ptr,len_word-1);
         
         if (check_reallocation(*arr_size,*addr)){

             *addr = realloc(*addr,( (*arr_size) + (DATA_SIZE/2) ) * sizeof(struct word) );
             *arr_size += (DATA_SIZE/2);
         }
	     *address_of_word = Search_for_word(dummpy_ptr,*addr,*arr_size);
         if ( *address_of_word != NULL){
             insert_word_info(*address_of_word,f_name,*line_number);
         }
         else{

             *address_of_word = *addr;
             while ( (*address_of_word)->single_word != NULL ){
                 (*address_of_word) += 1;
             }
	     (*address_of_word)->single_word = calloc(1,len_word * sizeof(char));
             StrCopy( (*address_of_word)->single_word,dummpy_ptr);
             (*address_of_word)->info = calloc(1,sizeof(struct single_word_info));
	     ((*address_of_word)->info)->filename = calloc(1,StrLength(f_name)+1);
             StrCopy(((*address_of_word)->info)->filename,f_name);
             ((*address_of_word)->info)->line_num = *line_number;
             ((*address_of_word)->info)->docID = *ID;
             ((*address_of_word)->info)->next = NULL; // I know I have calloced but just to make sure

         }
         Memset_to_zero(dummpy_ptr,len_word);
         free(dummpy_ptr);
         *begin = *moving;
     }

}



void manage_words(FILE** fp_dptr,Word** word_address,char* name_of_file,unsigned long *array_size,unsigned int* ID){
    int read_bytes;
    unsigned long line_num = 1;
    char * c,*d; // I want to change this pointer
    char * read_buf = calloc(DATA_SIZE,sizeof(char));
    char *ptr = read_buf;
    unsigned int read_len = DATA_SIZE;
    unsigned int len = read_len;
    unsigned long distance;

    while( fgets(ptr,len,*fp_dptr) != NULL ){ // How are you managing if read finishes between words.
        if ( *(read_buf +(StrLength(read_buf) - 1) ) != '\n'  && (StrLength(read_buf) == DATA_SIZE - 1)){ // doesn't end with newline
            read_len += (DATA_SIZE/2);
	        distance = ptr - read_buf;
            read_buf = realloc(read_buf,read_len * sizeof(char));
	        ptr = read_buf + distance;
            if (read_buf == NULL){
                fprintf(stderr,"Realloc failed\n");
                exit(1);
            }
            len = read_len - StrLength(ptr);
            ptr = ptr + StrLength(ptr);
            continue;
        }
        c = read_buf;
        d = c;
        read_and_extract_words(read_buf,&c,&d,&line_num,array_size, word_address,name_of_file,ID);
        Memset_to_zero(read_buf,read_len);
        ptr = read_buf;
        line_num++;
    }
    free(read_buf);
}

void Search_array(const Word *Big_array,unsigned long array_size,const char word[],char * write_buf_addr){
    int j = 0;
    const Word* array_ptr = Big_array;
    struct single_word_info* ptr;
    char *ptr4 = write_buf_addr;
    int len;
    *ptr4 = '\0'; // Pad a null character just in case that there is no search result

    for( ; j < (array_size - 1) ; j++){
        if (!array_ptr->single_word)
            return;
        else if ( !StrCompare(word,array_ptr->single_word)){
            for(ptr = array_ptr->info;ptr != NULL;ptr = ptr->next){
                len = sprintf(ptr4,"%s: line #%lu\n",ptr->filename,ptr->line_num);
                ptr4 = ptr4 + len;

            }
            return;
        }
        array_ptr += 1;
    }
    if ( (array_ptr->single_word) && !StrCompare(word,array_ptr->single_word)){
            for(ptr = array_ptr->info;ptr != NULL;ptr = ptr->next){
                len = sprintf(ptr4,"%s: line #%lu\n",ptr->filename,ptr->line_num);
                ptr4 = ptr4 + len;
            }
        }

    return;
}

void deallocate_all_mem(Word* Array,unsigned long arr_size){
    Word* del = Array,*loop = Array;
    struct single_word_info *curr,*next;
    int j = 0;

    for ( ; j< (arr_size - 1); j++){
        if (!loop->single_word){
            free(del);
            return;
        }
        else{
            free(loop->single_word); // if NULL nothing happens so it is fine
            for (curr = loop->info;curr != NULL;curr = next){
                next = curr->next;
                free(curr->filename);
                free(curr);
            }
        }
        loop += 1;
    }
    if (!loop->single_word){
        free(del);
        return;
    }
    
    free(loop->single_word);
    for (curr = loop->info;curr != NULL;curr = next){
        next = curr->next;
        free(curr->filename);
        free(curr);
    }
    
    free(del); //free the initial big array
}


