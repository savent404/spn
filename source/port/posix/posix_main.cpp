#include <arpa/inet.h>
#include <cstdio>
#include <getopt.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

#include <exception>
#include <memory>

struct raw_ethernet_if {
public:
    explicit raw_ethernet_if(const char* if_name)
        : if_name_(if_name)
    {
        struct ifreq ifr;
        sockfd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (sockfd_ == -1) {
            perror("socket");
            throw std::bad_exception();
        }

        memset(&socket_address_, 0, sizeof(socket_address_));
        socket_address_.sll_ifindex = if_nametoindex(if_name_);

        strcpy(ifr.ifr_name, if_name_);
        if (setsockopt(sockfd_, SOL_SOCKET, SO_BINDTODEVICE, (void*)&ifr, sizeof(ifr)) < 0) {
            perror("SO_BINDTODEVICE");
            close(sockfd_);
            throw std::bad_exception();
        }
    }

    ~raw_ethernet_if()
    {
        close(sockfd_);
    }

    int send(const void* buffer, size_t len)
    {
        int res;

        res = sendto(sockfd_, buffer, len, 0, (struct sockaddr*)&socket_address_, sizeof(socket_address_));
        if (res < 0) {
            perror("sendto");
            throw std::bad_exception();
        }

        return res;
    }

    int recv(void* buffer, size_t len)
    {
        int res;

        res = recvfrom(sockfd_, buffer, len, 0, NULL, NULL);
        if (res < 0) {
            perror("recvfrom");
            throw std::bad_exception();
        }

        return res;
    }

private:
    const char* if_name_;
    int sockfd_;
    struct sockaddr_ll socket_address_;
};

int main(int argc, char* argv[])
{
    unsigned char buffer[ETH_FRAME_LEN];
    int opt;
    char* netif = NULL;

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            netif = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s -i <netif>\n", argv[0]);
            return 1;
        }
    }

    auto eth_if = std::make_shared<raw_ethernet_if>(netif);

    // src mac: BB:BB:BB:BB:BB:BB
    // dst mac: AA:AA:AA:AA:AA:AA
    memset(buffer, 0xAA, 6);
    memset(buffer + 6, 0xBB, 6);

    eth_if->send(buffer, 64);

    auto res = eth_if->recv(buffer, ETH_FRAME_LEN);
    for (int i = 0; i < res; i++) {
        printf("%02x ", (unsigned)buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }

    return 0;
}
