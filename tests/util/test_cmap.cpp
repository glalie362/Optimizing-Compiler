//
// Created by Jamie on 27/01/2026.
//

#include <gtest/gtest.h>
#include <util/cmap.hpp>


TEST(CompileTimeMap, ThrowsRangeError)
{
    constexpr auto testValues = std::array<std::pair<int, char>, 4>{
        {{1, 'a'}, {2, 'b'}, {3, 'c'}},
    };
    constexpr auto compileTimeMap = cyrex::util::CompileTimeMap{testValues};
    ASSERT_THROW(compileTimeMap.at(-1), std::range_error);
}

TEST(CompileTimeMap, FindsItem)
{
    constexpr auto testValues = std::array<std::pair<int, char>, 4>{
        {{1, 'a'}, {2, 'b'}, {3, 'c'}},
    };
    constexpr auto compileTimeMap = cyrex::util::CompileTimeMap{testValues};
    ASSERT_EQ(compileTimeMap.at(2), 'b');
}

TEST(CompileTimeMap, FindReturnsEndOfData)
{
    constexpr auto testValues = std::array<std::pair<int, char>, 4>{
        {{1, 'a'}, {2, 'b'}, {3, 'c'}},
    };
    constexpr auto compileTimeMap = cyrex::util::CompileTimeMap{testValues};
    ASSERT_TRUE(compileTimeMap.find(-1) == compileTimeMap.data.end());
}
