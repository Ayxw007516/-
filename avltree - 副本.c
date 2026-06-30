/**
 * @file    avltree.c
 * @brief   AVL平衡二叉树的增删改查及平衡旋转实现。
 */

#include "avltree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * @brief 取两个整数中的较大值
 * @param a 整数a
 * @param b 整数b
 * @return 较大的整数
 */
static int max(int a, int b) { return a > b ? a : b; }

/**
 * @brief 获取节点高度
 * @param n 目标节点指针
 * @return 节点高度，空节点返回0
 */
static int height(AVLNode *n) { return n ? n->height : 0; }


/**
 * @brief 计算节点的平衡因子（左子树高度 - 右子树高度）
 * @param n 目标节点指针
 * @return 平衡因子值，空节点返回0
 */
static int balance_factor(AVLNode *n) { return n ? height(n->left) - height(n->right) : 0; }

/**
 * @brief 创建新的AVL树节点
 * @param r 记录数据指针
 * @return 新创建的节点指针，分配失败返回NULL
 */
static AVLNode* new_node(const Record *r) {
    AVLNode *n = (AVLNode*)malloc(sizeof(AVLNode));
    n->data = *r;
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

/**
 * @brief 右旋操作（LL旋转）
 * @param y 失衡节点
 * @return 旋转后新的子树根节点
 */
static AVLNode* rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

/**
 * @brief 左旋操作（RR旋转）
 * @param x 失衡节点
 * @return 旋转后新的子树根节点
 */
static AVLNode* rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

/**
 * @brief AVL树插入节点（递归实现）
 * @param node 当前子树根节点
 * @param r 待插入的记录指针
 * @return 插入后新的子树根节点
 */
static AVLNode* insert_node(AVLNode *node, const Record *r) {
    int cmp;
    int bf;
    if (!node) return new_node(r);
    cmp = strcmp(r->stuId, node->data.stuId);
    if (cmp < 0)
        node->left = insert_node(node->left, r);
    else if (cmp > 0)
        node->right = insert_node(node->right, r);
    else {
        node->data = *r;
        return node;
    }
    node->height = max(height(node->left), height(node->right)) + 1;
    bf = balance_factor(node);
    if (bf > 1 && strcmp(r->stuId, node->left->data.stuId) < 0)
        return rotate_right(node);
    if (bf < -1 && strcmp(r->stuId, node->right->data.stuId) > 0)
        return rotate_left(node);
    if (bf > 1 && strcmp(r->stuId, node->left->data.stuId) > 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1 && strcmp(r->stuId, node->right->data.stuId) < 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

/**
 * @brief 查找子树中的最小节点
 * @param node 子树根节点
 * @return 最小节点指针
 */
static AVLNode* min_node(AVLNode *node) {
    while (node->left) node = node->left;
    return node;
}

/**
 * @brief AVL树删除节点（递归实现）
 * @param node 当前子树根节点
 * @param stuId 待删除记录的学号
 * @param courseId 待删除记录的课程编号
 * @return 删除后新的子树根节点
 */
static AVLNode* delete_node(AVLNode *node, const char *stuId, const char *courseId) {
    int cmp;
    int bf;
    if (!node) return NULL;
    cmp = strcmp(stuId, node->data.stuId);
    if (cmp < 0)
        node->left = delete_node(node->left, stuId, courseId);
    else if (cmp > 0)
        node->right = delete_node(node->right, stuId, courseId);
    else {
        if (strcmp(courseId, node->data.courseId) != 0) return node;
        if (!node->left || !node->right) {
            AVLNode *tmp = node->left ? node->left : node->right;
            free(node);
            return tmp;
        } else {
            AVLNode *tmp = min_node(node->right);
            node->data = tmp->data;
            node->right = delete_node(node->right, tmp->data.stuId, tmp->data.courseId);
        }
    }
    if (!node) return NULL;
    node->height = max(height(node->left), height(node->right)) + 1;
    bf = balance_factor(node);
    if (bf > 1 && balance_factor(node->left) >= 0) return rotate_right(node);
    if (bf > 1 && balance_factor(node->left) < 0) {
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }
    if (bf < -1 && balance_factor(node->right) <= 0) return rotate_left(node);
    if (bf < -1 && balance_factor(node->right) > 0) {
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }
    return node;
}

/**
 * @brief AVL树按学号查找节点（递归实现）
 * @param node 当前子树根节点
 * @param stuId 待查找的学号
 * @return 找到的节点指针，未找到返回NULL
 */
static AVLNode* search_node(AVLNode *node, const char *stuId) {
    int cmp;
    if (!node) return NULL;
    cmp = strcmp(stuId, node->data.stuId);
    if (cmp < 0) return search_node(node->left, stuId);
    if (cmp > 0) return search_node(node->right, stuId);
    return node;
}

/**
 * @brief 中序遍历AVL树并打印所有记录
 * @param node 当前子树根节点
 */
static void traverse_node(AVLNode *node) {
    if (!node) return;
    traverse_node(node->left);
    print_record(&node->data);
    traverse_node(node->right);
}

/**
 * @brief 中序遍历AVL树，收集所有记录到数组
 * @param node 当前子树根节点
 * @param arr 记录数组指针（输出）
 * @param idx 当前索引指针（输出）
 */
static void get_all_node(AVLNode *node, Record **arr, int *idx) {
    if (!node) return;
    get_all_node(node->left, arr, idx);
    (*arr)[(*idx)++] = node->data;
    get_all_node(node->right, arr, idx);
}


/**
 * @brief 递归释放AVL树所有节点
 * @param node 当前子树根节点
 */
static void destroy_node(AVLNode *node) {
    if (!node) return;
    destroy_node(node->left);
    destroy_node(node->right);
    free(node);
}

/* ---------- 外部接口 ---------- */
/**
 * @brief 初始化AVL树
 * @param tree 目标AVL树指针
 */
void InitAVL(AVLTree *tree) {
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 向AVL树插入一条记录
 * @param tree 目标AVL树指针
 * @param r 待插入记录指针
 * @return 成功返回0
 */
int AVLInsert(AVLTree *tree, const Record *r) {
    tree->root = insert_node(tree->root, r);
    tree->size++;
    return 0;
}

/**
 * @brief 从AVL树删除一条记录
 * @param tree 目标AVL树指针
 * @param stuId 待删除记录的学号
 * @param courseId 待删除记录的课程编号
 * @return 成功返回0，记录不存在返回-1
 */
int AVLDelete(AVLTree *tree, const char *stuId, const char *courseId) {
    if (!search_node(tree->root, stuId)) return -1;
    tree->root = delete_node(tree->root, stuId, courseId);
    tree->size--;
    return 0;
}

/**
 * @brief 修改AVL树中指定记录的成绩
 * @param tree 目标AVL树指针
 * @param stuId 待修改记录的学号
 * @param courseId 待修改记录的课程编号
 * @param newScore 新的成绩
 * @return 成功返回0，记录不存在返回-1
 */
int AVLUpdate(AVLTree *tree, const char *stuId, const char *courseId, int newScore) {
    AVLNode *found = search_node(tree->root, stuId);
    if (!found || strcmp(found->data.courseId, courseId) != 0) return -1;
    found->data.score = newScore;
    return 0;
}

/**
 * @brief 按学号在AVL树中查找记录
 * @param tree 目标AVL树指针
 * @param stuId 待查找的学号
 * @return 找到的节点指针，未找到返回NULL
 */
AVLNode* AVLSearch(AVLTree *tree, const char *stuId) {
    return search_node(tree->root, stuId);
}

/**
 * @brief 中序遍历AVL树并打印所有记录
 * @param tree 目标AVL树指针
 */
void AVLTraverse(AVLTree *tree) {
    if (tree->size == 0) {
        printf(">> AVL树为空。\n");
        return;
    }
    print_record_header();
    traverse_node(tree->root);
    printf(">> 共 %d 条记录。\n", tree->size);
}

/**
 * @brief 销毁AVL树，释放所有节点内存
 * @param tree 目标AVL树指针
 */
void DestroyAVL(AVLTree *tree) {
    destroy_node(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

/**
 * @brief 获取AVL树中记录总数
 * @param tree 目标AVL树指针
 * @return 记录总数
 */
int AVLSize(AVLTree *tree) {
    return tree->size;
}

/**
 * @brief 获取AVL树中所有记录（按中序顺序）
 * @param tree 目标AVL树指针
 * @param arr 输出参数：记录数组指针
 * @param n 输出参数：记录总数
 */
void AVLGetAll(AVLTree *tree, Record **arr, int *n) {
    *n = tree->size;
    *arr = (Record*)malloc(*n * sizeof(Record));
    if (!*arr) { *n = 0; return; }
    int idx = 0;
    get_all_node(tree->root, arr, &idx);
}
