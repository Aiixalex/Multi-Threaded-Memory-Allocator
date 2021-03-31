#ifndef LIST_H   
#define LIST_H

/* Definition of struct nodeStruct */

struct nodeStruct {
    void* blockptr;
    struct nodeStruct *next;
};

/* Function prototypes */

struct nodeStruct* List_createNode (void* item);
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);
int  List_countNodes (struct nodeStruct *head);
struct nodeStruct* List_findNode (struct nodeStruct *head, void* item);
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);
void List_sort (struct nodeStruct **headRef);
void List_ascend_sort (struct nodeStruct **headRef);
void List_descend_sort (struct nodeStruct **headRef);


#endif