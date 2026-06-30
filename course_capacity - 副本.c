/**
 * @file    course_capacity.c
 * @brief   课程容量哈希表功能的实现，利用拉链法解决哈希冲突。
 */

#include "course_capacity.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_CAPACITY 60

void InitCapacityTable(CapacityTable *ct) {
    int i;
    for (i = 0; i < CAPACITY_TABLE_SIZE; i++) {
        ct->table[i] = NULL;
    }
    ct->size = 0;
}

int CapacityHash(const char *courseId) {
    unsigned int hash = 0;
    while (*courseId) {
        hash = (hash << 5) + *courseId++;
    }
    return hash % CAPACITY_TABLE_SIZE;
}

CapacityNode* FindCapacity(CapacityTable *ct, const char *courseId) {
    int idx = CapacityHash(courseId);
    CapacityNode *p = ct->table[idx];
    while (p) {
        if (strcmp(p->courseId, courseId) == 0) return p;
        p = p->next;
    }
    return NULL;
}

int SetCourseCapacity(CapacityTable *ct, const char *courseId, int capacity) {
    int idx;
    CapacityNode *found;
    CapacityNode *node;
    if (capacity <= 0) return -1;
    found = FindCapacity(ct, courseId);
    if (found) {
        found->capacity = capacity;
        return 0;
    }
    idx = CapacityHash(courseId);
    node = (CapacityNode*)malloc(sizeof(CapacityNode));
    if (!node) return -1;
    strcpy(node->courseId, courseId);
    node->capacity = capacity;
    node->enrolled = 0;
    node->next = ct->table[idx];
    ct->table[idx] = node;
    ct->size++;
    return 0;
}

int GetCourseCapacity(CapacityTable *ct, const char *courseId) {
    CapacityNode *found = FindCapacity(ct, courseId);
    return found ? found->capacity : -1;
}

int GetCourseEnrolled(CapacityTable *ct, const char *courseId) {
    CapacityNode *found = FindCapacity(ct, courseId);
    return found ? found->enrolled : -1;
}

int AddEnrollment(CapacityTable *ct, const char *courseId) {
    CapacityNode *found = FindCapacity(ct, courseId);
    if (!found) {
        SetCourseCapacity(ct, courseId, DEFAULT_CAPACITY);
        found = FindCapacity(ct, courseId);
    }
    if (found->enrolled >= found->capacity) return 0;
    found->enrolled++;
    return 1;
}

int RemoveEnrollment(CapacityTable *ct, const char *courseId) {
    CapacityNode *found = FindCapacity(ct, courseId);
    if (!found) return -1;
    if (found->enrolled > 0) found->enrolled--;
    return 0;
}

int IsCourseFull(CapacityTable *ct, const char *courseId) {
    CapacityNode *found = FindCapacity(ct, courseId);
    if (!found) return 0;
    return found->enrolled >= found->capacity;
}

void PrintCapacityInfo(CapacityTable *ct) {
    int i;
    CapacityNode *p;
    printf("\n========== 课程容量使用情况 ==========\n");
    printf("%-10s %-8s %-8s %-8s\n", "课程号", "容量", "已选", "剩余");
    printf("----------------------------------------\n");
    for (i = 0; i < CAPACITY_TABLE_SIZE; i++) {
        p = ct->table[i];
        while (p) {
            printf("%-10s %-8d %-8d %-8d\n", 
                   p->courseId, p->capacity, p->enrolled, p->capacity - p->enrolled);
            p = p->next;
        }
    }
}

void DestroyCapacityTable(CapacityTable *ct) {
    int i;
    for (i = 0; i < CAPACITY_TABLE_SIZE; i++) {
        CapacityNode *p = ct->table[i];
        while (p) {
            CapacityNode *tmp = p;
            p = p->next;
            free(tmp);
        }
        ct->table[i] = NULL;
    }
    ct->size = 0;
}

int SaveCapacityToFile(CapacityTable *ct, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    int n = ct->size;
    int i;
    CapacityNode *p;
    if (!fp) return -1;
    fwrite(&n, sizeof(int), 1, fp);
    for (i = 0; i < CAPACITY_TABLE_SIZE; i++) {
        p = ct->table[i];
        while (p) {
            fwrite(p, sizeof(CapacityNode), 1, fp);
            p = p->next;
        }
    }
    fclose(fp);
    return 0;
}

int LoadCapacityFromFile(CapacityTable *ct, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    int n;
    int i;
    CapacityNode node;
    CapacityNode *found;
    if (!fp) return -1;
    if (fread(&n, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }
    for (i = 0; i < n; i++) {
        if (fread(&node, sizeof(CapacityNode), 1, fp) != 1) break;
        SetCourseCapacity(ct, node.courseId, node.capacity);
        found = FindCapacity(ct, node.courseId);
        if (found) found->enrolled = node.enrolled;
    }
    fclose(fp);
    return 0;
}
