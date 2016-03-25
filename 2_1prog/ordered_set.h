#ifndef ORDERED_SET_H
#define ORDERED_SET_H
typedef double set_type;
typedef int (*relation)(set_type x, set_type y);

struct Node
{
    set_type value;
    struct Node* lft;
    struct Node* rgt;
    struct Node* par;
    relation func;
};

struct Node* node_ctor(set_type value, relation func);
void node_dtor(struct Node* node_p);
int node_ok(struct Node* node_p);
void node_dump(struct Node* node_p, unsigned level);
int node_insert(struct Node* node_p, set_type value);
int node_delete(struct Node** node_pp, set_type value);
struct Node* node_find(struct Node* node_p, set_type value);

/*=============================================================================
====================THE BIG BORDER BETWEEN=====================================
====================THE TREE THAT HIGHER  =====================================
====================AND SET THAT LOWER    =====================================
=============================================================================*/
struct ordered_set_iterator
{
    struct Node* tree;
};

struct ordered_set
{
    struct Node* tree;
    relation func;
    unsigned long num_of_elems;
};

typedef struct ordered_set os_type;
typedef struct ordered_set_iterator osi_type;

os_type* os_ctor(relation func);
void os_dtor(os_type* OS);
void clear(os_type* OS);
unsigned long count(os_type* OS, set_type val);
unsigned long size(os_type* OS);
int empty(os_type* OS);
int set_ok(os_type* os_p);
void set_dump(os_type* os_p);

void osi_inc(osi_type* OSI);
void osi_dec(osi_type* OSI);
osi_type begin(os_type* OS);
osi_type end(os_type* OS);
osi_type insert(os_type* OS, set_type val);
osi_type find(os_type* OS, set_type val);
void erase(os_type* os_p, osi_type* const position);
#endif // ORDERED_SET_H
