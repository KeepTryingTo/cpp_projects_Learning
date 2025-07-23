#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/queue.h>
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/http_struct.h>
#include <event2/http_compat.h>
#include <event2/keyvalq_struct.h>
#include <limits.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MYHTTPD_SIGNATURE   "myhttpd v 1.0"
#define DEFAULT_PORT        8080
#define DEFAULT_BACKLOG    128
#define DEFAULT_TIMEOUT    120

// 配置结构体
struct httpd_config {
    char *listen_addr;
    int listen_port;
    int daemon_mode;
    int timeout;
    char *doc_root;
};

// 全局变量
static struct event_base *base = NULL;
static struct evhttp *httpd = NULL;

// 显示帮助信息
void show_help(const char *progname) {
    fprintf(stderr, "Usage: %s [options]\n"
        "Options:\n"
        "  -l <addr>  Listen address (default: 0.0.0.0)\n"
        "  -p <port>  Listen port (default: %d)\n"
        "  -d         Run as daemon\n"
        "  -t <sec>   Request timeout (default: %d)\n"
        "  -r <path>  Document root directory\n"
        "  -h         Show this help\n",
        progname, DEFAULT_PORT, DEFAULT_TIMEOUT);
}

// 初始化配置
void init_config(struct httpd_config *cfg) {
    cfg->listen_addr = strdup("0.0.0.0");
    cfg->listen_port = DEFAULT_PORT;
    cfg->daemon_mode = 0;
    cfg->timeout = DEFAULT_TIMEOUT;
    cfg->doc_root = NULL;
}

// 释放配置
void free_config(struct httpd_config *cfg) {
    if (cfg->listen_addr) free(cfg->listen_addr);
    if (cfg->doc_root) free(cfg->doc_root);
}

// 解析命令行参数
int parse_args(int argc, char **argv, struct httpd_config *cfg) {
    int c;
    while ((c = getopt(argc, argv, "l:p:dt:r:h")) != -1) {
        switch (c) {
            case 'l':
                free(cfg->listen_addr);
                cfg->listen_addr = strdup(optarg);
                break;
            case 'p':
                cfg->listen_port = atoi(optarg);
                break;
            case 'd':
                cfg->daemon_mode = 1;
                break;
            case 't':
                cfg->timeout = atoi(optarg);
                break;
            case 'r':
                free(cfg->doc_root);
                cfg->doc_root = strdup(optarg);
                break;
            case 'h':
            default:
                show_help(argv[0]);
                return -1;
        }
    }
    return 0;
}

// 信号处理
void signal_handler(int sig) {
    switch (sig) {
        case SIGTERM:// 终止进程
        case SIGHUP: 
        case SIGQUIT:
        case SIGINT:
            printf("Received signal %d, shutting down...\n", sig);
            if (httpd) evhttp_free(httpd);
            if (base) event_base_loopexit(base, NULL);
            break;
    }
}

// 设置守护进程
int daemonize() {
    // 创建子进程
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // 父进程退出
    }

    // 子进程继续
    setsid(); // 创建新会话
    chdir("/"); // 切换工作目录
    umask(0); // 重置文件权限掩码

    // 关闭标准文件描述符
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // 重定向到/dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);

    return 0;
}

// 发送错误响应
void send_error(struct evhttp_request *req, int code, const char *reason) {
    struct evbuffer *buf = evbuffer_new();
    evbuffer_add_printf(buf, "%s\n", reason);
    
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain");
    evhttp_send_reply(req, code, reason, buf);
    evbuffer_free(buf);
}

