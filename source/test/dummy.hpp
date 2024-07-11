#pragma once

#include <lwip/opt.h>

#include <functional>
#include <gtest/gtest.h>
#include <spn/spn.h>
#include <future>

struct LwipInstance {
    static std::promise<void> promise;
    static std::future<void> future;
    static bool initialized;
    static void tcpip_init_cb(void* arg);
    explicit LwipInstance();
};

struct SpnInstance : public ::testing::Test {
    struct spn_ctx ctx;
    struct spn_cfg cfg;
    struct netif ifaces[2];

    std::function<err_t(struct netif*, struct pbuf*)> cb_low_level_output;
    std::function<err_t(struct netif*, struct pbuf*)> cb_low_level_input;
    std::function<err_t(struct netif*)> cb_low_level_init;
    std::function<err_t(struct netif*)> cb_low_level_poll;

    ~SpnInstance();

    virtual bool step(); // returns true until cb_low_level_pool returns error code
    void SetUp() override;
    void TearDown() override;

    LwipInstance lwip;
};
