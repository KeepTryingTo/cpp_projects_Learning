#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// 简单的URL解码函数（处理%20等编码）
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if (*src == '%' && (a = src[1]) && (b = src[2]) && isxdigit(a) && isxdigit(b)) {
            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'A' && a <= 'F') a -= 'A' - 10;
            else if (a >= 'a' && a <= 'f') a -= 'a' - 10;
            
            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'A' && b <= 'F') b -= 'A' - 10;
            else if (b >= 'a' && a <= 'f') b -= 'a' - 10;
            
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// 从查询字符串中获取参数值
char* get_param_value(const char *query, const char *param_name) {
    if (!query || !param_name) return NULL;
    
    char *query_copy = strdup(query);
    char *pair = strtok(query_copy, "&");
    char *value = NULL;
    
    while (pair) {
        char *eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            
            // 解码参数名
            char decoded_name[256];
            url_decode(decoded_name, pair);
            
            if (strcmp(decoded_name, param_name) == 0) {
                // 解码参数值
                char decoded_value[256];
                url_decode(decoded_value, eq + 1);
                value = strdup(decoded_value);
                break;
            }
        }
        pair = strtok(NULL, "&");
    }
    
    free(query_copy);
    return value;
}

// 将字符串转换为大写
void to_upper(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper(str[i]);
    }
}

int main() {
    // 获取环境变量
    char *method = getenv("REQUEST_METHOD");
    char *query_string = getenv("QUERY_STRING");
    
    // 默认颜色
    char *color = "blue";
    
    // 如果提供了color参数，则使用它
    if (method && strcmp(method, "GET") == 0 && query_string) {
        char *param_color = get_param_value(query_string, "color");
        if (param_color) {
            color = param_color;
        }
    }
    
    // 转换为大写用于标题
    char upper_color[256];
    strncpy(upper_color, color, sizeof(upper_color));
    to_upper(upper_color);
    
    // 输出HTTP响应
    printf("Content-Type: text/html\r\n\r\n");
    
    // 输出HTML
    printf("<html>\n");
    printf("<head><title>%s</title></head>\n", upper_color);
    printf("<body bgcolor=\"%s\">\n", color);
    printf("<h1>This is %s</h1>\n", color);
    printf("</body>\n");
    printf("</html>\n");
    
    // 清理内存
    if (color != "blue") {
        free(color);
    }
    
    return 0;
}