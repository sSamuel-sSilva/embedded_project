#ifndef CONNECTION_H
#define CONNECTION_H

#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/netif.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/err.h" 
#include "lwip/sockets.h"


typedef enum {
    HTTP_SUCCESS = 0,
    HTTP_ERROR_DNS_FAILED,
    HTTP_ERROR_CONNECTION_FAILED,
    HTTP_ERROR_SOCKET_FAILED,
    HTTP_ERROR_SEND_FAILED,
    HTTP_ERROR_TIMEOUT,
    HTTP_ERROR_MEMORY,
    HTTP_ERROR_INVALID_RESPONSE,
    HTTP_ERROR_SSL_FAILED,
    HTTP_ERROR_SERVER_ERROR,
    HTTP_ERROR_CLIENT_ERROR,
    HTTP_ERROR_NETWORK_DOWN
} http_error_t;


typedef struct http_request_t http_request_t;
typedef struct http_result_t http_result_t;


typedef void (*http_callback_t)(http_request_t* req, void* user_data);


typedef struct http_result_t {
    int http_status;
    http_error_t error;
    char response_data[512];
} http_result_t;


typedef struct http_request_t {
    char* host;
    int port;
    char* path;
    char* method;
    char* body;
    char* content_type;
    http_error_t error;
    int http_status;
    char error_msg[128];
    http_callback_t callback;
    void* user_data;
} http_request_t;


bool connect_wifi(const char* wifi_ssid, const char* wifi_password);
bool is_ip_address(const char* str);
void http_request(const char* method, const char* host_or_ip, int port, 
                  const char* path, const char* body, const char* content_type,
                  http_callback_t callback, void* user_data);

int get(const char* host, const char* path);
int post(const char* host, const char* path, const char* post_data);
int put(const char* host, const char* path, const char* put_data);



#endif