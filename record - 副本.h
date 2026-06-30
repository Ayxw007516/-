/**
 * @file    record.h
 * @brief   选课记录实体定义与基础操作函数声明。
 *          定义了Record结构体，以及后续用于多关键字排序和多条件筛选的辅助枚举。
 */

#ifndef RECORD_H
#define RECORD_H

#define MAX_STU_ID       13
#define MAX_NAME         10
#define MAX_COLLEGE      35
#define MAX_COURSE_ID    9
#define MAX_COURSE_NAME  35
#define MAX_TERM         7
#define MAX_DATE         11

/* 默认课程容量 */
#define DEFAULT_CAPACITY 60

/* 最大数据规模 */
#define MAX_DATA_SIZE    10000

/**
 * @struct Record
 * @brief  学生选课记录的核心数据实体。
 */
typedef struct Record {
    char stuId[MAX_STU_ID];
    char stuName[MAX_NAME];
    char college[MAX_COLLEGE];
    char courseId[MAX_COURSE_ID];
    char courseName[MAX_COURSE_NAME];
    float credit;
    char term[MAX_TERM];
    char selectDate[MAX_DATE];
    int score;
} Record;

/**
 * @enum SortField
 * @brief 用于多关键字排序的可选字段枚举。
 */
typedef enum {
    SORT_BY_STUDENT_ID = 1,
    SORT_BY_NAME,
    SORT_BY_COURSE,
    SORT_BY_TERM,
    SORT_BY_SCORE,
    SORT_BY_COLLEGE
} SortField;

/**
 * @enum SortOrder
 * @brief 排序方向枚举。
 */
typedef enum {
    ASCENDING = 0,
    DESCENDING = 1
} SortOrder;

/**
 * @struct SortRule
 * @brief 单条排序规则。
 */
typedef struct {
    SortField field;
    SortOrder order;
} SortRule;

/**
 * @struct FilterCondition
 * @brief 多条件筛选条件组合。
 */
typedef struct {
    char courseName[MAX_COURSE_NAME];
    int useNameFilter;
    
    char term[MAX_TERM];
    int useTermFilter;
    
    int scoreMin;
    int scoreMax;
    int useScoreFilter;
    
    char college[MAX_COLLEGE];
    int useCollegeFilter;
} FilterCondition;

/* 记录操作函数 */
int  input_record(Record *r);
void print_record(const Record *r);
void print_record_header(void);
int  date_compare(const char *d1, const char *d2);
void trim_newline(char *str);
void generate_records(Record *arr, int n);
void copy_record(Record *dest, const Record *src);
void print_records(Record *arr, int n, int max_show);
int  save_records_to_csv(const char *filename, Record *arr, int n);

#endif
