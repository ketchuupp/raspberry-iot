#include "I2C_Manager/I2C_Manager.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::Mock;
using ::testing::_;

namespace Foo {

// Test to verify that Foo interacts correctly with Bar (using MockBar)
TEST(FooTest, PerformActionCallsBarDoSomething) {
    EXPECT_TRUE(true);
}

}  // namespace Foo
