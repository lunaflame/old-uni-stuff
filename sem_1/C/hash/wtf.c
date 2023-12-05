#include "hashtable.h"

typedef struct dll_t {    // doubly linked list, not the extension
    void* data;

    struct dll_t* prev;
    struct dll_t* next;
} DLL;

int main() {
    htTable_t* ht = newHashtable(64);
    
}