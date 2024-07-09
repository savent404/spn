#include <chrono>
#include <lwip/opt.h>

#include "dummy.hpp"
#include "test_data.hpp"
#include <deque>
#include <gtest/gtest.h>
#include <lwip/errno.h>
#include <spn/dcp.h>
#include <thread>

namespace {

using frame_t = std::shared_ptr<std::vector<uint8_t>>;
std::shared_ptr<std::vector<uint8_t>> decode_frame(const char* frame)
{
    auto len = strlen(frame);
    char sbuf[3];
    auto buf = std::make_shared<std::vector<uint8_t>>(len / 2);
    for (size_t i = 0; i < len; i += 2) {
        sbuf[0] = frame[i];
        sbuf[1] = frame[i + 1];
        sbuf[2] = 0;
        buf->at(i / 2) = strtol(sbuf, nullptr, 16);
    }
    return buf;
}

struct DcpTest : public SpnInstance {
    explicit DcpTest()
    {
        cfg.dual_port = false;

        this->cb_low_level_output = [](struct netif* netif, struct pbuf* p) -> err_t {
            return ERR_OK;
        };

        this->cb_low_level_input = [this](struct netif* netif, struct pbuf* p) -> err_t {
            if (input_frames.empty()) {
                return -ERR_ABRT;
            }

            auto& frame = input_frames.front();

            auto pbuf = pbuf_alloc(PBUF_RAW, frame->size(), PBUF_RAM);
            memcpy(pbuf->payload, frame->data(), frame->size());
            auto res = netif->input(pbuf, netif);

            if (res != ERR_OK) {
                input_frames.pop_front();
                pbuf_free(pbuf);
                return res;
            }

            input_frames.pop_front();
            return ERR_OK;
        };

        this->cb_low_level_poll = [this](struct netif* netif) -> err_t {
            return cb_low_level_input(netif, nullptr);
        };
    }

    std::deque<frame_t> input_frames;
};

} // namespace

TEST_F(DcpTest, inputAllSelector)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpAllSelector) });
    this->step();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    // TODO: check the response
}

TEST_F(DcpTest, inputIdentRes)
{
    std::vector<frame_t> arr = {
        { decode_frame(test_data::dcp::kDcpIdentResp200smt) },
        { decode_frame(test_data::dcp::kDcpIdentRespEcoPn) },
        { decode_frame(test_data::dcp::kDcpIdentRespX208) },
    };
    for (auto& v : arr) {
        this->input_frames.push_back(v);
    }
    while (this->step()) { }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    // TODO: check the response
}

TEST_F(DcpTest, inputSetReq)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpSetReqX208) });
    this->step();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
}

TEST_F(DcpTest, inputSetResp)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpSetRespX208) });
    this->step();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
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
