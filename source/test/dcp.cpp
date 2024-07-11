#include <chrono>
#include <cstddef>
#include <lwip/opt.h>

#include "dummy.hpp"
#include "spn/spn_sys.h"
#include "test_data.hpp"
#include <deque>
#include <gtest/gtest.h>
#include <lwip/errno.h>
#include <spn/dcp.h>
#include <thread>

namespace {

using frame_t = std::shared_ptr<std::vector<uint8_t>>;
using frame_value_t = std::vector<uint8_t>;
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

        this->cb_low_level_output = [this](struct netif* netif, struct pbuf* p) -> err_t {
            auto vec_ptr = std::make_shared<frame_value_t>(p->len);
            memcpy(vec_ptr->data(), p->payload, p->len);
            this->promise.set_value();
            output_frames.push_back(vec_ptr);
            return ERR_OK;
        };

        this->cb_low_level_input = [this](struct netif* netif, struct pbuf* p) -> err_t {
            if (input_frames.empty()) {
                return -ERR_ABRT;
            }

            auto& frame = input_frames.front();

            auto pbuf = pbuf_alloc(PBUF_RAW, frame->size(), PBUF_RAM);
            if (pbuf == nullptr) {
                return -ERR_MEM;
            }
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

        future = promise.get_future();
    }

    frame_t get_output()
    {
        future.get();
        auto f = output_frames.front();
        output_frames.pop_front();
        return f;
    }

    std::promise<void> promise;
    std::future<void> future;
    std::deque<frame_t> input_frames;
    std::deque<frame_t> output_frames;
};

} // namespace

extern "C" struct spn_dcp_ctx dcp_ctx;

TEST_F(DcpTest, inputAllSelector)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpAllSelector) });
    this->step();

    auto f = this->get_output();
    ASSERT_NE(f, nullptr);
#if 0
    // TODO: add assert to check frame valid
#else
    // Put resp input stack again to verify the ident.resp parser
    dcp_ctx.dev_session[0].resp.xid = 0x01000001;
    dcp_ctx.dev_session[0].state = dcp_dev_state_ident;
    this->input_frames.push_back(f);
    this->step();
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    ASSERT_TRUE(this->output_frames.empty());
    ASSERT_EQ(dcp_ctx.dev_session[0].state, dcp_dev_state_active);
    ASSERT_STREQ(dcp_ctx.dev_session[0].resp.station_of_name, spn_sys_get_station_name());
    ASSERT_STREQ(dcp_ctx.dev_session[0].resp.vendor_of_name, spn_sys_get_vendor_name());
#endif
}

TEST_F(DcpTest, inputIdentResX208)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpIdentRespX208) });
    // this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpIdentRespEcoPn) });
    // this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpIdentResp200smt) });
    while (this->step()) { }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    // TODO: check the response
}
TEST_F(DcpTest, inputIdentResEcoPn)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpIdentRespEcoPn) });
    while (this->step()) { }
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    // TODO: check the response
}
TEST_F(DcpTest, inputIdentRes200smt)
{
    this->input_frames.push_back({ decode_frame(test_data::dcp::kDcpIdentResp200smt) });
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
