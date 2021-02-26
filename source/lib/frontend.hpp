#pragma once

#include <com/strcat.hpp>
#include <map>

#include "cmd/callback.hpp"

namespace miu::cmd {

class frontend {
  public:
    auto insert(com::strcat const& name, callback::func_type const& cb) {
        return _cbs.emplace(name.str(), cb).second;
    }

    auto clear() { _cbs.clear(); }

    auto handle(std::string_view name, com::variant const* args, uint32_t size) try {
        auto it = _cbs.find(name.data());
        if (_cbs.end() == it) {
            return com::variant { std::string("UKN_CMD") };
        }
        return it->second(args, size);
    } catch (std::exception const& err) {
        return com::variant { std::string(err.what()) };
    } catch (...) {
        return com::variant { std::string("ILL_CMD_CALL") };
    }

  private:
    std::map<std::string, callback::func_type> _cbs;
};

}    // namespace miu::cmd
