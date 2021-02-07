#include <gtest/gtest.h>

#include "source/lib/message.hpp"

using miu::com::variant;

TEST(ut_message, create) {
    char buf[512];
    auto msg = miu::cmd::message::create(buf, sizeof(buf), 1, { "cmd", 1 }, 1, 1.2);

    EXPECT_EQ(1U, msg->seq_num());
    EXPECT_EQ(54U, msg->size());
    EXPECT_EQ(38U, msg->payload_size());
    EXPECT_STREQ("cmd.1", msg->cmd());
    EXPECT_EQ((miu::com::variant const*)(buf + 16), msg->args());
    EXPECT_EQ(2U, msg->args_count());
    EXPECT_EQ(variant(1), msg->args()[0]);
    EXPECT_EQ(variant(1.2), msg->args()[1]);

    EXPECT_EQ(nullptr, miu::cmd::message::create(buf, 16, 2, { "cmd", 1 }, 1, 1.2));
}

