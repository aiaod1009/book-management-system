#include "logic.h"
#include "data.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 1. 记录借阅操作到二进制文件
void log_loan(const char *isbn, int quantity) {
    if (isbn == NULL || quantity <= 0) return;

    // 打开二进制追加文件
    FILE *fp = fopen("loan_records.bin", "ab");
    if (fp == NULL) {
        printf("错误：暂无借阅记录文件\n");
        return;
    }

    // 定义借阅记录结构体（ISBN+数量+时间）
    typedef struct {
        char isbn[20];   // isbn
        int quantity;    // 借阅数量
        char time[30];   // 借阅时间
    } LoanRecord;

    LoanRecord record; //临时存储 “待写入文件的单条借阅记录” 的容器
    // 赋值ISBN（避免数组越界）
    strncpy(record.isbn, isbn, sizeof(record.isbn)-1);
    record.isbn[sizeof(record.isbn)-1] = '\0';
    // 赋值借阅数量
    record.quantity = quantity;
    // 生成当前时间
    time_t now = time(NULL);
    strftime(record.time, sizeof(record.time), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // 写入二进制数据
    fwrite(&record, sizeof(LoanRecord), 1, fp);
    fclose(fp);
}

// 2. 从二进制文件加载借阅记录
void load_loans(BookNode *head) {
    if (head == NULL) return; // 图书链表是空的，直接退出

    FILE *fp = fopen("loan_records.bin", "rb");
    if (fp == NULL) {
        printf("提示：暂无借阅记录文件\n");
        return;
    }
    //结构体和log_loan中定义一样（保证读写格式一致）
    typedef struct {
        char isbn[20];
        int quantity;
        char time[30];
    } LoanRecord;

    LoanRecord record; //临时存储 “从文件中读取的单条借阅记录” 的容器
    // 循环读取二进制文件里的每条借阅记录
    while (fread(&record, sizeof(LoanRecord), 1, fp) == 1) {
        BookNode *current = head;
        while (current != NULL) {
            // 遍历图书链表，找和当前借阅记录ISBN匹配的图书
            if (strcmp(current->isbn, record.isbn) == 0) {
                if (record.quantity <= 0) {
                    // 忽略非法借阅数量
                    printf("警告：忽略非法借阅数量 %d for ISBN %s\n", record.quantity, record.isbn);
                } else if (current->stock < record.quantity) {
                    // 如果借阅量大于库存，跳过并打印警告
                    printf("警告：ISBN %s 借阅 %d 超过库存 %d，已跳过该记录\n",
                           record.isbn, record.quantity, current->stock);
                } else {
                   // 匹配成功，更新这本书的库存和已借出量 
                    current->loaned += record.quantity; // 已借出数量 += 借阅数量
                    current->stock -= record.quantity; // 库存数量 -= 借阅数量
                }
                break; // 找到匹配的图书，退出图书链表的遍历
            }
            current = current->next; // 找下一本
        }
    }
    fclose(fp);
}

// 3. 持久化图书到JSON文件
int persist_books_json(const char *filename, BookNode *head) {
    if (filename == NULL || head == NULL) return -1; //文件名或链表为空时返回-1表示失败

    // 创建JSON根对象
     cJSON *root = cJSON_CreateObject();
    // 构建metadata子对象
    cJSON *metadata = cJSON_CreateObject();
    // 添加版本号
    cJSON_AddStringToObject(metadata, "version", "1.0");
    // 添加创建时间（格式化时间字符串，比时间戳更易读）
    time_t now = time(NULL);
    char create_time[30];
    strftime(create_time, sizeof(create_time), "%Y-%m-%d %H:%M:%S", localtime(&now));
    cJSON_AddStringToObject(metadata, "created", create_time);
    // 把metadata添加到根对象
    cJSON_AddItemToObject(root, "metadata", metadata);

    // 遍历图书链表，构建books数组
    cJSON *books_array = cJSON_CreateArray();
    BookNode *current = head;
    while (current != NULL) {
        cJSON *book_obj = cJSON_CreateObject();
        //字符串类型
        cJSON_AddStringToObject(book_obj, "isbn", current->isbn);
        cJSON_AddStringToObject(book_obj, "title", current->title);
        cJSON_AddStringToObject(book_obj, "author", current->author);
        //数字类型
        cJSON_AddNumberToObject(book_obj, "stock", current->stock);
        cJSON_AddNumberToObject(book_obj, "loaned", current->loaned);

        cJSON_AddItemToArray(books_array, book_obj);
        current = current->next; // 继续遍历下一本图书
    }
    // 把books数组添加到根对象
    cJSON_AddItemToObject(root, "books", books_array);

    // 生成JSON字符串并写入文件
    char *json_str = cJSON_Print(root);
    // 打开文件，写入JSON字符串
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        free(json_str);
        cJSON_Delete(root);
        return -1;
    }
    fputs(json_str, fp); // 把JSON字符串写入文件
    fclose(fp);

    free(json_str);   // 释放JSON字符串的内存
    cJSON_Delete(root); // 释放JSON数组的内存
    return 0;
}

