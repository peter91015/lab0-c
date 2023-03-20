#include <linux/types.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASH_TABLE_SIZE 64
#define MAX_STRING_SIZE 512


#include "hlist.h"
#include "queue.h"

const size_t element_size = sizeof(element_t);
/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (new)
        INIT_LIST_HEAD(new);
    return new;
}



/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    struct list_head *current = NULL, *safe = NULL;
    list_for_each_safe (current, safe, l) {
        element_t *tmp = list_entry(current, element_t, list);
        current = current->next;
        free(tmp->value);
        free(tmp);
    }
    free(l);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    // if (!s)
    //     return false;
    if (!head)
        return false;
    element_t *new_item = malloc(element_size);
    if (!new_item)
        return false;
    new_item->value = strdup(s);
    // size_t len_s = strlen(s) + 1;
    // new_item->value = malloc(len_s);
    if (!new_item->value) {
        free(new_item);
        return false;
    }
    // strncpy(new_item->value, s, len_s);
    list_add(&new_item->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    // if (!s)
    //     return false;
    if (!head)
        return false;
    element_t *new_item = malloc(element_size);
    if (!new_item)
        return false;
    // size_t len_s = strlen(s) + 1;
    // new_item->value = malloc(len_s);
    new_item->value = strdup(s);
    if (!new_item->value) {
        free(new_item);
        return false;
    }
    // strncpy(new_item->value, s, len_s);
    list_add_tail(&new_item->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head)
        return NULL;
    if (list_empty(head))
        return NULL;
    element_t *pop_item = list_entry(head->next, element_t, list);
    list_del(head->next);
    if (sp) {
        // memcpy(sp, pop_item->value, bufsize - 1);
        strncpy(sp, pop_item->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return pop_item;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head)
        return NULL;
    if (list_empty(head))
        return NULL;
    element_t *pop_item = list_entry(head->prev, element_t, list);
    list_del(head->prev);
    if (sp) {
        // memcpy(sp, pop_item->value, bufsize - 1);
        strncpy(sp, pop_item->value, bufsize);
        sp[bufsize - 1] = '\0';
    }
    return pop_item;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    struct list_head *current = NULL;
    int cnt = 0;  // the counter for the number of element
    list_for_each (current, head)
        cnt++;
    return cnt;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || head->prev == head)
        return false;
    struct list_head *next = NULL, *prev = NULL;
    for (next = head->next, prev = head->prev;
         next != prev->prev && next != prev;
         prev = prev->prev, next = next->next)
        ;
    // next will be the node deleted after for loop
    list_del(next);
    element_t *pop_item = list_entry(next, element_t, list);
    free(pop_item->value);
    free(pop_item);
    return true;
}

/*typedef struct {
    struct list_head list;
    int *dup;
    char *key;
} h_table_list;*/
typedef struct {
    char *value;
    struct hlist_node hlist;
    int flag;
} hash_element_t;
#define GOLDEN_RATIO_32 0x61C88647
#define __hash_32 __hash_32_generic
static inline __u32 __hash_32_generic(__u32 val)
{
    return val * GOLDEN_RATIO_32;
}

