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


typedef struct {
    char* host;
    int port;
    char* path;
    char* method;
    char* body;
    char* content_type;
    char* headers;
} http_request_t;


bool connect_wifi(const char* wifi_ssid, const char* wifi_password);
static err_t http_raw_request(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t http_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static void http_dns_found(const char *name, const ip_addr_t *ipaddr, void *arg);

bool is_ip_address(const char* str);
void free_request(http_request_t* req);
void connect_via_dns(http_request_t* req);
void connect_via_ip(http_request_t* req);
void http_request(const char* method, const char* host_or_ip, int port, 
                  const char* path, const char* body, const char* content_type);

void get(const char* host, const char* path);
void post(const char* host, const char* path,const char* post_data);
void put(const char* host, const char* path,const char* put_data);
void http_task(void *pvParameters);

void http_task(void *pvParameters);

#endif