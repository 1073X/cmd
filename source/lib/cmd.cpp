
#include "cmd/cmd.hpp"

#include <atomic>
#include <log/log.hpp>

#include "cmd/client.hpp"

#include "frontend.hpp"
#include "server.hpp"

namespace miu::cmd {

static frontend g_front;
static server g_svr { &g_front };

std::string svr_type() {
    return "uds";
}

time::delta interval() {
    return g_svr.interval();
}

void do_insert(com::strcat const& cmd, callback::func_type const& func) {
    if (g_front.insert(cmd.str(), func)) {
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
            g_svr.reset(std::move(sock));
        }
    } else {
        auto sock = net::socket {};
        g_svr.reset(std::move(sock));
        g_front.clear();
    }
}

void handle() {
    g_svr.handle();
}

}    // namespace miu::cmd
