/**
 * @file    record.c
 * @brief   记录基础操作的实现（输入、输出、生成测试数据、文件存储）。
 */

#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief  去掉字符串末尾的换行符。
 * @param  str 目标字符串。
 */
void trim_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}

void copy_record(Record *dest, const Record *src) {
    *dest = *src;
}

/**
 * @brief  交互式输入选课记录。
 * @param  r 指向待填充 Record 的指针。
 * @return 1成功，0取消。
 */
int input_record(Record *r) {
    char buf[100];
    int year, semester_type;
    
    printf("学号 (12位): ");
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);
    if (strlen(buf) == 0) return 0;
    strcpy(r->stuId, buf);
    
    printf("姓名: ");
    fgets(r->stuName, sizeof(r->stuName), stdin);
    trim_newline(r->stuName);
    
    printf("学院: ");
    fgets(r->college, sizeof(r->college), stdin);
    trim_newline(r->college);
    
    printf("课程编号 (8位): ");
    fgets(r->courseId, sizeof(r->courseId), stdin);
    trim_newline(r->courseId);
    
    printf("课程名称: ");
    fgets(r->courseName, sizeof(r->courseName), stdin);
    trim_newline(r->courseName);
    
    printf("学分: ");
    fgets(buf, sizeof(buf), stdin);
    r->credit = atof(buf);
    
    /* =========================================================== */
    /* 修改点 1：手动录入时，将学期改为 "某年上" 或 "某年下" 的形式 */
    /* =========================================================== */
    printf("选课年份 (例如 2025): ");
    fgets(buf, sizeof(buf), stdin);
    year = atoi(buf);

    printf("请选择本学期学期 (1. 上学期(春季)  2. 下学期(秋季)): ");
    fgets(buf, sizeof(buf), stdin);
    semester_type = atoi(buf);

    if (semester_type == 1) {
        sprintf(r->term, "%d上", year);
    } else {
        sprintf(r->term, "%d下", year);
    }
    /* =========================================================== */
    
    printf("选课日期 (YYYY-MM-DD): \n");
    while (getchar() != '\n'); 
    fgets(r->selectDate, sizeof(r->selectDate), stdin);
    trim_newline(r->selectDate);

    printf("成绩 (0-100): ");
    fgets(buf, sizeof(buf), stdin);
    r->score = atoi(buf);
    
    if (r->score < 0 || r->score > 100) r->score = 0;
    return 1;
}

void print_record(const Record *r) {
    printf("%-12s %-8s %-20s %-8s %-20s %4.1f %-8s %-10s %3d\n",
           r->stuId, r->stuName, r->college, r->courseId, r->courseName,
           r->credit, r->term, r->selectDate, r->score);
}

void print_record_header(void) {
    printf("--------------------------------------------------------------------------------------------------------------\n");
    printf("%-12s %-8s %-20s %-8s %-20s %4s %-8s %-10s %3s\n",
           "学号", "姓名", "学院", "课程号", "课程名", "学分", "学期", "日期", "成绩");
    printf("--------------------------------------------------------------------------------------------------------------\n");
}

void print_records(Record *arr, int n, int max_show) {
    int i;
    int show_count;
    if (n == 0) {
        printf(">> 没有记录可显示。\n");
        return;
    }
    print_record_header();
    show_count = (n > max_show) ? max_show : n;
    for (i = 0; i < show_count; i++) {
        print_record(&arr[i]);
    }
    if (n > max_show) {
        printf("... 还有 %d 条记录未显示（共 %d 条）\n", n - max_show, n);
    }
    printf("--------------------------------------------------------------------------------------------------------------\n");
    printf(">> 共 %d 条记录\n", n);
}

int date_compare(const char *d1, const char *d2) {
    return strcmp(d1, d2);
}

