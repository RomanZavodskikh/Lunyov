#include "../bad_funcs.h"
#include "ordered_set.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define POISON 13

#define NODE_OK 0
unsigned NODE_ERRNO = NODE_OK;

int root(struct Node* node_p)
{
    return node_p->par == NULL;
}

int right_child(struct Node* node_p)
{
    if(root(node_p))
    {
        return 0;
    }
    return node_p == node_p->par->rgt;
}

int left_child(struct Node* node_p)
{
    if(root(node_p))
    {
        return 0;
    }
    return node_p == node_p->par->lft;
}

struct Node* find_next(struct Node* node_p)
{
    if(node_p->rgt)
    {
        node_p=node_p->rgt;
        while(node_p->lft)
        {
            node_p=node_p->lft;
        }
        return node_p;
    }
    else if (!node_p->par)
    {
        return NULL;
    }
    else
    {
        while(right_child(node_p))
        {
            node_p=node_p->par;
        }
        node_p=node_p->par;
        return node_p;
    }
}

struct Node* find_prev(struct Node* node_p)
{
    if(node_p->lft)
    {
        node_p=node_p->lft;
        while(node_p->rgt)
        {
            node_p=node_p->rgt;
        }
        return node_p;
    }
    else if (!node_p->par)
    {
        return NULL;
    }
    else
    {
        while(left_child(node_p))
        {
            node_p=node_p->par;
        }
        node_p=node_p->par;
        return node_p;
    }
}

void swap(set_type* a_p, set_type* b_p)
{
    set_type tmp = *a_p;
    *a_p = *b_p;
    *b_p = tmp;
}

int delete_root(struct Node** node_pp)
{
    struct Node* node_p = *node_pp;
    int rtr_val = 0;

    if(!node_p->lft && !node_p->rgt)
    {
        node_dtor(node_p);
        node_p = NULL;
        rtr_val = 1;
    }
    else if (node_p->lft && !node_p->rgt)
    {
        node_p = node_p->lft;
        node_p->par->lft = NULL;
        node_dtor(node_p->par);
        node_p->par = NULL;
        rtr_val = 1;
    }
    else if (!node_p->lft && node_p->rgt)
    {
        node_p = node_p->rgt;
        node_p->par->rgt = NULL;
        node_dtor(node_p->par);
        node_p->par = NULL;
        rtr_val = 1;
    }
    else //if(node_p->lft && node_p->rgt)
    {
        //isolate movable node
        struct Node* node_to_mv = find_next(node_p);
        if(root(node_to_mv->par))
        {
            node_to_mv->par->rgt = NULL;
            swap(&node_to_mv->value, &node_p->value);
            node_dtor(node_to_mv);
        }
        else
        {
            node_to_mv->par->lft = node_to_mv->rgt;
            if (node_to_mv->rgt)
            {
                node_to_mv->rgt->par = node_to_mv->par;
            }
            node_to_mv->par = NULL;
            node_to_mv->rgt = NULL;
            
            swap(&node_to_mv->value, &node_p->value);
            node_dtor(node_to_mv);
        }
            
        rtr_val = 0;
    }
    *node_pp = node_p;
    return rtr_val;
}

int node_delete_no_children(struct Node* node_p)
{
    if(left_child(node_p))
    {
        node_p->par->lft = NULL;
    }
    else 
    {
        node_p->par->rgt = NULL;
    }
    node_dtor(node_p);
    return 1;
}

int node_delete_only_rgt_child(struct Node* node_p)
{
    if(left_child(node_p))
    {
        node_p->par->lft = node_p->rgt;
    }
    else 
    {
        node_p->par->rgt = node_p->rgt;
    }
    node_p->rgt->par = node_p->par;
    node_p->rgt = NULL;
    node_dtor(node_p);
    return 1;
}

int node_delete_only_lft_child(struct Node* node_p)
{
    if(left_child(node_p))
    {
        node_p->par->lft = node_p->lft;
    }
    else
    {
        node_p->par->rgt = node_p->lft;
    }
    node_p->lft->par = node_p->par;
    node_p->lft = NULL;
    node_dtor(node_p);
    return 1;
}

int node_delete_both_child(struct Node* node_p)
{
    node_p->rgt->par = node_p->par;
    if(left_child(node_p))
    {
        node_p->par->lft = node_p->rgt;
    }
    else if (right_child(node_p))
    {
        node_p->par->rgt = node_p->rgt;
    }

    node_p->lft->par = node_p->rgt;
    node_p->rgt->lft = node_p->lft;

    node_p->lft = node_p->rgt = NULL;
    node_dtor(node_p);
    return 1;
}

