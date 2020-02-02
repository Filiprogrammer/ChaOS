#include "list.h"
#include "kheap.h"
#include "paging.h"

listHead_t* list_create() {
    listHead_t* hd = (listHead_t*)malloc(sizeof(listHead_t), 0);
    if (hd) {
        hd->head = hd->tail = 0;
        return hd;
    }

    return 0;
}

bool list_append(listHead_t* hd, void* data) {
    element_t* ap = (element_t*)malloc(sizeof(element_t), 0);

    if (ap) {
        ap->data = data;
        ap->next = 0;

        if (hd->head == 0) {  // there exist no list element
            hd->head = ap;
        } else {  // there exist at least one list element
            element_t* ptr = hd->head;
            while (ptr->next != 0) ptr = ptr->next;
            ptr->next = ap;
        }

        hd->tail = ap;
        return true;
    }

    return false;
}

/**
 * @brief Delete all elements with the given value from the list.
 * 
 * @param list A pointer to the list
 * @param data The value to look for
 */
void list_delete(listHead_t* list, void* data) {
    element_t* cur = list->head;

    if (cur->data == data) {
        element_t* temp = cur;
        cur = cur->next;
        list->head = cur;
        if (list->head == list->tail) list->tail = 0;
        free(temp);
    }

    while (cur != 0 && cur->next != 0) {
        if (cur->next->data == data) {
            element_t* temp = cur->next;
            if (cur->next == list->tail) list->tail = cur;
            cur->next = cur->next->next;
            free(temp);
        }
        cur = cur->next;
    }
}

void list_deleteAtWithoutData(listHead_t* list, uint32_t pos) {
    element_t* cur = list->head;
    uint32_t index = 0;

    while (cur != 0 && cur->next != 0) {
        ++index;

        if (pos == index) {
            element_t* temp = cur->next;

            if (cur->next == list->tail)
                list->tail = cur;

            cur->next = cur->next->next;
            free(temp);
        }
        cur = cur->next;
    }
}

void list_deleteAt(listHead_t* list, uint32_t pos) {
    element_t* cur = list->head;
    uint32_t index = 0;

    while (cur != 0 && cur->next != 0) {
        ++index;

        if (pos == index) {
            element_t* temp = cur->next;

            if (cur->next == list->tail)
                list->tail = cur;

            cur->next = cur->next->next;
            free(temp->data);
            free(temp);
        }
        cur = cur->next;
    }
}

void list_deleteAllWithoutData(struct listHead* hd) {
    element_t* cur = hd->head;
    element_t* nex;

    while (cur) {
        nex = cur->next;
        free(cur);
        cur = nex;
    }

    free(hd);
}

void list_deleteAll(struct listHead* hd) {
    element_t* cur = hd->head;
    element_t* nex;

    while (cur) {
        nex = cur->next;
        free(cur->data);
        free(cur);
        cur = nex;
    }

    free(hd);
}

void list_show(listHead_t* hd) {
    puts("List elements:\n");
    element_t* cur = hd->head;

    if (!cur) {
        puts("The list is empty.");
    } else {
        while (cur) {
            printf("%X\t", cur->data);
            cur = cur->next;
        }
    }
}

/**
 * @brief Retrieve the element at the given position of the list.
 * 
 * @param hd A pointer to the list
 * @param pos The position of the element
 * @return void* The element from the list
 */
void* list_getElement(listHead_t* hd, uint32_t pos) {
    element_t* cur = hd->head;

    uint32_t index = 1;

    while (cur) {
        if (index == pos)
            return cur->data;

        ++index;
        cur = cur->next;
    }

    return NULL;
}

/**
 * @brief Get the number of elements in the list.
 * 
 * @param list A pointer to the list
 * @return size_t The number of elements
 */
size_t list_getSize(listHead_t* list) {
    size_t i = 0;
    element_t* cur = list->head;

    while (cur != 0) {
        cur = cur->next;
        ++i;
    }

    return i;
}

/**
 * @brief Get the index of the first occurence of the given element in the list.
 * 
 * @param list A pointer to the list
 * @param data The element to look for
 * @return uint32_t If found the index of the first occurence, otherwise 0
 */
uint32_t list_findElement(listHead_t* list, void* data) {
    int32_t i = 1;
    element_t* cur = list->head;

    if (cur->data == data)
        return 1;

    while (cur != 0 && cur->next != 0) {
        ++i;
        if (cur->next->data == data) {
            return i;
        }
        cur = cur->next;
    }

    return 0;
}
