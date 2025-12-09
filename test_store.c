// tests/test_store.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "data.h"
#include "store.h"

/* 手工创建 BookNode 节点（不使用 add_book）以避免依赖项目中 add_book 的实现差异 */
static BookNode *create_book(const char *isbn, const char *title, const char *author, int stock, int loaned) {
    BookNode *n = (BookNode *)malloc(sizeof(BookNode));
    if (!n) return NULL;
    strncpy(n->isbn, isbn, sizeof(n->isbn) - 1); n->isbn[sizeof(n->isbn)-1] = '\0';
    strncpy(n->title, title, sizeof(n->title) - 1); n->title[sizeof(n->title)-1] = '\0';
    strncpy(n->author, author, sizeof(n->author) - 1); n->author[sizeof(n->author)-1] = '\0';
    n->stock = stock;
    n->loaned = loaned;
    n->next = NULL;
    return n;
}

static void print_list(BookNode *head, const char *label) {
    printf("---- %s ----\n", label);
    BookNode *cur = head;
    while (cur) {
        printf("ISBN: %s | Title: %s | Author: %s | Stock: %d | Loaned: %d\n",
               cur->isbn, cur->title, cur->author, cur->stock, cur->loaned);
        cur = cur->next;
    }
    printf("-----------------\n");
}

/* 检查文件是否存在并打印其大小（可选验证） */
static void check_file(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        printf("File '%s' exists, size=%ld bytes\n", path, (long)st.st_size);
    } else {
        printf("File '%s' does NOT exist\n", path);
    }
}

int main(void) {
    // 清理旧的借阅记录文件以便测试可重复
    remove("loan_records.bin");
    remove("test_books.csv");
    remove("test_books.json");
    remove("persist.json");

    // 手动构造链表： head -> b1 -> b2
    BookNode *head = create_book("9780001", "Book One", "Author A", 10, 1);
    BookNode *b2 = create_book("9780002", "Book Two", "Author B", 3, 0);
    if (!head || !b2) {
        fprintf(stderr, "内存分配失败\n");
        return 1;
    }
    head->next = b2;

    print_list(head, "初始书目");

    // 1) 导出 CSV
    printf("\n>> 导出 CSV: test_books.csv\n");
    export_to_csv("test_books.csv", head);
    check_file("test_books.csv");

    // 2) 导出 JSON（外部导出）
    printf("\n>> 导出 JSON: test_books.json\n");
    export_to_json("test_books.json", head);
    check_file("test_books.json");

    // 3) log_loan 写入二进制借阅记录
    printf("\n>> 写入借阅记录（log_loan）\n");
    log_loan("9780001", 2); // 给 Book One 借2本
    log_loan("9780001", 1); // 再借1本
    log_loan("9780002", 5); // 给 Book Two 借5本（可能使库存为负，用于测试）
    check_file("loan_records.bin");

    // 4) 在 load_loans 之前打印当前状态
    print_list(head, "加载借阅记录前");

    // 5) 调用 load_loans 更新链表（会读取 loan_records.bin 并更新 stock/loaned）
    printf("\n>> 加载借阅记录（load_loans）\n");
    load_loans(head);

    // 6) 加载后打印，验证 stock/loaned 是否更新
    print_list(head, "加载借阅记录后");

    // 7) persist_books_json（系统内部持久化） -> persist.json
    printf("\n>> 持久化到 persist.json（persist_books_json）\n");
    if (persist_books_json("persist.json", head) == 0) {
        check_file("persist.json");
    } else {
        printf("persist_books_json 返回失败\n");
    }

    // 8) 使用 load_books_from_json 恢复链表并打印（注意：store.c 中的实现可能会以不同参数顺序调用 add_book，导致字段错位）
    printf("\n>> 从 persist.json 恢复链表（load_books_from_json）\n");
    BookNode *loaded = load_books_from_json("persist.json");
    if (loaded) {
        print_list(loaded, "从 JSON 恢复的书目（loaded）");
    } else {
        printf("load_books_from_json 返回 NULL\n");
    }

    // 9) 清理内存（使用项目提供的 destroy_list）
    printf("\n>> 释放链表内存\n");
    destroy_list(&loaded);
    destroy_list(&head);

    printf("\n测试完成。\n");
    return 0;
}