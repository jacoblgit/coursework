// Generic dictionary implementation
// adapted from Aspenes' Notes

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gmap.h"     

struct dictElt {
    unsigned long hash;           /* full hash of key */
    void *key;
    void *value;
    struct dictElt *next;
};

struct _gmap {
    int tableSize;          /* number of slots in table */
    int numElements;        /* number of elements */
    struct dictElt **table; /* linked list heads */
    
    // key operations (passed as arguements at creation)
    void *(*cp)(const void *);
    int (*comp)(const void *, const void *);    // evaluates to 0 when equal
    size_t (*h)(const void *s);
    void (*f)(void *);
};

#define INITIAL_TABLESIZE (16)
#define TABLESIZE_MULTIPLIER (2)
#define TABLE_GROW_DENSITY (1)

// helper functions
static struct dictElt * dictFetch(const gmap* d, const void *key);
static void dictGrow(gmap* d);
// void gmap_print_helper(const void* key, void* value, void* arg);
void print_element(struct dictElt* e);


// DONE AND CHECKED
gmap* gmap_create(void *(*cp)(const void *), int (*comp)(const void *, const void *), size_t (*h)(const void *s), void (*f)(void *)) {
    gmap* d;
    int i;

    d = malloc(sizeof(*d));
    if(d == 0) return 0;

    d->tableSize = INITIAL_TABLESIZE;
    d->numElements = 0; 
    d->cp = cp;
    d->comp = comp;
    d->h = h;
    d->f = f;
    d->table = malloc(sizeof(*(d->table)) * d->tableSize);
    if(d->table == 0) {
        free(d);
        return 0;
    }

    for(i = 0; i < d->tableSize; i++) d->table[i] = 0;

    return d;
}

// DONE AND CHECKED
size_t gmap_size(const gmap *m) {
    return m->numElements;
}

// NEED TO FINISH (SEE NOTE ABOUT GMAP_ERROR)
void *gmap_put(gmap *d, const void *key, void *value)
{
    int tablePosition;
    struct dictElt *e;
    void* prev_val;

    e = dictFetch(d, key);
    if(e != 0) {
        /* change existing setting */
        prev_val = e->value;
        e->value = value;
        return prev_val;
    } else {
        /* create new element */
        e = malloc(sizeof(*e));

        // CHANGE THIS LINE TO INVOLVE EXTERN CHAR *GMAP_ERROR
        // TO REPORT ERROR
        if(e == 0) abort();

        e->hash = d->h(key);
        e->key = d->cp(key);
        e->value = value;

        /* link it in */
        tablePosition = e->hash % d->tableSize;
        e->next = d->table[tablePosition];
        d->table[tablePosition] = e;

        d->numElements++;

        if(d->numElements > d->tableSize * TABLE_GROW_DENSITY) {
            /* grow and rehash */
            dictGrow(d);
        }

        // for testing
        // print_element(e);

        return NULL;
    }
}

// DONE AND CHECKED (NEED TO BUGTEST)
void *gmap_remove(gmap *d, const void *key)
{
    struct dictElt *e;
    struct dictElt *e_prev;
    int i;
    void* prev_val;

    e = dictFetch(d, key);
    if(e != 0) {
        prev_val = e->value;

        // relink list to exclude e
        i = e->hash % d->tableSize;
        
        // e is first element in list
        if (d->table[i] == e) {
            d->table[i] = e->next;

        } else {
            /* e is not first element in list */
            for(e_prev = d->table[i]; e_prev->next != e; e_prev = e_prev->next) {}
            e_prev->next = e->next;
        }
           
        // for(e_prev = d->table[i]; ((e_prev->next != e) && (e_prev->next != NULL)); e_prev = e_prev->next) {}
        // e_prev->next = e->next;
        
        // free memory
        d->f(e->key);
        free(e);

        // book keeping
        d->numElements--;

        return prev_val;
    } else {
        /* key is not present */
        return NULL;
    }
}

