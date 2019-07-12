#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "disfft.h"

using Node = SplittingTree::Node;
using NodePtr = SplittingTree::NodePtr;

bool CheckEqual(complex_t a, complex_t b) {
    return !NonZero(a - b);
}

using namespace std::complex_literals;

TEST_CASE("Filters frequency simple 1") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto b = root->MakeRight();

    auto filter_a = Filter(a, 2);
    REQUIRE(CheckEqual(filter_a.FilterFrequency(1), 1));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(0), 0.));

    auto filter_b = Filter(b, 2);
    REQUIRE(CheckEqual(filter_b.FilterFrequency(1), 0));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(0), 1.));
}

TEST_CASE("Filters frequency simple 2") {
    {
        auto root = std::make_shared<Node>();
        auto a = root->MakeLeft();

        auto filter_a = Filter(a, 2);
        REQUIRE(CheckEqual(filter_a.FilterFrequency(1), 1));
    }
    {
        auto root = std::make_shared<Node>();
        auto b = root->MakeRight();

        auto filter_b = Filter(b, 2);
        REQUIRE(CheckEqual(filter_b.FilterFrequency(0), 1));
    }
}


TEST_CASE("Filters frequency simple 3") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto b = root->MakeRight();

    auto filter_a = Filter(a, 4);
    REQUIRE(CheckEqual(filter_a.FilterFrequency(1), 1));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(3), 1));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(0), 0.));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(2), 0.));

    auto filter_b = Filter(b, 4);
    REQUIRE(CheckEqual(filter_b.FilterFrequency(1), 0));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(3), 0));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(0), 1.));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(2), 1.));
}

TEST_CASE("Filters frequency simple 4") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto b = root->MakeRight();
    auto a1 = a->MakeLeft();
    auto b2 = b->MakeRight();

    auto filter_a = Filter(a1, 4);
    REQUIRE(CheckEqual(filter_a.FilterFrequency(3), 1));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(0), 0.));

    auto filter_b = Filter(b2, 4);
    REQUIRE(CheckEqual(filter_b.FilterFrequency(3), 0));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(0), 1.));
}

TEST_CASE("Filters frequency simple 5") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto a1 = a->MakeLeft();
    auto a2 = a->MakeRight();

    auto filter_a = Filter(a1, 4);
    REQUIRE(CheckEqual(filter_a.FilterFrequency(3), 1));
    REQUIRE(CheckEqual(filter_a.FilterFrequency(1), 0.));

    auto filter_b = Filter(a2, 4);
    REQUIRE(CheckEqual(filter_b.FilterFrequency(3), 0));
    REQUIRE(CheckEqual(filter_b.FilterFrequency(1), 1.));
}

TEST_CASE("Filters time simple 1") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto filter = Filter(a, 2);
    auto it = filter.FilterTime().find(0);
    REQUIRE(it != filter.FilterTime().end());
    REQUIRE(it->second == 1.);
    REQUIRE(filter.FilterTime().find(1) == filter.FilterTime().end());
}

TEST_CASE("Filters time simple 2") {
    auto root = std::make_shared<Node>();
    auto a = root->MakeLeft();
    auto b = root->MakeRight();
    auto filter = Filter(a, 2);
    auto& filter_time = filter.FilterTime();
    auto it = filter_time.find(0);
    REQUIRE(it != filter_time.end());
    REQUIRE(it->second == 0.5);
    it = filter_time.find(1);
    REQUIRE(it != filter_time.end());
    REQUIRE(CheckEqual(it->second, -0.5));
}

