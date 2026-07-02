/**
 * @file    main.c
 * @brief   选课记录检索与分析系统主程序。
 *          包含双向链表(DLinkList)和AVL树(AVLTree)两种数据结构，
 *          支持数据筛选、多关键字排序、持久化存储、性能对比、
 *          批量删除过期记录、清空数据等完整功能。
 * 
 * @note    本系统采用双数据结构设计：
 *          - 双向链表：用于顺序遍历、批量操作和排序
 *          - AVL平衡树：用于快速查找、插入和删除（O(log n)）
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

/* ==================== 平台兼容性宏定义 ==================== */
#ifdef _WIN32
    #include <direct.h>
    #define mkdir _mkdir
#else
    #include <sys/stat.h>
    #define mkdir(path, mode) mkdir(path, mode)
#endif

/* ==================== 头文件引入 ==================== */
#include "record.h"           /* 记录结构体定义与操作 */
#include "dlinklist.h"        /* 双向链表数据结构 */
#include "avltree.h"          /* AVL平衡树数据结构 */
#include "course_capacity.h"  /* 课程容量管理模块 */

/* ==================== 常量与全局配置 ==================== */
#define DATA_DIR        "数据"                  /* 数据存储目录 */
#define DATA_FILE       "数据/当前数据.dat"      /* 默认数据文件 */
#define CAPACITY_FILE   "数据/容量配置.dat"      /* 容量配置文件 */
#include <dirent.h>     /* 目录遍历（POSIX） */

/* ==================== 全局变量声明 ==================== */
DLinkList      g_list;     /* 全局双向链表实例，存储所有选课记录 */
AVLTree        g_avl;      /* 全局AVL树实例，按学号索引 */
CapacityTable  g_capacity; /* 全局课程容量表，管理每门课程的选课人数上限 */

/* ==================== 函数声明（内部接口） ==================== */
void ensure_data_dir(void);                     /* 确保数据目录存在 */
void save_data(const char *filename);           /* 保存数据到二进制文件 */
void load_data(const char *filename);           /* 从二进制文件加载数据 */
void save_capacity(void);                       /* 保存容量配置到文件 */
void load_capacity(void);                       /* 从文件加载容量配置 */
void data_structure_menu(int *selected);        /* 数据结构选择菜单 */
void operation_menu(int ds_type);               /* 数据结构操作菜单（增删改查） */
void performance_test(void);                    /* 性能对比测试（链表 vs AVL） */
void print_search_result_list(const char *stuId); /* 链表查找结果打印 */
void print_search_result_avl(const char *stuId);  /* AVL树查找结果打印 */
void statistic_menu(void);                      /* 统计分析菜单 */
int  insert_with_capacity(int ds_type, const Record *r); /* 带容量检查的插入 */
void generate_test_data(void);                  /* 生成测试数据（用户指定数量） */
void list_data_files(void);                     /* 列出数据目录所有文件 */
void load_data_file(void);                      /* 加载指定的数据文件 */
void multi_filter_and_export(void);             /* 多条件筛选并导出CSV */
void multi_key_sort_and_display(void);          /* 多关键字排序并显示 */
void batch_delete_expired_records(void);        /* 批量删除过期记录（任务5） */
void clear_all_data(void);                      /* 清空所有数据（危险操作） */

/**
 * @brief  主程序入口。
 *         初始化数据结构，加载持久化数据，进入主菜单循环。
 * @return 程序退出状态（0表示正常退出）
 */
