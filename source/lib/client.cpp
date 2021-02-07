
#include "cmd/client.hpp"

#include "message.hpp"

namespace miu::cmd {

client::client(std::string_view server) {
    _name = std::string("CMD(") + server.data() + ")";

    _sock = net::udsock::create_client(server);
    if (!_sock) {
        log::error(_name, +"X> failed to connect");
    }
}

client::client(net::socket&& sock)
    : _name("CMD(_SVR_)")
    , _sock(std::move(sock)) {
}

bool client::send(uint32_t seq_num, com::strcat const& cmd, std::vector<com::variant> const& args) {
    char buf[512] = {};
    auto req      = message::create(buf, sizeof(buf), seq_num, cmd, args);
    if (_sock.send(req, req->size()) != (int32_t)req->size()) {
        log::error(_name, +"X>", seq_num, cmd.str(), args);
        return false;
    }

    log::info(_name, +">>", seq_num, cmd.str(), args);
    return true;
}

message const* client::recv(char* buf, uint32_t len) {
    assert(len > sizeof(message));

    auto recv_len = _sock.recv(buf, sizeof(message));
    if (recv_len < (int32_t)sizeof(message)) {
        log::error(_name, +"<X timeout");
        return nullptr;
    }

    auto msg = (message*)buf;
    assert(len - sizeof(message) > msg->payload_size());

    recv_len = _sock.recv(msg + 1, msg->payload_size());
    if (recv_len < (int32_t)msg->payload_size()) {
        log::error(_name,
                   +"<X",
                   msg->seq_num(),
                   +"payload timeout len:",
                   msg->cmd_length(),
                   +"args:",
                   msg->args_count());
        return msg;
    }

    auto args_begin = msg->args();
    auto args_end   = msg->args() + msg->args_count();
    std::vector<com::variant> args(args_begin, args_end);
    log::info(_name, +"<<", msg->seq_num(), std::string(msg->cmd()), args);

    return msg;
}

com::variant client::do_request(uint32_t seq_num,
                                com::strcat const& cmd,
                                std::vector<com::variant> const& args) {
    com::variant ret;
    if (send(seq_num, cmd, args)) {
        char buf[512] {};
        auto rsp = recv(buf, sizeof(buf));
        if (rsp && rsp->args_count() > 0) {
            ret = rsp->args()[0];
        }
    }
    return ret;
}

}    // namespace miu::cmd
