#include "spn_iface.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <exception>
#include <getopt.h>
#include <linux/if_packet.h>
#include <memory>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SPN_IFACE_PORT_0_NAME "ens33"
#define SPN_IFACE_PORT_NUM 1

struct raw_ethernet_if {
public:
    explicit raw_ethernet_if(const char* if_name, int port)
        : if_name_(if_name), port_(port)
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

    static raw_ethernet_if* get_instance(int port, const char* if_name)
    {
        static std::shared_ptr<raw_ethernet_if> instance[SPN_IFACE_PORT_NUM] = { nullptr };
        if (instance[port]) {
            return instance[port].get();
        }
        instance[port] = std::make_shared<raw_ethernet_if>(if_name, port);
        return instance[port].get();
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

    int send(const void* buffer, size_t len, uint32_t timeout)
    {
        // set sock timeout
        struct timeval tv;
        int res;
        tv.tv_sec = timeout / (int)1e6;
        tv.tv_usec = timeout % (int)1e6;

        if (setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            perror("setsockopt");
            throw std::bad_exception();
        }

        res = send(buffer, len);

        tv.tv_sec = 0xFFFF;
        tv.tv_usec = 0xFFFF;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
            perror("setsockopt");
            throw std::bad_exception();
        }

        return res;
    }

    int recv(void* buffer, size_t len, uint32_t timeout)
    {
        // set sock timeout
        struct timeval tv;
        int res;
        tv.tv_sec = timeout / (int)1e6;
        tv.tv_usec = timeout % (int)1e6;

        if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("setsockopt");
            throw std::bad_exception();
        }

        res = recvfrom(sockfd_, buffer, len, 0, nullptr, nullptr);

        tv.tv_sec = 0xFFFF;
        tv.tv_usec = 0xFFFF;
        if (setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            perror("setsockopt");
            throw std::bad_exception();
        }

        return res;
    }

    int set_ip(uint8_t* address)
    {
        return 0;
    }

    int get_ip(uint8_t* address)
    {
        const uint8_t const_address[] = {192, 168, 1, 100};

        memcpy(address, const_address, 4);
        address[3] = const_address[3] + port_;
        return 0;
    }

    int get_mac(uint8_t* address)
    {
        const uint8_t const_address[] = {0x00, 0x0c, 0x29, 0x4f, 0x4f, 0x4f};
        memcpy(address, const_address, 6);
        address[5] = const_address[5] + port_;
        return 0;
    }

private:
    const char* if_name_;
    int sockfd_;
    struct sockaddr_ll socket_address_;
    int port_;
};

extern "C" {
bool spn_iface_input_poll(int port, void* frame, size_t len, uint32_t timeout)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    return iface->recv(frame, len, timeout) > 0;
}

bool spn_iface_output_poll(int port, const void* frame, size_t len, uint32_t timeout)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    return iface->send(frame, len, timeout) > 0;
}

bool spn_iface_output(int port, const void* frame, size_t len)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    return iface->send(frame, len) > 0;
}

bool spn_iface_get_ip_address(int port, uint8_t* address)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    return iface->get_ip(address) == 0;
}

bool spn_iface_set_ip_address(int port, const char* ip_address)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    uint8_t address[4];
    if (inet_pton(AF_INET, ip_address, address) <= 0) {
        return false;
    }
    return iface->set_ip(address) == 0;
}

bool spn_iface_get_mac_address(int port, uint8_t* mac_address)
{
    raw_ethernet_if* iface = raw_ethernet_if::get_instance(port, SPN_IFACE_PORT_0_NAME);
    return iface->get_mac(mac_address) == 0;
}
}
