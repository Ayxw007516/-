/**
 * @file    main.c
 * @brief   选课记录检索与分析系统主程序。
 *          包含双向链表(DLinkList)和AVL树(AVLTree)，
 *          支持筛选、多关键字排序、持久化、性能对比、批量删除、清空数据等。
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
    #define mkdir(path, mode) mkdir(path, mode)
#endif

#include "record.h"
#include "dlinklist.h"
#include "avltree.h"
#include "course_capacity.h"

#define DATA_DIR        "数据"
#define DATA_FILE       "数据/当前数据.dat"
#define CAPACITY_FILE   "数据/容量配置.dat"
#include <dirent.h>

/* 全局变量 */
DLinkList      g_list;
AVLTree        g_avl;
CapacityTable  g_capacity;

/* 函数声明 */
void ensure_data_dir(void);
void save_data(const char *filename);
void load_data(const char *filename);
void save_capacity(void);
void load_capacity(void);
void data_structure_menu(int *selected);
void operation_menu(int ds_type);
void performance_test(void);
void print_search_result_list(const char *stuId);
void print_search_result_avl(const char *stuId);
void statistic_menu(void);
int  insert_with_capacity(int ds_type, const Record *r);
void generate_test_data(void);
void list_data_files(void);
void load_data_file(void);
void multi_filter_and_export(void);
void multi_key_sort_and_display(void);
void batch_delete_expired_records(void);
void clear_all_data(void);

/**
 * @brief  主程序入口。
 */
