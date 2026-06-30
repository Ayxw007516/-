/**
 * @file    dlinklist.h
 * @brief   双向链表的数据结构定义与接口声明。
 */

#ifndef DLINKLIST_H
#define DLINKLIST_H

#include "record.h"

typedef struct ListNode {
    Record data;
    struct ListNode *prev;
    struct ListNode *next;
} ListNode;

typedef struct {
    ListNode *head;
    ListNode *tail;
    int size;
} DLinkList;

void InitList(DLinkList *list);
int  ListInsert(DLinkList *list, const Record *r);
int  ListDelete(DLinkList *list, const char *stuId, const char *courseId);
int  ListUpdate(DLinkList *list, const char *stuId, const char *courseId, int newScore);
ListNode* ListSearch(DLinkList *list, const char *stuId);
void ListTraverse(DLinkList *list);
void DestroyList(DLinkList *list);
int  ListSize(DLinkList *list);
void ListGetAll(DLinkList *list, Record **arr, int *n);

#endif