static inline __u32 hash_32(__u32 val, unsigned int bits)
{
    return __hash_32(val) >> (32 - bits);
}
static unsigned compute_key(char *const value)
{
    char *current = NULL;
    char accumulate = 0;
    for (current = value; *current; current++)
        accumulate = accumulate ^ *current;
    return (unsigned) accumulate;
}
/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    struct hlist_head hash_table[HASH_TABLE_SIZE];
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].first = NULL;
    }
    element_t *current = NULL, *next = NULL;
    // first traverse the list to build the hash table

    list_for_each_entry (current, head, list) {
        size_t key = (size_t) compute_key(current->value);
        key = hash_32(key, 6);

        hash_element_t *cur_hlist =
            hlist_entry_safe(hash_table[key].first, hash_element_t, hlist);
        hlist_for_each_entry_from(cur_hlist, hlist)
        {
            int tmp = strcmp(cur_hlist->value, current->value);
            if (!tmp) {  // there are duplicate strings
                cur_hlist->flag = 1;
                break;
            }
        }
        if (!cur_hlist) {  // cur_hlist is NULL means that all nodes in hlist
                           // are traversed or hlist has no node
            // add a new node for hlist
            hash_element_t *new_item = malloc(sizeof(hash_element_t));
            new_item->value = strdup(current->value);
            new_item->flag = 0;  // means no duplicate so far
            new_item->hlist.next = NULL;
            new_item->hlist.pprev = NULL;
            hlist_add_head(&new_item->hlist, &hash_table[key]);
        }
    }

    // second traversal to delete the duplicate nodes with hash table
    current = NULL;
    list_for_each_entry_safe (current, next, head, list) {
        size_t key = (size_t) compute_key(current->value);
        key = hash_32(key, 6);
        hash_element_t *cur_hlist =
            hlist_entry_safe(hash_table[key].first, hash_element_t, hlist);
        hlist_for_each_entry_from(cur_hlist, hlist)
        {
            int tmp = strcmp(cur_hlist->value, current->value);
            if (!tmp) {
                if (cur_hlist->flag) {
                    list_del(&(current->list));
                    free(current->value);
                    free(current);
                    break;
                }
            }
        }
    }
    // free the hash table
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_element_t *current = NULL;
        struct hlist_node *next = NULL;
        hlist_for_each_entry_safe(current, next, &hash_table[i], hlist)
        {
            free(current->value);
            free(current);
        }
    }
    return true;
}
/*{
    element_t *current = NULL, *next = NULL;
    struct list_head *table_list = malloc(sizeof(struct list_head));
    INIT_LIST_HEAD(table_list);
    hcreate(HASH_TABLE_SIZE);
    ENTRY e;
    ENTRY *ep;
    list_for_each_entry (current, head, list) {
        e.key = current->value;
        e.data = (void *) 0;
        ep = hsearch(e, FIND);
        if (!ep) {
            int str_len = strlen(current->value) + 1;
            e.key = malloc(str_len);
            e.data = malloc(sizeof(int));
            *((int *) (e.data)) = 1;
            h_table_list *entry = malloc(sizeof(h_table_list));
            entry->dup = e.data;
            entry->key = e.key;
            list_add(&entry->list, table_list);
            if (!e.key)
                return false;
            memcpy(e.key, current->value, str_len);
            ep = hsearch(e, ENTER);
            if (!ep)
                return false;
        } else
            *((int *) (ep->data)) = 2;
        // free(e.key);
    }
    list_for_each_entry_safe (current, next, head, list) {
        e.key = current->value;
        e.data = 0;
        ep = hsearch(e, FIND);
        if ((*(int *) ep->data) == 2) {
            list_del(&(current->list));
            free(current->value);
            free(current);
        }
    }

    h_table_list *cur = NULL, *safe = NULL;
    list_for_each_entry_safe (cur, safe, table_list, list) {
        free(cur->dup);
        free(cur->key);
        free(cur);
    }
    free(table_list);
    hdestroy();
    return true;
}*/

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    q_reverseK(head, 2);
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *prev = head, *current = NULL, *next = NULL;
    list_for_each_safe (current, next, head) {
        current->next = prev;
        current->prev = next;
        prev = current;
    }
    // swap the next and prev pointers of head
    prev = head->next;
    head->next = head->prev;
    head->prev = prev;
}
struct list_head *traverse_k_node(struct list_head *head,
                                  struct list_head *start,
                                  int k)
{
    int i;
    struct list_head *current = NULL;
    for (i = 0, current = start; i < k - 1 && current != head;
         i++, current = current->next)
        ;
    return current == head ? head : current;
}
void reverse_nodes(struct list_head *head,
                   struct list_head *first,
                   struct list_head *last)
{
    head->next = first;
    first->prev = head;
    head->prev = last;
    last->next = head;
    q_reverse(head);
    first->next = NULL;
    last->prev = NULL;
}
/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    struct list_head *current = NULL, *tail = NULL;
    for (current = head->next, tail = traverse_k_node(head, current, k);
         tail != head && current != head;
         current = current->next, tail = traverse_k_node(head, current, k)) {
        struct list_head tmp_head, *prev = current->prev, *next = tail->next;
        reverse_nodes(&tmp_head, current, tail);
        prev->next = tail;
        tail->prev = prev;
        next->prev = current;
        current->next = next;
    }
}

/* merge two sorted lists into l1 in the ascending order*/
void merge_2_list(struct list_head *l1, struct list_head *l2)
{
    struct list_head tmp;
    INIT_LIST_HEAD((&tmp));
    list_splice(l1, &tmp);  // temporarily put l1 into tmp
    INIT_LIST_HEAD(l1);
    for (; !list_empty(&tmp) && !list_empty(l2);) {
        char *str_1 = list_entry(tmp.next, element_t, list)->value,
             *str_2 = list_entry(l2->next, element_t, list)->value;
        strcmp(str_1, str_2) < 0 ? list_move_tail(tmp.next, l1)
                                 : list_move_tail(l2->next, l1);
    }
    // splice the rest of queue
    list_empty(&tmp) ? list_splice_tail(l2, l1) : list_splice_tail(&tmp, l1);
    INIT_LIST_HEAD(&tmp);
    INIT_LIST_HEAD(l2);
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || head->next == head->prev)
        return;
    struct list_head *next = NULL, *prev = NULL;
    // divide the list into two lists
    LIST_HEAD(first);
    // meet the middle node
    for (next = head->next, prev = head->prev;
         next != prev->prev && next != prev;
         prev = prev->prev, next = next->next)
        ;

    list_cut_position(&first, head, next);
    q_sort(&first);
    q_sort(head);
    merge_2_list(head, &first);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || head->next == head->prev)
        return 0;
    int size = q_size(head);
    struct list_head *current = NULL, *next = NULL;
    char *max_str = list_entry(head->prev, element_t, list)->value;
    for (current = head->prev, next = head->prev->prev; current != head;
         current = next, next = next->prev) {
        element_t *cur_element = list_entry(current, element_t, list);
        int tmp = strcmp(cur_element->value, max_str);
        if (tmp > 0)
            max_str = cur_element->value;
        // list_del(&current->list);
        else if (tmp < 0) {
            list_del(&(cur_element->list));
            free(cur_element->value);
            free(cur_element);
            size--;
        }
    }
    return size;
}



/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    if (!head)
        return 0;
    struct list_head *current = NULL,
                     *first_q =
                         list_entry(head->next, queue_contex_t, chain)->q,
                     *second_q = NULL;

    for (current = head->next->next; current != head; current = current->next) {
        // merge current queue the first queue
        second_q = list_entry(current, queue_contex_t, chain)->q;
        merge_2_list(first_q, second_q);
    }
    return q_size(first_q);
}