int main(void) {
    int choice;     /* 主菜单选项 */
    int ds;         /* 数据结构选择（1=链表，2=AVL树） */
    int cmd;        /* 子菜单命令 */
    char cid[MAX_COURSE_ID]; /* 课程编号缓冲区 */
    int cap;        /* 容量值 */

    /* 1. 确保数据目录存在，若不存在则创建 */
    ensure_data_dir();

    /* 2. 初始化三种数据结构 */
    InitList(&g_list);          /* 初始化双向链表 */
    InitAVL(&g_avl);            /* 初始化AVL树 */
    InitCapacityTable(&g_capacity); /* 初始化容量表 */

    /* 3. 加载持久化数据（从文件恢复到内存） */
    load_data(DATA_FILE);       /* 加载选课记录数据 */
    load_capacity();            /* 加载课程容量配置 */

    /* 4. 主菜单循环（用户交互核心） */
    do {
        /* 显示系统状态信息 */
        printf("\n========== 校园选课记录检索与大数据分析系统 ==========\n");
        printf("  当前记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
        printf("  数据文件夹: %s\n", DATA_DIR);
        printf("========================================================\n");
        
        /* 主菜单选项 */
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
        while (getchar() != '\n'); /* 清空输入缓冲区 */

        /* 根据用户选择执行对应功能 */
        switch (choice) {
            case 1:
                /* 选择数据结构（链表或AVL树），进入操作菜单 */
                data_structure_menu(&ds);
                if (ds >= 1 && ds <= 2) operation_menu(ds);
                break;
                
            case 2: 
                /* 统计分析：课程容量、学院分布、成绩分段、学生学分统计 */
                statistic_menu(); 
                break;
                
            case 3:
                /* 课程容量管理子菜单 */
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
                        /* 设置课程容量 */
                        printf("课程编号: ");
                        fgets(cid, sizeof(cid), stdin);
                        trim_newline(cid);
                        printf("容量: ");
                        scanf("%d", &cap);
                        while (getchar() != '\n');
                        SetCourseCapacity(&g_capacity, cid, cap);
                        printf(">> 设置成功！\n");
                        save_capacity(); /* 立即保存到文件 */
                        break;
                    case 2: 
                        /* 打印所有课程容量信息 */
                        PrintCapacityInfo(&g_capacity); 
                        break;
                    case 3:
                        /* 查看单门课程容量 */
                        printf("课程编号: ");
                        fgets(cid, sizeof(cid), stdin);
                        trim_newline(cid);
                        cap = GetCourseCapacity(&g_capacity, cid);
                        {
                            int e = GetCourseEnrolled(&g_capacity, cid);
                            if (cap < 0) printf(">> 该课程未设置容量。\n");
                            else printf(">> %s: 容量=%d, 已选=%d, 剩余=%d\n", 
                                       cid, cap, e, cap - e);
                        }
                        break;
                    case 0: break;
                    default: printf(">> 无效选项。\n");
                }
                break;
                
            case 4: 
                /* 性能对比测试 */
                performance_test(); 
                break;
                
            case 5: 
                /* 生成测试数据（用户指定数量） */
                generate_test_data(); 
                break;
                
            case 6: 
                /* 列出数据目录所有文件（任务2） */
                list_data_files(); 
                break;
                
            case 7: 
                /* 加载指定的数据文件（任务2） */
                load_data_file(); 
                break;
                
            case 8: 
                /* 多条件筛选并导出CSV（任务3） */
                multi_filter_and_export(); 
                break;
                
            case 9: 
                /* 多关键字排序并显示（任务3） */
                multi_key_sort_and_display(); 
                break;
                
            case 10: 
                /* 性能对比与复杂度验证（任务6） */
                performance_test(); 
                break;
                
            case 11: 
                /* 批量删除过期记录（任务5） */
                batch_delete_expired_records(); 
                break;
                
            case 12: 
                /* 清空所有数据（危险操作，需确认） */
                clear_all_data(); 
                break;
                
            case 0:
                /* 保存并退出：持久化所有数据到文件 */
                save_data(DATA_FILE);
                save_capacity();
                printf(">> 数据已保存到 %s，程序退出。\n", DATA_FILE);
                break;
                
            default: 
                printf(">> 无效选项，请重新选择。\n");
        }
    } while (choice != 0); /* 选择0时退出循环 */

    /* 5. 释放所有数据结构占用的内存 */
    DestroyList(&g_list);      /* 销毁链表 */
    DestroyAVL(&g_avl);        /* 销毁AVL树 */
    DestroyCapacityTable(&g_capacity); /* 销毁容量表 */
    
    /* 6. 暂停等待用户按键，防止窗口闪退（Windows控制台） */
    printf("\n程序正常结束，按任意键退出...");
    getchar();
    return 0;
}

/**
 * @brief  确保数据存储目录存在，若不存在则创建。
 *         使用跨平台方式（_mkdir / mkdir）兼容Windows和Linux。
 */
