
#define DATA_SIZE 2048


typedef struct single_word_info{
    char *filename;
    unsigned long line_num;
    unsigned int docID;
    struct single_word_info* next;

}Info;


typedef struct word
{
    char* single_word;
    struct single_word_info *info;

}Word;




void manage_words(FILE** fp_dptr,Word** word_address,char* name_of_file,unsigned long *array_size,unsigned int* ID);

void Search_array(const Word *Big_array,unsigned long array_size,const char word[],char * addr);

Word *Search_for_word(const char* word, Word* Word_table,unsigned long array_size);

int check_reallocation(unsigned long arr_size,Word *address);

void insert_word_info(struct word *word,char *name,unsigned long num);

void read_and_extract_words(const char buf[],char** begin,char** moving,unsigned long *line_number,
  unsigned long* arr_size, Word ** addr,char *f_name, unsigned int* ID);

void deallocate_all_mem(Word* Array,unsigned long arr_size);
