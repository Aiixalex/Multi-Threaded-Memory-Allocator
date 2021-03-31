#include "list.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode (void* item)
{
    struct nodeStruct *newNode = malloc(sizeof(struct nodeStruct));
    if(!newNode) {
        return NULL;
    } else {
        newNode->blockptr = item;
        newNode->next = NULL;
        return newNode;
    }
}

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node)
{
    if (node == NULL) { return; }
    node -> next = *headRef;
    *headRef = node;
}

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node)
{
    if (node == NULL) { return; }
    
    if(*headRef == NULL) {
        (*headRef) = node;
        return;
    }

    struct nodeStruct *tmp = *headRef;
    while (tmp -> next != NULL) {
        tmp = tmp -> next;
    }
    tmp -> next = node;
}

/*
 * Count numver of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head)
{
    if (head == NULL) { return 0; }
    int cnt = 0;
    struct nodeStruct *tmp = head;
    while (tmp != NULL) {
        cnt++;
        tmp = tmp -> next;
    }
    return cnt;
}

/*
 * Return the first node holding the value item, return NULL if none found
 */
struct nodeStruct* List_findNode (struct nodeStruct *head, void* item)
{
    if(head == NULL) { return NULL; }

    struct nodeStruct *tmp = head;
    while (tmp != NULL) {
        if(tmp -> blockptr == item) {
            return tmp;
        } else {
            tmp = tmp -> next;
        }
    }
    return NULL;
}

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set to a valid node
 * in the list. For example, the client code may have found it by calling
 * List_findNode(). If the list contains only one node, the head of the list
 * should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node)
{
    if (node == NULL) { return; }
    
    if (List_countNodes(*headRef) == 1 && *headRef == node) {
        free(*headRef);
        *headRef = NULL;
        return;
    }

    if (*headRef == node) {
        *headRef = (*headRef) -> next;
        free(node);
        return;
    }

    struct nodeStruct *curr = (*headRef) -> next;
    struct nodeStruct *prev = *headRef;
    while(node != curr) {
        prev = curr;
        if (curr -> next != NULL) {
            curr = curr -> next;
        } else {
            break;
        }
    }
    prev -> next = curr -> next;
    free(curr);
}

/*
 * Sort the list in ascending order based on the address of block.
 */
void List_sort (struct nodeStruct **headRef)
{   
    struct nodeStruct *ptr_j = *headRef;
    struct nodeStruct *ptr_i = NULL;
    
    if (*headRef == NULL) { return; }
    // Bubble sort
    int swap_completed = 0;
    while (!swap_completed) {
        swap_completed = 1;
        ptr_j = *headRef;
        while (ptr_j->next != ptr_i) {
            if (ptr_j->blockptr > ptr_j->next->blockptr) {
                void* temp = ptr_j->blockptr;
                ptr_j->blockptr = ptr_j->next->blockptr;
                ptr_j->next->blockptr = temp;
                swap_completed = 0;
            }
            ptr_j = ptr_j->next;
        }
        ptr_i = ptr_j;
    }
}

/*
 * Sort the list in ascending order based on the size of block. (read size from the header)
 */
void List_ascend_sort (struct nodeStruct **headRef)
{   
    struct nodeStruct *ptr_j = *headRef;
    struct nodeStruct *ptr_i = NULL;
    
    if (*headRef == NULL) { return; }
    // Bubble sort
    int swap_completed = 0;
    while (!swap_completed) {
        swap_completed = 1;
        ptr_j = *headRef;
        while (ptr_j->next != ptr_i) {
            if (*(long *)(ptr_j->blockptr - 8) > (*(long *)(ptr_j->next->blockptr - 8))) {
                void* temp = ptr_j->blockptr;
                ptr_j->blockptr = ptr_j->next->blockptr;
                ptr_j->next->blockptr = temp;
                swap_completed = 0;
            }
            ptr_j = ptr_j->next;
        }
        ptr_i = ptr_j;
    }
}

/*
 * Sort the list in descending order based on the size of block. (read size from the header)
 */
void List_descend_sort (struct nodeStruct **headRef)
{   
    struct nodeStruct *ptr_j = *headRef;
    struct nodeStruct *ptr_i = NULL;
    
    if (*headRef == NULL) { return; }
    // Bubble sort
    int swap_completed = 0;
    while (!swap_completed) {
        swap_completed = 1;
        ptr_j = *headRef;
        while (ptr_j->next != ptr_i) {
            if (*(long *)(ptr_j->blockptr - 8) < (*(long *)(ptr_j->next->blockptr - 8))) {
                void* temp = ptr_j->blockptr;
                ptr_j->blockptr = ptr_j->next->blockptr;
                ptr_j->next->blockptr = temp;
                swap_completed = 0;
            }
            ptr_j = ptr_j->next;
        }
        ptr_i = ptr_j;
    }
}