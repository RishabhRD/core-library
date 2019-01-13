#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
typedef struct mem{
    struct mem* free;
    int size;
    short allocated;
}Memory;
Memory *head=NULL,*tail=NULL,*cur=NULL;
pthread_mutex_t global_malloc_lock;
void* findMem(int size){
    if(!head) return NULL;
    Memory* tmp = head;
    while(tmp!=NULL){
        if(tmp->allocated==0&&tmp->size>=size){
            return (void*) tmp;
        }
        tmp = tmp->free;
    }
    return NULL;
}
void* rmalloc(int size){
    if(!size) return NULL;
    pthread_mutex_lock(&global_malloc_lock);
    void* ptr = findMem(size);
    if(ptr){
        Memory* mem = (Memory*) ptr;
        mem->allocated = 1;
        pthread_mutex_unlock(&global_malloc_lock);
        return (Memory*)ptr+1;
    }
    ptr = sbrk(sizeof(Memory)+size);
    if(ptr==(void*)-1){
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    Memory* mem = (Memory*) ptr;
    mem->free = NULL;
    mem->size = size;
    mem->allocated = 1;
    tail = mem;
    if(!head){ 
        head = mem;
        cur = head;
    }else{
        cur->free = mem;
        cur = mem;
    }
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(mem+1);
}
void rfree(void* ptr){
    if(!ptr) return;
    pthread_mutex_lock(&global_malloc_lock);
    Memory* mem = (Memory*)ptr -1;
    mem->allocated = 0;
    if(mem->free==NULL){
        if(head==tail){
            head=NULL;
            tail=NULL;
        }else{
            Memory* tmp = head;
            while(tmp->free!=tail){
                tmp=tmp->free;
            }   
            tail = tmp;
        }
        sbrk(0-(sizeof(Memory)+mem->size));
    }
    pthread_mutex_unlock(&global_malloc_lock);
}



