#pragma once

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>
#include <string.h>

class http_conn
{
public:
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT
    };

    // 解析客户端请求时，主状态机的状态
    enum CHECK_STATE
    {
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    // 从状态机的状态
    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    static int m_epollfd;    // 所有的socket事件都被注册到同一个epoll对象
    static int m_user_count; // 统计用户的数量

    static const int READ_BUFFER_SIZE = 2048;  // 读缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024; // 写缓冲区的大小

    http_conn() {}
    ~http_conn() {}

    void process();                                 // 处理客户端的请求
    void init(int sockfd, const sockaddr_in &addr); // 初始化新接收的连接
    void close_conn();                              // 关闭连接
    bool read();                                    // 非阻塞读
    bool write();                                   // 非阻塞写

private:
    void init();                              // 初始化连接其余的信息
    HTTP_CODE process_read();                 // 解析HTTP请求
    HTTP_CODE parse_request_line(char *text); // 解析请求首行
    HTTP_CODE parse_headers(char *text);      // 解析请求头
    HTTP_CODE parse_content(char *text);      // 解析请求体

    LINE_STATUS parse_line(); // 解析一行
    char *get_line()
    {
        return m_read_buf + m_start_line;
    }

    HTTP_CODE do_request();

    int m_sockfd;                      // 该http连接的socket
    sockaddr_in m_address;             // 通信的socket地址
    char m_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    int m_read_idx;                    // 标识读缓冲区中已经读入的客户端数据的最后一个字节的下一次位置

    int m_checked_index; // 当前正在分析的字符在读缓冲区的位置
    int m_start_line;    // 当前正在解析的行的起始位置
    char *m_url;         // 请求目标文件的文件名
    char *m_version;     // 协议版本,只支持HTTP1.1
    METHOD m_method;     // 请求方法
    char *m_host;        // 主机名
    bool m_linger;       // 判断HTTP请求是否要保持连接

    CHECK_STATE m_check_state; // 主状态机当前状态
};