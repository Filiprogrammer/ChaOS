#ifndef LIST_H
#define LIST_H

#include "os.h"

struct element;
typedef struct element element_t;

struct element {
    void* data;
    element_t* next;
};

typedef struct listHead {
    element_t* head;
    element_t* tail;
} listHead_t;

listHead_t* list_create();
bool list_append(listHead_t* hd, void* data);
void list_delete(listHead_t* list, void* data);
void list_deleteAllWithoutData(listHead_t* list);
void list_deleteAll(listHead_t* hd);
void list_show(listHead_t* hd);
void* list_getElement(listHead_t* hd, uint32_t pos);
size_t list_getSize(listHead_t* list);
uint32_t list_findElement(listHead_t* list, void* data);

#endif
