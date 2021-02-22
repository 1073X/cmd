#include <gtest/gtest.h>

#include <future>
#include <net/udsock.hpp>
#include <thread>

#include "cmd/cmd.hpp"
#include "source/lib/server.hpp"

using namespace std::chrono_literals;
using miu::cmd::message;

struct ut_server : public testing::Test {
    void SetUp() override {
        using miu::log::severity;
        // miu::log::log::instance()->reset(severity::DEBUG, 1024);
    }

    void TearDown() override {
        // miu::log::log::instance()->dump();
    }

    miu::cmd::frontend frontend;
};

TEST_F(ut_server, handle) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_server");
        ready.set_value(true);
        miu::cmd::server server { &frontend };
        server.reset(std::move(sock));
        server.handle();
    });

    ready.get_future().get();
    auto sock = miu::net::udsock::create_client("ut_server");

    // request
    char req_buf[512] = {};
    auto req          = message::create(req_buf, sizeof(req_buf), 99, { "cmd", 123 }, 1, 2);
    EXPECT_EQ(req->size(), sock.send(req, req->size()));

    // response
    char rsp_buf[512] = {};
    EXPECT_EQ(40, sock.recv(rsp_buf, sizeof(rsp_buf)));
    auto rsp = (message const*)rsp_buf;
    EXPECT_EQ(99U, rsp->seq_num());
    EXPECT_STREQ(req->cmd(), rsp->cmd());
    EXPECT_EQ(1, rsp->args_count());
    EXPECT_EQ(miu::com::variant { std::string("UNKNOWN_CMD") }, rsp->args()[0]);

    thrd.join();
}

TEST_F(ut_server, accept_timeout) {
    auto sock = miu::net::udsock::create_server("ut_server");
    sock.set_timeout(1ms);

    miu::cmd::server server { &frontend };
    server.reset(std::move(sock));

    server.handle();
    std::this_thread::sleep_for(2ms);
}

TEST_F(ut_server, message_timeout) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_server");
        sock.set_timeout(1ms);

        miu::cmd::server server { &frontend };
        server.reset(std::move(sock));

        ready.set_value(true);
        server.handle();
    });

    ready.get_future().get();
    auto sock = miu::net::udsock::create_client("ut_server");

    // response
    char rsp_buf[512] = {};
    sock.recv(rsp_buf, sizeof(rsp_buf));
    auto rsp = (message const*)rsp_buf;
    EXPECT_EQ(0U, rsp->seq_num());
    EXPECT_EQ(1, rsp->args_count());
    EXPECT_STREQ("NONE", rsp->cmd());
    EXPECT_EQ(miu::com::variant { std::string("CMD_TIMEOUT") }, rsp->args()[0]);

    thrd.join();
}

TEST_F(ut_server, payload_timeout) {
    std::promise<bool> ready;
    std::thread thrd([&]() {
        auto sock = miu::net::udsock::create_server("ut_server");
        sock.set_timeout(1ms);

        miu::cmd::server server { &frontend };
        server.reset(std::move(sock));

        ready.set_value(true);
        server.handle();
    });

    ready.get_future().get();
    auto sock = miu::net::udsock::create_client("ut_server");

    // request, send only message head
    char req_buf[512] = {};
    auto req          = message::create(req_buf, sizeof(req_buf), 100, { "cmd", 123 }, 1, 2);
    EXPECT_EQ(sizeof(message), sock.send(req, sizeof(message)));

    // response
    char rsp_buf[512] = {};
    sock.recv(rsp_buf, sizeof(rsp_buf));
    auto rsp = (message const*)rsp_buf;
    EXPECT_EQ(100U, rsp->seq_num());
    EXPECT_EQ(1, rsp->args_count());
    EXPECT_EQ(miu::com::variant { std::string("UNKNOWN_CMD") }, rsp->args()[0]);

    thrd.join();
}

