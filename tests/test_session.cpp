#include <gtest/gtest.h>
#include "session_manager.h"
#include <thread>
#include <chrono>
#include <algorithm>

TEST(SessionManager, CreateExistsCleanup) {
    SessionManager sm(1); // таймаут 1 сек
    EXPECT_FALSE(sm.exists("X"));
    EXPECT_TRUE(sm.create("X"));
    EXPECT_TRUE(sm.exists("X"));
    EXPECT_FALSE(sm.create("X")); // уже есть

    // ждём пока истечёт
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));

    bool expired = false;
    sm.cleanup([&](auto imsi){
        expired = (imsi == "X");
    });
    EXPECT_TRUE(expired);
    EXPECT_FALSE(sm.exists("X"));
}

TEST(SessionManager, GracefulStop) {
    SessionManager sm(1000);
    sm.create("A"); sm.create("B"); sm.create("C");

    std::vector<std::string> deleted;
    sm.stop_graceful(10, [&](auto imsi){
        deleted.push_back(imsi);
    });

    EXPECT_EQ(deleted.size(), 3u);
    EXPECT_NE(std::find(deleted.begin(), deleted.end(), "A"), deleted.end());
    EXPECT_NE(std::find(deleted.begin(), deleted.end(), "B"), deleted.end());
    EXPECT_NE(std::find(deleted.begin(), deleted.end(), "C"), deleted.end());
}