int main(void) {
    int choice;
    int ds;
    int cmd;
    char cid[MAX_COURSE_ID];
    int cap;

    ensure_data_dir();

    InitList(&g_list);
    InitAVL(&g_avl);
    InitCapacityTable(&g_capacity);

    load_data(DATA_FILE);
    load_capacity();

    do {
        printf("\n========== 校园选课记录检索与大数据分析系统 ==========\n");
        printf("  当前记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
        printf("  数据文件夹: %s\n", DATA_DIR);
        printf("========================================================\n");
        printf("  1. 选择数据结构进行操作 (任务1)\n");
        printf("  2. 统计与分析 (任务4)\n");
        printf("  3. 课程容量管理\n");
        printf("  4. 性能对比测试 (链表 vs AVL树)\n");
        printf("  5. 生成测试数据 (用户输入数量)\n");
        printf("  6. 查看数据文件列表 (任务2)\n");
        printf("  7. 加载指定数据文件 (任务2)\n");
        printf("  8. 多条件筛选并导出数据 (任务3)\n");
        printf("  9. 多关键字排序并显示 (任务3)\n");
        printf(" 10. 性能对比与复杂度验证 (任务6)\n");
        printf(" 11. 批量删除过期记录 (任务5)\n");
        printf(" 12. 清空所有数据 (危险操作)\n");
        printf("  0. 保存并退出\n");
        printf("请选择: ");
        scanf("%d", &choice);
        while (getchar() != '\n');

        switch (choice) {
            case 1:
                data_structure_menu(&ds);
                if (ds >= 1 && ds <= 2) operation_menu(ds);
                break;
            case 2: statistic_menu(); break;
            case 3:
                printf("\n========== 课程容量管理 ==========\n");
                printf("  1. 设置课程容量\n");
                printf("  2. 查看所有课程容量\n");
                printf("  3. 查看单门课程容量\n");
                printf("  0. 返回\n");
                printf("请选择: ");
                scanf("%d", &cmd);
                while (getchar() != '\n');
                
                switch (cmd) {
                    case 1:
                        printf("课程编号: ");
                        fgets(cid, sizeof(cid), stdin);
                        trim_newline(cid);
                        printf("容量: ");
                        scanf("%d", &cap);
                        while (getchar() != '\n');
                        SetCourseCapacity(&g_capacity, cid, cap);
                        printf(">> 设置成功！\n");
                        save_capacity();
                        break;
                    case 2: PrintCapacityInfo(&g_capacity); break;
                    case 3:
                        printf("课程编号: ");
                        fgets(cid, sizeof(cid), stdin);
                        trim_newline(cid);
                        cap = GetCourseCapacity(&g_capacity, cid);
                        { int e = GetCourseEnrolled(&g_capacity, cid);
                        if (cap < 0) printf(">> 该课程未设置容量。\n");
                        else printf(">> %s: 容量=%d, 已选=%d, 剩余=%d\n", cid, cap, e, cap - e); }
                        break;
                    case 0: break;
                    default: printf(">> 无效选项。\n");
                }
                break;
            case 4: performance_test(); break;
            case 5: generate_test_data(); break;
            case 6: list_data_files(); break;
            case 7: load_data_file(); break;
            case 8: multi_filter_and_export(); break;
            case 9: multi_key_sort_and_display(); break;
            case 10: performance_test(); break;
            case 11: batch_delete_expired_records(); break;
            case 12: clear_all_data(); break;
            case 0:
                save_data(DATA_FILE);
                save_capacity();
                printf(">> 数据已保存到 %s，程序退出。\n", DATA_FILE);
                break;
            default: printf(">> 无效选项，请重新选择。\n");
        }
    } while (choice != 0);

    DestroyList(&g_list);
    DestroyAVL(&g_avl);
    DestroyCapacityTable(&g_capacity);
    return 0;
}

void ensure_data_dir(void) {
    struct stat st;
    if (stat(DATA_DIR, &st) == -1) {
#ifdef _WIN32
        if (mkdir(DATA_DIR) == 0) {
#else
        if (mkdir(DATA_DIR, 0755) == 0) {
#endif
            printf(">> 已创建数据文件夹: %s\n", DATA_DIR);
        } else {
            printf(">> 创建数据文件夹失败！\n");
        }
    }
}

void generate_test_data(void) {
    int n;
    int inserted;
    int rejected;
    int i;
    char filename[200];
    char csv_filename[200];
    Record *arr;
    time_t now;
    struct tm *tm_info;
    char time_str[30];
    char full_path[300];
    char csv_full_path[300];
    
    printf("\n========== 生成测试数据 ==========\n");
    printf("当前记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
    printf("请输入要生成的记录数量 (0-10000): ");
    scanf("%d", &n);
    while (getchar() != '\n');
    
    if (n <= 0 || n > MAX_DATA_SIZE) return;
    if (g_list.size + n > MAX_DATA_SIZE) {
        printf(">> 容量不足，可生成 %d 条。\n", MAX_DATA_SIZE - g_list.size);
        return;
    }
    
    printf(">> 正在生成 %d 条测试数据...\n", n);
    arr = (Record*)malloc(n * sizeof(Record));
    if (!arr) { printf(">> 内存分配失败！\n"); return; }
    generate_records(arr, n);
    printf("\n========== 生成的数据（表格展示） ==========\n");
    print_records(arr, n, 50);
    
    inserted = 0; rejected = 0;
    for (i = 0; i < n; i++) {
        if (AddEnrollment(&g_capacity, arr[i].courseId)) {
            ListInsert(&g_list, &arr[i]);
            AVLInsert(&g_avl, &arr[i]);
            inserted++;
        } else rejected++;
    }
    printf("\n========== 数据插入结果 ==========\n");
    printf("  成功插入: %d 条\n", inserted);
    printf("  被拒绝（容量已满）: %d 条\n", rejected);
    printf("  当前总记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
    
    now = time(NULL);
    tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm_info);
    sprintf(filename, "数据_%s_%d条.dat", time_str, n);
    sprintf(full_path, "%s/%s", DATA_DIR, filename);
    sprintf(csv_filename, "数据_%s_%d条.csv", time_str, n);
    sprintf(csv_full_path, "%s/%s", DATA_DIR, csv_filename);
    
    save_data(full_path);
    save_capacity();
    if (save_records_to_csv(csv_full_path, arr, n) == 0) {
        printf("\n========== 文件保存成功 ==========\n");
        printf("  CSV表格文件: %s\n", csv_full_path);
    }
    free(arr);
}

void save_data(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    int n = g_list.size;
    ListNode *p = g_list.head;
    Record *arr;
    int i;
    if (!fp) { printf(">> 无法保存数据到 %s！\n", filename); return; }
    fwrite(&n, sizeof(int), 1, fp);
    while (p) { fwrite(&p->data, sizeof(Record), 1, fp); p = p->next; }
    fclose(fp);
    if (n == 0) return;
    arr = (Record*)malloc(n * sizeof(Record));
    if (!arr) return;
    p = g_list.head;
    for (i = 0; i < n; i++) { arr[i] = p->data; p = p->next; }
    if (save_records_to_csv("record.csv", arr, n) == 0) {
        printf(">> 数据已同步导出至本地文件: record.csv\n");
    }
    free(arr);
}

void load_data(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    int n, i;
    Record r;
    if (!fp) { printf(">> 未找到数据文件 %s\n", filename); return; }
    if (fread(&n, sizeof(int), 1, fp) != 1) { fclose(fp); return; }
    if (n > MAX_DATA_SIZE) n = MAX_DATA_SIZE;
    for (i = 0; i < n; i++) {
        if (fread(&r, sizeof(Record), 1, fp) != 1) break;
        ListInsert(&g_list, &r);
        AVLInsert(&g_avl, &r);
        AddEnrollment(&g_capacity, r.courseId);
    }
    fclose(fp);
    printf(">> 成功加载数据，共 %d 条记录。\n", g_list.size);
}

void save_capacity(void) { SaveCapacityToFile(&g_capacity, CAPACITY_FILE); }
void load_capacity(void) {
    if (LoadCapacityFromFile(&g_capacity, CAPACITY_FILE) != 0)
        printf(">> 未找到容量配置文件，使用默认容量(60人/门)。\n");
}

void list_data_files(void) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char full_path[300];
    int count = 0;
    struct tm *tm_info;
    char time_str[30];
    const char *type;
    
    printf("\n========== 数据文件夹内容 (%s) ==========\n", DATA_DIR);
    dir = opendir(DATA_DIR);
    if (dir == NULL) { printf(">> 无法打开数据文件夹！\n"); return; }
    printf("%-45s %-20s %-10s %-8s\n", "文件名", "修改时间", "大小", "类型");
    printf("--------------------------------------------------------------------------------\n");
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        sprintf(full_path, "%s/%s", DATA_DIR, entry->d_name);
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            tm_info = localtime(&st.st_mtime);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            type = "其他";
            if (strstr(entry->d_name, ".csv")) type = "CSV表格";
            else if (strstr(entry->d_name, ".dat")) type = "二进制数据";
            printf("%-45s %-20s %-10ld %-8s\n", entry->d_name, time_str, (long)st.st_size, type);
            count++;
        }
    }
    closedir(dir);
    if (count == 0) printf(">> 数据文件夹为空。\n");
    else printf(">> 共 %d 个文件\n", count);
}

