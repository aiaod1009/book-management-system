// test_data.c - 专门测试data模块的main函数
#include "data.h"
#include <stdio.h>

int main() {
    // 1. 初始化主链表头指针（必须置NULL）
    BookNode *main_head = NULL;
    printf("===== 开始测试data模块 =====\n");

    // 2. 测试添加图书
    printf("\n【测试添加图书】\n");
    // 正常添加
    add_book(&main_head, "9787532781234", "三体", "刘慈欣", 5);
    add_book(&main_head, "9787115588644", "Python编程", "埃里克·马瑟斯", 3);
    // 测试重复ISBN
    add_book(&main_head, "9787532781234", "三体（重复）", "刘慈欣", 10);
    // 测试非法ISBN（非13位数字）
    add_book(&main_head, "123456", "无效ISBN", "测试作者", 2);

    // 3. 测试按ISBN精确查询
    printf("\n【测试精确查询（ISBN：9787532781234）】\n");
    BookNode *found = search_by_isbn(main_head, "9787532781234");
    if (found) {
        printf("找到图书：%s - %s（库存：%d）\n", found->isbn, found->title, found->stock);
    } else {
        printf("未找到该图书\n");
    }

    // 4. 测试模糊搜索
    printf("\n【测试模糊搜索（关键词：刘慈欣）】\n");
    BookNode *result = search_by_keyword(main_head, "刘慈欣");
    // 遍历打印搜索结果
    BookNode *current = result;
    int count = 0;
    while (current != NULL) {
        count++;
        printf("匹配结果%d：%s - %s\n", count, current->isbn, current->title);
        current = current->next;
    }
    if (count == 0) {
        printf("无匹配结果\n");
    }

    // 5. 销毁结果链表（用完立即销毁，避免泄漏）
    destroy_list(&result);

    // 6. 销毁主链表（程序退出前）
    printf("\n【销毁主链表】\n");
    destroy_list(&main_head);

    printf("\n===== 测试结束 =====\n");
    return 0;
}