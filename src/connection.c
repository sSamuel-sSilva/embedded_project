#include "connection.h"

bool is_ip_address(const char* str);
void free_request(http_request_t* req);
void connect_via_dns(http_request_t* req);
void connect_via_ip(http_request_t* req);
void http_request(const char* method, const char* host_or_ip, int port, 
                  const char* path, const char* body, const char* content_type);


bool connect_wifi(const char* wifi_ssid, const char* wifi_password)
{
    printf("Conectando ao WiFi...\n");
    
    if (cyw43_arch_init())
    {
        printf("Falha ao inicializar WiFi\n");
        return false;
    }
    
    cyw43_arch_enable_sta_mode();
    
    if (cyw43_arch_wifi_connect_timeout_ms(wifi_ssid, wifi_password, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Falha ao conectar\n");
        cyw43_arch_deinit();
        return false;
    }
    
    printf("Conectado! IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_default)));
    return true;
}


static err_t http_raw_request(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    http_request_t* req = (http_request_t*)arg;
    
    if (p != NULL) 
    {
        char response[1024];
        memcpy(response, p->payload, p->len > 1023 ? 1023 : p->len);
        response[p->len > 1023 ? 1023 : p->len] = '\0';
        
        printf("HTTP Response:\n%s\n", response);
        pbuf_free(p);
        
        tcp_close(tpcb);
        free(req);
        return ERR_OK;
    }
    else if (err == ERR_OK)
    {
        char request[512];
        int content_length = req->body ? strlen(req->body) : 0;
        
        snprintf(request, sizeof(request),
            "%s %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n"
            "\r\n"
            "%s",
            req->method, req->path, req->host,
            content_length,
            req->body ? req->body : "");
        
        tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
        
        return ERR_OK;
    }
    else
    {
        printf("HTTP error: %d\n", err);
        tcp_close(tpcb);
        free(req);
        return ERR_OK;
    }
}


static err_t http_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    if (err == ERR_OK)
    {
        http_request_t* req = (http_request_t*)arg;
        
        char request[512];
        int content_length = req->body ? strlen(req->body) : 0;
        
        if (req->content_type)
        {
            snprintf(request, sizeof(request),
                "%s %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Content-Type: %s\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                req->method, req->path, req->host,
                req->content_type,
                content_length,
                req->body ? req->body : "");
        } 
        else
        {
            snprintf(request, sizeof(request),
                "%s %s HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Content-Length: %d\r\n"
                "\r\n"
                "%s",
                req->method, req->path, req->host,
                content_length,
                req->body ? req->body : "");

        }
        
        tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
    }
    else 
    {
        printf("Connection failed: %d\n", err);
        free(arg);
        tcp_close(tpcb);
    }
    return ERR_OK;
}


static void http_dns_found(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    http_request_t* req = (http_request_t*)arg;
    
    if (ipaddr == NULL) {
        printf("DNS failed for: %s\n", name);
        free_request(req);
        return;
    }
    
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb) {
        printf("Failed to create TCP PCB\n");
        free_request(req);
        return;
    }

    tcp_arg(pcb, req);
    tcp_err(pcb, NULL);
    tcp_recv(pcb, http_raw_request);
    
    if (tcp_connect(pcb, ipaddr, req->port, http_connected) != ERR_OK)
    {
        printf("Failed to connect to %s\n", ipaddr_ntoa(ipaddr));
        free_request(req);
        tcp_close(pcb);
    }
}


bool is_ip_address(const char* str)
{
    for (int i = 0; str[i]; i++) 
    if (!(str[i] == '.' || (str[i] >= '0' && str[i] <= '9'))) return false;
    
    int a, b, c, d;
    if (sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d) == 4)
    return (a >= 0 && a <= 255 && b >= 0 && b <= 255 &&
        c >= 0 && c <= 255 && d >= 0 && d <= 255);
        
        return false;
}
    

void free_request(http_request_t* req)
{
    if (req)
    {
        free(req->host);
        free(req->path);
        free(req->method);
        if (req->body) free(req->body);
        if (req->content_type) free(req->content_type);
        free(req);
    }
}


void connect_via_dns(http_request_t* req)
{
    ip_addr_t target_ip;
    if (dns_gethostbyname(req->host, &target_ip, http_dns_found, req) == ERR_OK) http_dns_found(req->host, &target_ip, req);
} 


void connect_via_ip(http_request_t* req)
{
    struct tcp_pcb *pcb = tcp_new();
    tcp_arg(pcb, req);
    tcp_err(pcb, NULL);
    tcp_recv(pcb, http_raw_request);
    
    ip_addr_t target_ip;
    unsigned char a, b, c, d;
    sscanf(req->host, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d);
    IP4_ADDR(&target_ip, a, b, c, d);
    
    if (tcp_connect(pcb, &target_ip, req->port, http_connected) != ERR_OK) 
    {
        printf("Failed to connect to server\n");
        free_request(req);
        tcp_close(pcb);
    }
}


void http_request(const char* method, 
                const char* host_or_ip, 
                int port, 
                const char* path, 
                const char* body, 
                const char* content_type)
{
    http_request_t* req = malloc(sizeof(http_request_t));
    req->host = strdup(host_or_ip);
    req->port = port;
    req->path = strdup(path);
    req->method = strdup(method);
    req->body = body ? strdup(body) : NULL;
    req->content_type = content_type ? strdup(content_type) : NULL;
    
    if (is_ip_address(host_or_ip)) connect_via_ip(req);
    else connect_via_dns(req);
}


void get(const char* host, const char* path)
{
    http_request("GET", host, 5000, path, NULL, NULL);
}


void post(const char* host, const char* path,const char* post_data)
{    
    http_request("POST", host, 5000, path, post_data, "application/json");
}


void put(const char* host, const char* path,const char* put_data)
{
    http_request("PUT", host, 5000, path, put_data, "application/json");
}