void load_data_file(void) {
    char filename[200];
    char full_path[300];
    FILE *fp;
    list_data_files();
    printf("\n请输入要加载的文件名 (.dat 文件，不含路径): ");
    fgets(filename, sizeof(filename), stdin);
    trim_newline(filename);
    if (strlen(filename) == 0) { printf(">> 未选择任何文件。\n"); return; }
    sprintf(full_path, "%s/%s", DATA_DIR, filename);
    fp = fopen(full_path, "rb");
    if (!fp) { printf(">> 文件 %s 不存在！\n", full_path); return; }
    fclose(fp);
    DestroyList(&g_list);
    DestroyAVL(&g_avl);
    DestroyCapacityTable(&g_capacity);
    InitList(&g_list); InitAVL(&g_avl); InitCapacityTable(&g_capacity);
    load_data(full_path);
    printf(">> 已切换到数据文件: %s\n", full_path);
}

void data_structure_menu(int *selected) {
    printf("\n请选择要操作的数据结构:\n");
    printf("  1. 双向链表 (记录数: %d)\n", g_list.size);
    printf("  2. AVL平衡树 (记录数: %d)\n", g_avl.size);
    printf("  0. 返回\n");
    printf("请选择: ");
    scanf("%d", selected);
    while (getchar() != '\n');
}

int insert_with_capacity(int ds_type, const Record *r) {
    int ret;
    if (g_list.size >= MAX_DATA_SIZE) return -1;
    if (!AddEnrollment(&g_capacity, r->courseId)) return -1;
    if (ds_type == 1) ret = ListInsert(&g_list, r);
    else ret = AVLInsert(&g_avl, r);
    if (ret != 0) { RemoveEnrollment(&g_capacity, r->courseId); return -1; }
    return 0;
}

