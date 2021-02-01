#include <gmock/gmock.h>

#include "cmd/callback.hpp"
#include "source/lib/frontend.hpp"

using miu::cmd::callback;
using miu::com::variant;
using testing::Return;

struct ut_frontend : public testing::Test {
    struct mock {
        MOCK_METHOD(int32_t, func, (int32_t));
    } m;

    miu::cmd::frontend frontend;
};

TEST_F(ut_frontend, insert) {
    frontend.insert({ "cmd", 1 }, callback::make(&mock::func, &m));

    variant arg[] = { 123 };
    EXPECT_CALL(m, func(123)).WillOnce(Return(321));
    auto ret = frontend.handle("cmd.1", arg, 1);
    EXPECT_EQ(variant(321), ret);
}

TEST_F(ut_frontend, duplicated) {
    frontend.insert({ "cmd", 2 }, callback::make(&mock::func, &m));
    EXPECT_FALSE(frontend.insert({ "cmd", 2 }, callback::make(&mock::func, &m)));
}

TEST_F(ut_frontend, unknown) {
    auto ret = frontend.handle("not_exist", nullptr, 0);
    EXPECT_EQ(variant(std::string("UNKNOWN_CMD")), ret);
}
