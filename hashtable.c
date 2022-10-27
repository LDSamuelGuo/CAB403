

#include <inttypes.h> // for portable integer declarations
#include <stdbool.h>  // for bool type
#include <stdio.h>    // for printf()
#include <stdlib.h>   // for malloc(), free(), NULL
#include <string.h>   // for strcmp()

// An item inserted into a hash table.
typedef struct item item_t;


struct item
{
    char *key;
    int value;
    item_t *next;
};

void item_print(item_t *i)
{
    printf("key=%s value=%d", i->key, i->value);
}

// A hash table mapping a string to an integer.
typedef struct htab htab_t;
struct htab
{
    item_t **buckets;
    size_t size;
};

// Initialise a new hash table with n buckets.
bool htab_init(htab_t *h, size_t n)
{
    // TODO: implement this function
    h->buckets = (item_t **)calloc(n,sizeof(item_t *));
    h->size = n; 

    return h->buckets != NULL;    
}

// The Bernstein hash function.

size_t djb_hash(char *s)
{
    size_t hash = 5381;
    int c;
    while ((c = *s++) != '\0')
    {
        // hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

// Calculate the offset for the bucket for key in hash table.
size_t htab_index(htab_t *h, char *key)
{
    return djb_hash(key) % h->size;
}

// Find pointer to head of list for key in hash table.
item_t *htab_bucket(htab_t *h, char *key)
{
    // TODO: implement this function (uses htab_index())
    return h->buckets[htab_index(h, key)];
}

// Find an item for key in hash table.

item_t *htab_find(htab_t *h, char *key)
{
    // TODO: implement this function
    item_t *current = htab_bucket(h, key);

    for (; current != NULL; current = current->next){
        if (strcmp(current->key,key) == 0){
            return current;
        }
    }

    return NULL;
}

// Add a key with value to the hash table.

bool htab_add(htab_t *h, char *key, int value)
{
 
    item_t *new_node = (item_t *)malloc(sizeof(item_t));
    if (new_node == NULL){
        return false;
    }

    size_t bucket_idx = htab_index(h,key);

    new_node->key = key;
    new_node->value = value;
    new_node->next = h->buckets[bucket_idx];
    h->buckets[bucket_idx] = new_node;

    return true;
}

// Print the hash table.

void htab_print(htab_t *h)
{
    printf("hash table with %d buckets\n", h->size);
    for (size_t i = 0; i < h->size; ++i)
    {
        printf("bucket %d: ", i);
        if (h->buckets[i] == NULL)
        {
            printf("empty\n");
        }
        else
        {
            for (item_t *j = h->buckets[i]; j != NULL; j = j->next)
            {
                item_print(j);
                if (j->next != NULL)
                {
                    printf(" -> ");
                }
            }
            printf("\n");
        }
    }
}

// Delete an item with key from the hash table.

void htab_delete(htab_t *h, char *key)
{
    item_t *head = htab_bucket(h, key);
    item_t *current = head;
    item_t *previous = NULL;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (previous == NULL)
            { // first item in list
                h->buckets[htab_index(h, key)] = current->next;
            }
            else
            {
                previous->next = current->next;
            }
            free(current);
            break;
        }
        previous = current;
        current = current->next;
    }
}

// Destroy an initialised hash table.

void htab_destroy(htab_t *h)
{
   
    for (int i = 0; i < h->size; i++){
        item_t *current = h->buckets[i];

        while (current != NULL){
            item_t *next = current->next;
            free(current);

            current = next;
        }
    }

    free(h->buckets);
    h->buckets= NULL;
    h->size = 0;
}

int main(int argc, char **argv)
{
    // create a hash table with 10 buckets
    printf("creating hash table:\n");
    size_t buckets = 10;
    htab_t h;
    if (!htab_init(&h, buckets))
    {
        printf("failed to initialise hash table\n");
        return EXIT_FAILURE;
    }