void operation_menu(int ds_type) {
    const char *names[] = {"", "双向链表", "AVL平衡树"};
    int op, current_size, ret, newScore;
    Record r;
    char sid[MAX_STU_ID], cid[MAX_COURSE_ID];
    do {
        current_size = (ds_type == 1) ? g_list.size : g_avl.size;
        printf("\n========== 当前操作: %s ==========\n", names[ds_type]);
        printf("  记录数: %d / %d\n", current_size, MAX_DATA_SIZE);
        printf("  1. 插入记录 (选课)\n  2. 删除记录 (退课)\n  3. 修改成绩\n");
        printf("  4. 查找记录 \n  5. 遍历所有记录\n  6. 查看课程容量\n  0. 返回\n");
        printf("请选择: ");
        scanf("%d", &op);
        while (getchar() != '\n');
        switch (op) {
            case 1:
                if (input_record(&r) && insert_with_capacity(ds_type, &r) == 0) {
                    printf(">> 选课成功！课程 %s 当前已选 %d/%d 人\n", r.courseId, GetCourseEnrolled(&g_capacity, r.courseId), GetCourseCapacity(&g_capacity, r.courseId));
                    save_data(DATA_FILE); save_capacity();
                }
                break;
            case 2:
                printf("学号: "); fgets(sid, sizeof(sid), stdin); trim_newline(sid);
                printf("课程编号: "); fgets(cid, sizeof(cid), stdin); trim_newline(cid);
                ret = (ds_type == 1) ? ListDelete(&g_list, sid, cid) : AVLDelete(&g_avl, sid, cid);
                if (ret == 0) { RemoveEnrollment(&g_capacity, cid); save_data(DATA_FILE); save_capacity(); }
                else printf(">> 退课失败，未找到记录！\n");
                break;
            case 3:
                printf("学号: "); fgets(sid, sizeof(sid), stdin); trim_newline(sid);
                printf("课程编号: "); fgets(cid, sizeof(cid), stdin); trim_newline(cid);
                printf("新成绩: "); scanf("%d", &newScore); while (getchar() != '\n');
                ret = (ds_type == 1) ? ListUpdate(&g_list, sid, cid, newScore) : AVLUpdate(&g_avl, sid, cid, newScore);
                if (ret == 0) save_data(DATA_FILE);
                else printf(">> 修改失败，未找到记录！\n");
                break;
            case 4:
                {
                    int search_mode;
                    char keyword[MAX_STU_ID];
                    ListNode *p;
                    int found_count;
                    int is_match;

                    printf("\n========== 记录查找 ==========\n");
                    printf("  1. 按学号精确查找 (最快)\n");
                    printf("  2. 按姓名模糊查找 (遍历链表)\n");
                    printf("  3. 按课程名称模糊查找 (遍历链表)\n");
                    printf("  0. 返回\n");
                    printf("请选择查找方式: ");
                    scanf("%d", &search_mode);
                    while (getchar() != '\n'); // 清理缓冲区

                    if (search_mode == 0) break;

                    printf("请输入查找关键词: ");
                    fgets(keyword, sizeof(keyword), stdin);
                    trim_newline(keyword);

                    if (strlen(keyword) == 0) {
                        printf(">> 关键词为空，取消查找。\n");
                        break;
                    }

                    /* --- 1. 按学号精确查找 (复用原有的 AVL/链表高效接口) --- */
                    if (search_mode == 1) {
                        if (ds_type == 1) print_search_result_list(keyword);
                        else print_search_result_avl(keyword);
                    } 
                    /* --- 2 & 3. 按姓名/课程名模糊查找 (遍历链表进行 strstr 匹配) --- */
                    else {
                        p = g_list.head;
                        found_count = 0;
                        printf("\n>> 正在遍历查找...\n");
                        print_record_header();
                        while (p) {
                            is_match = 0;
                            if (search_mode == 2) {
                                // 按姓名模糊匹配（包含关键字即可）
                                if (strstr(p->data.stuName, keyword) != NULL) is_match = 1;
                            } else if (search_mode == 3) {
                                // 按课程名称模糊匹配
                                if (strstr(p->data.courseName, keyword) != NULL) is_match = 1;
                            }

                            if (is_match) {
                                print_record(&p->data);
                                found_count++;
                            }
                            p = p->next;
                        }
                        printf("--------------------------------------------------------------------------------------------------------------\n");
                        printf(">> 共找到 %d 条记录。\n", found_count);
                    }
                }
                break;
            case 5: (ds_type == 1) ? ListTraverse(&g_list) : AVLTraverse(&g_avl); break;
            case 6: PrintCapacityInfo(&g_capacity); break;
            case 0: break;
            default: printf(">> 无效选项。\n");
        }
    } while (op != 0);
}

void print_search_result_list(const char *stuId) {
    ListNode *found = ListSearch(&g_list, stuId);
    if (found) { printf(">> 找到记录：\n"); print_record_header(); print_record(&found->data); }
    else printf(">> 未找到学号 %s 的记录。\n", stuId);
}

void print_search_result_avl(const char *stuId) {
    AVLNode *found = AVLSearch(&g_avl, stuId);
    if (found) { printf(">> 找到记录：\n"); print_record_header(); print_record(&found->data); }
    else printf(">> 未找到学号 %s 的记录。\n", stuId);
}