void generate_records(Record *arr, int n) {
    const char *colleges[] = {"计算机科学与工程学院", "电子信息工程学院", 
                              "数学与统计学院", "外国语学院", 
                              "经济管理学院", "土木工程学院"};
    const char *courses[] = {"数据结构与算法", "操作系统", "计算机网络", 
                             "数据库系统", "软件工程", "算法设计", 
                             "人工智能", "机器学习"};
    const char *course_ids[] = {"CS0001", "CS0002", "CS0003", "CS0004", 
                                "CS0005", "CS0006", "CS0007", "CS0008",
                                "CS0009", "CS0010"};
    
    const char *surnames[] = {"赵", "钱", "孙", "李", "周", "吴", "郑", "王", "冯", "陈", 
                              "褚", "卫", "蒋", "沈", "韩", "杨", "朱", "秦", "尤", "许", 
                              "何", "吕", "施", "张", "孔", "曹", "严", "华", "金", "魏", 
                              "陶", "姜", "戚", "谢", "邹", "喻", "柏", "水", "窦", "章", 
                              "云", "苏", "潘", "葛", "奚", "范", "彭", "鲁", "韦", "昌", 
                              "马", "苗", "凤", "花", "方", "俞", "任", "袁", "柳", "酆"};
    
    const char *given_names[] = {"伟", "芳", "娜", "秀英", "敏", "静", "丽", "强", "磊", "洋", 
                                 "艳", "勇", "军", "杰", "娟", "涛", "明", "超", "秀兰", "霞", 
                                 "平", "刚", "桂英", "文", "华", "飞", "玉兰", "斌", "宇", "鑫", 
                                 "浩", "然", "博", "晨", "曦", "睿", "智", "天", "逸", "翰", 
                                 "子", "涵", "梓", "轩", "雨", "桐", "语", "彤", "瑾", "瑜"};

    int i;
    int course_idx;
    int year, sem, m, d;
    int surname_idx, name1_idx, name2_idx;
    char name_buf[20];
    
    srand((unsigned)time(NULL));
    
    for (i = 0; i < n; i++) {
        sprintf(arr[i].stuId, "2024%04d", i + 1);

        surname_idx = rand() % (sizeof(surnames)/sizeof(surnames[0]));
        name1_idx = rand() % (sizeof(given_names)/sizeof(given_names[0]));
        if (rand() % 10 < 3) {
            sprintf(name_buf, "%s%s", surnames[surname_idx], given_names[name1_idx]);
        } else {
            do {
                name2_idx = rand() % (sizeof(given_names)/sizeof(given_names[0]));
            } while (name2_idx == name1_idx);
            sprintf(name_buf, "%s%s%s", surnames[surname_idx], given_names[name1_idx], given_names[name2_idx]);
        }
        strcpy(arr[i].stuName, name_buf);

        strcpy(arr[i].college, colleges[rand() % (sizeof(colleges)/sizeof(colleges[0]))]);
        course_idx = rand() % (sizeof(course_ids)/sizeof(course_ids[0]));
        strcpy(arr[i].courseId, course_ids[course_idx]);
        strcpy(arr[i].courseName, courses[course_idx % (sizeof(courses)/sizeof(courses[0]))]);
        arr[i].credit = 2.0 + (rand() % 30) / 10.0;
        
        /* =========================================================== */
        /* 修改点 2：自动生成测试数据时，改为 "某年上" 或 "某年下" 的形式 */
        /* =========================================================== */
        year = 2024 + rand() % 3;
        sem = (rand() % 2) + 1; /* 1或2 */
        if (sem == 1) {
            sprintf(arr[i].term, "%d上", year);
        } else {
            sprintf(arr[i].term, "%d下", year);
        }
        /* =========================================================== */
        
        m = 1 + rand() % 12;
        d = 1 + rand() % 28;
        sprintf(arr[i].selectDate, "%d-%02d-%02d", 2024 + rand() % 3, m, d);
        arr[i].score = rand() % 101;
    }
}

int save_records_to_csv(const char *filename, Record *arr, int n) {
    FILE *fp = fopen(filename, "w");
    int i;
    if (!fp) return -1;
    fprintf(fp, "学号,姓名,学院,课程编号,课程名称,学分,选课学期,选课日期,成绩\n");
    for (i = 0; i < n; i++) {
        /* 
         * 注：在学期前加 \t 是为了防止 Excel/WPS 打开 CSV 时 
         * 自动把 "2025上" 或 "2025下" 这种文本转换成别的格式。
         */
        fprintf(fp, "%s,%s,%s,%s,%s,%.1f,\t%s,%s,%d\n",
                arr[i].stuId, arr[i].stuName, arr[i].college,
                arr[i].courseId, arr[i].courseName, arr[i].credit,
                arr[i].term, arr[i].selectDate, arr[i].score);
    }
    fclose(fp);
    return 0;
}