// 4. 从JSON文件加载图书
BookNode *load_books_from_json(const char *filename) {
    if (filename == NULL) return NULL;  // 文件名空，返回空链表

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) return NULL;  // 文件打开失败，返回空链表

    // 读取文件总大小
    // 指针移到文件末尾
    fseek(fp, 0, SEEK_END);
    // 获得偏移量
    long file_size = ftell(fp); 
    // 指针移回文件开头
    fseek(fp, 0, SEEK_SET);  
    // 分配内存存储文件内容
    char *json_content = (char *)malloc(file_size + 1);
    fread(json_content, 1, file_size, fp);  // 读取文件内容到json_content
    json_content[file_size] = '\0';   // 手动加字符串结束符
    fclose(fp);

    // 解析根对象
    cJSON *root = cJSON_Parse(json_content);
    free(json_content);   // 解析完成后，释放文件内容的内存
    if (root == NULL) return NULL;  // JSON解析失败，返回空链表

    // 从根对象中取出books数组
    cJSON *books_array = cJSON_GetObjectItem(root, "books");
    if (books_array == NULL || !cJSON_IsArray(books_array)) {  // 校验是否是数组
        cJSON_Delete(root);
        return NULL;
    }
    

    BookNode *head = NULL; // 新链表的头指针
    int array_size = cJSON_GetArraySize(books_array);  // 获取JSON数组的长度（图书数量）
    for (int i = 0; i < array_size; i++) {
        cJSON *book_obj = cJSON_GetArrayItem(books_array, i);
        // 从JSON对象中取出对应字段的cJSON节点
        cJSON *isbn = cJSON_GetObjectItem(book_obj, "isbn");
        cJSON *title = cJSON_GetObjectItem(book_obj, "title");
        cJSON *author = cJSON_GetObjectItem(book_obj, "author");
        cJSON *stock = cJSON_GetObjectItem(book_obj, "stock");
        cJSON *loaned = cJSON_GetObjectItem(book_obj, "loaned");

        // 校验字段
        if (!cJSON_IsString(isbn) || !cJSON_IsString(title) ||
            !cJSON_IsString(author) || !cJSON_IsNumber(stock) || !cJSON_IsNumber(loaned)) {
            continue;// 如果字段类型不匹配，跳过这条记录
        }

        // 调用logic里的add_book1函数，把图书加入链表
        add_book1(&head, title->valuestring, author->valuestring, isbn->valuestring, stock->valueint,loaned->valueint);
    }

    cJSON_Delete(root); // 释放cJSON数组的内存
    return head;  // 返回重建后的图书链表头指针
}

// 5. 导出图书到CSV文件
void export_to_csv(const char *filename, BookNode *head) {
    if (filename == NULL || head == NULL) return;  //文件名或图书链表为空，直接退出

    //打开CSV文件
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        printf("错误：无法创建CSV文件\n");
        return;
    }

    // 写入CSV表头
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

// 6. 导出图书到JSON文件
void export_to_json(const char *filename, BookNode *head) {
    // 复用persist_books_json的逻辑
    persist_books_json(filename, head);
}