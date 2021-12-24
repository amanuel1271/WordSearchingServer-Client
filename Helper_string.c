#include <stdio.h>
#include <assert.h>



void Memset_to_zero(char * buf, int size_of_buf){
    int i = 0;
    for (;i < size_of_buf - 1;i++){
        *(buf + i) = '\0';
    }
    *(buf + i) = '\0';
}




char *StrCopy(char *pcDest, const char* pcSrc)
{
    assert(pcDest != NULL && pcSrc != NULL);
    char *ptr = pcDest;
    const char *src = pcSrc;
    while (*src != '\0'){
        *ptr = *src;
        ptr++;src++;
    }
    *ptr = '\0';
    return pcDest;
}





int StrLength(const char* begin)
{
    const char *final;
    int count = 0;
    assert(begin);
    final = begin;

    while (*final != '\0'){
        final++;
        count++;
    }
    return count;
}







int StrCompare(const char* pcS1, const char* pcS2)
{
    
    assert(pcS1 != NULL && pcS2 != NULL);
    const char* lhs = pcS1;
    const char* rhs = pcS2;
    
    while (*lhs != '\0'){
        if (*lhs != *rhs){
            if (*lhs < *rhs)
                return -1;
            return 1;
        }
        lhs++;rhs++;
    }
    if (*lhs != *rhs){
        if (*lhs < *rhs)
            return -1;
        return 1;
    }
   
   return 0;
}






char *StrConcat(char *pcDest, const char* pcSrc)
{
    
    assert(pcDest != NULL && pcSrc != NULL);
    char *ptr = pcDest;
    const char *src = pcSrc;
    while (*ptr != '\0')
        ptr++;
    while (*src != '\0'){
        *ptr = *src;
        src++;ptr++;
    }
    *ptr = '\0';
    return pcDest;
    
    
    
}






void Selectcopy(const char* str,char *copy_string,unsigned int len){
    const char *ptr = str;
    for (int i=0;i <len;i++){
        *copy_string = *ptr;
        ptr++; copy_string++;
    }
    *copy_string = '\0';
}
