#include <gmock/gmock.h>

#include "cmd/callback.hpp"

using miu::cmd::callback;
using miu::com::variant;
using testing::Return;

struct ut_callback : public testing::Test {
    struct mock {
        mock() { instance = this; }
        ~mock() { instance = nullptr; }

        MOCK_METHOD(void, set, ());
        static void s_set() { instance->set(); }
        MOCK_METHOD(void, set_arg, (int32_t, double));
        static void s_set_arg(int32_t a, double b) { instance->set_arg(a, b); }
        MOCK_METHOD(int32_t, set_rtn, ());
        static int32_t s_set_rtn() { return instance->set_rtn(); }

        inline static mock* instance;
    } m;
};

TEST_F(ut_callback, member_func) {
    EXPECT_CALL(m, set());
    callback::make(&mock::set, &m)(nullptr, 0);

    EXPECT_CALL(m, set_rtn()).WillOnce(Return(123));
    EXPECT_EQ(variant(123), callback::make(&mock::set_rtn, &m)(nullptr, 0));

    variant args[] = { 123, 1.2, +"abc" };    // well, it's fine to pass more args than expected
    EXPECT_CALL(m, set_arg(123, 1.2));
    callback::make(&mock::set_arg, &m)(args, 3);

    EXPECT_ANY_THROW(callback::make(&mock::set_arg, &m)(args, 1));    // lack of args

    variant invalid_args[] = { +"abc", 1.2 };    // string cannot be int
    EXPECT_ANY_THROW(callback::make(&mock::set_arg, &m)(invalid_args, 2));
}

TEST_F(ut_callback, global_func) {
    EXPECT_CALL(m, set());
    callback::make (&mock::s_set)(nullptr, 0);

    variant args[] = { 123, 1.2 };
    EXPECT_CALL(m, set_arg(123, 1.2));
    callback::make (&mock::s_set_arg)(args, 2);

    EXPECT_CALL(m, set_rtn()).WillOnce(Return(123));
    EXPECT_EQ(variant(123), callback::make(&mock::s_set_rtn)(nullptr, 0));
}