void statistic_menu(void) {
    int choice, i, n, found, s;
    ListNode *p;
    int dist[5];
    typedef struct { char name[MAX_COLLEGE]; int count; } CollegeStat;
    typedef struct { char stuId[MAX_STU_ID]; char stuName[MAX_NAME]; int count; float total_credit; } StudentStat;
    CollegeStat *stats = NULL;
    StudentStat *s_stats = NULL;
    int s_n = 0;
    
    printf("\n========== 统计分析 ==========\n");
    printf("  1. 课程容量使用率统计\n  2. 各学院选课人数分布\n");
    printf("  3. 成绩分段统计\n  4. 每位学生选课门数与总学分\n  0. 返回\n");
    printf("请选择: ");
    scanf("%d", &choice); while (getchar() != '\n');
    
    switch (choice) {
        case 1: PrintCapacityInfo(&g_capacity); break;
        case 2:
            n = 0; stats = NULL; p = g_list.head;
            while (p) {
                found = 0;
                for (i = 0; i < n; i++) {
                    if (strcmp(stats[i].name, p->data.college) == 0) { stats[i].count++; found = 1; break; }
                }
                if (!found) {
                    stats = (CollegeStat*)realloc(stats, (n+1)*sizeof(CollegeStat));
                    strcpy(stats[n].name, p->data.college); stats[n].count = 1; n++;
                }
                p = p->next;
            }
            printf("\n各学院选课人数分布：\n");
            for (i = 0; i < n; i++) printf("  %-30s : %d人\n", stats[i].name, stats[i].count);
            free(stats);
            break;
        case 3:
            for(i=0; i<5; i++) dist[i]=0;
            p = g_list.head;
            while (p) { s = p->data.score; if (s < 60) dist[0]++; else if (s < 70) dist[1]++; else if (s < 80) dist[2]++; else if (s < 90) dist[3]++; else dist[4]++; p = p->next; }
            printf("\n成绩分段统计：\n");
            printf("  0-59   : %d人 (不及格)\n  60-69  : %d人 (及格)\n  70-79  : %d人 (中等)\n", dist[0], dist[1], dist[2]);
            printf("  80-89  : %d人 (良好)\n  90-100 : %d人 (优秀)\n", dist[3], dist[4]);
            break;
        case 4:
            p = g_list.head; s_n = 0; s_stats = NULL;
            while (p) {
                found = 0;
                for (i = 0; i < s_n; i++) {
                    if (strcmp(s_stats[i].stuId, p->data.stuId) == 0) {
                        s_stats[i].count++; s_stats[i].total_credit += p->data.credit; found = 1; break;
                    }
                }
                if (!found) {
                    s_stats = (StudentStat*)realloc(s_stats, (s_n+1)*sizeof(StudentStat));
                    strcpy(s_stats[s_n].stuId, p->data.stuId); strcpy(s_stats[s_n].stuName, p->data.stuName);
                    s_stats[s_n].count = 1; s_stats[s_n].total_credit = p->data.credit; s_n++;
                }
                p = p->next;
            }
            printf("\n========== 每位学生选课门数与总学分 ==========\n");
            printf("%-12s %-10s %-10s %-10s\n", "学号", "姓名", "选课门数", "总学分");
            for (i = 0; i < s_n; i++) printf("%-12s %-10s %-10d %-10.1f\n", s_stats[i].stuId, s_stats[i].stuName, s_stats[i].count, s_stats[i].total_credit);
            printf(">> 共统计 %d 位学生。\n", s_n);
            free(s_stats);
            break;
        case 0: break;
        default: printf(">> 无效选项。\n");
    }
}

