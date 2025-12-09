#include "logic.h"
#include "data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: 实现图书添加（检查ISBN重复、动态分配内存）
BookNode *add_book1(BookNode **head, const char *title, const char *author, const char *isbn, int stock) {
    /* --------------------
     *       [要求]
     * 1. 检查ISBN是否重复
     * 2. 使用malloc创建节点
     * 3. 返回新链表头
     * -------------------- */
    // 1. logic层分配内存
    BookNode *new_node = (BookNode *)malloc(sizeof(BookNode));
    // 1. 用malloc创建节点
    if (new_node == NULL) { 
        // 检查内存分配是否成功
        printf("错误：内存分配失败，无法创建图书节点！\n");
        return *head; // 返回当前头指针
    }

    // 2. 给节点赋值
    strncpy(new_node->isbn, isbn, sizeof(new_node->isbn)-1);
    new_node->isbn[sizeof(new_node->isbn)-1] = '\0';

    strncpy(new_node->title, title, sizeof(new_node->title)-1);
    new_node->title[sizeof(new_node->title)-1] = '\0';

    strncpy(new_node->author, author, sizeof(new_node->author)-1);
    new_node->author[sizeof(new_node->author)-1] = '\0';
    new_node->stock = stock;
    new_node->loaned = 0;

    // 3. 调用data层检查ISBN是否重复
    int ret = add_book(head, isbn, title, author, stock);
    if (ret == 1) {
        free(new_node);// 释放已分配的内存，避免泄漏
        printf("错误：ISBN %s 已存在！\n", isbn);
        return *head; 
    }

    // 4. 手动插入节点到链表
    new_node->next = *head;
    *head = new_node;

    printf("成功添加图书：《%s》（ISBN：%s）\n", title, isbn); 
    return *head; // 这里返回*head（BookNode*，匹配函数返回值）
}

// TODO: 手写快速排序
// 通用 partition 函数（合并stock/loan逻辑）
int partition(BookNode** arr, int low, int high, int sort_type) {
    // 基准值二选一：sort_type：0=按stock升序，1=按loaned降序
    int pivot = (sort_type == 0) ? arr[high]->stock : arr[high]->loaned;
    int i = low - 1;

    for (int j = low; j <= high - 1; j++) {
        // 根据排序类型选择比较逻辑
        int cmp_result;
        if (sort_type == 0) {
            // stock升序：当前值 <= 基准值
            cmp_result = (arr[j]->stock <= pivot);
        } else {
            // loaned降序：当前值 >= 基准值
            cmp_result = (arr[j]->loaned >= pivot);
        }

        if (cmp_result) {
            i++;
            // 交换节点指针
            BookNode* temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // 交换基准节点
    BookNode* temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

void _quick_sort_arr(BookNode** arr, int low, int high, int sort_type) {
    if (low < high) {
        int pi = partition(arr, low, high, sort_type); 
        _quick_sort_arr(arr, low, pi - 1, sort_type);
        _quick_sort_arr(arr, pi + 1, high, sort_type);
    }
}

void quick_sort(BookNode** head, int sort_type) {
    /* --------------------
     *       [要求]
     * 1. 递归实现快速排序
     * 2. 链表头作为参数
     * -------------------- */
    if (head == NULL || *head == NULL) return;
    // 1：统计链表节点数量
    int count = 0;
    BookNode* cur = *head;
    while (cur != NULL) { count++; cur = cur->next; }
    // 2：创建数组
    BookNode** arr = (BookNode**)malloc(count * sizeof(BookNode*));
    if (arr == NULL) return;
    // 3：把链表节点塞进数组
    cur = *head;
    for (int i = 0; i < count; i++) { arr[i] = cur; cur = cur->next; }

    _quick_sort_arr(arr, 0, count - 1, sort_type);

    // 数组转回链表
    *head = arr[0];
    cur = *head;
    for (int i = 1; i < count; i++) { cur->next = arr[i]; cur = cur->next; }
    
    cur->next = NULL;

    free(arr);
}

// 实现sort_by_stock按库存量升序排序
void sort_by_stock(BookNode **head) {
    if (head == NULL || *head == NULL) {
        printf("错误：链表为空，无法按库存排序！\n");
        return;
    }
    quick_sort(head, 0); // 0=按stock升序
    printf("已按库存量升序排序完成！\n");
}

// 最终实现 sort_by_loan按借阅量降序排序
void sort_by_loan(BookNode **head) {
    if (head == NULL || *head == NULL) {
        printf("错误：链表为空，无法按借阅量排序！\n");
        return;
    }
    quick_sort(head, 1); // 1=按loaned降序
    printf("已按借阅量降序排序完成！\n");
}

// 生成统计报告：输出库存>5的图书数量、最热门图书（loaned最高）
void generate_report(BookNode *head) {
    if (head == NULL) {
        printf("统计报告：当前无图书数据！\n");
        return;
    }

    int stock_gt5_cnt = 0; // 库存>5的图书数量
    BookNode *hottest_book = head; // 最热门图书（默认第一个）

    // 遍历链表统计数据
    BookNode *cur = head;
    while (cur != NULL) {
        // 统计库存>5的图书
        if (cur->stock > 5) {
            stock_gt5_cnt++;
        }
        // 更新最热门图书（loaned更高则替换）
        if (cur->loaned > hottest_book->loaned) {
            hottest_book = cur;
        }
        cur = cur->next;
    }

    // 输出报告
    printf("\n===== 图书统计报告 =====\n");
    printf("1. 库存>5的图书数量：%d 本\n", stock_gt5_cnt);
    printf("2. 最热门图书：《%s》\n", hottest_book->title);
    printf("   作者：%s | 借阅量：%d 次\n", hottest_book->author, hottest_book->loaned);
    printf("=========================\n");
}