#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct temp{
    struct list_head list;
    int i;
};

int main()
{
#if 0
    struct temp *head = (struct temp *)malloc(sizeof(struct temp));
    struct temp *entry;

    INIT_LIST_HEAD(&head->list);

    entry = list_first_entry(&head->list, struct temp, list);
    printf("first%d %p\n",entry->i,entry);



    struct temp *a = (struct temp *)malloc(sizeof(struct temp));
    a->i=10;
    list_add_tail(&a->list, &head->list);

    struct temp *b = (struct temp *)malloc(sizeof(struct temp));
    b->i=11;
    list_add_tail(&b->list, &head->list);

    struct temp *c = (struct temp *)malloc(sizeof(struct temp));
    c->i=12;
    list_add_tail(&c->list, &head->list);

    //list_del(&a->list);
    //free(a);

    list_for_each_entry(entry,&head->list,list){
	printf("%d %p\n",entry->i,entry);
    }

    printf("\n");

    entry = list_first_entry(&head->list, struct temp, list);
    printf("first%d %p\n",entry->i,entry);

    list_for_each_entry(entry,&head->list,list){
	printf("%d %p\n",entry->i,entry);
    }
#endif
    LIST_HEAD(head);


    typedef struct node{
	    int i;
	    struct list_head list;
    } *NODE;

    NODE i;

    int j;
    for(j=0;j<10;j++){
	    i=malloc(sizeof(*i));
	    i->i=j;
	    //list_add(&i->list, &head);
    }

    list_for_each_entry(i, &head, list)
	    printf("%d\n",i->i);

    printf("\n");
    i=list_entry(head.prev, struct node, list);
    printf("%p %p\n",&head, i);
    list_del(&i->list);
    list_add(&i->list, &head);

    list_for_each_entry(i, &head, list)
	    printf("%d\n",i->i);

    return 1;
}
