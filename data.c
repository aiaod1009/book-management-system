#include "data.h"
#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// 添加新书到链表（头插法）
int add_book(BookNode **head, const char *isbn, const char *title, const char *author, int stock) {
    // 1. 检查参数非空
    if (head == NULL || isbn == NULL || title == NULL || author == NULL) {
        return -2; // 参数非法，返回错误标识
    }

    // 2. 调用search_by_isbn检查ISBN是否存在
    if (search_by_isbn(*head, isbn) != NULL) {
        return -1; // ISBN已存在，返回-1
    }
    
    return 0; // 添加成功，返回0
}

//通过ISBN精确查找图书
BookNode *search_by_isbn(BookNode *head, const char *isbn) {
    if (head == NULL || isbn == NULL) {
        return NULL;
    }

    BookNode *current = head;
    while (current != NULL) {
        if (strcmp(current->isbn, isbn) == 0) {
            return current; // 返回匹配节点
        }
        current = current->next;
    }
    return NULL; // 未找到
}

// 按关键词（书名/作者）模糊搜索
BookNode *search_by_keyword(BookNode *head, const char *keyword) {
    if (head == NULL || keyword == NULL || strlen(keyword) == 0) {
        return NULL;
    }

    BookNode *result_head = NULL; // 定义结果链表头指针
    BookNode *current = head;

    while (current != NULL) {
        // 匹配书名或作者包含关键词
        if (strstr(current->title, keyword) != NULL || strstr(current->author, keyword) != NULL) {
            // 将匹配的图书添加到结果链表（调用logic里的add_book1函数）
            add_book1(&result_head, current->isbn, current->title, current->author, current->stock, current->stock);
        }
        current = current->next;
    }

    return result_head; 
}

//销毁整个链表，释放内存
void destroy_list(BookNode **head) {
    if (head == NULL || *head == NULL) {
        return;
    }

    BookNode *current = *head;
    BookNode *next_node = NULL;

    // 遍历释放所有节点
    while (current != NULL) {
        next_node = current->next;
        free(current);
        current = next_node;
    }

    *head = NULL; // 置空头指针，避免野指针
    // printf("链表已销毁，内存已释放！\n");
}



