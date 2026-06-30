/**
 * @file    course_capacity.h
 * @brief   课程容量哈希表（拉链法）结构与接口声明。
 */

#ifndef COURSE_CAPACITY_H
#define COURSE_CAPACITY_H

#define MAX_COURSE_ID 9
#define CAPACITY_TABLE_SIZE 1009

typedef struct CapacityNode {
    char courseId[MAX_COURSE_ID];
    int capacity;
    int enrolled;
    struct CapacityNode *next;
} CapacityNode;

typedef struct {
    CapacityNode *table[CAPACITY_TABLE_SIZE];
    int size;
} CapacityTable;

void InitCapacityTable(CapacityTable *ct);
int  CapacityHash(const char *courseId);
CapacityNode* FindCapacity(CapacityTable *ct, const char *courseId);
int  SetCourseCapacity(CapacityTable *ct, const char *courseId, int capacity);
int  GetCourseCapacity(CapacityTable *ct, const char *courseId);
int  GetCourseEnrolled(CapacityTable *ct, const char *courseId);
int  AddEnrollment(CapacityTable *ct, const char *courseId);
int  RemoveEnrollment(CapacityTable *ct, const char *courseId);
int  IsCourseFull(CapacityTable *ct, const char *courseId);
void PrintCapacityInfo(CapacityTable *ct);
void DestroyCapacityTable(CapacityTable *ct);
int  SaveCapacityToFile(CapacityTable *ct, const char *filename);
int  LoadCapacityFromFile(CapacityTable *ct, const char *filename);

#endif
