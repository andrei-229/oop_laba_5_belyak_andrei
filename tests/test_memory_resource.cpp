#include <gtest/gtest.h>
#include <memory_resource>
#include "../include/memory_resource_reuse.hpp"
#include "../include/stack.hpp"

TEST(ReusingMemoryResource, BasicAllocateDeallocateReuse) {
    ReusingMemoryResource rmr;
    EXPECT_EQ(rmr.in_use_count(), 0u);
    EXPECT_EQ(rmr.free_count(), 0u);

    {
        PmrStack<int> st(&rmr);
        st.emplace(1);
        st.emplace(2);
        st.emplace(3);
        EXPECT_EQ(st.size(), 3u);
        EXPECT_EQ(rmr.in_use_count(), 3u);
        EXPECT_EQ(rmr.free_count(), 0u);

        st.pop(); // one node freed
        EXPECT_EQ(rmr.in_use_count(), 2u);
        EXPECT_EQ(rmr.free_count(), 1u);

        st.emplace(42); // should reuse freed block
        EXPECT_EQ(st.size(), 3u);
        EXPECT_EQ(rmr.in_use_count(), 3u);
        // free list should have shrunk back to 0 after reuse
        EXPECT_EQ(rmr.free_count(), 0u);
    }
    // After stack destruction all its nodes moved to free list
    EXPECT_EQ(rmr.in_use_count(), 0u);
    EXPECT_GT(rmr.free_count(), 0u);
}

TEST(ReusingMemoryResource, PmrStringAllocations) {
    ReusingMemoryResource rmr;
    {
        // Force dynamic allocation (bypass SSO) by using long strings
        std::pmr::string a(100, 'a', &rmr);
        std::pmr::string b(200, 'b', &rmr);
        EXPECT_GE(rmr.in_use_count(), 2u);
    }
    // strings destroyed -> blocks moved to free list
    EXPECT_EQ(rmr.in_use_count(), 0u);
    EXPECT_GE(rmr.free_count(), 2u);
}
