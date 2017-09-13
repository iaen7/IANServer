#ifndef _IAN_LIST_H
#define _IAN_LIST_H

#ifndef NULL
#define NULL 0
#endif

typedef struct list_head{
    struct list_head *prev, *next;
} list_head_t;

//初始化链表
#define INIT_LIST_HEAD(ptr) do{\
    list_head_t *_ptr = (list_head_t *)ptr;        \
    (_ptr)->next = (_ptr); (_ptr->prev) = (_ptr);   \
}while(0)

//获取MEMBER在结构体中的位置
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                  \
    const typeof( ((type *)0)-> member) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type, member) );    \
})

//计算结构体首地址
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

//后向遍历
#define list_of_each(pos, head) \
    for(pos = (head)->next; pos!=(head); pos=pos->next)

//前向遍历
#define list_of_each_prev(pos, head)\
    for(pos = (head)->prev; pos!=(head); pos=pos->prev)

//插入新节点
static inline void __list_add(list_head_t *_new, list_head_t *prev, list_head_t *next){
    _new->next = next;
    next->prev = _new;
    prev->next = _new;
    _new->prev = prev;
}

//头部之后新增
static inline void list_add(list_head_t *_new, list_head_t *head){
    __list_add(_new, head, head->next);
}

//头部之前新增
static inline void list_add_prev(list_head_t *_new, list_head_t *head){
    __list_add(_new, head->prev, head);
}

//删除prev与next之间的节点
static inline void __list_del(list_head_t *prev, list_head_t *next){
    prev->next = next;
    next->prev = prev;
}

//删除entry节点
static inline void list_del(list_head_t *entry){
    __list_del(entry->prev, entry->next);
}

//判断链表是否为空
static inline int list_empty(list_head_t *head){
    return (head->next == head) && (head->prev == head);
}

#endif