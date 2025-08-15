#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 解析查询字符串（简单实现，不处理URL编码）
void parse_query(const char *query, char ***keys, char ***values, int *count) {
    *count = 0;
    if (!query || !*query) return;

    // 临时存储键值对
    char *temp_keys[100];
    char *temp_values[100];
    
    char *query_copy = strdup(query);
    char *pair = strtok(query_copy, "&");
    
    while (pair && *count < 100) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            temp_keys[*count] = pair;
            temp_values[*count] = eq + 1;
        } else {
            temp_keys[*count] = pair;
            temp_values[*count] = "";
        }
        (*count)++;
        pair = strtok(NULL, "&");
    }
    
    // 分配内存并复制结果
    *keys = malloc(*count * sizeof(char*));
    *values = malloc(*count * sizeof(char*));
    for (int i = 0; i < *count; i++) {
        (*keys)[i] = strdup(temp_keys[i]);
        (*values)[i] = strdup(temp_values[i]);
    }
    
    free(query_copy);
}

int main() {
    // 获取请求方法
    char *method = getenv("REQUEST_METHOD");
    char *query_string = getenv("QUERY_STRING");
    
    // 解析参数（简单实现）
    char **keys = NULL;
    char **values = NULL;
    int param_count = 0;
    
    if (method && strcmp(method, "GET") == 0 && query_string) {
        parse_query(query_string, &keys, &values, &param_count);
    }
    // 注意：POST处理需要从stdin读取CONTENT_LENGTH长度的数据
    
    // 输出HTTP响应头
    printf("Content-Type: text/html\r\n\r\n");
    
    // 输出HTML内容
    printf("<html>\n");
    printf("<head><title>Example CGI script</title></head>\n");
    printf("<body bgcolor=\"red\">\n");
    printf("<h1>CGI Example</h1>\n");
    printf("<p>This is an example of CGI</p>\n");
    printf("<p>Parameters given to this script:</p>\n");
    printf("<ul>\n");
    
    // 输出参数列表
    for (int i = 0; i < param_count; i++) {
        printf("<li>%s: %s</li>\n", keys[i], values[i]);
    }
    printf("</ul>\n");
    printf("</body>\n");
    printf("</html>\n");
    
    // 释放内存
    for (int i = 0; i < param_count; i++) {
        free(keys[i]);
        free(values[i]);
    }
    free(keys);
    free(values);
    
    return 0;
}