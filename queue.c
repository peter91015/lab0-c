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

typedef struct {
    struct list_head list;
    int *dup;
    char *key;
} h_table_list;
/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
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
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *current = NULL, *safe = NULL;
    for (current = head->next->next, safe = current->next->next;
         current != head && current->prev != head;
         current = safe, safe = current->next->next) {
        struct list_head *prev = current->prev;

        prev->prev->next = current;
        current->prev = prev->prev;

        prev->prev = current;
        current->next = prev;

        prev->next = safe->prev;
        safe->prev->prev = prev;
    }
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
    if (!head || head->next == head->prev)
        return 0;
    int size = q_size(head);
    // element_t *current = NULL, *next = NULL;
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
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return size;
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
    // free(l2);
    // l2 = NULL;
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
    // no need to free the queue_contex_t merged
    for (current = head->next->next; current != head; current = current->next) {
        // merge current queue the first queue
        second_q = list_entry(current, queue_contex_t, chain)->q;
        merge_2_list(first_q, second_q);
    }
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return q_size(first_q);
}
