#include <gmock/gmock.h>

#include <future>
#include <log/log.hpp>
#include <thread>

#include "cmd/cmd.hpp"

struct ut_cmd : public testing::Test {
    MOCK_METHOD(int32_t, func, (int32_t));

    void SetUp() override {
        using miu::log::severity;
        // miu::log::log::instance()->reset(severity::DEBUG, 1024);
    }

    void TearDown() override {
        // miu::log::log::instance()->dump();
    }
};

TEST_F(ut_cmd, insert) {
    miu::cmd::insert({ "cmd", 123 }, &ut_cmd::func, this);
    EXPECT_ANY_THROW(miu::cmd::insert({ "cmd", 123 }, &ut_cmd::func, this););
}

TEST_F(ut_cmd, reset) {
    miu::cmd::reset("ut_cmd_reset", miu::time::delta { 1000 });
    std::thread thrd([]() { miu::cmd::handle(); });

    EXPECT_EQ("uds", miu::cmd::svr_type());
    EXPECT_EQ(miu::time::delta(1000), miu::cmd::interval());

    miu::cmd::reset();
    thrd.join();
}
