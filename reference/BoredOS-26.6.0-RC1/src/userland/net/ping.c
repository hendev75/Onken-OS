#include <stdlib.h>
#include <syscall.h>

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

static int resolve_host(const char* host, net_ipv4_address_t* ip) {
    if (parse_ip(host, ip) == 0) return 0;
    return sys_dns_lookup(host, ip);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: ping <host>\n");
        return 1;
    }
    
    if (!sys_network_is_initialized()) {
        printf("Initializing network...\n");
        sys_network_init();
    }
    
    const char *host = argv[1];
    net_ipv4_address_t ip;
    
    if (resolve_host(host, &ip) != 0) {
        printf("Failed to resolve %s\n", host);
        return 1;
    }
    
    printf("Pinging %s (%d.%d.%d.%d)...\n", host, ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3]);
    
    int successful = 0;
    for (int i = 0; i < 4; i++) {
        int rtt = sys_icmp_ping(&ip);
        if (rtt >= 0) {
            printf("64 bytes from %d.%d.%d.%d: icmp_seq=%d time=%dms\n", 
                   ip.bytes[0], ip.bytes[1], ip.bytes[2], ip.bytes[3], i + 1, rtt);
            successful++;
        } else {
            printf("Request timeout for icmp_seq %d\n", i + 1);
        }
        // Small delay between pings
        for(volatile int d=0; d<1000000; d++);
    }
    
    printf("\n--- %s ping statistics ---\n", host);
    printf("4 packets transmitted, %d received, %d%% packet loss\n", successful, (4-successful)*25);
    
    return successful > 0 ? 0 : 1;
}