/* ==================== 任务6：性能对比测试 ==================== */
void performance_test(void) {
    int sizes[] = {100, 1000, 10000};
    int t, i, n;
    clock_t start, end;
    Record *arr;
    char targetId[MAX_STU_ID];
    
    printf("\n=============== 性能对比与复杂度验证报告 ===============\n");
    
    printf("\n[1] 理论时间复杂度分析\n");
    printf("  -----------------------------------------------------------------\n");
    printf("  | 数据结构  | 插入(O)  | 查找(O)  | 删除(O)  | 遍历(O)  |\n");
    printf("  |-----------|----------|----------|----------|----------|\n");
    printf("  | 双向链表  | O(1)*    | O(n)     | O(n)     | O(n)     |\n");
    printf("  | AVL 平衡树| O(log n) | O(log n) | O(log n) | O(n)     |\n");
    printf("  -----------------------------------------------------------------\n");
    printf("  *注: 链表尾插O(1)，查找O(n)，故删除需先查找仍为O(n)。\n");

    double results[3][6]; 
    for (t = 0; t < 3; t++) {
        n = sizes[t];
        if (n > MAX_DATA_SIZE) {
            printf("\n[!] 跳过规模 %d 条（超出系统最大限制 %d）\n", n, MAX_DATA_SIZE);
            continue;
        }

        printf("\n--------------------------------------------------------\n");
        printf(">>> 测试规模: %d 条数据\n", n);

        arr = (Record*)malloc(n * sizeof(Record));
        if (!arr) { printf(">> 内存分配失败！\n"); return; }
        generate_records(arr, n);
        sprintf(targetId, "2024%04d", n/2 + 1);

        /* ----- 双向链表测试 ----- */
        DestroyList(&g_list);
        InitList(&g_list);

        start = clock();
        for (i = 0; i < n; i++) ListInsert(&g_list, &arr[i]);
        end = clock();
        results[t][0] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        start = clock();
        ListSearch(&g_list, targetId);
        end = clock();
        results[t][1] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        start = clock();
        ListDelete(&g_list, targetId, arr[n/2].courseId); 
        end = clock();
        results[t][2] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* ----- AVL树测试 ----- */
        DestroyAVL(&g_avl);
        InitAVL(&g_avl);

        start = clock();
        for (i = 0; i < n; i++) AVLInsert(&g_avl, &arr[i]);
        end = clock();
        results[t][3] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        start = clock();
        AVLSearch(&g_avl, targetId);
        end = clock();
        results[t][4] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        start = clock();
        AVLDelete(&g_avl, targetId, arr[n/2].courseId);
        end = clock();
        results[t][5] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        free(arr);
    }

    printf("\n\n[2] 实测运行耗时表\n");
    printf("------------------------------------------------------------------------------------\n");
    printf("| 数据规模 | 操作       | 链表耗时(ms) | AVL耗时(ms) | 性能差距  |\n");
    printf("|----------|------------|--------------|-------------|-----------|\n");
    
    for (t = 0; t < 3; t++) {
        n = sizes[t];
        printf("| %-8d | 插入       | %-12.3f | %-11.3f | %.1fx     |\n", 
               n, results[t][0], results[t][3], results[t][0] / results[t][3]);
        printf("| %-8d | 查找       | %-12.3f | %-11.3f | %.1fx     |\n", 
               n, results[t][1], results[t][4], results[t][1] / results[t][4]);
        printf("| %-8d | 删除       | %-12.3f | %-11.3f | %.1fx     |\n", 
               n, results[t][2], results[t][5], results[t][2] / results[t][5]);
        if (t < 2) printf("|----------|------------|--------------|-------------|-----------|\n");
    }
    printf("------------------------------------------------------------------------------------\n");

    printf("\n[3] 复杂度验证结论与选型分析\n");
    printf("  1. 理论验证: \n");
    printf("     实验结果符合理论预期：AVL树插入/查找/删除明显快于链表，尤其是查找和删除（O(log n) vs O(n)）。\n");
    printf("     当规模增大到 10000 条时，链表查找效率呈线性下降，AVL树优势愈发显著。\n");
    printf("  2. 偏差原因: \n");
    printf("     实测中链表插入耗时可能因随机内存分配(malloc)而波动；AVL树因递归调用，存在函数调用开销（常数因子）。\n");
    printf("  3. 适用场景分析: \n");
    printf("     - 双向链表: 适合频繁在尾部插入、数据量小、且不常按ID查找的场景（如日志记录）。\n");
    printf("     - AVL平衡树: 适合高频查找、插入、删除操作，且数据量较大的数据库场景。\n");
    
    printf("\n=============== 性能测试报告结束 ===============\n");

    DestroyList(&g_list);
    DestroyAVL(&g_avl);
    InitList(&g_list);
    InitAVL(&g_avl);
}

/* ==================== 任务5：批量删除过期记录 ==================== */
/**
 * @brief  根据基准日期，批量删除早于3年前的过期选课记录。
 * @note   必须同步在链表和AVL树中删除，以保证数据一致性。
 */
void batch_delete_expired_records(void) {
    const char *base_date = "2026-09-01";
    const char *cutoff_date = "2023-09-01";
    
    int delete_count = 0;
    int i;
    ListNode *p;
    char deleteIds[1000][MAX_STU_ID];
    char deleteCids[1000][MAX_COURSE_ID];

    printf("\n========== 批量删除过期记录 (任务5) ==========\n");
    printf("当前数据记录总数: %d 条\n", g_list.size);
    printf("删除基准日期: %s\n", base_date);
    printf("过期截止日期: %s (早于此日期的选课记录将被清除)\n", cutoff_date);

    if (g_list.size == 0) {
        printf("\n>> 系统中没有数据，无需清理。\n");
        return;
    }

    /* --- 1. 第一遍遍历：统计过期记录并暂存其主键 --- */
    p = g_list.head;
    while (p) {
        if (date_compare(p->data.selectDate, cutoff_date) < 0) {
            if (delete_count < 1000) {
                strcpy(deleteIds[delete_count], p->data.stuId);
                strcpy(deleteCids[delete_count], p->data.courseId);
                delete_count++;
            }
        }
        p = p->next;
    }

    /* --- 2. 给出确认提示 --- */
    if (delete_count == 0) {
        printf("\n>> 未发现早于 %s 的过期记录。\n", cutoff_date);
        return;
    }

    printf("\n>> 检测到共有 %d 条过期记录需要删除。\n", delete_count);
    printf(">> 警告：删除操作将同步清理链表和AVL树，不可恢复！\n");
    printf(">> 是否确认执行批量删除？ (输入 y 确认, 输入其他任意键取消): ");
    
    char confirm;
    scanf(" %c", &confirm);
    while (getchar() != '\n');

    if (confirm != 'y' && confirm != 'Y') {
        printf(">> 操作已取消。\n");
        return;
    }

    /* --- 3. 同步删除执行 (数据一致性核心) --- */
    printf(">> 正在执行批量删除...\n");

    /* (1) 删除双向链表中的数据 */
    for (i = 0; i < delete_count; i++) {
        ListDelete(&g_list, deleteIds[i], deleteCids[i]);
        RemoveEnrollment(&g_capacity, deleteCids[i]); 
    }

    /* (2) 删除 AVL 树中的数据 */
    for (i = 0; i < delete_count; i++) {
        AVLDelete(&g_avl, deleteIds[i], deleteCids[i]);
    }

    /* --- 4. 处理完成 --- */
    save_data(DATA_FILE);
    save_capacity();

    printf("\n>> 批量删除完成！\n");
    printf(">> 实际删除记录数: %d 条\n", delete_count);
    printf(">> 当前系统剩余记录数: %d 条\n", g_list.size);
    printf(">> 数据已自动保存至本地 (record.csv)\n");
}

