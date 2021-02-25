
#include <cfg/cmd_source.hpp>
#include <cfg/settings.hpp>
#include <log/log.hpp>

#include "cmd/client.hpp"
#include "cmd/version.hpp"

using namespace miu;

int32_t main(int32_t argc, const char* argv[]) {
    log::reset(log::severity::DEBUG, 1024);

    cfg::cmd_source src { argc, argv };
    cfg::settings settings { &src };

    if (settings.optional<bool>("version", false)) {
        std::cout << miu::meta::info() << ": send command to service." << std::endl;
        std::cout << miu::cmd::version() << std::endl;
        std::cout << miu::cmd::build_info() << std::endl;
        std::cout << "\nUsage: sendcmd --svc <service> --cmd <the.cmd.string> --args <arg1 "
                     "<arg2 ...>>"
                  << std::endl;
    } else {
        auto cli = cmd::client(settings.required<std::string>("svc"));
        auto cmd = settings.required<std::string>("cmd");

        std::vector<com::variant> vec;
        auto args = settings.optional<cfg::settings>("args");
        if (args) {
            vec.reserve(args.size());
            for (auto i = 0U; i < args.size(); i++) {
                // send args as string
                vec.emplace_back(args.required<std::string>(i));
            }
        }

        auto ret = cli.request(1, cmd, vec);
        log::info(+"result:", ret);
    }

    log::dump();
    return 0;
}
