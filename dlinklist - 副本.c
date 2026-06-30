/**
 * @file    dlinklist.c
 * @brief   双向链表的操作实现。
 */

#include "dlinklist.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief 初始化双向链表
 * @param list 目标链表指针
 */
void InitList(DLinkList *list) {
    list->head = list->tail = NULL;
    list->size = 0;
}

/**
 * @brief 在链表尾部插入一条记录
 * @param list 目标链表指针
 * @param r 待插入记录指针
 * @return 成功返回0，内存分配失败返回-1
 */
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

/**
 * @brief 从链表中删除指定记录（按学号+课程号）
 * @param list 目标链表指针
 * @param stuId 待删除记录的学号
 * @param courseId 待删除记录的课程编号
 * @return 成功返回0，记录不存在返回-1
 */
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

/**
 * @brief 修改链表中指定记录的成绩
 * @param list 目标链表指针
 * @param stuId 待修改记录的学号
 * @param courseId 待修改记录的课程编号
 * @param newScore 新的成绩
 * @return 成功返回0，记录不存在返回-1
 */
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

/**
 * @brief 按学号在链表中查找记录（返回第一个匹配节点）
 * @param list 目标链表指针
 * @param stuId 待查找的学号
 * @return 找到的节点指针，未找到返回NULL
 */
ListNode* ListSearch(DLinkList *list, const char *stuId) {
    ListNode *p = list->head;
    while (p) {
        if (strcmp(p->data.stuId, stuId) == 0) return p;
        p = p->next;
    }
    return NULL;
}

/**
 * @brief 遍历链表并打印所有记录
 * @param list 目标链表指针
 */
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

/**
 * @brief 销毁链表，释放所有节点内存
 * @param list 目标链表指针
 */
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

/**
 * @brief 获取链表中记录总数
 * @param list 目标链表指针
 * @return 记录总数
 */
int ListSize(DLinkList *list) {
    return list->size;
}

/**
 * @brief 获取链表中所有记录
 * @param list 目标链表指针
 * @param arr 输出参数：记录数组指针
 * @param n 输出参数：记录总数
 */
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
