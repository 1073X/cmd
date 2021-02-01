#pragma once

#include <functional>

#include "wrapper.hpp"

namespace miu::cmd {

class callback {
  public:
    using func_type = std::function<com::variant(com::variant const*, uint32_t)>;

    template<typename F, typename... ARGS>
    static func_type make(F const& f, ARGS&&... args) {
        using namespace std::placeholders;
        return std::bind(&wrapper<F>::bounce, f, std::forward<ARGS>(args)..., _1, _2);
    }
};

}    // namespace miu::cmd
