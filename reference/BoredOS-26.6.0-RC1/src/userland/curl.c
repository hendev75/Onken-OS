#include <stdlib.h>
#include <syscall.h>
#include <stdint.h> // Added for uint8_t

static int parse_ip(const char* str, net_ipv4_address_t* ip) {
    int val = 0;
    int part = 0;
    const char* p = str;
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            val = val * 10 + (*p - '0');
            if (val > 255) return -1;
        } else if (*p == '.') {
            if (part > 3) return -1;
            ip->bytes[part++] = (uint8_t)val;
            val = 0;
        } else {
            return -1;
        }
        p++;
    }
    if (part != 3) return -1;
    ip->bytes[3] = (uint8_t)val;
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: curl http://a.b.c.d[:port]/path\n");
        return 1;
    }

    const char* url = argv[1];
    const char* host_start = url;
    int is_https = 0;
    
    if (url[0] == 'h' && url[1] == 't' && url[2] == 't' && url[3] == 'p') {
        if (url[4] == 's' && url[5] == ':') {
            is_https = 1;
            host_start = url + 8;
        } else if (url[4] == ':') {
            host_start = url + 7;
        }
    }
    
    if (is_https) {
        printf("Error: HTTPS is not yet supported in BoredOS. Please use http://\n");
        return 1;
    }

    char hostname[256];
    int port = 80;
    int i = 0;
    while (host_start[i] && host_start[i] != '/' && host_start[i] != ':' && i < 255) {
        hostname[i] = host_start[i];
        i++;
    }
    hostname[i] = 0;

    int host_len = i;
    if (host_start[i] == ':') {
        i++;
        char port_str[10];
        int j = 0;
        while (host_start[i] && host_start[i] != '/' && j < 9) {
            port_str[j++] = host_start[i++];
        }
        port_str[j] = 0;
        port = atoi(port_str);
    }
    
    net_ipv4_address_t ip;
    if (parse_ip(hostname, &ip) != 0) {
        if (sys_dns_lookup(hostname, &ip) != 0) {
            printf("Failed to resolve %s\n", hostname);
            return 1;
        }
    }
    
    printf("Connecting to %s (", hostname);
    printf("%d.%d.%d.%d", ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3]);
    printf("):%d...\n", port);

    if (sys_tcp_connect(&ip, port) != 0) {
        printf("Failed to connect to %s:%d\n", hostname, port);
        return 1;
    }
    
    const char* path = host_start + i;
    if (*path == 0) path = "/";
    
    char request[1024];
    int req_len = 0;
    
    // GET path HTTP/1.1
    // Host: hostname
    // Connection: close
    
    char* r = request;
    char* s;
    
    s = "GET "; while(*s) *r++ = *s++;
    s = (char*)path; while(*s) *r++ = *s++;
    s = " HTTP/1.1\r\nHost: "; while(*s) *r++ = *s++;
    s = hostname; while(*s) *r++ = *s++;
    s = "\r\nUser-Agent: BoredOS/1.0\r\nAccept: */*\r\nConnection: close\r\n\r\n"; while(*s) *r++ = *s++;
    req_len = r - request;

    sys_tcp_send(request, req_len);
    
    char buf[4096];
    int total = 0;
    while (1) {
        int len = sys_tcp_recv(buf, 4095);
        if (len < 0) {
            printf("\n[Error: Connection closed or error]\n");
            break;
        }
        if (len == 0) break;
        buf[len] = 0;
        printf("%s", buf);
        total += len;
        if (total > 1000000) {
            printf("\n[Error: Data limit exceeded]\n");
            break;
        }
    }
    
    sys_tcp_close();
    return 0;
}