struct Node* node_ctor(set_type value, relation func)
{
    struct Node* rtr_val = calloc_bad(sizeof(*rtr_val), 1);
    if (rtr_val == NULL)
    {
        return rtr_val;
    }

    rtr_val->value = value;
    rtr_val->lft = rtr_val->rgt = rtr_val->par = NULL;
    rtr_val->func = func;

    assert(node_ok(rtr_val));
    return rtr_val;
}

void node_dtor(struct Node* node_p)
{
    if (!node_p)
    {
        return;
    }

    assert(node_ok(node_p));

    node_p->value = POISON;
    if (node_p->lft)
    {
        node_dtor(node_p->lft);
    }
    if (node_p->rgt)
    {
        node_dtor(node_p->rgt);
    }
    node_p->lft = node_p->rgt = node_p->par = NULL;
    free(node_p);
}

int node_ok(struct Node* node_p)
{
    if (node_p && node_p->lft && 
        (!node_ok(node_p->lft) || node_p->lft->par != node_p))
    {
        return 0;
    }
    if (node_p && node_p->rgt && 
        (!node_ok(node_p->rgt) || node_p->rgt->par != node_p))
    {
        return 0;
    }
    if (node_p && node_p->lft &&
        !node_p->func(node_p->lft->value, node_p->value))
    {
        return 0;
    }
    if (node_p && node_p->rgt && 
        !node_p->func(node_p->value, node_p->rgt->value))
    {
        return 0;
    }
    NODE_ERRNO = NODE_OK;
    return 1;
}

void node_dump(struct Node* node_p, unsigned level)
{
    assert(node_ok(node_p));

    for (unsigned i = 0; i < level; ++i) {
        printf(" ");
    }
    if (node_p==NULL)
    {
        printf("nil\n");
        return;
    }
    printf("[%p, l:%p, r:%p, p:%p]%f", (void*)node_p, (void*)node_p->lft, 
        (void*)node_p->rgt, (void*)node_p->par, node_p->value);
    printf("\n");
    node_dump(node_p->lft, level+1);
    node_dump(node_p->rgt, level+1);

    assert(node_ok(node_p));
}

int node_insert(struct Node* node_p, set_type value)
{
    assert(node_ok(node_p));
    int rtr_val = 0;

    if(node_p->func(value, node_p->value) && node_p->lft)
    {
        rtr_val = node_insert(node_p->lft, value);
    }
    else if(node_p->func(value, node_p->value) && !node_p->lft)
    {
        struct Node* node_to_ins = node_ctor(value, node_p->func);
        if (node_to_ins == NULL)
        {
            return 0;
        }
        node_p->lft = node_to_ins;
        node_p->lft->par = node_p;
        rtr_val = 1;
    }
    else if (node_p->func(node_p->value, value) && node_p->rgt)
    {
        rtr_val = node_insert(node_p->rgt, value);
    }
    else if (node_p->func(node_p->value, value) && !node_p->rgt)
    {
        struct Node* node_to_ins = node_ctor(value, node_p->func);
        if (node_to_ins == NULL)
        {
            return 0;
        }
        node_p->rgt = node_to_ins;
        node_p->rgt->par = node_p;
        rtr_val = 1;
    }
    else if (value == node_p->value)
    {
        return 0;
    }

    assert(node_ok(node_p));
    return rtr_val;
}

int node_delete(struct Node** node_pp, set_type value)
{
    struct Node* node_p = *node_pp;

    assert(node_ok(node_p));
    int rtr_val = 0;

    if(!node_p)
    {
        rtr_val = 0;
    }
    else if(node_p->value == value && root(node_p))
    {
        rtr_val = delete_root(node_pp);
    }
    else if(node_p->value == value && !node_p->rgt && !node_p->lft)
    {
        rtr_val = node_delete_no_children(node_p);
    }
    else if (node_p->value == value && node_p->rgt && !node_p->lft)
    {
        rtr_val = node_delete_only_rgt_child(node_p);
    }
    else if (node_p->value == value && !node_p->rgt && node_p->lft)
    {
        rtr_val = node_delete_only_lft_child(node_p);
    }
    else if (node_p->value == value && node_p->rgt && node_p->lft)
    {
        rtr_val = node_delete_both_child(node_p);
    }
    else if(node_p->func(value, node_p->value))
    {
        if (!node_p->lft)
        {
            rtr_val = 0;
        }
        else
        {
            rtr_val = node_delete(&(node_p->lft), value);
        }
    }
    else if(node_p->func(node_p->value, value))
    {
        if (!node_p->rgt)
        {
            rtr_val = 0;
        }
        else
        {
            rtr_val = node_delete(&(node_p->rgt), value);
        }
    }

    return rtr_val;
}

