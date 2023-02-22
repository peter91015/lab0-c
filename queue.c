#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define HASH_TABLE_SIZE 100

#include "queue.h"

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
    element_t *new_item = malloc(sizeof(element_t));
    if (!new_item || !s)
        return false;
    size_t len_s = strlen(s) + 1;
    new_item->value = malloc(len_s);
    if (!new_item->value) {
        free(new_item);
        return false;
    }
    memcpy(new_item->value, s, len_s);
    list_add(&new_item->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *new_item = malloc(sizeof(element_t));
    if (!new_item || !s)
        return false;
    size_t len_s = strlen(s) + 1;
    new_item->value = malloc(len_s);
    if (!new_item->value) {
        free(new_item);
        return false;
    }
    memcpy(new_item->value, s, len_s);
    list_add_tail(&new_item->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;
    element_t *pop_item = list_entry(head->next, element_t, list);
    list_del(head->next);
    if (sp) {
        memcpy(sp, pop_item->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return pop_item;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (list_empty(head))
        return NULL;
    element_t *pop_item = list_entry(head->prev, element_t, list);
    list_del(head->prev);
    if (sp) {
        memcpy(sp, pop_item->value, bufsize - 1);
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

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    element_t *current = NULL, *next = NULL;
    hcreate(HASH_TABLE_SIZE);
    ENTRY e;
    ENTRY *ep;
    list_for_each_entry (current, head, list) {
        int str_len = strlen(current->value) + 1;
        e.key = malloc(str_len);
        if (!e.key)
            return false;
        memcpy(e.key, current->value, str_len);
        e.data = malloc(sizeof(int));
        if (!e.key)
            return false;
        *((int *) (e.data)) = 1;
        ep = hsearch(e, FIND);
        if (!ep) {
            ep = hsearch(e, ENTER);
            if (!ep)
                return false;
        } else
            *((int *) (ep->data)) = 2;
    }
    list_for_each_entry_safe (current, next, head, list) {
        e.key = current->value;
        e.data = 0;
        ep = hsearch(e, FIND);
        if (*((int *) ep->data) == 2) {
            list_del(&current->list);
            free(current->value);
            free(current);
        }
    }
    hdestroy();
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
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

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

/* Sort elements of queue in ascending order */
void q_sort(struct list_head *head)
{
    if (!head || head->next == head->prev)
        return;
    element_t *current = NULL, *next = NULL;

    char *pivot = list_entry(head->next, element_t, list)->value;
    struct list_head greater, less;
    INIT_LIST_HEAD(&greater);
    INIT_LIST_HEAD(&less);
    list_for_each_entry_safe (current, next, head, list) {
        int tmp = strcmp(current->value, pivot);
        if (tmp > 0) {
            list_del(&current->list);
            list_add(&current->list, &greater);
        } else if (tmp < 0) {
            list_del(&current->list);
            list_add(&current->list, &less);
        }
    }
    q_sort(&less);
    q_sort(&greater);
    list_splice_tail(&greater, head);
    list_splice(&less, head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending order */
int q_merge(struct list_head *head)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