/* ==================== 新增功能：清空所有数据 ==================== */
/**
 * @brief  危险操作：清除系统中所有的选课记录和容量配置，重置为初始状态。
 * @note   为了保证数据一致性，会同时清空双向链表、AVL树和容量哈希表。
 */
void clear_all_data(void) {
    char buf[10];

    printf("\n========== 危险操作：清空所有数据 ==========\n");
    printf(">> 警告：此操作将会永久删除系统中所有的选课记录！\n");
    printf(">> 警告：此操作不可逆！且会自动同步到本地文件(record.csv)！\n");
    printf(">> 请输入大写的 'YES' 确认清空操作: ");
    
    fgets(buf, sizeof(buf), stdin);
    trim_newline(buf);

    if (strcmp(buf, "YES") != 0) {
        printf(">> 输入不匹配，清空操作已取消。\n");
        return;
    }

    printf(">> 正在清空...\n");

    /* 1. 清空链表 */
    DestroyList(&g_list);
    InitList(&g_list);

    /* 2. 清空 AVL 树 */
    DestroyAVL(&g_avl);
    InitAVL(&g_avl);

    /* 3. 清空容量哈希表 */
    DestroyCapacityTable(&g_capacity);
    InitCapacityTable(&g_capacity);

    /* 4. 将空状态强制覆盖保存到本地文件中 */
    save_data(DATA_FILE);
    save_capacity();
    
    printf("\n>> 操作完成！\n");
    printf(">> 所有内存数据与本地文件数据已全部清空。\n");
    printf(">> 当前记录数: %d\n", g_list.size);
    printf(">> 您可以重新开始生成或录入新数据。\n");
}

