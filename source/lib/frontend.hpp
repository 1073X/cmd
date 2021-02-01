#pragma once

#include <com/strcat.hpp>

#include "cmd/callback.hpp"

namespace miu::cmd {

class frontend {
  public:
    auto insert(com::strcat const& name, callback::func_type const& cb) {
        return _cbs.emplace(name.str(), cb).second;
    }

    auto handle(std::string_view name, com::variant const* args, uint32_t size) {
        auto it = _cbs.find(name.data());
        if (_cbs.end() == it) {
            return com::variant { std::string("UNKNOWN_CMD") };
        }
        return it->second(args, size);
    }

  private:
    std::map<std::string, callback::func_type> _cbs;
};

}    // namespace miu::cmd
