
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
        log::info(+"cmd ADD", cmd.str());
    } else {
        log::error(+"cmd DUP", cmd.str());
        FATAL_ERROR<std::logic_error>("duplicated cmd", cmd.str());
    }
}

void reset(std::string_view name, time::delta timeout) {
    if (!name.empty()) {
        auto sock = net::udsock::create_server(name);
        if (sock) {
            sock.set_timeout(timeout);
            g_server.reset(std::move(sock));
        }
    } else {
        auto sock = net::socket {};
        g_server.reset(std::move(sock));
    }
}

void handle() {
    g_server.handle();
}

}    // namespace miu::cmd