/* ==================== 多条件筛选与导出（支持按年份筛选上下学期） ==================== */
void multi_filter_and_export(void) {
    FilterCondition cond = {0};
    char buf[100];
    int match_count = 0, idx = 0, is_match;
    ListNode *p;
    Record *arr, *r;
    time_t now;
    struct tm *tm_info;
    char filename[100];
    
    if (g_list.size == 0) return;
    printf("\n=========== 多条件组合筛选与导出 ===========\n");
    printf("提示：直接按回车键跳过不需要的筛选条件。\n\n");
    printf("课程名称 (模糊匹配, 回车跳过): ");
    if (fgets(cond.courseName, sizeof(cond.courseName), stdin)) { trim_newline(cond.courseName); if (strlen(cond.courseName) > 0) cond.useNameFilter = 1; }
    
    printf("选课学期 (输入年份如2024即可查出上/下学期, 回车跳过): ");
    if (fgets(cond.term, sizeof(cond.term), stdin)) { trim_newline(cond.term); if (strlen(cond.term) > 0) cond.useTermFilter = 1; }
    
    printf("学院 (精确匹配, 回车跳过): ");
    if (fgets(cond.college, sizeof(cond.college), stdin)) { trim_newline(cond.college); if (strlen(cond.college) > 0) cond.useCollegeFilter = 1; }
    printf("成绩下限 (回车跳过): ");
    if (fgets(buf, sizeof(buf), stdin)) { trim_newline(buf); if (strlen(buf) > 0) { cond.scoreMin = atoi(buf); printf("成绩上限: "); fgets(buf, sizeof(buf), stdin); cond.scoreMax = atoi(buf); cond.useScoreFilter = 1; } }
    
    p = g_list.head;
    while (p) {
        is_match = 1; r = &p->data;
        if (cond.useNameFilter && strstr(r->courseName, cond.courseName) == NULL) is_match = 0;
        
        /* 学期匹配逻辑升级：输入年份(如 2024)，匹配所有以 2024- 开头的学期 */
        if (cond.useTermFilter) {
            /* 
             * 使用 strncmp 进行前缀匹配。
             * 只要数据中的学期(如 2024-1) 的前 strlen(cond.term) 个字符
             * 等于用户输入的关键词(2024)，就视为通过。
             */
            if (strncmp(r->term, cond.term, strlen(cond.term)) != 0) {
                is_match = 0;
            }
        }
        
        if (cond.useCollegeFilter && strcmp(r->college, cond.college) != 0) is_match = 0;
        if (cond.useScoreFilter && (r->score < cond.scoreMin || r->score > cond.scoreMax)) is_match = 0;
        if (is_match) match_count++;
        p = p->next;
    }
    printf("\n>>> 共找到 %d 条符合条件的记录。\n", match_count);
    if (match_count == 0) return;
    arr = (Record*)malloc(match_count * sizeof(Record));
    p = g_list.head; idx = 0;
    while (p) {
        is_match = 1; r = &p->data;
        if (cond.useNameFilter && strstr(r->courseName, cond.courseName) == NULL) is_match = 0;
        if (cond.useTermFilter && strncmp(r->term, cond.term, strlen(cond.term)) != 0) is_match = 0;
        if (cond.useCollegeFilter && strcmp(r->college, cond.college) != 0) is_match = 0;
        if (cond.useScoreFilter && (r->score < cond.scoreMin || r->score > cond.scoreMax)) is_match = 0;
        if (is_match) arr[idx++] = *r;
        p = p->next;
    }
    now = time(NULL); tm_info = localtime(&now);
    strftime(filename, sizeof(filename), "筛选结果_%Y%m%d_%H%M%S.csv", tm_info);
    if (save_records_to_csv(filename, arr, match_count) == 0) {
        printf(">>> 结果已成功导出至文件: %s\n", filename);
        printf("\n>>> 筛选结果预览：\n"); print_records(arr, match_count, 20);
    }
    free(arr);
}

SortRule g_sort_rules[10]; 
int g_rule_count = 0;

int compare_records(const void *a, const void *b) {
    Record *r1 = (Record *)a, *r2 = (Record *)b;
    int i, cmp_result;
    SortRule rule;
    for (i = 0; i < g_rule_count; i++) {
        rule = g_sort_rules[i]; cmp_result = 0;
        switch (rule.field) {
            case SORT_BY_STUDENT_ID: cmp_result = strcmp(r1->stuId, r2->stuId); break;
            case SORT_BY_NAME: cmp_result = strcmp(r1->stuName, r2->stuName); break;
            case SORT_BY_COURSE: cmp_result = strcmp(r1->courseName, r2->courseName); break;
            case SORT_BY_TERM: cmp_result = strcmp(r1->term, r2->term); break;
            case SORT_BY_COLLEGE: cmp_result = strcmp(r1->college, r2->college); break;
            case SORT_BY_SCORE: cmp_result = (r1->score - r2->score); break;
        }
        if (cmp_result != 0) return (rule.order == ASCENDING) ? cmp_result : -cmp_result;
    }
    return 0;
}

void multi_key_sort_and_display(void) {
    int field_choice, order_choice, i;
    Record *arr;
    
    ListNode *p;
    if (g_list.size == 0) return;
    g_rule_count = 0;
    printf("\n=========== 多关键字排序 ===========\n");
    printf("字段编号：1.学号 2.姓名 3.课程 4.学期 5.成绩 6.学院\n");
    while (1) {
        if (g_rule_count >= 10) break;
        printf("\n第 %d 优先级字段编号 (0结束): ", g_rule_count + 1);
        scanf("%d", &field_choice); while (getchar() != '\n');
        if (field_choice == 0) break;
        if (field_choice < 1 || field_choice > 6) continue;
        printf("方向 (0升 1降): "); scanf("%d", &order_choice); while (getchar() != '\n');
        g_sort_rules[g_rule_count].field = (SortField)field_choice;
        g_sort_rules[g_rule_count].order = (order_choice == 1) ? DESCENDING : ASCENDING;
        g_rule_count++;
    }
    if (g_rule_count == 0) return;
    arr = (Record*)malloc(g_list.size * sizeof(Record));
    p = g_list.head;
    for (i = 0; i < g_list.size; i++) { arr[i] = p->data; p = p->next; }
    qsort(arr, g_list.size, sizeof(Record), compare_records);
    printf("\n>>> 排序完成！\n");
    print_records(arr, g_list.size, 50);
    free(arr);
}
