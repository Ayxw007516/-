/**
 * @file    avltree.h
 * @brief   AVL平衡二叉树的数据结构与接口声明。
 */

#ifndef AVLTREE_H
#define AVLTREE_H

#include "record.h"

/* AVL树节点 */
typedef struct AVLNode {
    Record data;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

/* AVL树结构 */
typedef struct {
    AVLNode *root;
    int size;
} AVLTree;

/* 接口函数 */
void InitAVL(AVLTree *tree);
int  AVLInsert(AVLTree *tree, const Record *r);
int  AVLDelete(AVLTree *tree, const char *stuId, const char *courseId);
int  AVLUpdate(AVLTree *tree, const char *stuId, const char *courseId, int newScore);
AVLNode* AVLSearch(AVLTree *tree, const char *stuId);
void AVLTraverse(AVLTree *tree);
void DestroyAVL(AVLTree *tree);
int  AVLSize(AVLTree *tree);
void AVLGetAll(AVLTree *tree, Record **arr, int *n);

#endif
