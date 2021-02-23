#pragma once

#include <com/variant.hpp>
#include <time/delta.hpp>

#include "callback.hpp"

namespace miu::cmd {

extern std::string svr_type();
extern time::delta interval();

extern void do_insert(com::strcat const&, callback::func_type const&);
template<typename F, typename... ARGS>
auto insert(com::strcat const& cmd, F const& func, ARGS&&... args) {
    do_insert(cmd, callback::make(func, std::forward<ARGS>(args)...));
}

extern void reset(std::string_view = "", time::delta = 0);
extern void handle();

}    // namespace miu::cmd
