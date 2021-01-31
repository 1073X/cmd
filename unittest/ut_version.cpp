#include <gtest/gtest.h>

#include <iostream>
#include <meta/info.hpp>

#include "cmd/version.hpp"

TEST(ut_version, version) {
    std::cout << miu::cmd::version() << std::endl;
    std::cout << miu::cmd::build_info() << std::endl;

    std::cout << miu::meta::info() << std::endl;
}
