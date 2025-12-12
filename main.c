#include "data.h"
#include "logic.h"
#include "store.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PERSISTENCE_FILE "library_data.json"
#define MAX_INPUT_LEN 256
#define MAX_KEYWORD_LEN 100
#define MAX_FILENAME_LEN 50

/**
 * @brief 打印帮助信息
 */
void print_help() {
    printf("Library Management System\n");
    printf("Commands:\n");
    printf("  add <isbn> <title> <author> <stock>   - 添加一本新书\n");
    printf("  search <keyword>                      - 按关键词（书名/作者）搜索\n");
    printf("  isbn <isbn>                           - 按ISBN搜索\n");
    printf("  loan <isbn> <quantity>                - 记录借阅\n");
    printf("  sort stock                            - 按库存数量升序排列书籍\n");
    printf("  sort loan                             - 按借阅次数降序排列书籍\n");
    printf("  report                                - 生成统计报告\n");
    printf("  export csv <filename>                 - 将书籍导出为CSV文件\n");
    printf("  export json <filename>                - 将书籍导出为JSON文件\n");
    printf("  exit                                  - 退出程序\n");
}

/**
 * @brief 主命令解析循环
 */
void command_loop(BookNode **head) {// 接收图书链表的头指针（二级指针，用于修改链表）
    char input[MAX_INPUT_LEN];
    char cmd[20];

    while (1) {
        printf("> "); // 显示输入提示符
        if (!fgets(input, sizeof(input), stdin)) { // 读取用户输入
            break; // 输入错误或EOF时退出
        }

        // 移除末尾换行符
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        // 解析命令的第一个单词（比如"add"、"help"）
        if (sscanf(input, "%19s", cmd) != 1) {
            continue;// 空输入则忽略
        }
        // 处理exit命令
        if (strcmp(cmd, "exit") == 0) {
            break;
        } 
        // 处理help命令
        else if (strcmp(cmd, "help") == 0) {
            print_help();
        }
        // 处理add命令（匹配logic.c中的add_book1函数参数顺序）
        else if (strcmp(cmd, "add") == 0) {
            // TODO: 解析add命令参数
            // 用sscanf提取参数，调用add_book
            char isbn[20], title[100], author[50];
            int stock;
            int ret;
            // 先尝试匹配带引号的标题（格式：add <isbn> "<title>" <author> <stock>）
        ret = sscanf(input, "add %19s \"%99[^\"]\" %49s %d", 
                 isbn, title, author, &stock);
        if (ret != 4) {
            // 不匹配则尝试不带引号的标题（格式：add <isbn> <title> <author> <stock>）
            ret = sscanf(input, "add %19s %99s %49s %d", 
                         isbn, title, author, &stock);
        }
    
        // 检查参数是否完整 + 库存是否合法
        if (ret != 4 || stock <= 0) {
            printf("Invalid format. Usage: add <isbn> [\"<title>\"] <author> <stock>\n");
            continue;
        }
    
        // 检查ISBN是否重复
        if (search_by_isbn(*head, isbn)) {
            printf("Error: ISBN %s already exists\n", isbn);
            continue;
        }

        // 调用logic层的add_book1
        add_book1(head, title, author, isbn, stock, 0);
        printf("Book added successfully.\n");
        } 
        // 处理search命令（模糊搜索）
        else if (strcmp(cmd, "search") == 0) {
            // TODO: 解析搜索关键词，调用search_by_keyword
            char keyword[MAX_KEYWORD_LEN];
            if (sscanf(input, "search %99[^\n]", keyword) != 1) {
                printf("Invalid format. Usage: search <keyword>\n");
                continue;
            }
            BookNode *results = search_by_keyword(*head, keyword);
            if (results) {
                printf("Search results for '%s':\n", keyword);
                BookNode *curr = results;
                int count = 0;
                while (curr) {
                    count++;
                    printf("[%d] ISBN: %s, Title: %s, Author: %s, Stock: %d, Loaned: %d\n",
                           count, curr->isbn, curr->title, curr->author, curr->stock, curr->loaned);
                    curr = curr->next;
                }
                destroy_list(&results); // 释放结果链表
            } else {
                printf("No books found matching '%s'\n", keyword);
            }

        } 
        // 处理isbn查询命令（精确搜索）
        else if (strcmp(cmd, "isbn") == 0) {
            // TODO: 解析ISBN，调用search_by_isbn
            char isbn[20];
            if (sscanf(input, "isbn %19s", isbn) != 1) {
                printf("Invalid format. Usage: isbn <isbn>\n");
                continue;
            }
            // 调用data.c的search_by_isbn
            BookNode *book = search_by_isbn(*head, isbn);
            if (book) {
                printf("Found book:\n");
                printf("ISBN: %s\nTitle: %s\nAuthor: %s\nStock: %d\nLoaned: %d\n",
                       book->isbn, book->title, book->author, book->stock, book->loaned);
            } else {
                printf("No book found with ISBN: %s\n", isbn);
            }
        } 
        // 处理loan命令
        else if (strcmp(cmd, "loan") == 0) {
            // TODO: 解析loan命令，调用log_loan
            char isbn[20];
            int quantity;
            if (sscanf(input, "loan %19s %d", isbn, &quantity) != 2) {
                printf("Invalid format. Usage: loan <isbn> <quantity>\n");
                continue;
            }
            if (quantity <= 0) {
                printf("Quantity must be positive.\n");
                continue;
            }
            BookNode *book = search_by_isbn(*head, isbn);
            if (!book) {
                printf("Book with ISBN %s not found.\n", isbn);
                continue;
            }
            if (book->stock < quantity) {
                printf("Insufficient stock. Available: %d\n", book->stock);
                continue;
            }
            // 调用store层的log_loan记录借阅
            log_loan(isbn, quantity);
            book->stock -= quantity;
            book->loaned += quantity;
            printf("Loan recorded. New stock: %d, Total loaned: %d\n",
                   book->stock, book->loaned); 
        } 

        // 处理sort命令
        else if (strcmp(cmd, "sort") == 0) {
            // TODO: 解析排序类型
            char sort_type[10];
            if (sscanf(input, "sort %9s", sort_type) != 1) {
                printf("Invalid format. Usage: sort <stock|loan>\n");
                continue;
            }
            if (strcmp(sort_type, "stock") == 0) {
                sort_by_stock(head); // 调用logic.c的按库存排序
            } else if (strcmp(sort_type, "loan") == 0) {
                sort_by_loan(head); // 调用logic.c的按借阅量排序
            } else {
                printf("Invalid sort type. Use 'stock' or 'loan'.\n");
            }
        } 

        // 处理report命令
        else if (strcmp(cmd, "report") == 0) {
            generate_report(*head);//调用logic.c的报告生成函数
        } 
        // 处理export命令
        else if (strncmp(cmd, "export", 6) == 0) {
            // TODO: 解析导出命令
            char format[5], filename[MAX_FILENAME_LEN];
            if (sscanf(input, "export %4s %49s", format, filename) != 2) {
                printf("Invalid format. Usage: export <csv|json> <filename>\n");
                continue;
            }
            if (strcmp(format, "csv") == 0) {
                export_to_csv(filename, *head);
                printf("Data exported to %s\n", filename);
            } else if (strcmp(format, "json") == 0) {
                export_to_json(filename, *head);
                printf("Data exported to %s\n", filename);
            } else {
                printf("Invalid format. Use 'csv' or 'json'.\n");
            }
        } 
        // 处理未知命令
        else {
            printf("Unknown command. Type 'help' for usage.\n");
        }
    }
}

int main() {
    BookNode *head = NULL;

    // 尝试从持久化文件加载数据(加载已有图书数据)
    BookNode *loaded = load_books_from_json(PERSISTENCE_FILE);
    if (loaded) {
        head = loaded;
        printf("Loaded library data from %s\n", PERSISTENCE_FILE);
    } else {
        printf("No existing library data found. Starting with empty library.\n");
    }

    // 加载历史借阅记录
    load_loans(head);

    printf("Library Management System (Type 'help' for commands)\n");
    command_loop(&head);

    // 退出前保存数据
    printf("Saving library data to %s...\n", PERSISTENCE_FILE);
    if (persist_books_json(PERSISTENCE_FILE, head) == 0) {
        printf("Data saved successfully.\n");
    } else {
        printf("Warning: Failed to save library data.\n");
    }

    // 清理资源
    destroy_list(&head);
    printf("Exiting program.\n");

    return 0;
}
