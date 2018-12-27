// Copyright 2018 amirnyy <alex.mirnyy@gmail.com>

#include <gtest/gtest.h>
#include <server.hpp>

TEST(talk_to_client, Test2) {
    auto talk_ptr = std::make_shared<talk_to_client>();
    SUCCEED();
}
