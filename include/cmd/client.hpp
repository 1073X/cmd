#pragma once

#include <com/variant.hpp>
#include <log/log.hpp>
#include <net/udsock.hpp>

namespace miu::cmd {

class message;

class client {
  public:
    explicit client(std::string_view server);
    explicit client(net::socket&& sock);

    bool send(uint32_t seq_num, com::strcat const& cmd, std::vector<com::variant> const& args);
    message const* recv(char* buf, uint32_t len);

    template<typename... ARGS>
    com::variant request(uint32_t seq_num, com::strcat const& cmd, ARGS&&... args) {
        return do_request(seq_num, cmd, std::vector<com::variant> { std::forward<ARGS>(args)... });
    }

  private:
    com::variant do_request(uint32_t, com::strcat const&, std::vector<com::variant> const&);

  private:
    std::string _name;
    net::socket _sock;
};

}    // namespace miu::cmd
