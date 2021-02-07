
#include "cmd/cmd.hpp"

#include <atomic>
#include <log/log.hpp>

#include "cmd/client.hpp"

#include "frontend.hpp"
#include "server.hpp"

namespace miu::cmd {

static frontend g_frontend;
static server g_server { &g_frontend };

void do_insert(com::strcat const& cmd, callback::func_type const& func) {
    if (g_frontend.insert(cmd.str(), func)) {
        log::info(+"CMD(_SVR_) new", cmd.str());
    } else {
        log::error(+"CMD(_SVR_) dup", cmd.str());
        FATAL_ERROR<std::logic_error>("duplicated CMD", cmd.str());
    }
}

void reset(std::string_view name, com::microseconds timeout) {
    auto sock = net::udsock::create_server(name);
    if (sock) {
        sock.set_timeout(timeout);
        g_server.reset(std::move(sock));
    }
}

void handle() {
    g_server.handle();
}

}    // namespace miu::cmd