struct Node* node_find(struct Node* node_p, set_type value)
{
    assert(node_ok(node_p));
    if(value==node_p->value)
    {
        assert(node_ok(node_p));
        return node_p;
    }
    else if (node_p->func(value, node_p->value))
    {
        if (node_p->lft)
        {
            return node_find(node_p->lft, value);
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        if (node_p->rgt)
        {
            return node_find(node_p->rgt, value);
        }
        else
        {
            return NULL;
        }
    }
}

/*============================================================================= 
====================THE BIG BORDER BETWEEN===================================== 
====================THE TREE THAT HIGHER  ===================================== 
====================AND SET THAT LOWER    ===================================== 
=============================================================================*/

int set_ok(os_type* os_p)
{
    return node_ok(os_p->tree);
}

os_type* os_ctor(relation func)
{
    os_type* rtr_val = calloc_bad(sizeof(*rtr_val),1);
    if (!rtr_val)
    {
        return rtr_val;
    }

    rtr_val->tree = NULL;
    rtr_val->func = func;
    rtr_val->num_of_elems = 0UL;
    
    assert(set_ok(rtr_val));

    return rtr_val;
}

void os_dtor(os_type* os_p)
{
    if(!os_p)
    {
        return;
    }

    assert(set_ok(os_p));
    node_dtor(os_p->tree);
    os_p->func = NULL;
    os_p->num_of_elems = POISON;
    free(os_p);
}

void clear(os_type* os_p)
{
    assert(set_ok(os_p));
    node_dtor(os_p->tree);
    os_p->tree = NULL;
    assert(set_ok(os_p));
}

int empty(os_type* os_p)
{
    assert(set_ok(os_p));
    return (os_p->tree == NULL);
}

osi_type insert(os_type* os_p, set_type val)
{
    assert(set_ok(os_p));

    if(os_p->num_of_elems == 0)
    {
        os_p->tree = node_ctor(val, os_p->func);
    }
    else
    {
        node_insert(os_p->tree, val);
    }

    if(os_p->tree == NULL)
    {
        osi_type osi;
        osi.tree = NULL;
        assert(set_ok(os_p));
        return osi;
    }

    os_p->num_of_elems++;
    osi_type osi;
    osi.tree = node_find(os_p->tree, val);
    assert(set_ok(os_p));
    return osi;
}

osi_type find(os_type* os_p, set_type val)
{
    assert(set_ok(os_p));
    osi_type osi;
    osi.tree = node_find(os_p->tree, val);
    assert(set_ok(os_p));
    return osi;
}

osi_type begin(os_type* os_p)
{
    assert(set_ok(os_p));
    struct Node* point = os_p->tree;
    while(point->lft)
    {
        point=point->lft;
    }
    osi_type osi;
    osi.tree = point;
    assert(set_ok(os_p));
    return osi;
}

osi_type end(os_type* os_p)
{
    assert(set_ok(os_p));
    struct Node* point = os_p->tree;
    while(point->rgt)
    {
        point=point->rgt;
    }
    osi_type osi;
    osi.tree = point;
    assert(set_ok(os_p));
    return osi;
}

unsigned long count (os_type* os_p, set_type val)
{
    assert(set_ok(os_p));
    return find(os_p, val).tree != NULL;
}

unsigned long size (os_type* os_p)
{
    assert(set_ok(os_p));
    return os_p->num_of_elems;
}

void erase (os_type* os_p, osi_type* const position)
{
    if (position->tree == NULL)
    {
        return;
    }

    os_p->num_of_elems -= node_delete(&(os_p->tree), position->tree->value);
}

void set_dump (os_type* os_p)
{
    if (!os_p)
    {
        return;
    }

    printf("Set: size %li, tree %p\n", os_p->num_of_elems,
        (void*)os_p->tree);
    node_dump(os_p->tree, 1);
}

void osi_inc(osi_type* osi_p)
{
    (*osi_p).tree = find_next((*osi_p).tree);
}

void osi_dec(osi_type* osi_p)
{
    (*osi_p).tree = find_prev((*osi_p).tree);
}

