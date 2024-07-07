#include <gtest/gtest.h>
#include <spn/dcp.h>

namespace {

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