TEST_CASE("Tree test remove") {
    auto tree = SplittingTree();
    auto root = tree.GetRoot();
    auto a = root->MakeLeft();
    auto b = root->MakeRight();
    REQUIRE(root->left == a);
    REQUIRE(root->right == b);
    REQUIRE(a->parent == root.get());
    REQUIRE(b->parent == root.get());
    REQUIRE(root->parent == nullptr);
    REQUIRE(a->left == nullptr);
    REQUIRE(a->right == nullptr);
    REQUIRE(b->left == nullptr);
    REQUIRE(b->right == nullptr);

    tree.RemoveNode(a);
    REQUIRE(root->left == nullptr);
    REQUIRE(root->right == b);
    REQUIRE(b->parent == root.get());
    REQUIRE(root->parent == nullptr);
    REQUIRE(b->left == nullptr);
    REQUIRE(b->right == nullptr);

    tree.RemoveNode(b);
    REQUIRE(!tree.IsNonEmpty());
    REQUIRE(tree.GetRoot() == nullptr);
}

TEST_CASE("Tree get lightest 1") {
    auto tree = SplittingTree();
    auto root = tree.GetRoot();
    REQUIRE(root == tree.GetLightestNode());
    auto a = root->MakeLeft();
    REQUIRE(a == tree.GetLightestNode());
    auto b = root->MakeRight();
    auto node = tree.GetLightestNode();
    REQUIRE(b == node);
    tree.RemoveNode(b);
    node = tree.GetLightestNode();
    REQUIRE(node == a);
}

TEST_CASE("Tree get lightest 2") {
    auto tree = SplittingTree();
    auto root = tree.GetRoot();
    auto a1 = root->MakeLeft();
    auto a2 = root->MakeRight();
    auto b1 = a1->MakeLeft();
    auto b2 = a1->MakeRight();
    auto c1 = b2->MakeLeft();
    auto c2 = b2->MakeRight();
    REQUIRE(a2 == tree.GetLightestNode());
    tree.RemoveNode(a2);
    REQUIRE(tree.GetLightestNode() == b1);
    tree.RemoveNode(b1);
    REQUIRE(c2 == tree.GetLightestNode());
    tree.RemoveNode(c2);
    REQUIRE(c1 == tree.GetLightestNode());
    tree.RemoveNode(c1);
    REQUIRE(!tree.IsNonEmpty());
}

TEST_CASE("DataSignal") {
    complex_t data[] = {0, 1, 2, 3, 4};
    DataSignal x(5, data);
    int p = 0;
    for (int t = -10; t < 10; ++t) {
        REQUIRE(x.ValueAtTime(t) == data[p]);
        p = (p + 1) % 5;

    }
}

TEST_CASE("ZeroTest 1") {
    auto tree = SplittingTree();
    auto root = tree.GetRoot();
    auto a1 = root->MakeLeft();
    auto a2 = root->MakeRight();
    FrequencyMap chi{};
    IndexGenerator delta(4, 321);

    {
        // ifft([1, 0, 0, 0])
        complex_t data[] = {0.25 + 0.j, 0.25 - 0.j, 0.25 + 0.j, 0.25 + 0.j};
        DataSignal x(4, data);

        REQUIRE(!ZeroTest(x, chi, a1, 4, 1, delta));
        REQUIRE(ZeroTest(x, chi, a2, 4, 1, delta));
    }
    {
        // ifft([1, 0, 1, 0])
        complex_t data[] = {0.5+0.j, 0. -0.j, 0.5+0.j, 0. +0.j};
        DataSignal x(4, data);

        REQUIRE(!ZeroTest(x, chi, a1, 4, 2, delta));
        REQUIRE(ZeroTest(x, chi, a2, 4, 2, delta));
    }
    {
        // ifft([0, 1, 1, 0])
        complex_t data[] = {0.5 +0.j  , -0.25+0.25j,  0.  +0.j  , -0.25-0.25j};
        DataSignal x(4, data);

        REQUIRE(ZeroTest(x, chi, a1, 4, 2, delta));
        REQUIRE(ZeroTest(x, chi, a2, 4, 2, delta));
    }
}

