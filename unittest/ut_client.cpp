#include <gmock/gmock.h>

#include <future>
#include <net/udsock.hpp>
#include <thread>

#include "cmd/client.hpp"
#include "source/lib/server.hpp"

using namespace std::chrono_literals;
using miu::cmd::message;

struct ut_client : public testing::Test {
    MOCK_METHOD(int32_t, func, (int32_t));

    void SetUp() override {
        using miu::log::severity;
        // miu::log::log::instance()->reset(severity::DEBUG, 1024);
    }

    void TearDown() override {
        // miu::log::log::instance()->dump();
    }
};

TEST_F(ut_client, connect) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        auto cli_sock = sock.accept();
        EXPECT_TRUE(cli_sock);
    });

    ready.get_future().get();
    miu::cmd::client("ut_client");

    thrd.join();
}

TEST_F(ut_client, send) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        auto cli_sock = sock.accept();

        char buf[512] {};
        cli_sock.recv(buf, sizeof(buf));

        auto msg = (miu::cmd::message const*)buf;
        EXPECT_EQ(99U, msg->seq_num());
        EXPECT_STREQ("cmd.123", msg->cmd());
        EXPECT_EQ(2U, msg->args_count());
    });

    ready.get_future().get();

    auto client = miu::cmd::client("ut_client");
    EXPECT_TRUE(client.send(99, { "cmd", 123 }, { 1, 2 }));

    thrd.join();
}

TEST_F(ut_client, send_failed) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        sock.accept();
    });

    ready.get_future().get();
    auto client = miu::cmd::client("ut_client");
    thrd.join();

    EXPECT_FALSE(client.send(99, { "cmd", 123 }, { 1, 2 }));
}

TEST_F(ut_client, recv) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        auto cli_sock = sock.accept();

        char buf[512] {};
        auto msg = miu::cmd::message::create(buf, sizeof(buf), 100, "cmd.123", 1, 2);

        cli_sock.send(buf, msg->size());
    });

    ready.get_future().get();

    auto client = miu::cmd::client("ut_client");

    char buf[512] {};
    auto msg = client.recv(buf, sizeof(buf));
    EXPECT_NE(nullptr, msg);
    EXPECT_EQ(100U, msg->seq_num());
    EXPECT_STREQ("cmd.123", msg->cmd());
    EXPECT_EQ(2U, msg->args_count());

    thrd.join();
}

TEST_F(ut_client, recv_failed) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        sock.accept();
    });

    ready.get_future().get();

    auto client = miu::cmd::client("ut_client");
    thrd.join();

    char buf[512];
    EXPECT_EQ(nullptr, client.recv(buf, sizeof(buf)));
}

TEST_F(ut_client, recv_not_enough_buf) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_client");
        ready.set_value(true);

        sock.accept();
    });

    ready.get_future().get();

    auto client = miu::cmd::client("ut_client");
    thrd.join();

    char buf[512];
    EXPECT_EQ(nullptr, client.recv(buf, sizeof(message) - 1));    // not enough for message header
    EXPECT_EQ(nullptr, client.recv(buf, sizeof(message)));        // not enough for payload
}

TEST_F(ut_client, disconnected) {
    miu::cmd::client client { "ut_client" };
    EXPECT_FALSE(client.send(100, { "cmd", 123 }, { 1, 2 }));

    char buf[512];
    EXPECT_EQ(nullptr, client.recv(buf, sizeof(buf)));
}

TEST_F(ut_client, request) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        miu::cmd::frontend frontend;
        auto func = miu::cmd::callback::make(&ut_client::func, this);
        frontend.insert("cmd.123", func);

        auto sock = miu::net::udsock::create_server("ut_client");
        miu::cmd::server server { &frontend };
        server.reset(std::move(sock));

        EXPECT_CALL(*this, func(2)).WillOnce(testing::Return(3));

        ready.set_value(true);
        server.handle();
    });

    ready.get_future().get();

    auto client = miu::cmd::client("ut_client");
    auto ret    = client.request(12, "cmd.123", 2);
    EXPECT_EQ(miu::com::variant(3), ret);

    thrd.join();
}