// 处理静态文件请求
void handle_file(struct evhttp_request *req, const char *path, const char *doc_root) {
    char full_path[PATH_MAX];
    struct stat st;
    
    // 构建完整路径
    if (strstr(path, "..")) {
        send_error(req, HTTP_BADREQUEST, "Invalid path");
        return;
    }
    // 格式化得到完整路径
    snprintf(full_path, sizeof(full_path), "%s%s", doc_root, path);
    
    // 检查文件是否存在
    if (stat(full_path, &st) == -1) {
        if (errno == ENOENT) {
            send_error(req, HTTP_NOTFOUND, "File not found");
        } else {
            send_error(req, HTTP_INTERNAL, "Internal server error");
        }
        return;
    }
    
    // 检查是否是目录
    if (S_ISDIR(st.st_mode)) {
        send_error(req, HTTP_BADMETHOD, "Directory listing not allowed");
        return;
    }
    
    // 打开文件
    FILE *f = fopen(full_path, "rb");
    if (!f) {
        send_error(req, HTTP_INTERNAL, "Cannot open file");
        return;
    }
    
    // 设置内容类型
    const char *content_type = "application/octet-stream";
    if (strstr(path, ".html")) content_type = "text/html";
    else if (strstr(path, ".css")) content_type = "text/css";
    else if (strstr(path, ".js")) content_type = "application/javascript";
    else if (strstr(path, ".png")) content_type = "image/png";
    else if (strstr(path, ".jpg") || strstr(path, ".jpeg")) content_type = "image/jpeg";
    
    // 准备响应
    struct evbuffer *buf = evbuffer_new();
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", content_type);
    char size_str[32];
    // 获得文件大小
    snprintf(size_str, sizeof(size_str), "%lu", st.st_size);
    evhttp_add_header(req->output_headers, "Content-Length", size_str);
    
    // 读取文件内容
    char data[4096];
    size_t n;
    // 读取文件中的内容
    while ((n = fread(data, 1, sizeof(data), f)) > 0) {
        evbuffer_add(buf, data, n);
    }
    fclose(f);
    
    // 响应给客户端状态码以及内容
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

// 默认请求处理
void httpd_handler(struct evhttp_request *req, void *arg) {
    struct httpd_config *cfg = (struct httpd_config *)arg;
    const char *uri = evhttp_request_uri(req);

    // 获得接收请求类型，比如GET或者POST等
    enum evhttp_cmd_type method = evhttp_request_get_command(req);
    
    // 新代码：
    const char *method_str;
    switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET:     method_str = "GET";     break;
        case EVHTTP_REQ_POST:    method_str = "POST";    break;
        case EVHTTP_REQ_HEAD:    method_str = "HEAD";    break;
        case EVHTTP_REQ_PUT:     method_str = "PUT";     break;
        case EVHTTP_REQ_DELETE:  method_str = "DELETE";  break;
        default:                 method_str = "UNKNOWN"; break;
    }
    printf("Received %s request for %s\n", method_str, uri);
    
    // 处理GET参数，对URL请求进行解析
    struct evkeyvalq params;
    evhttp_parse_query(uri, &params);
    
    // 如果是静态文件服务且有文档根目录
    if (cfg->doc_root) {
        // 服务端读取文件并响应给浏览器
        handle_file(req, uri, cfg->doc_root);
        return;
    }
    
    // 默认响应，添加hello from，用于后面响应给浏览器
    struct evbuffer *buf = evbuffer_new();
    evbuffer_add_printf(buf, "Hello from %s\n", MYHTTPD_SIGNATURE);
    // evbuffer_add_printf(buf, "Method: %s\n", evhttp_cmd_type(method));
    // 新代码：
    switch (evhttp_request_get_command(req)) {
        case EVHTTP_REQ_GET:     method_str = "GET";     break;
        case EVHTTP_REQ_POST:    method_str = "POST";    break;
        case EVHTTP_REQ_HEAD:    method_str = "HEAD";    break;
        case EVHTTP_REQ_PUT:     method_str = "PUT";     break;
        case EVHTTP_REQ_DELETE:  method_str = "DELETE";  break;
        default:                 method_str = "UNKNOWN"; break;
    }
    printf("Received %s request for %s\n", method_str, uri);

    // 继续添加其他内容响应给浏览器
    evbuffer_add_printf(buf, "URI: %s\n", uri);
    
    // 添加GET参数
    struct evkeyval *header;
    // TAILQ_FOREACH(header, &params, next) {
    //     evbuffer_add_printf(buf, "Param %s: %s\n", header->key, header->value);
    // }
    
    // 添加POST数据
    size_t post_len = evbuffer_get_length(req->input_buffer);
    if (post_len > 0) {
        char *post_data = malloc(post_len + 1);
        evbuffer_copyout(req->input_buffer, post_data, post_len);
        post_data[post_len] = '\0';
        evbuffer_add_printf(buf, "POST data: %s\n", post_data);
        free(post_data);
    }
    
    // 设置响应头
    evhttp_add_header(req->output_headers, "Server", MYHTTPD_SIGNATURE);
    evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
    evhttp_add_header(req->output_headers, "Connection", "close");
    
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

int main(int argc, char **argv) {
    struct httpd_config cfg;
    // 包含初始化监听地址，监听端口，监听需求，监听超时时间以及监听文档的跟目录
    init_config(&cfg);
    
    // 对输入参数进行解析
    if (parse_args(argc, argv, &cfg) != 0) {
        free_config(&cfg);
        return EXIT_FAILURE;
    }
    
    // 设置为守护进程
    if (cfg.daemon_mode && daemonize() != 0) {
        free_config(&cfg);
        return EXIT_FAILURE;
    }
    
    // 设置信号处理
    signal(SIGHUP, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    
    // 初始化libevent
    base = event_base_new();
    if (!base) {
        fprintf(stderr, "Could not initialize libevent!\n");
        free_config(&cfg);
        return EXIT_FAILURE;
    }
    
    // 创建HTTP服务器
    httpd = evhttp_new(base);
    if (!httpd) {
        fprintf(stderr, "Could not create http server!\n");
        event_base_free(base);
        free_config(&cfg);
        return EXIT_FAILURE;
    }
    
    // 绑定地址和端口
    if (evhttp_bind_socket(httpd, cfg.listen_addr, cfg.listen_port) != 0) {
        fprintf(stderr, "Could not bind to %s:%d\n", cfg.listen_addr, cfg.listen_port);
        evhttp_free(httpd);
        event_base_free(base);
        free_config(&cfg);
        return EXIT_FAILURE;
    }
    
    // 设置http连接超时时间
    evhttp_set_timeout(httpd, cfg.timeout);
    
    // 设置通用回调（文件描述符，回调函数以及回调函数参数）
    evhttp_set_gencb(httpd, httpd_handler, &cfg);
    

    printf("Server started on http://%s:%d\n", cfg.listen_addr, cfg.listen_port);
    if (cfg.doc_root) {
        printf("Serving files from: %s\n", cfg.doc_root);
    }
    
    // 进入事件循环
    event_base_dispatch(base);
    
    // 清理资源
    evhttp_free(httpd);
    event_base_free(base);
    free_config(&cfg);
    
    printf("Server stopped\n");
    return EXIT_SUCCESS;
}