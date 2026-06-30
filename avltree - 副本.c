/**
 * @file    avltree.c
 * @brief   AVL平衡二叉树的增删改查及平衡旋转实现。
 */

#include "avltree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int max(int a, int b) { return a > b ? a : b; }
static int height(AVLNode *n) { return n ? n->height : 0; }
static int balance_factor(AVLNode *n) { return n ? height(n->left) - height(n->right) : 0; }

static AVLNode* new_node(const Record *r) {
    AVLNode *n = (AVLNode*)malloc(sizeof(AVLNode));
    n->data = *r;
    n->height = 1;
    n->left = n->right = NULL;
    return n;
}

static AVLNode* rotate_right(AVLNode *y) {
    AVLNode *x = y->left;
    AVLNode *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

static AVLNode* rotate_left(AVLNode *x) {
    AVLNode *y = x->right;
    AVLNode *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

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

static AVLNode* min_node(AVLNode *node) {
    while (node->left) node = node->left;
    return node;
}

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

static AVLNode* search_node(AVLNode *node, const char *stuId) {
    int cmp;
    if (!node) return NULL;
    cmp = strcmp(stuId, node->data.stuId);
    if (cmp < 0) return search_node(node->left, stuId);
    if (cmp > 0) return search_node(node->right, stuId);
    return node;
}

static void traverse_node(AVLNode *node) {
    if (!node) return;
    traverse_node(node->left);
    print_record(&node->data);
    traverse_node(node->right);
}

static void get_all_node(AVLNode *node, Record **arr, int *idx) {
    if (!node) return;
    get_all_node(node->left, arr, idx);
    (*arr)[(*idx)++] = node->data;
    get_all_node(node->right, arr, idx);
}

static void destroy_node(AVLNode *node) {
    if (!node) return;
    destroy_node(node->left);
    destroy_node(node->right);
    free(node);
}

/* ---------- 外部接口 ---------- */
void InitAVL(AVLTree *tree) {
    tree->root = NULL;
    tree->size = 0;
}

int AVLInsert(AVLTree *tree, const Record *r) {
    tree->root = insert_node(tree->root, r);
    tree->size++;
    return 0;
}

int AVLDelete(AVLTree *tree, const char *stuId, const char *courseId) {
    if (!search_node(tree->root, stuId)) return -1;
    tree->root = delete_node(tree->root, stuId, courseId);
    tree->size--;
    return 0;
}

int AVLUpdate(AVLTree *tree, const char *stuId, const char *courseId, int newScore) {
    AVLNode *found = search_node(tree->root, stuId);
    if (!found || strcmp(found->data.courseId, courseId) != 0) return -1;
    found->data.score = newScore;
    return 0;
}

AVLNode* AVLSearch(AVLTree *tree, const char *stuId) {
    return search_node(tree->root, stuId);
}

void AVLTraverse(AVLTree *tree) {
    if (tree->size == 0) {
        printf(">> AVL树为空。\n");
        return;
    }
    print_record_header();
    traverse_node(tree->root);
    printf(">> 共 %d 条记录。\n", tree->size);
}

void DestroyAVL(AVLTree *tree) {
    destroy_node(tree->root);
    tree->root = NULL;
    tree->size = 0;
}

int AVLSize(AVLTree *tree) {
    return tree->size;
}

void AVLGetAll(AVLTree *tree, Record **arr, int *n) {
    *n = tree->size;
    *arr = (Record*)malloc(*n * sizeof(Record));
    if (!*arr) { *n = 0; return; }
    int idx = 0;
    get_all_node(tree->root, arr, &idx);
}
