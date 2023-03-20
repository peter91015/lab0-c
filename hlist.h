/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_LIST_H
#define _LINUX_LIST_H

//#include <linux/container_of.h>
#include <linux/stddef.h>
#include <linux/types.h>
//#include <linux/poison.h>
#include <linux/const.h>
#include <stdbool.h>

//#include <asm/barrier.h>



struct hlist_head {
    struct hlist_node *first;
};

struct hlist_node {
    struct hlist_node *next, **pprev;
};

#define __READ_ONCE(x) (*(volatile typeof(x) *) &(x))

#define READ_ONCE(x)                                                \
    ({                                                              \
        switch (size) {                                             \
        case 1:                                                     \
            *(__u8 *) res = *(volatile __u8 *) p;                   \
            break;                                                  \
        case 2:                                                     \
            *(__u16 *) res = *(volatile __u16 *) p;                 \
            break;                                                  \
        case 4:                                                     \
            *(__u32 *) res = *(volatile __u32 *) p;                 \
            break;                                                  \
        case 8:                                                     \
            *(__u64 *) res = *(volatile __u64 *) p;                 \
            break;                                                  \
        default:                                                    \
            barrier();                                              \
            __builtin_memcpy((void *) res, (const void *) p, size); \
            barrier();                                              \
        }                                                           \
        typeof(x) *__xp = &(x);                                     \
        kcsan_check_atomic_read(__xp, sizeof(*__xp));               \
        __kcsan_disable_current();                                  \
        ({                                                          \
            typeof(x) __x = __READ_ONCE(*__xp);                     \
            __kcsan_enable_current();                               \
            smp_read_barrier_depends();                             \
            __x;                                                    \
        });                                                         \
    })

#define WRITE_ONCE(x, val)                             \
    do {                                               \
        typeof(x) *__xp = &(x);                        \
        kcsan_check_atomic_write(__xp, sizeof(*__xp)); \
        __kcsan_disable_current();                     \
        *(volatile typeof(x) *) __xp = (val);          \
        __kcsan_enable_current();                      \
    } while (0)
#define HLIST_HEAD_INIT \
    {                   \
        .first = NULL   \
    }
#define HLIST_HEAD(name) struct hlist_head name = {.first = NULL}
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
    h->next = NULL;
    h->pprev = NULL;
}

/**
 * hlist_unhashed - Has node been removed from list and reinitialized?
 * @h: Node to be checked
 *
 * Not that not all removal functions will leave a node in unhashed
 * state.  For example, hlist_nulls_del_init_rcu() does leave the
 * node in unhashed state, but hlist_nulls_del() does not.
 */
static inline int hlist_unhashed(const struct hlist_node *h)
{
    return !h->pprev;
}


/**
 * hlist_add_head - add a new entry at the beginning of the hlist
 * @n: new entry to be added
 * @h: hlist head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
    struct hlist_node *first = h->first;
    n->next = first;
    if (first)
        first->pprev = &n->next;
    h->first = n;
    n->pprev = &h->first;
    // h->first->next = n;
    // n->next = first;
}

/**
 * hlist_add_before - add a new entry before the one specified
 * @n: new entry to be added
 * @next: hlist node to add it before, which must be non-NULL
 */
static inline void hlist_add_before(struct hlist_node *n,
                                    struct hlist_node *next)
{
}

/**
 * hlist_add_behind - add a new entry after the one specified
 * @n: new entry to be added
 * @prev: hlist node to add it after, which must be non-NULL
 */
static inline void hlist_add_behind(struct hlist_node *n,
                                    struct hlist_node *prev)
{
}

/**
 * hlist_add_fake - create a fake hlist consisting of a single headless node
 * @n: Node to make a fake list out of
 *
 * This makes @n appear to be its own predecessor on a headless hlist.
 * The point of this is to allow things like hlist_del() to work correctly
 * in cases where there is no list.
 */
static inline void hlist_add_fake(struct hlist_node *n)
{
    n->pprev = &n->next;
}

/**
 * hlist_fake: Is this node a fake hlist?
 * @h: Node to check for being a self-referential fake hlist.
 */
static inline bool hlist_fake(struct hlist_node *h)
{
    return h->pprev == &h->next;
}

/**
 * hlist_is_singular_node - is node the only element of the specified hlist?
 * @n: Node to check for singularity.
 * @h: Header for potentially singular list.
 *
 * Check whether the node is the only node of the head without
 * accessing head, thus avoiding unnecessary cache misses.
 */
static inline bool hlist_is_singular_node(struct hlist_node *n,
                                          struct hlist_head *h)
{
    return !n->next && n->pprev == &h->first;
}

/**
 * hlist_move_list - Move an hlist
 * @old: hlist_head for old list.
 * @new: hlist_head for new list.
 *
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlist_move_list(struct hlist_head *old,
                                   struct hlist_head *new)
{
    new->first = old->first;
    if (new->first)
        new->first->pprev = &new->first;
    old->first = NULL;
}

#define hlist_entry(ptr, type, member) container_of(ptr, type, member)

#define hlist_for_each(pos, head) \
    for (pos = (head)->first; pos; pos = pos->next)

#define hlist_for_each_safe(pos, n, head)        \
    for (pos = (head)->first; pos && ({          \
                                  n = pos->next; \
                                  1;             \
                              });                \
         pos = n)

#define hlist_entry_safe(ptr, type, member)                  \
    ({                                                       \
        typeof(ptr) ____ptr = (ptr);                         \
        ____ptr ? hlist_entry(____ptr, type, member) : NULL; \
    })

/**
 * hlist_for_each_entry	- iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry(pos, head, member)                              \
    for (pos = hlist_entry_safe((head)->first, typeof(*(pos)), member); pos; \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_continue - iterate over a hlist continuing after current
 * point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_continue(pos, member)                           \
    for (pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member); \
         pos;                                                                \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_from - iterate over a hlist continuing from current
 * point
 * @pos:	the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_from(pos, member) \
    for (; pos;                                \
         pos = hlist_entry_safe((pos)->member.next, typeof(*(pos)), member))

/**
 * hlist_for_each_entry_safe - iterate over list of given type safe against
 * removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		a &struct hlist_node to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlist_node within the struct.
 */
#define hlist_for_each_entry_safe(pos, n, head, member)               \
    for (pos = hlist_entry_safe((head)->first, typeof(*pos), member); \
         pos && ({                                                    \
             n = pos->member.next;                                    \
             1;                                                       \
         });                                                          \
         pos = hlist_entry_safe(n, typeof(*pos), member))

#endif