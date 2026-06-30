/**
 * @file    dlinklist.c
 * @brief   双向链表的操作实现。
 */

#include "dlinklist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void InitList(DLinkList *list) {
    list->head = list->tail = NULL;
    list->size = 0;
}

int ListInsert(DLinkList *list, const Record *r) {
    ListNode *node = (ListNode*)malloc(sizeof(ListNode));
    if (!node) return -1;
    node->data = *r;
    node->prev = list->tail;
    node->next = NULL;
    if (list->tail) list->tail->next = node;
    else list->head = node;
    list->tail = node;
    list->size++;
    return 0;
}

int ListDelete(DLinkList *list, const char *stuId, const char *courseId) {
    ListNode *p = list->head;
    while (p) {
        if (strcmp(p->data.stuId, stuId) == 0 &&
            strcmp(p->data.courseId, courseId) == 0) {
            if (p->prev) p->prev->next = p->next;
            else list->head = p->next;
            if (p->next) p->next->prev = p->prev;
            else list->tail = p->prev;
            free(p);
            list->size--;
            return 0;
        }
        p = p->next;
    }
    return -1;
}

int ListUpdate(DLinkList *list, const char *stuId, const char *courseId, int newScore) {
    ListNode *p = list->head;
    while (p) {
        if (strcmp(p->data.stuId, stuId) == 0 && strcmp(p->data.courseId, courseId) == 0) {
            p->data.score = newScore;
            return 0;
        }
        p = p->next;
    }
    return -1;
}

ListNode* ListSearch(DLinkList *list, const char *stuId) {
    ListNode *p = list->head;
    while (p) {
        if (strcmp(p->data.stuId, stuId) == 0) return p;
        p = p->next;
    }
    return NULL;
}

void ListTraverse(DLinkList *list) {
    ListNode *p = list->head;
    if (list->size == 0) {
        printf(">> 链表为空。\n");
        return;
    }
    print_record_header();
    while (p) {
        print_record(&p->data);
        p = p->next;
    }
    printf(">> 共 %d 条记录。\n", list->size);
}

void DestroyList(DLinkList *list) {
    ListNode *p = list->head;
    while (p) {
        ListNode *tmp = p;
        p = p->next;
        free(tmp);
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

int ListSize(DLinkList *list) {
    return list->size;
}

void ListGetAll(DLinkList *list, Record **arr, int *n) {
    *n = list->size;
    *arr = (Record*)malloc(*n * sizeof(Record));
    if (!*arr) { *n = 0; return; }
    ListNode *p = list->head;
    int idx = 0;
    while (p) {
        (*arr)[idx++] = p->data;
        p = p->next;
    }
}