TEST_CASE("ZeroTest 2") {
    auto tree = SplittingTree();
    auto root = tree.GetRoot();
    auto a = root->MakeRight();
    auto b1 = root->MakeLeft();
    auto b2 = b1->MakeLeft();
    auto b3 = b2->MakeRight();
    IndexGenerator delta(4, 321);

    {
        // ifft([0, 0, 0, 0, 0, 0, 0, 0])
        complex_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};
        FrequencyMap chi{};
        DataSignal x(8, data);

        REQUIRE(!ZeroTest(x, chi, a, 8, 1, delta));
        REQUIRE(!ZeroTest(x, chi, b3, 8, 1, delta));
    }
    {
        // ifft([0, 0, 0, 1, 0, 0, 0, 0])
        complex_t data[] = {0.125     +0.j        , -0.08838835+0.08838835j,
                            0.        -0.125j     ,  0.08838835+0.08838835j,
                            -0.125     +0.j        ,  0.08838835-0.08838835j,
                            0.        +0.125j     , -0.08838835-0.08838835j };
        FrequencyMap chi{};
        DataSignal x(8, data);

        REQUIRE(!ZeroTest(x, chi, a, 8, 1, delta));
        REQUIRE(ZeroTest(x, chi, b3, 8, 1, delta));
    }
    {
        // ifft([0, 0, 1, 1, 1, 0, 0, 0])
        complex_t data[] = { 0.375     +0.j        , -0.21338835+0.21338835j,
                             0.        -0.125j     , -0.03661165-0.03661165j,
                             0.125     +0.j        , -0.03661165+0.03661165j,
                             0.        +0.125j     , -0.21338835-0.21338835j};
        FrequencyMap chi{};
        DataSignal x(8, data);

        REQUIRE(ZeroTest(x, chi, a, 8, 3, delta));
        REQUIRE(ZeroTest(x, chi, b3, 8, 3, delta));
    }
    {
        // ifft([0, 0, 0, 1, 0, 0, 0, 1])
        complex_t data[] = { 0.25+0.j  ,  0.  -0.j  ,  0.  -0.25j,  0.  -0.j  , -0.25+0.j  ,
                             0.  +0.j  ,  0.  +0.25j,  0.  +0.j  };
        FrequencyMap chi;
        chi[7] = 1.;
        DataSignal x(8, data);

        REQUIRE(!ZeroTest(x, chi, a, 8, 2, delta));
        REQUIRE(ZeroTest(x, chi, b3, 8, 2, delta));
    }
    {
        // ifft([0, 0, 0, 2, 0, -1, 0, 3])
        complex_t data[] = { 0.5      +0.00000000e+00j,  0.1767767-1.38777878e-17j,
                             0.       -7.50000000e-01j, -0.1767767-1.38777878e-17j,
                             -0.5      +0.00000000e+00j, -0.1767767+1.38777878e-17j,
                             0.       +7.50000000e-01j,  0.1767767+1.38777878e-17j};
        FrequencyMap chi;
        chi[7] = 3.;
        chi[5] = -1.;
        DataSignal x(8, data);

        REQUIRE(!ZeroTest(x, chi, a, 8, 3, delta));
        REQUIRE(ZeroTest(x, chi, b3, 8, 3, delta));
    }
    {
        // ifft([0, 0, 0, 0, 0, -1, 0, 3])
        complex_t data[] = { 0.25      +0.j       ,  0.35355339-0.1767767j,
                             0.        -0.5j      , -0.35355339-0.1767767j,
                             -0.25      +0.j       , -0.35355339+0.1767767j,
                             0.        +0.5j      ,  0.35355339+0.1767767j};
        FrequencyMap chi;
        chi[7] = 3.;
        chi[5] = -1.;
        DataSignal x(8, data);

        REQUIRE(!ZeroTest(x, chi, a, 8, 3, delta));
        REQUIRE(!ZeroTest(x, chi, b3, 8, 3, delta));
    }
}