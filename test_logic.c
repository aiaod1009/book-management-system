#include "data.h"   // 包含BookNode结构体定义
#include "logic.h"  // 包含排序、统计函数声明
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 辅助函数：创建图书节点（模拟添加图书）
BookNode* create_book(const char* title, const char* author, const char* isbn, int stock, int loaned) {
    BookNode* node = (BookNode*)malloc(sizeof(BookNode));
    if (node == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    strncpy(node->title, title, sizeof(node->title)-1);
    strncpy(node->author, author, sizeof(node->author)-1);
    strncpy(node->isbn, isbn, sizeof(node->isbn)-1);
    node->stock = stock;
    node->loaned = loaned;
    node->next = NULL;
    return node;
}

// 辅助函数：打印链表（验证排序结果）
void print_book_list(BookNode* head) {
    if (head == NULL) {
        printf("链表为空！\n");
        return;
    }
    printf("\n===== 图书列表 =====\n");
    BookNode* cur = head;
    int idx = 1;
    while (cur != NULL) {
        printf("第%d本：《%s》 | 作者：%s | 库存：%d | 借阅量：%d\n",
               idx++, cur->title, cur->author, cur->stock, cur->loaned);
        cur = cur->next;
    }
    printf("====================\n");
}

// 辅助函数：释放链表内存（避免内存泄漏）
void free_book_list(BookNode** head) {
    if (head == NULL || *head == NULL) return;
    BookNode* cur = *head;
    while (cur != NULL) {
        BookNode* temp = cur;
        cur = cur->next;
        free(temp);
    }
    *head = NULL;
}

// 主测试函数
int main() {
    // 1. 创建测试链表
    BookNode* head = NULL;
    // 头插法添加测试数据
    head = create_book("算法导论", "Thomas H. Cormen", "9787111407010", 8, 56);
    head->next = create_book("数据结构", "严蔚敏", "9787302147510", 3, 120);
    head->next->next = create_book("C语言程序设计", "谭浩强", "9787302224281", 10, 89);
    head->next->next->next = create_book("Python编程", "Mark Lutz", "9787115487827", 5, 78);

    // 2. 打印原始列表
    printf("【原始图书列表】");
    print_book_list(head);

    // 3. 测试按库存升序排序
    sort_by_stock(&head);
    printf("\n【按库存升序排序后】");
    print_book_list(head);

    // 4. 测试按借阅量降序排序
    sort_by_loan(&head);
    printf("\n【按借阅量降序排序后】");
    print_book_list(head);

    // 5. 测试生成统计报告
    printf("\n【图书统计报告】");
    generate_report(head);

    // 6. 释放内存
    free_book_list(&head);
    printf("\n测试完成，内存已释放！\n");

    return 0;
}