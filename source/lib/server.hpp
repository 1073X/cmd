#pragma once

#include <net/socket.hpp>

#include "cmd/client.hpp"

#include "frontend.hpp"
#include "message.hpp"

namespace miu::cmd {

class server {
  public:
    server(frontend* fe)
        : _frontend(fe) {}

    auto reset(net::socket&& sock) {
        assert(sock.acceptconn());
        std::swap(sock, _svr_sock);
    }

    void handle() {
        auto cli_sock = _svr_sock.accept();
        if (cli_sock) {
            cli_sock.set_timeout(_svr_sock.timeout());
            auto cli = client { std::move(cli_sock) };

            char req_buf[4096] {};
            auto req = cli.recv(req_buf, sizeof(req_buf));
            if (req) {
                auto ret = _frontend->handle(req->cmd(), req->args(), req->args_count());
                cli.send(req->seq_num(), req->cmd(), { ret });
            } else {
                cli.send(0, "NONE", { std::string("CMD_TIMEOUT") });
            }
        }
    }

  private:
    frontend* _frontend;
    net::socket _svr_sock;
};

}    // namespace miu::cmd
