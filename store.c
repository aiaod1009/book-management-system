#include "logic.h"
#include "data.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 1. 记录借阅操作到二进制文件（匹配store.h声明）
void log_loan(const char *isbn, int quantity) {
    if (isbn == NULL || quantity <= 0) return;

    // 打开二进制追加文件
    FILE *fp = fopen("loan_records.bin", "ab");
    if (fp == NULL) {
        printf("错误：无法打开借阅记录文件\n");
        return;
    }

    // 定义借阅记录结构体（匹配ISBN+数量+时间）
    typedef struct {
        char isbn[20];   // 匹配BookNode的isbn字段长度
        int quantity;    // 借阅数量
        char time[30];   // 借阅时间
    } LoanRecord;

    LoanRecord record;
    // 赋值（严格匹配字段）
    strncpy(record.isbn, isbn, sizeof(record.isbn)-1);
    record.isbn[sizeof(record.isbn)-1] = '\0';
    record.quantity = quantity;
    // 生成当前时间
    time_t now = time(NULL);
    strftime(record.time, sizeof(record.time), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // 写入二进制数据
    fwrite(&record, sizeof(LoanRecord), 1, fp);
    fclose(fp);
}

// 2. 从二进制文件加载借阅记录（匹配store.h声明）
void load_loans(BookNode *head) {
    if (head == NULL) return;

    FILE *fp = fopen("loan_records.bin", "rb");
    if (fp == NULL) {
        printf("提示：暂无借阅记录文件\n");
        return;
    }

    typedef struct {
        char isbn[20];
        int quantity;
        char time[30];
    } LoanRecord;

    LoanRecord record;
    // 读取并更新图书状态
    while (fread(&record, sizeof(LoanRecord), 1, fp) == 1) {
        BookNode *current = head;
        while (current != NULL) {
            if (strcmp(current->isbn, record.isbn) == 0) {
                if (record.quantity <= 0) {
                    // 忽略非法数量
                    printf("警告：忽略非法借阅数量 %d for ISBN %s\n", record.quantity, record.isbn);
                } else if (current->stock < record.quantity) {
                    // 如果借阅量大于库存，跳过并打印警告（也可按需求改为限额借出）
                    printf("警告：ISBN %s 借阅 %d 超过库存 %d，已跳过该记录\n",
                           record.isbn, record.quantity, current->stock);
                } else {
                    current->loaned += record.quantity;
                    current->stock -= record.quantity;
                }
                break;
            }
            current = current->next;
        }
    }
    fclose(fp);
}

// 3. 持久化图书到JSON文件（匹配store.h声明）
int persist_books_json(const char *filename, BookNode *head) {
    if (filename == NULL || head == NULL) return -1;

    // 创建JSON数组
    cJSON *books_array = cJSON_CreateArray();
    BookNode *current = head;

    while (current != NULL) {
        cJSON *book_obj = cJSON_CreateObject();
        // 严格匹配BookNode的字段（isbn/title/author/stock/loaned）
        cJSON_AddStringToObject(book_obj, "isbn", current->isbn);
        cJSON_AddStringToObject(book_obj, "title", current->title);
        cJSON_AddStringToObject(book_obj, "author", current->author);
        cJSON_AddNumberToObject(book_obj, "stock", current->stock);
        cJSON_AddNumberToObject(book_obj, "loaned", current->loaned);

        cJSON_AddItemToArray(books_array, book_obj);
        current = current->next;
    }

    // 生成JSON字符串
    char *json_str = cJSON_Print(books_array);
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        free(json_str);
        cJSON_Delete(books_array);
        return -1;
    }
    fputs(json_str, fp);
    fclose(fp);

    // 释放内存（用free，不是destroy_list）
    free(json_str);
    cJSON_Delete(books_array);
    return 0;
}

// 4. 从JSON文件加载图书（匹配store.h声明）
BookNode *load_books_from_json(const char *filename) {
    if (filename == NULL) return NULL;

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return NULL;

    // 读取文件内容
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *json_content = (char *)malloc(file_size + 1);
    fread(json_content, 1, file_size, fp);
    json_content[file_size] = '\0';
    fclose(fp);

    // 解析JSON
    cJSON *books_array = cJSON_Parse(json_content);
    free(json_content);  // 释放文件内容（用free）
    if (books_array == NULL) return NULL;

    // 构建图书链表
    BookNode *head = NULL;
    int array_size = cJSON_GetArraySize(books_array);
    for (int i = 0; i < array_size; i++) {
        cJSON *book_obj = cJSON_GetArrayItem(books_array, i);
        // 读取JSON字段（匹配BookNode）
        cJSON *isbn = cJSON_GetObjectItem(book_obj, "isbn");
        cJSON *title = cJSON_GetObjectItem(book_obj, "title");
        cJSON *author = cJSON_GetObjectItem(book_obj, "author");
        cJSON *stock = cJSON_GetObjectItem(book_obj, "stock");
        cJSON *loaned = cJSON_GetObjectItem(book_obj, "loaned");

        // 校验字段
        if (!cJSON_IsString(isbn) || !cJSON_IsString(title) ||
            !cJSON_IsString(author) || !cJSON_IsNumber(stock) || !cJSON_IsNumber(loaned)) {
            continue;
        }

        // 用logic里的add_book添加节点（匹配你的函数）
        add_book1(&head, title->valuestring, author->valuestring, isbn->valuestring, stock->valueint);
        // 手动设置loaned字段

        BookNode *current = search_by_isbn(head, isbn->valuestring);
        if (current != NULL) {
            current->loaned = loaned->valueint;
        }
    }

    cJSON_Delete(books_array);
    return head;
}

// 5. 导出图书到CSV文件（匹配store.h声明）
void export_to_csv(const char *filename, BookNode *head) {
    if (filename == NULL || head == NULL) return;

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法创建CSV文件\n");
        return;
    }

    // 写入CSV表头（匹配BookNode字段）
    fprintf(fp, "ISBN,书名,作者,库存,已借出\n");
    BookNode *current = head;
    while (current != NULL) {
        fprintf(fp, "%s,%s,%s,%d,%d\n",
                current->isbn,
                current->title,
                current->author,
                current->stock,
                current->loaned);
        current = current->next;
    }
    fclose(fp);
}

// 6. 导出图书到JSON文件（匹配store.h声明）
void export_to_json(const char *filename, BookNode *head) {
    // 复用persist_books_json的逻辑（功能一致）
    persist_books_json(filename, head);
}