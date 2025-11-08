#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <memory_resource>
#include "../include/memory_resource_reuse.hpp"
#include "../include/stack.hpp"

TEST(PmrStack, PushPopTopAndSize) {
    ReusingMemoryResource rmr;
    PmrStack<int> st(&rmr);

    EXPECT_TRUE(st.empty());
    st.push(10);
    st.push(20);
    st.emplace(30);

    EXPECT_EQ(st.size(), 3u);
    EXPECT_EQ(st.top(), 30);

    st.pop();
    EXPECT_EQ(st.size(), 2u);
    EXPECT_EQ(st.top(), 20);
}

TEST(PmrStack, ForwardIterationOrder) {
    ReusingMemoryResource rmr;
    PmrStack<int> st(&rmr);
    st.push(10);
    st.push(20);
    st.push(30);

    std::vector<int> got;
    for (auto it = st.begin(); it != st.end(); ++it) got.push_back(*it);

    // LIFO order: head is the last pushed element
    std::vector<int> expected{30, 20, 10};
    EXPECT_EQ(got, expected);
}

struct Person {
    int id;
    std::pmr::string name;
    double score;
    Person(int i, const std::pmr::string& n, double s) : id(i), name(n), score(s) {}
};

TEST(PmrStack, WorksWithComplexTypeAndPmrStrings) {
    ReusingMemoryResource rmr;

    std::pmr::string a{"Alice", &rmr};
    std::pmr::string b{"Bob", &rmr};

    PmrStack<Person> st(&rmr);
    st.emplace(1, a, 10.5);
    st.emplace(2, b, 20.0);

    ASSERT_EQ(st.size(), 2u);

    auto it = st.begin();
    ASSERT_NE(it, st.end());
    EXPECT_EQ(it->id, 2);
    EXPECT_EQ(it->name, (std::pmr::string{"Bob", &rmr}));

    ++it;
    ASSERT_NE(it, st.end());
    EXPECT_EQ(it->id, 1);
    EXPECT_EQ(it->name, (std::pmr::string{"Alice", &rmr}));
}