// DONE AND CHECKED
bool gmap_contains_key(const gmap *d, const void *key)
{
    struct dictElt* e;
    e = dictFetch(d, key);

    if (e == NULL) return false;
    else return true;
}

// DONE AND CHECKED
void *gmap_get(gmap *d, const void *key) {
    struct dictElt *e;

    e = dictFetch(d, key);
    if(e != 0) {
        return e->value;
    } else {
        return 0;
    }
}

// DONE AND CHECKED (NEED TO BUGTEST)
const void **gmap_keys(gmap *d)
{
    const void** keys;
    int table_row, counter = 0;
    struct dictElt *e;

    keys = malloc(sizeof(void*) * d->numElements);
    if (keys == NULL) return NULL;
    
    for(table_row = 0; table_row < d->tableSize; table_row++) {
        for(e = d->table[table_row]; e != 0; e = e->next) {
            keys[counter] = e->key;
            counter++;
        }
    }

    return keys;
}

// DONE AND CHECKED (NEED TO BUGTEST)
void gmap_for_each(gmap *d, void (*f)(const void *, void *, void *), void *arg)
{
    int i;
    struct dictElt *e;
    
    for(i = 0; i < d->tableSize; i++) {
        for(e = d->table[i]; e != 0; e = e->next) {
            f(e->key, e->value, arg);
        }
    }
}

// DONE AND CHECKED
void gmap_destroy(gmap *d)
{
    if (d == NULL) return;

    int i;
    struct dictElt *e;
    struct dictElt *next;

    for(i = 0; i < d->tableSize; i++) {
        for(e = d->table[i]; e != 0; e = next) {
            next = e->next;
            d->f(e->key);
            free(e);
        }
    }
    free(d->table);
    free(d);
}


// ****************************** //
//      HELPER FUNCTIONS          //
// ****************************** //

/* return pointer to element with given key, if any */
static struct dictElt * dictFetch(const gmap* d, const void *key)
{
    unsigned long h;
    int i;
    struct dictElt *e;

    h = d->h(key);
    i = h % d->tableSize;
    for(e = d->table[i]; e != 0; e = e->next) {
        if(e->hash == h && (d->comp(key, e->key) == 0)) {
            /* found it */
            return e;
        }
    }
    /* didn't find it */
    return 0;
}

/* increase the size of the dictionary, rehashing all table elements */
static void dictGrow(gmap* d)
{
    struct dictElt **old_table;
    int old_size;
    int i;
    struct dictElt *e;
    struct dictElt *next;
    int new_pos;

    /* save old table */
    old_table = d->table;
    old_size = d->tableSize;

    /* make new table */
    d->tableSize *= TABLESIZE_MULTIPLIER;
    d->table = malloc(sizeof(*(d->table)) * d->tableSize);
    if(d->table == 0) {
        /* put the old one back */
        d->table = old_table;
        d->tableSize = old_size;
        return;
    }
    /* else */
    /* clear new table */
    for(i = 0; i < d->tableSize; i++) d->table[i] = 0;

    /* move all elements of old table to new table */
    for(i = 0; i < old_size; i++) {
        for(e = old_table[i]; e != 0; e = next) {
            next = e->next;
            /* find the position in the new table */
            new_pos = e->hash % d->tableSize;
            e->next = d->table[new_pos];
            d->table[new_pos] = e;
        }
    }

    /* don't need this any more */
    free(old_table);
}

// MUST CHANGE FOR DATA TYPES
void print_element(struct dictElt* e) {
    printf("%s, %d, %lu\n", (char*) e->key, *((int*) e->value), e->hash);
}

// DELETE THIS AFTER TESTING. ONLY WORKS ON STRING, INT pairs
// void gmap_print_helper(const void* key, void* value, void* arg)
// {
//     printf("%s, %d\n", *((char*) key), *((int*) value));
// }

