#include <chrono>
#include <lwip/opt.h>

#include "dummy.hpp"
#include "test_data.hpp"
#include <gtest/gtest.h>
#include <lwip/errno.h>
#include <spn/dcp.h>
#include <thread>

namespace {
struct DcpTest : public SpnInstance {
    explicit DcpTest()
    {
        cfg.dual_port = false;

        input_frames.push_back(std::make_pair(test_data::dcp::kDcpAllSelector, sizeof(test_data::dcp::kDcpAllSelector)));

        this->cb_low_level_output = [](struct netif* netif, struct pbuf* p) -> err_t {
            return ERR_OK;
        };

        this->cb_low_level_input = [this](struct netif* netif, struct pbuf* p) -> err_t {
            if (input_frames.empty()) {
                return -ERR_ABRT;
            }

            auto& frame = input_frames.front();

            auto pbuf = pbuf_alloc(PBUF_RAW, frame.second, PBUF_RAM);
            memcpy(pbuf->payload, frame.first, frame.second);
            auto res = netif->input(pbuf, netif);

            if (res != ERR_OK) {
                pbuf_free(pbuf);
                return res;
            }

            return ERR_OK;
        };

        this->cb_low_level_poll = [this](struct netif* netif) -> err_t {
            return cb_low_level_input(netif, nullptr);
        };
    }

    std::vector<std::pair<const char*, size_t>> input_frames;
};
}

TEST_F(DcpTest, init)
{
    this->step();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

TEST(DCP, resp_delay_default)
{
    int rand = 11;
    int resp_delay_factor = 1;
    int delay = spn_dcp_resp_delay(rand, resp_delay_factor);
    ASSERT_EQ(delay, 0);
}

TEST(DCP, resp_delay_timeout_default)
{
    int rand = 11;
    int resp_delay_factor = 1;
    int delay = spn_dcp_resp_delay_timeout(rand, resp_delay_factor);
    ASSERT_EQ(delay, SPN_DCP_RESPONSE_DELAY_DEFAULT);
}

TEST(DCP, resp_delay)
{
    int rand = 11;
    int resp_delay_factor = 2;
    int delay = spn_dcp_resp_delay(rand, resp_delay_factor);
    ASSERT_EQ(delay, 10);
}

TEST(DCP, resp_delay_timeout)
{
    int rand = 11;
    int resp_delay_factor = 2;
    int delay = spn_dcp_resp_delay_timeout(rand, resp_delay_factor);
    ASSERT_EQ(delay, 2000);
}

TEST(DCP, resp_delay_max)
{
    int rand = 6401;
    int resp_delay_factor = 6402;
    int delay = spn_dcp_resp_delay(rand, resp_delay_factor);
    ASSERT_EQ(delay, SPN_DCP_RESPONSE_DELAY_MAX);
}

TEST(DCP, resp_delay_timeout_max)
{
    int rand = 6401;
    int resp_delay_factor = 6402;
    int delay = spn_dcp_resp_delay_timeout(rand, resp_delay_factor);
    ASSERT_EQ(delay, SPN_DCP_RESPONSE_DELAY_MAX);
}