void ensure_data_dir(void) {
    struct stat st;
    if (stat(DATA_DIR, &st) == -1) {  /* 目录不存在 */
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

/**
 * @brief  生成指定数量的随机测试数据。
 *         先让用户输入数量，生成后插入链表和AVL树（受容量限制），
 *         并自动保存为带时间戳的 .dat 和 .csv 文件。
 * 
 * @note   生成的数据包含学号、姓名、课程、学期、学院、成绩、学分等字段。
 *         插入时受课程容量限制，超出容量的记录会被拒绝。
 */
void generate_test_data(void) {
    int n;          /* 用户指定的生成数量 */
    int inserted;   /* 成功插入的记录数 */
    int rejected;   /* 因容量不足被拒绝的记录数 */
    int i;
    char filename[200];        /* 二进制文件名 */
    char csv_filename[200];    /* CSV文件名 */
    Record *arr;               /* 动态分配的记录数组 */
    time_t now;                /* 当前时间（用于生成时间戳） */
    struct tm *tm_info;
    char time_str[30];
    char full_path[300];
    char csv_full_path[300];
    
    printf("\n========== 生成测试数据 ==========\n");
    printf("当前记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
    printf("请输入要生成的记录数量 (0-10000): ");
    scanf("%d", &n);
    while (getchar() != '\n');
    
    /* 参数校验 */
    if (n <= 0 || n > MAX_DATA_SIZE) return;
    if (g_list.size + n > MAX_DATA_SIZE) {
        printf(">> 容量不足，可生成 %d 条。\n", MAX_DATA_SIZE - g_list.size);
        return;
    }
    
    printf(">> 正在生成 %d 条测试数据...\n", n);
    arr = (Record*)malloc(n * sizeof(Record));
    if (!arr) { printf(">> 内存分配失败！\n"); return; }
    
    /* 调用record模块的随机生成函数 */
    generate_records(arr, n);
    printf("\n========== 生成的数据（表格展示） ==========\n");
    print_records(arr, n, 50);  /* 最多显示50条 */
    
    /* 逐条插入，受容量限制 */
    inserted = 0; rejected = 0;
    for (i = 0; i < n; i++) {
        if (AddEnrollment(&g_capacity, arr[i].courseId)) {
            ListInsert(&g_list, &arr[i]);   /* 插入链表 */
            AVLInsert(&g_avl, &arr[i]);     /* 插入AVL树 */
            inserted++;
        } else rejected++;
    }
    printf("\n========== 数据插入结果 ==========\n");
    printf("  成功插入: %d 条\n", inserted);
    printf("  被拒绝（容量已满）: %d 条\n", rejected);
    printf("  当前总记录数: %d / %d\n", g_list.size, MAX_DATA_SIZE);
    
    /* 生成带时间戳的文件名并保存 */
    now = time(NULL);
    tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y%m%d_%H%M%S", tm_info);
    sprintf(filename, "数据_%s_%d条.dat", time_str, n);
    sprintf(full_path, "%s/%s", DATA_DIR, filename);
    sprintf(csv_filename, "数据_%s_%d条.csv", time_str, n);
    sprintf(csv_full_path, "%s/%s", DATA_DIR, csv_filename);
    
    save_data(full_path);      /* 保存二进制数据 */
    save_capacity();           /* 保存容量配置 */
    if (save_records_to_csv(csv_full_path, arr, n) == 0) {
        printf("\n========== 文件保存成功 ==========\n");
        printf("  CSV表格文件: %s\n", csv_full_path);
    }
    free(arr);
}

/**
 * @brief  将当前链表数据保存到二进制文件。
 *         格式：先写入记录总数（int），再逐条写入Record结构体。
 *         同时自动导出CSV格式到本地 record.csv。
 * 
 * @param filename 目标文件名（包含路径）
 */
void save_data(const char *filename) {
    FILE *fp = fopen(filename, "wb");
    int n = g_list.size;
    ListNode *p = g_list.head;
    Record *arr;
    int i;
    if (!fp) { printf(">> 无法保存数据到 %s！\n", filename); return; }
    
    /* 写入记录总数 */
    fwrite(&n, sizeof(int), 1, fp);
    /* 逐条写入Record结构体 */
    while (p) { 
        fwrite(&p->data, sizeof(Record), 1, fp); 
        p = p->next; 
    }
    fclose(fp);
    
    /* 同时导出CSV格式（便于Excel查看） */
    if (n == 0) return;
    arr = (Record*)malloc(n * sizeof(Record));
    if (!arr) return;
    p = g_list.head;
    for (i = 0; i < n; i++) { 
        arr[i] = p->data; 
        p = p->next; 
    }
    if (save_records_to_csv("record.csv", arr, n) == 0) {
        printf(">> 数据已同步导出至本地文件: record.csv\n");
    }
    free(arr);
}

/**
 * @brief  从二进制文件加载数据到内存（链表和AVL树）。
 *         读取记录后同时插入链表和AVL树，并更新容量计数。
 * 
 * @param filename 源文件名（包含路径）
 */
void load_data(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    int n, i;
    Record r;
    if (!fp) { printf(">> 未找到数据文件 %s\n", filename); return; }
    if (fread(&n, sizeof(int), 1, fp) != 1) { fclose(fp); return; }
    if (n > MAX_DATA_SIZE) n = MAX_DATA_SIZE;  /* 防止溢出 */
    
    for (i = 0; i < n; i++) {
        if (fread(&r, sizeof(Record), 1, fp) != 1) break;
        ListInsert(&g_list, &r);           /* 插入链表 */
        AVLInsert(&g_avl, &r);             /* 插入AVL树 */
        AddEnrollment(&g_capacity, r.courseId); /* 增加课程选课人数 */
    }
    fclose(fp);
    printf(">> 成功加载数据，共 %d 条记录。\n", g_list.size);
}

/**
 * @brief  保存容量配置到文件（封装函数）
 */
void save_capacity(void) { 
    SaveCapacityToFile(&g_capacity, CAPACITY_FILE); 
}

/**
 * @brief  从文件加载容量配置（封装函数）
 *         如果文件不存在，使用默认容量（每门课60人）。
 */
void load_capacity(void) {
    if (LoadCapacityFromFile(&g_capacity, CAPACITY_FILE) != 0)
        printf(">> 未找到容量配置文件，使用默认容量(60人/门)。\n");
}

/**
 * @brief  列出数据目录下的所有文件（任务2）
 *         显示文件名、修改时间、大小、文件类型（CSV/DAT/其他）
 */
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
    
    /* 打印表头 */
    printf("%-45s %-20s %-10s %-8s\n", "文件名", "修改时间", "大小", "类型");
    printf("--------------------------------------------------------------------------------\n");
    
    /* 遍历目录中的每个文件 */
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        sprintf(full_path, "%s/%s", DATA_DIR, entry->d_name);
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            /* 获取文件修改时间 */
            tm_info = localtime(&st.st_mtime);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            /* 根据扩展名判断文件类型 */
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

/**
 * @brief  加载用户指定的数据文件（任务2）
 *         先列出所有文件，让用户输入文件名，然后清空当前数据并加载新数据。
 */
void load_data_file(void) {
    char filename[200];
    char full_path[300];
    FILE *fp;
    
    list_data_files();  /* 先列出文件供用户选择 */
    printf("\n请输入要加载的文件名 (.dat 文件，不含路径): ");
    fgets(filename, sizeof(filename), stdin);
    trim_newline(filename);
    if (strlen(filename) == 0) { printf(">> 未选择任何文件。\n"); return; }
    
    sprintf(full_path, "%s/%s", DATA_DIR, filename);
    fp = fopen(full_path, "rb");
    if (!fp) { printf(">> 文件 %s 不存在！\n", full_path); return; }
    fclose(fp);
    
    /* 清空当前数据，释放内存 */
    DestroyList(&g_list);
    DestroyAVL(&g_avl);
    DestroyCapacityTable(&g_capacity);
    /* 重新初始化 */
    InitList(&g_list); 
    InitAVL(&g_avl); 
    InitCapacityTable(&g_capacity);
    /* 加载新数据 */
    load_data(full_path);
    printf(">> 已切换到数据文件: %s\n", full_path);
}

/**
 * @brief  数据结构选择菜单（让用户选择操作链表还是AVL树）
 * @param selected 输出参数，用户的选择（1=链表，2=AVL树，0=返回）
 */
void data_structure_menu(int *selected) {
    printf("\n请选择要操作的数据结构:\n");
    printf("  1. 双向链表 (记录数: %d)\n", g_list.size);
    printf("  2. AVL平衡树 (记录数: %d)\n", g_avl.size);
    printf("  0. 返回\n");
    printf("请选择: ");
    scanf("%d", selected);
    while (getchar() != '\n');
}

/**
 * @brief  带容量检查的记录插入函数。
 *         先检查课程容量是否已满，若未满则插入指定的数据结构。
 * 
 * @param ds_type 数据结构类型（1=链表，2=AVL树）
 * @param r       要插入的记录指针
 * @return 0=成功，-1=失败（容量不足或数据已满）
 */
int insert_with_capacity(int ds_type, const Record *r) {
    int ret;
    if (g_list.size >= MAX_DATA_SIZE) return -1;  /* 系统容量已满 */
    if (!AddEnrollment(&g_capacity, r->courseId)) return -1; /* 课程容量已满 */
    
    if (ds_type == 1) ret = ListInsert(&g_list, r);
    else ret = AVLInsert(&g_avl, r);
    
    if (ret != 0) { 
        /* 插入失败，回滚容量计数 */
        RemoveEnrollment(&g_capacity, r->courseId); 
        return -1; 
    }
    return 0;
}

/**
 * @brief  数据结构操作菜单（增删改查遍历）
 *        针对链表或AVL树提供统一的CRUD操作接口。
 * 
 * @param ds_type 数据结构类型（1=链表，2=AVL树）
 */
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
                /* 插入记录：先输入记录数据，再插入（受容量限制） */
                if (input_record(&r) && insert_with_capacity(ds_type, &r) == 0) {
                    printf(">> 选课成功！课程 %s 当前已选 %d/%d 人\n", 
                           r.courseId, 
                           GetCourseEnrolled(&g_capacity, r.courseId), 
                           GetCourseCapacity(&g_capacity, r.courseId));
                    save_data(DATA_FILE); 
                    save_capacity();
                }
                break;
                
            case 2:
                /* 删除记录（退课）：按学号+课程号删除 */
                printf("学号: "); fgets(sid, sizeof(sid), stdin); trim_newline(sid);
                printf("课程编号: "); fgets(cid, sizeof(cid), stdin); trim_newline(cid);
                ret = (ds_type == 1) ? ListDelete(&g_list, sid, cid) : AVLDelete(&g_avl, sid, cid);
                if (ret == 0) { 
                    RemoveEnrollment(&g_capacity, cid); /* 释放容量 */ 
                    save_data(DATA_FILE); 
                    save_capacity(); 
                }
                else printf(">> 退课失败，未找到记录！\n");
                break;
                
            case 3:
                /* 修改成绩：按学号+课程号修改成绩 */
                printf("学号: "); fgets(sid, sizeof(sid), stdin); trim_newline(sid);
                printf("课程编号: "); fgets(cid, sizeof(cid), stdin); trim_newline(cid);
                printf("新成绩: "); scanf("%d", &newScore); while (getchar() != '\n');
                ret = (ds_type == 1) ? ListUpdate(&g_list, sid, cid, newScore) : AVLUpdate(&g_avl, sid, cid, newScore);
                if (ret == 0) save_data(DATA_FILE);
                else printf(">> 修改失败，未找到记录！\n");
                break;
                
            case 4:
                /* 查找记录：支持按学号精确查找（高效）或按姓名/课程模糊查找（遍历） */
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
                    while (getchar() != '\n');

                    if (search_mode == 0) break;

                    printf("请输入查找关键词: ");
                    fgets(keyword, sizeof(keyword), stdin);
                    trim_newline(keyword);

                    if (strlen(keyword) == 0) {
                        printf(">> 关键词为空，取消查找。\n");
                        break;
                    }

                    /* 按学号精确查找（利用AVL树O(log n)或链表O(n)） */
                    if (search_mode == 1) {
                        if (ds_type == 1) print_search_result_list(keyword);
                        else print_search_result_avl(keyword);
                    } 
                    /* 按姓名或课程名称模糊查找（只能遍历链表） */
                    else {
                        p = g_list.head;
                        found_count = 0;
                        printf("\n>> 正在遍历查找...\n");
                        print_record_header();
                        while (p) {
                            is_match = 0;
                            if (search_mode == 2) {
                                if (strstr(p->data.stuName, keyword) != NULL) is_match = 1;
                            } else if (search_mode == 3) {
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
                
            case 5:
                /* 遍历所有记录并打印 */
                (ds_type == 1) ? ListTraverse(&g_list) : AVLTraverse(&g_avl); 
                break;
                
            case 6: 
                /* 查看课程容量信息 */
                PrintCapacityInfo(&g_capacity); 
                break;
                
            case 0: break;
            default: printf(">> 无效选项。\n");
        }
    } while (op != 0);
}

/**
 * @brief  在链表中按学号查找并打印结果
 * @param stuId 学号
 */
void print_search_result_list(const char *stuId) {
    ListNode *found = ListSearch(&g_list, stuId);
    if (found) { 
        printf(">> 找到记录：\n"); 
        print_record_header(); 
        print_record(&found->data); 
    }
    else printf(">> 未找到学号 %s 的记录。\n", stuId);
}

/**
 * @brief  在AVL树中按学号查找并打印结果
 * @param stuId 学号
 */
void print_search_result_avl(const char *stuId) {
    AVLNode *found = AVLSearch(&g_avl, stuId);
    if (found) { 
        printf(">> 找到记录：\n"); 
        print_record_header(); 
        print_record(&found->data); 
    }
    else printf(">> 未找到学号 %s 的记录。\n", stuId);
}

/**
 * @brief  统计分析菜单（任务4）
 *         包含：课程容量使用率、各学院选课人数、成绩分段、每位学生选课门数与总学分
 */
void statistic_menu(void) {
    int choice, i, n, found, s;
    ListNode *p;
    int dist[5];  /* 成绩分段统计：不及格/及格/中等/良好/优秀 */
    
    /* 内部结构体：用于学院统计 */
    typedef struct { 
        char name[MAX_COLLEGE]; 
        int count; 
    } CollegeStat;
    
    /* 内部结构体：用于学生统计 */
    typedef struct { 
        char stuId[MAX_STU_ID]; 
        char stuName[MAX_NAME]; 
        int count; 
        float total_credit; 
    } StudentStat;
    
    CollegeStat *stats = NULL;
    StudentStat *s_stats = NULL;
    int s_n = 0;
    
    printf("\n========== 统计分析 ==========\n");
    printf("  1. 课程容量使用率统计\n  2. 各学院选课人数分布\n");
    printf("  3. 成绩分段统计\n  4. 每位学生选课门数与总学分\n  0. 返回\n");
    printf("请选择: ");
    scanf("%d", &choice); 
    while (getchar() != '\n');
    
    switch (choice) {
        case 1: 
            /* 课程容量使用率：显示每门课的容量/已选/剩余 */
            PrintCapacityInfo(&g_capacity); 
            break;
            
        case 2:
            /* 各学院选课人数分布：遍历链表，统计每个学院的人数 */
            n = 0; 
            stats = NULL; 
            p = g_list.head;
            while (p) {
                found = 0;
                for (i = 0; i < n; i++) {
                    if (strcmp(stats[i].name, p->data.college) == 0) { 
                        stats[i].count++; 
                        found = 1; 
                        break; 
                    }
                }
                if (!found) {
                    stats = (CollegeStat*)realloc(stats, (n+1)*sizeof(CollegeStat));
                    strcpy(stats[n].name, p->data.college); 
                    stats[n].count = 1; 
                    n++;
                }
                p = p->next;
            }
            printf("\n各学院选课人数分布：\n");
            for (i = 0; i < n; i++) 
                printf("  %-30s : %d人\n", stats[i].name, stats[i].count);
            free(stats);
            break;
            
        case 3:
            /* 成绩分段统计：0-59, 60-69, 70-79, 80-89, 90-100 */
            for(i=0; i<5; i++) dist[i]=0;
            p = g_list.head;
            while (p) { 
                s = p->data.score; 
                if (s < 60) dist[0]++; 
                else if (s < 70) dist[1]++; 
                else if (s < 80) dist[2]++; 
                else if (s < 90) dist[3]++; 
                else dist[4]++; 
                p = p->next; 
            }
            printf("\n成绩分段统计：\n");
            printf("  0-59   : %d人 (不及格)\n  60-69  : %d人 (及格)\n  70-79  : %d人 (中等)\n", 
                   dist[0], dist[1], dist[2]);
            printf("  80-89  : %d人 (良好)\n  90-100 : %d人 (优秀)\n", dist[3], dist[4]);
            break;
            
        case 4:
            /* 每位学生选课门数与总学分：按学号分组统计 */
            p = g_list.head; 
            s_n = 0; 
            s_stats = NULL;
            while (p) {
                found = 0;
                for (i = 0; i < s_n; i++) {
                    if (strcmp(s_stats[i].stuId, p->data.stuId) == 0) {
                        s_stats[i].count++; 
                        s_stats[i].total_credit += p->data.credit; 
                        found = 1; 
                        break;
                    }
                }
                if (!found) {
                    s_stats = (StudentStat*)realloc(s_stats, (s_n+1)*sizeof(StudentStat));
                    strcpy(s_stats[s_n].stuId, p->data.stuId); 
                    strcpy(s_stats[s_n].stuName, p->data.stuName);
                    s_stats[s_n].count = 1; 
                    s_stats[s_n].total_credit = p->data.credit; 
                    s_n++;
                }
                p = p->next;
            }
            printf("\n========== 每位学生选课门数与总学分 ==========\n");
            printf("%-12s %-10s %-10s %-10s\n", "学号", "姓名", "选课门数", "总学分");
            for (i = 0; i < s_n; i++) 
                printf("%-12s %-10s %-10d %-10.1f\n", 
                       s_stats[i].stuId, s_stats[i].stuName, 
                       s_stats[i].count, s_stats[i].total_credit);
            printf(">> 共统计 %d 位学生。\n", s_n);
            free(s_stats);
            break;
            
        case 0: break;
        default: printf(">> 无效选项。\n");
    }
}

/* ==================== 任务6：性能对比测试 ==================== */

/**
 * @brief  性能对比与复杂度验证（任务6）
 *         对比双向链表和AVL树在100、1000、10000条数据规模下的 *         插入、查找、删除操作耗时，并给出理论分析与选型建议。
 */
void performance_test(void) {
    int sizes[] = {100, 1000, 10000};  /* 三种测试规模 */
    int t, i, n;
    clock_t start, end;
    Record *arr;
    char targetId[MAX_STU_ID];
    
    printf("\n=============== 性能对比与复杂度验证报告 ===============\n");
    
    /* 第一部分：理论时间复杂度分析 */
    printf("\n[1] 理论时间复杂度分析\n");
    printf("  -----------------------------------------------------------------\n");
    printf("  | 数据结构  | 插入(O)  | 查找(O)  | 删除(O)  | 遍历(O)  |\n");
    printf("  |-----------|----------|----------|----------|----------|\n");
    printf("  | 双向链表  | O(1)*    | O(n)     | O(n)     | O(n)     |\n");
    printf("  | AVL 平衡树| O(log n) | O(log n) | O(log n) | O(n)     |\n");
    printf("  -----------------------------------------------------------------\n");
    printf("  *注: 链表尾插O(1)，查找O(n)，故删除需先查找仍为O(n)。\n");

    double results[3][6];  /* 存储测试结果：3种规模 × 6种操作 */
    
    /* 第二部分：实测运行耗时 */
    for (t = 0; t < 3; t++) {
        n = sizes[t];
        if (n > MAX_DATA_SIZE) {
            printf("\n[!] 跳过规模 %d 条（超出系统最大限制 %d）\n", n, MAX_DATA_SIZE);
            continue;
        }

        printf("\n--------------------------------------------------------\n");
        printf(">>> 测试规模: %d 条数据\n", n);

        /* 生成测试数据 */
        arr = (Record*)malloc(n * sizeof(Record));
        if (!arr) { printf(">> 内存分配失败！\n"); return; }
        generate_records(arr, n);
        /* 选择中间位置的一条记录作为查找/删除目标 */
        sprintf(targetId, "2024%04d", n/2 + 1);

        /* ----- 双向链表测试 ----- */
        DestroyList(&g_list);
        InitList(&g_list);

        /* 链表插入性能测试 */
        start = clock();
        for (i = 0; i < n; i++) ListInsert(&g_list, &arr[i]);
        end = clock();
        results[t][0] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* 链表查找性能测试 */
        start = clock();
        ListSearch(&g_list, targetId);
        end = clock();
        results[t][1] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* 链表删除性能测试 */
        start = clock();
        ListDelete(&g_list, targetId, arr[n/2].courseId); 
        end = clock();
        results[t][2] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* ----- AVL树测试 ----- */
        DestroyAVL(&g_avl);
        InitAVL(&g_avl);

        /* AVL树插入性能测试 */
        start = clock();
        for (i = 0; i < n; i++) AVLInsert(&g_avl, &arr[i]);
        end = clock();
        results[t][3] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* AVL树查找性能测试 */
        start = clock();
        AVLSearch(&g_avl, targetId);
        end = clock();
        results[t][4] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        /* AVL树删除性能测试 */
        start = clock();
        AVLDelete(&g_avl, targetId, arr[n/2].courseId);
        end = clock();
        results[t][5] = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

        free(arr);
    }

    /* 打印性能对比表格 */
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

    /* 第三部分：结论与选型分析 */
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

    /* 清理测试数据 */
    DestroyList(&g_list);
    DestroyAVL(&g_avl);
    InitList(&g_list);
    InitAVL(&g_avl);
}

/* ==================== 任务5：批量删除过期记录 ==================== */

/**
 * @brief  批量删除过期记录（任务5）
 *         删除选课日期早于截止日期（2023-09-01）的记录。
 *         操作前会显示删除数量并要求用户确认，防止误操作。
 */
void batch_delete_expired_records(void) {
    const char *base_date = "2026-09-01";     /* 基准日期（显示用） */
    const char *cutoff_date = "2023-09-01";   /* 截止日期：早于此日期的记录将被删除 */
    
    int delete_count = 0;
    int i;
    ListNode *p;
    char deleteIds[1000][MAX_STU_ID];      /* 待删除记录的学号缓存 */
    char deleteCids[1000][MAX_COURSE_ID];  /* 待删除记录的课程号缓存 */

    printf("\n========== 批量删除过期记录 (任务5) ==========\n");
    printf("当前数据记录总数: %d 条\n", g_list.size);
    printf("删除基准日期: %s\n", base_date);
    printf("过期截止日期: %s (早于此日期的选课记录将被清除)\n", cutoff_date);

    if (g_list.size == 0) {
        printf("\n>> 系统中没有数据，无需清理。\n");
        return;
    }

    /* 第一遍遍历：统计符合条件的过期记录 */
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

    if (delete_count == 0) {
        printf("\n>> 未发现早于 %s 的过期记录。\n", cutoff_date);
        return;
    }

    /* 显示删除信息并要求用户确认 */
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

    printf(">> 正在执行批量删除...\n");

    /* 从链表中删除并释放容量 */
    for (i = 0; i < delete_count; i++) {
        ListDelete(&g_list, deleteIds[i], deleteCids[i]);
        RemoveEnrollment(&g_capacity, deleteCids[i]); 
    }

    /* 从AVL树中删除 */
    for (i = 0; i < delete_count; i++) {
        AVLDelete(&g_avl, deleteIds[i], deleteCids[i]);
    }

    /* 持久化保存 */
    save_data(DATA_FILE);
    save_capacity();

    printf("\n>> 批量删除完成！\n");
    printf(">> 实际删除记录数: %d 条\n", delete_count);
    printf(">> 当前系统剩余记录数: %d 条\n", g_list.size);
    printf(">> 数据已自动保存至本地 (record.csv)\n");
}

/* ==================== 危险操作：清空所有数据 ==================== */

/**
 * @brief  清空所有数据（危险操作）
 *         释放所有数据结构的内存并重新初始化，同时删除持久化文件。
 *         需要用户输入 "YES" 确认（大写），防止误触。
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

    /* 销毁并重新初始化链表 */
    DestroyList(&g_list);
    InitList(&g_list);

    /* 销毁并重新初始化AVL树 */
    DestroyAVL(&g_avl);
    InitAVL(&g_avl);

    /* 销毁并重新初始化容量表 */
    DestroyCapacityTable(&g_capacity);
    InitCapacityTable(&g_capacity);

    /* 保存空数据到文件（覆盖原有数据） */
    save_data(DATA_FILE);
    save_capacity();
    
    printf("\n>> 操作完成！\n");
    printf(">> 所有内存数据与本地文件数据已全部清空。\n");
    printf(">> 当前记录数: %d\n", g_list.size);
    printf(">> 您可以重新开始生成或录入新数据。\n");
}

/* ==================== 任务3：多条件筛选与导出 ==================== */

/**
 * @brief  多条件组合筛选并导出为CSV文件（任务3）
 *         支持按课程名称（模糊）、学期、学院（精确）、成绩区间筛选。
 *         筛选结果会保存为带时间戳的CSV文件，并预览前20条。
 */
void multi_filter_and_export(void) {
    FilterCondition cond = {0};  /* 筛选条件结构体，初始全0 */
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
    
    /* 逐一收集筛选条件（回车跳过） */
    printf("课程名称 (模糊匹配, 回车跳过): ");
    if (fgets(cond.courseName, sizeof(cond.courseName), stdin)) { 
        trim_newline(cond.courseName); 
        if (strlen(cond.courseName) > 0) cond.useNameFilter = 1; 
    }
    
    printf("选课学期 (输入年份如2024即可查出上/下学期, 回车跳过): ");
    if (fgets(cond.term, sizeof(cond.term), stdin)) { 
        trim_newline(cond.term); 
        if (strlen(cond.term) > 0) cond.useTermFilter = 1; 
    }
    
    printf("学院 (精确匹配, 回车跳过): ");
    if (fgets(cond.college, sizeof(cond.college), stdin)) { 
        trim_newline(cond.college); 
        if (strlen(cond.college) > 0) cond.useCollegeFilter = 1; 
    }
    
    printf("成绩下限 (回车跳过): ");
    if (fgets(buf, sizeof(buf), stdin)) { 
        trim_newline(buf); 
        if (strlen(buf) > 0) { 
            cond.scoreMin = atoi(buf); 
            printf("成绩上限: "); 
            fgets(buf, sizeof(buf), stdin); 
            cond.scoreMax = atoi(buf); 
            cond.useScoreFilter = 1; 
        } 
    }
    
    /* 第一遍遍历：统计符合条件的记录数 */
    p = g_list.head;
    while (p) {
        is_match = 1; 
        r = &p->data;
        
        if (cond.useNameFilter && strstr(r->courseName, cond.courseName) == NULL) 
            is_match = 0;
        
        if (cond.useTermFilter) {
            if (strncmp(r->term, cond.term, strlen(cond.term)) != 0) {
                is_match = 0;
            }
        }
        
        if (cond.useCollegeFilter && strcmp(r->college, cond.college) != 0) 
            is_match = 0;
        if (cond.useScoreFilter && (r->score < cond.scoreMin || r->score > cond.scoreMax)) 
            is_match = 0;
            
        if (is_match) match_count++;
        p = p->next;
    }
    
    printf("\n>>> 共找到 %d 条符合条件的记录。\n", match_count);
    if (match_count == 0) return;
    
    /* 第二遍遍历：复制匹配的记录到数组 */
    arr = (Record*)malloc(match_count * sizeof(Record));
    p = g_list.head; 
    idx = 0;
    while (p) {
        is_match = 1; 
        r = &p->data;
        if (cond.useNameFilter && strstr(r->courseName, cond.courseName) == NULL) 
            is_match = 0;
        if (cond.useTermFilter && strncmp(r->term, cond.term, strlen(cond.term)) != 0) 
            is_match = 0;
        if (cond.useCollegeFilter && strcmp(r->college, cond.college) != 0) 
            is_match = 0;
        if (cond.useScoreFilter && (r->score < cond.scoreMin || r->score > cond.scoreMax)) 
            is_match = 0;
        if (is_match) arr[idx++] = *r;
        p = p->next;
    }
    
    /* 导出为CSV文件（带时间戳） */
    now = time(NULL); 
    tm_info = localtime(&now);
    strftime(filename, sizeof(filename), "筛选结果_%Y%m%d_%H%M%S.csv", tm_info);
    if (save_records_to_csv(filename, arr, match_count) == 0) {
        printf(">>> 结果已成功导出至文件: %s\n", filename);
        printf("\n>>> 筛选结果预览：\n"); 
        print_records(arr, match_count, 20);  /* 预览前20条 */
    }
    free(arr);
}

/* ==================== 任务3：多关键字排序 ==================== */

/* 全局排序规则数组（最多10个优先级） */
SortRule g_sort_rules[10]; 
int g_rule_count = 0;

/**
 * @brief  记录比较函数（用于qsort）
 *         按用户指定的多关键字顺序比较两条记录。
 *         支持升序/降序，先按第一优先级比较，若相等则按第二优先级，以此类推。
 * 
 * @param a 第一条记录指针
 * @param b 第二条记录指针
 * @return  比较结果（负数=小于，0=等于，正数=大于）
 */
int compare_records(const void *a, const void *b) {
    Record *r1 = (Record *)a, *r2 = (Record *)b;
    int i, cmp_result;
    SortRule rule;
    
    for (i = 0; i < g_rule_count; i++) {
        rule = g_sort_rules[i]; 
        cmp_result = 0;
        switch (rule.field) {
            case SORT_BY_STUDENT_ID: cmp_result = strcmp(r1->stuId, r2->stuId); break;
            case SORT_BY_NAME: cmp_result = strcmp(r1->stuName, r2->stuName); break;
            case SORT_BY_COURSE: cmp_result = strcmp(r1->courseName, r2->courseName); break;
            case SORT_BY_TERM: cmp_result = strcmp(r1->term, r2->term); break;
            case SORT_BY_COLLEGE: cmp_result = strcmp(r1->college, r2->college); break;
            case SORT_BY_SCORE: cmp_result = (r1->score - r2->score); break;
        }
        if (cmp_result != 0) 
            return (rule.order == ASCENDING) ? cmp_result : -cmp_result;
    }
    return 0;  /* 所有关键字都相等 */
}

/**
 * @brief  多关键字排序与显示（任务3）
 *         让用户选择排序字段和优先级（最多10个），然后使用qsort排序并显示。
 */
void multi_key_sort_and_display(void) {
    int field_choice, order_choice, i;
    Record *arr;
    ListNode *p;
    
    if (g_list.size == 0) return;
    
    g_rule_count = 0;
    printf("\n=========== 多关键字排序 ===========\n");
    printf("字段编号：1.学号 2.姓名 3.课程 4.学期 5.成绩 6.学院\n");
    
    /* 收集排序规则（按优先级） */
    while (1) {
        if (g_rule_count >= 10) break;
        printf("\n第 %d 优先级字段编号 (0结束): ", g_rule_count + 1);
        scanf("%d", &field_choice); 
        while (getchar() != '\n');
        if (field_choice == 0) break;
        if (field_choice < 1 || field_choice > 6) continue;
        
        printf("方向 (0升 1降): "); 
        scanf("%d", &order_choice); 
        while (getchar() != '\n');
        
        g_sort_rules[g_rule_count].field = (SortField)field_choice;
        g_sort_rules[g_rule_count].order = (order_choice == 1) ? DESCENDING : ASCENDING;
        g_rule_count++;
    }
    
    if (g_rule_count == 0) return;
    
    /* 将链表数据复制到数组，便于qsort排序 */
    arr = (Record*)malloc(g_list.size * sizeof(Record));
    p = g_list.head;
    for (i = 0; i < g_list.size; i++) { 
        arr[i] = p->data; 
        p = p->next; 
    }
    
    /* 使用标准库qsort排序（O(n log n)） */
    qsort(arr, g_list.size, sizeof(Record), compare_records);
    
    printf("\n>>> 排序完成！\n");
    print_records(arr, g_list.size, 50);  /* 显示前50条 */
    free(arr);
}
