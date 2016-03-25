#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "ordered_set.h"

int less (set_type x, set_type y)
{
    return x < y;
}

int main()
{
    //defining random number for imitating calloc() falls
    srand(time(NULL));

    //making invalid tree to test node_ok()
    for (int i = 0; i < 900; ++i)
    {
        struct Node* node_p2 = node_ctor(1, less);
        if (!node_p2) goto eoc2;
        if (!node_insert(node_p2, 2)) goto eoc2;
        if (!node_insert(node_p2, 0)) goto eoc2;
        node_p2->rgt->par = NULL;
        node_ok(node_p2);
        node_p2->rgt->par = node_p2;
        node_p2->lft->par = NULL;
        node_ok(node_p2);
        node_p2->lft->par = node_p2;
        node_p2->rgt->value = 0;
        node_ok(node_p2);
        node_p2->rgt->value = 2;
        node_p2->lft->value = 2;
        node_ok(node_p2);
        node_p2->lft->value = 0;
eoc2:
        node_dtor(node_p2);
    }

    //testing all the rest node funcs
    for (int i = 0; i < 1000; ++i)
    {
        printf("==========================================================\n");
        struct Node* node_p = node_ctor(30, less);
        if (!node_p) goto eoc;
        node_insert(node_p, 15);
        node_insert(node_p, 10);
        node_insert(node_p, 35);
        node_insert(node_p, 40);
        node_insert(node_p, 17);
        node_insert(node_p, 19);
        node_insert(node_p, 33);
        node_insert(node_p, 37);
        node_insert(node_p, 40);
        node_dump(node_p, 0);
        printf("\n\n");

        node_delete(&node_p, 10);
        node_delete(&node_p, 15);
        node_delete(&node_p, 40);
        node_delete(&node_p, 35);
        node_dump(node_p, 0);
        printf("\n\n");

        node_delete(&node_p, 30);
        node_dump(node_p, 0);
        printf("\n\n");

        node_delete(&node_p, 37);
        node_delete(&node_p, 33);
        node_dump(node_p, 0);
        printf("\n\n");

        node_delete(&node_p, 17);
        node_delete(&node_p, 19);
        node_dump(node_p, 0);
        printf("\n\n");
        node_dtor(node_p);

        node_p = node_ctor(9, less);
        if (!node_p) goto eoc;
        node_insert(node_p, 1);
        node_insert(node_p, 40);
        node_insert(node_p, 39);
        node_insert(node_p, 20);
        node_insert(node_p, 30);
        node_insert(node_p, 25);
        node_insert(node_p, 35);
        node_dump(node_p, 0);
        printf("\n\n");

        printf("%p ", (void*)node_find(node_p, 1));
        printf("%p ", (void*)node_find(node_p, 2));
        printf("%p ", (void*)node_find(node_p, 3));
        printf("%p ", (void*)node_find(node_p, 4));
        printf("%p ", (void*)node_find(node_p, 19));
        printf("%p ", (void*)node_find(node_p, 25));
        printf("\n\n");

        node_delete(&node_p, 9);
        node_dump(node_p, 0);
        printf("\n\n");
    eoc:
        node_dtor(node_p);
    }

    //testing the ordered set itself
    for (int i = 0; i < 1000; ++i)
    {
        printf("==========================================================\n");

        os_type* os_p = os_ctor(less);
        set_dump(os_p);
        printf("\n\n");
        if(!os_p) goto eoc3;

        printf("%i\n", empty(os_p));
        printf("\n\n");

        insert(os_p, 8);
        osi_type osi = insert(os_p, 3);
        insert(os_p, 9);
        set_dump(os_p);
        printf("\n\n");

        osi = begin(os_p);
        while(osi.tree)
        {
            printf("%lg ", osi.tree->value);
            osi_inc(&osi);
        }
        printf("\n\n");

        osi = end(os_p);
        while(osi.tree)
        {
            printf("%lg ", osi.tree->value);
            osi_dec(&osi);
        }
        printf("\n\n");

        printf("%i\n", empty(os_p));
        printf("%p %p\n", (void*)find(os_p, 1).tree,
            (void*)find(os_p, 3).tree);
        printf("%p %p\n", (void*)begin(os_p).tree,
            (void*)end(os_p).tree);
        printf("%li %li\n", count(os_p, 3), count(os_p, 4));
        printf("%li\n", size(os_p));
        printf("\n\n");

        osi = find(os_p, 8);
        erase(os_p, &osi);
        set_dump(os_p);
        printf("%li\n", size(os_p));
        printf("\n\n");

        clear(os_p);
        set_dump(os_p);
        printf("%i\n", empty(os_p));
        printf("\n\n");
    eoc3:
        os_dtor(os_p);
    }

    return 0;
}


