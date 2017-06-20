#include "gtest/gtest.h"
#include "indexed_dpd_varray.hpp"

using namespace std;
using namespace MArray;

template <typename T, unsigned... Sizes>
struct arrays_helper;

template <typename T, unsigned Size1, unsigned Size2>
struct arrays_helper<T, Size1, Size2>
{
    typedef array<vector<T>,Size1> type;
};

template <typename T, unsigned Size1, unsigned Size2, unsigned Size3>
struct arrays_helper<T, Size1, Size2, Size3>
{
    typedef array<array<vector<T>,Size2>,Size1> type;
};

template <typename T, unsigned... Sizes>
using arrays = typename arrays_helper<T, Sizes...>::type;

static array<dpd_layout,6> layouts =
{{
    PREFIX_ROW_MAJOR,
    PREFIX_COLUMN_MAJOR,
    BLOCKED_ROW_MAJOR,
    BLOCKED_COLUMN_MAJOR,
    BALANCED_ROW_MAJOR,
    BALANCED_COLUMN_MAJOR,
}};

static arrays<unsigned,6,4> perms =
    {{{3,2,1,0}, {0,1,2,3}, {3,2,1,0}, {0,1,2,3}, {3,2,1,0}, {0,1,2,3}}};

static arrays<unsigned,8,4> irreps =
    {{{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {1,1,1,0},
      {0,0,0,1}, {1,1,0,1}, {1,0,1,1}, {0,1,1,1}}};
static arrays<len_type,8,4> lengths =
    {{{1,2,1,3}, {3,2,1,3}, {3,2,2,3}, {1,2,2,3},
      {3,2,1,4}, {1,2,1,4}, {1,2,2,4}, {3,2,2,4}}};

static arrays<stride_type,6,8,4> strides =
{{
    {{{42,11, 3, 1}, {42,11, 3, 1}, {42,10, 3, 1}, {42,10, 3, 1},
      {42,10, 4, 1}, {42,10, 4, 1}, {42,11, 4, 1}, {42,11, 4, 1}}},
    {{{ 1, 1, 8,24}, { 1, 3, 8,24}, { 1, 3, 8,24}, { 1, 1, 8,24},
      { 1, 3, 8,24}, { 1, 1, 8,24}, { 1, 1, 8,24}, { 1, 3, 8,24}}},
    {{{ 6, 3, 3, 1}, { 6, 3, 3, 1}, {12, 6, 3, 1}, {12, 6, 3, 1},
      { 8, 4, 4, 1}, { 8, 4, 4, 1}, {16, 8, 4, 1}, {16, 8, 4, 1}}},
    {{{ 1, 1, 2, 2}, { 1, 3, 6, 6}, { 1, 3, 6,12}, { 1, 1, 2, 4},
      { 1, 3, 6, 6}, { 1, 1, 2, 2}, { 1, 1, 2, 4}, { 1, 3, 6,12}}},
    {{{22,11, 3, 1}, {22,11, 3, 1}, {20,10, 3, 1}, {20,10, 3, 1},
      {20,10, 4, 1}, {20,10, 4, 1}, {22,11, 4, 1}, {22,11, 4, 1}}},
    {{{ 1, 1, 8, 8}, { 1, 3, 8, 8}, { 1, 3, 8,16}, { 1, 1, 8,16},
      { 1, 3, 8, 8}, { 1, 1, 8, 8}, { 1, 1, 8,16}, { 1, 3, 8,16}}}
}};

static arrays<stride_type,6,8> offsets =
{{
     {126, 20,  4,152,  0,148,129, 23},
     {  0,  2,  8, 14, 72, 78, 80, 82},
     {126, 60, 24,156,  0,148,132, 78},
     {  0,  6, 24, 60, 72, 96,104,120},
     { 80+66, 80   ,     0+4,  0+60+4,
        0   ,  0+60, 80+66+3, 80   +3},
     {  0   ,  0   +2, 88   , 88   +6,
       88+48, 88+48+6,  0+24,  0+24+2}
}};

#define CHECK_INDEXED_DPD_VARRAY_RESET(v) \
    EXPECT_EQ(0u, v.dimension()); \
    EXPECT_EQ(0u, v.dense_dimension()); \
    EXPECT_EQ(0u, v.indexed_dimension()); \
    EXPECT_EQ(0u, v.num_indices()); \
    EXPECT_EQ((std::vector<unsigned>{}), v.permutation()); \
    EXPECT_EQ((matrix<len_type>{}), v.lengths()); \
    EXPECT_EQ(0u, v.data().length()); \
    EXPECT_EQ(nullptr, v.data().data());

#define CHECK_INDEXED_DPD_VARRAY(v,j,value,...) \
    SCOPED_TRACE(j); \
    EXPECT_EQ(6u, v.dimension()); \
    EXPECT_EQ(4u, v.dense_dimension()); \
    EXPECT_EQ(2u, v.indexed_dimension()); \
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {0, 2}, {0, 5}}), v.lengths()); \
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v.dense_lengths()); \
    EXPECT_EQ((std::vector<len_type>{2, 5}), v.indexed_lengths()); \
    EXPECT_EQ(3u, v.num_indices()); \
    EXPECT_EQ((std::vector<unsigned>{1, 1}), v.indexed_irreps()); \
    EXPECT_EQ((matrix<len_type>{{0, 0}, {1, 3}, {0, 3}}), v.indices()); \
    EXPECT_EQ(value, v.data(0)[0]); \
    EXPECT_EQ(1u, v.irrep()); \
    EXPECT_EQ(2u, v.num_irreps()); \
    EXPECT_EQ(perms[j], v.permutation()); \
    \
    for (unsigned m = 0;m < 3u;m++) \
    { \
        SCOPED_TRACE(m); \
        { \
            auto vs = v[m](1,0,0,0); \
            EXPECT_EQ(v.data(m) + offsets[j][0], vs.data()); \
            for (unsigned k = 0;k < 4;k++) \
            { \
                EXPECT_EQ(lengths[0][k], vs.length(k)); \
                EXPECT_EQ(strides[j][0][k], vs.stride(k)); \
            } \
        } \
        \
        { \
            auto vs = v[m]({0,1,0,0}); \
            EXPECT_EQ(v.data(m) + offsets[j][1], vs.data()); \
            EXPECT_EQ(lengths[1], vs.lengths()); \
            EXPECT_EQ(strides[j][1], vs.strides()); \
        } \
        \
        for (unsigned i = 2;i < 8;i++) \
        { \
            SCOPED_TRACE(i); \
            auto vs = v[m](irreps[i]); \
            EXPECT_EQ(v.data(m) + offsets[j][i], vs.data()); \
            EXPECT_EQ(lengths[i], vs.lengths()); \
            EXPECT_EQ(strides[j][i], vs.strides()); \
        } \
    }

TEST(indexed_dpd_varray, constructor)
{
    indexed_dpd_varray<double> v1;
    CHECK_INDEXED_DPD_VARRAY_RESET(v1)

    for (int j = 0;j < 6;j++)
    {
        indexed_dpd_varray<double> v2(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                      {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v2, j, 0.0)

        indexed_dpd_varray<double> v3(1, 2, arrays<char,6,2>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}}},
                                      vector<char>{1, 1}, arrays<char,3,2>{{{0, 0}, {1, 3}, {0, 3}}}, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v3, j, 0.0)

        indexed_dpd_varray<double> v21(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                       {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v21, j, 1.0)

        indexed_dpd_varray<double> v31(1, 2, arrays<char,6,2>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}}},
                                       vector<char>{1, 1}, arrays<char,3,2>{{{0, 0}, {1, 3}, {0, 3}}}, 1.0, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v31, j, 1.0)

        indexed_dpd_varray<double> v5(v21.view(), layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v5, j, 1.0)

        indexed_dpd_varray<double> v52(v21.cview(), layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v52, j, 1.0)

        indexed_dpd_varray<double> v51(v21);
        CHECK_INDEXED_DPD_VARRAY(v51, j, 1.0)

        indexed_dpd_varray<double> v53(v21, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v53, j, 1.0)
    }
}

TEST(indexed_dpd_varray, reset)
{
    indexed_dpd_varray<double> v1;
    CHECK_INDEXED_DPD_VARRAY_RESET(v1)

    for (int j = 0;j < 6;j++)
    {
        indexed_dpd_varray<double> v2(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                      {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[j]);

        v1.reset(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                      {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 0.0)

        v1.reset(1, 2, arrays<char,6,2>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}}},
                                      vector<char>{1, 1}, arrays<char,3,2>{{{0, 0}, {1, 3}, {0, 3}}}, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 0.0)

        v1.reset(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                       {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)

        v1.reset(1, 2, arrays<char,6,2>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}}},
                                       vector<char>{1, 1}, arrays<char,3,2>{{{0, 0}, {1, 3}, {0, 3}}}, 1.0, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)

        v1.reset(v2.view(), layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)

        v1.reset(v2.cview(), layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)

        v1.reset(v2);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)

        v1.reset(v2, layouts[j]);
        CHECK_INDEXED_DPD_VARRAY(v1, j, 1.0)
    }

    v1.reset();
    CHECK_INDEXED_DPD_VARRAY_RESET(v1)
}

TEST(indexed_dpd_varray, view)
{
    indexed_dpd_varray<double> v1(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                  {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[0]);

    auto v2 = v1.cview();
    CHECK_INDEXED_DPD_VARRAY(v2, 0, 1.0)

    auto v3 = v1.view();
    CHECK_INDEXED_DPD_VARRAY(v3, 0, 1.0)

    auto v4 = const_cast<const indexed_dpd_varray<double>&>(v1).view();
    CHECK_INDEXED_DPD_VARRAY(v4, 0, 1.0)
}

TEST(indexed_dpd_varray, access)
{
    indexed_dpd_varray<double> v1(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                  {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[0]);

    auto v2 = v1[0];
    EXPECT_EQ(v1.data(0), v2.data());
    EXPECT_EQ(1u, v2.irrep());
    EXPECT_EQ(2u, v2.num_irreps());
    EXPECT_EQ(perms[0], v2.permutation());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v2.lengths());

    auto v3 = const_cast<const indexed_dpd_varray<double>&>(v1)[2];
    EXPECT_EQ(v1.data(2), v3.data());
    EXPECT_EQ(1u, v3.irrep());
    EXPECT_EQ(2u, v3.num_irreps());
    EXPECT_EQ(perms[0], v3.permutation());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v3.lengths());
}

TEST(indexed_dpd_varray, index_iteration)
{
    int indices[3][2] = {{0, 0}, {1, 3}, {0, 3}};
    array<int,3> visited;

    indexed_dpd_varray<double> v1(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                  {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[0]);
    const indexed_dpd_varray<double> v2(v1);

    visited = {};
    v1.for_each_index(
    [&](const dpd_varray_view<double>& v, const std::vector<len_type>& idx)
    {
        EXPECT_EQ(idx.size(), 2u);
        len_type i = idx[0];
        len_type j = idx[1];
        bool found = false;
        for (int m = 0;m < 3;m++)
        {
            if (i == indices[m][0] && j == indices[m][1])
            {
                EXPECT_EQ(v1.data(m), v.data());
                found = true;
                visited[m]++;
            }
        }
        EXPECT_TRUE(found);
        EXPECT_EQ(1u, v.irrep());
        EXPECT_EQ(2u, v.num_irreps());
        EXPECT_EQ(perms[0], v.permutation());
        EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v.lengths());
    });

    for (len_type i = 0;i < 3;i++)
    {
        EXPECT_EQ(visited[i], 1);
    }

    visited = {};
    v2.for_each_index(
    [&](const dpd_varray_view<const double>& v, const std::vector<len_type>& idx)
    {
        EXPECT_EQ(idx.size(), 2u);
        len_type i = idx[0];
        len_type j = idx[1];
        bool found = false;
        for (int m = 0;m < 3;m++)
        {
            if (i == indices[m][0] && j == indices[m][1])
            {
                EXPECT_EQ(v2.data(m), v.data());
                found = true;
                visited[m]++;
            }
        }
        EXPECT_TRUE(found);
        EXPECT_EQ(1u, v.irrep());
        EXPECT_EQ(2u, v.num_irreps());
        EXPECT_EQ(perms[0], v.permutation());
        EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v.lengths());
    });

    for (len_type i = 0;i < 3;i++)
    {
        EXPECT_EQ(visited[i], 1);
    }

    visited = {};
    v1.for_each_index<4,2>(
    [&](const dpd_marray_view<double,4>& v, len_type i, len_type j)
    {
        bool found = false;
        for (int m = 0;m < 3;m++)
        {
            if (i == indices[m][0] && j == indices[m][1])
            {
                EXPECT_EQ(v1.data(m), v.data());
                found = true;
                visited[m]++;
            }
        }
        EXPECT_TRUE(found);
        EXPECT_EQ(1u, v.irrep());
        EXPECT_EQ(2u, v.num_irreps());
        EXPECT_EQ((*reinterpret_cast<std::array<unsigned,4>*>(perms[0].data())), v.permutation());
        EXPECT_EQ((std::array<std::array<len_type,8>,4>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}}}), v.lengths());
    });

    for (len_type i = 0;i < 3;i++)
    {
        EXPECT_EQ(visited[i], 1);
    }

    visited = {};
    v2.for_each_index<4,2>(
    [&](const dpd_marray_view<const double,4>& v, len_type i, len_type j)
    {
        bool found = false;
        for (int m = 0;m < 3;m++)
        {
            if (i == indices[m][0] && j == indices[m][1])
            {
                EXPECT_EQ(v2.data(m), v.data());
                found = true;
                visited[m]++;
            }
        }
        EXPECT_TRUE(found);
        EXPECT_EQ(1u, v.irrep());
        EXPECT_EQ(2u, v.num_irreps());
        EXPECT_EQ((*reinterpret_cast<std::array<unsigned,4>*>(perms[0].data())), v.permutation());
        EXPECT_EQ((std::array<std::array<len_type,8>,4>{{{3, 1}, {2, 2}, {1, 2}, {3, 4}}}), v.lengths());
    });

    for (len_type i = 0;i < 3;i++)
    {
        EXPECT_EQ(visited[i], 1);
    }
}

TEST(indexed_dpd_varray, element_iteration)
{
    arrays<len_type,3,2> indices = {{{0, 0}, {1, 3}, {0, 3}}};
    std::array<std::array<int,3>,31> visited;
    arrays<len_type,5,2> len = {{{2, 3}, {1, 2}, {3, 1}, {2, 2}, {4, 5}}};

    indexed_dpd_varray<double> v1(0, 2, len, vector<int>{1, 1}, indices, 1.0, layouts[0]);
    const indexed_dpd_varray<double> v2(v1);

    visited = {};
    v1.for_each_element(
    [&](double& v, const std::vector<unsigned>& irreps, const std::vector<len_type>& idx)
    {
        EXPECT_EQ(irreps.size(), 5u);
        EXPECT_EQ(idx.size(), 5u);
        unsigned a = irreps[0];
        unsigned b = irreps[1];
        unsigned c = irreps[2];
        unsigned d = irreps[3];
        unsigned e = irreps[4];
        EXPECT_LT(a, 2u);
        EXPECT_LT(b, 2u);
        EXPECT_LT(c, 2u);
        EXPECT_EQ(d, 1u);
        EXPECT_EQ(e, 1u);
        EXPECT_EQ(a^b^c^d^e, 0u);
        len_type i = idx[0];
        len_type j = idx[1];
        len_type k = idx[2];
        len_type l = idx[3];
        len_type m = idx[4];
        EXPECT_GE(i, 0);
        EXPECT_LT(i, len[0][a]);
        EXPECT_GE(j, 0);
        EXPECT_LT(j, len[1][b]);
        EXPECT_GE(k, 0);
        EXPECT_LT(k, len[2][c]);
        bool found = false;
        for (int n = 0;n < 3;n++)
        {
            if (l == indices[n][0] && m == indices[n][1])
            {
                auto v3 = v1[n](a, b, c);
                EXPECT_EQ(&v, &v3(i, j, k));
                visited[&v - v1.data(n)][n]++;
                found = true;
            }
        }
        EXPECT_TRUE(found);
    });

    for (len_type i = 0;i < 31;i++)
    {
        for (len_type j = 0;j < 3;j++)
        {
            EXPECT_EQ(visited[i][j], 1);
        }
    }

    visited = {};
    v2.for_each_element(
    [&](const double& v, const std::vector<unsigned>& irreps, const std::vector<len_type>& idx)
    {
        EXPECT_EQ(irreps.size(), 5u);
        EXPECT_EQ(idx.size(), 5u);
        unsigned a = irreps[0];
        unsigned b = irreps[1];
        unsigned c = irreps[2];
        unsigned d = irreps[3];
        unsigned e = irreps[4];
        EXPECT_LT(a, 2u);
        EXPECT_LT(b, 2u);
        EXPECT_LT(c, 2u);
        EXPECT_EQ(d, 1u);
        EXPECT_EQ(e, 1u);
        EXPECT_EQ(a^b^c^d^e, 0u);
        len_type i = idx[0];
        len_type j = idx[1];
        len_type k = idx[2];
        len_type l = idx[3];
        len_type m = idx[4];
        EXPECT_GE(i, 0);
        EXPECT_LT(i, len[0][a]);
        EXPECT_GE(j, 0);
        EXPECT_LT(j, len[1][b]);
        EXPECT_GE(k, 0);
        EXPECT_LT(k, len[2][c]);
        bool found = false;
        for (int n = 0;n < 3;n++)
        {
            if (l == indices[n][0] && m == indices[n][1])
            {
                auto v3 = v2[n](a, b, c);
                EXPECT_EQ(&v, &v3(i, j, k));
                visited[&v - v2.data(n)][n]++;
                found = true;
            }
        }
        EXPECT_TRUE(found);
    });

    for (len_type i = 0;i < 31;i++)
    {
        for (len_type j = 0;j < 3;j++)
        {
            EXPECT_EQ(visited[i][j], 1);
        }
    }

    visited = {};
    v1.for_each_element<3,2>(
    [&](double& v, unsigned a, unsigned b, unsigned c, unsigned d, unsigned e,
        len_type i, len_type j, len_type k, len_type l, len_type m)
    {
        EXPECT_LT(a, 2u);
        EXPECT_LT(b, 2u);
        EXPECT_LT(c, 2u);
        EXPECT_EQ(d, 1u);
        EXPECT_EQ(e, 1u);
        EXPECT_EQ(a^b^c^d^e, 0u);
        EXPECT_GE(i, 0);
        EXPECT_LT(i, len[0][a]);
        EXPECT_GE(j, 0);
        EXPECT_LT(j, len[1][b]);
        EXPECT_GE(k, 0);
        EXPECT_LT(k, len[2][c]);
        bool found = false;
        for (int n = 0;n < 3;n++)
        {
            if (l == indices[n][0] && m == indices[n][1])
            {
                auto v3 = v1[n](a, b, c);
                EXPECT_EQ(&v, &v3(i, j, k));
                visited[&v - v1.data(n)][n]++;
                found = true;
            }
        }
        EXPECT_TRUE(found);
    });

    for (len_type i = 0;i < 31;i++)
    {
        for (len_type j = 0;j < 3;j++)
        {
            EXPECT_EQ(visited[i][j], 1);
        }
    }

    visited = {};
    v2.for_each_element<3,2>(
    [&](const double& v, unsigned a, unsigned b, unsigned c, unsigned d, unsigned e,
        len_type i, len_type j, len_type k, len_type l, len_type m)
    {
        EXPECT_LT(a, 2u);
        EXPECT_LT(b, 2u);
        EXPECT_LT(c, 2u);
        EXPECT_EQ(d, 1u);
        EXPECT_EQ(e, 1u);
        EXPECT_EQ(a^b^c^d^e, 0u);
        EXPECT_GE(i, 0);
        EXPECT_LT(i, len[0][a]);
        EXPECT_GE(j, 0);
        EXPECT_LT(j, len[1][b]);
        EXPECT_GE(k, 0);
        EXPECT_LT(k, len[2][c]);
        bool found = false;
        for (int n = 0;n < 3;n++)
        {
            if (l == indices[n][0] && m == indices[n][1])
            {
                auto v3 = v2[n](a, b, c);
                EXPECT_EQ(&v, &v3(i, j, k));
                visited[&v - v2.data(n)][n]++;
                found = true;
            }
        }
        EXPECT_TRUE(found);
    });

    for (len_type i = 0;i < 31;i++)
    {
        for (len_type j = 0;j < 3;j++)
        {
            EXPECT_EQ(visited[i][j], 1);
        }
    }
}

TEST(indexed_dpd_varray, swap)
{
    indexed_dpd_varray<double> v1(1, 2, {{3, 1}, {2, 2}, {1, 2}, {3, 4}, {2, 2}, {4, 5}},
                                  {1, 1}, {{0, 0}, {1, 3}, {0, 3}}, 1.0, layouts[0]);
    indexed_dpd_varray<double> v2(0, 2, {{2, 3}, {1, 2}, {3, 1}, {2, 2}, {4, 5}},
                                  {1, 0}, {{0, 0}, {1, 0}, {1, 2}}, 1.0, layouts[0]);

    auto data1 = v1.data();
    auto data2 = v2.data();

    v1.swap(v2);

    EXPECT_EQ(6u, v2.dimension());
    EXPECT_EQ(4u, v2.dense_dimension());
    EXPECT_EQ(2u, v2.indexed_dimension());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {0, 2}, {0, 5}}), v2.lengths());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v2.dense_lengths());
    EXPECT_EQ((std::vector<len_type>{2, 5}), v2.indexed_lengths());
    EXPECT_EQ(3u, v2.num_indices());
    EXPECT_EQ((std::vector<unsigned>{1, 1}), v2.indexed_irreps());
    EXPECT_EQ((matrix<len_type>{{0, 0}, {1, 3}, {0, 3}}), v2.indices());
    EXPECT_EQ(1u, v2.irrep());
    EXPECT_EQ(2u, v2.num_irreps());

    EXPECT_EQ(5u, v1.dimension());
    EXPECT_EQ(3u, v1.dense_dimension());
    EXPECT_EQ(2u, v1.indexed_dimension());
    EXPECT_EQ((matrix<len_type>{{2, 3}, {1, 2}, {3, 1}, {0, 2}, {4, 0}}), v1.lengths());
    EXPECT_EQ((matrix<len_type>{{2, 3}, {1, 2}, {3, 1}}), v1.dense_lengths());
    EXPECT_EQ((std::vector<len_type>{2, 4}), v1.indexed_lengths());
    EXPECT_EQ(3u, v1.num_indices());
    EXPECT_EQ((std::vector<unsigned>{1, 0}), v1.indexed_irreps());
    EXPECT_EQ((matrix<len_type>{{0, 0}, {1, 0}, {1, 2}}), v1.indices());
    EXPECT_EQ(0u, v1.irrep());
    EXPECT_EQ(2u, v1.num_irreps());

    swap(v2, v1);

    EXPECT_EQ(6u, v1.dimension());
    EXPECT_EQ(4u, v1.dense_dimension());
    EXPECT_EQ(2u, v1.indexed_dimension());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}, {0, 2}, {0, 5}}), v1.lengths());
    EXPECT_EQ((matrix<len_type>{{3, 1}, {2, 2}, {1, 2}, {3, 4}}), v1.dense_lengths());
    EXPECT_EQ((std::vector<len_type>{2, 5}), v1.indexed_lengths());
    EXPECT_EQ(3u, v1.num_indices());
    EXPECT_EQ((std::vector<unsigned>{1, 1}), v1.indexed_irreps());
    EXPECT_EQ((matrix<len_type>{{0, 0}, {1, 3}, {0, 3}}), v1.indices());
    EXPECT_EQ(1u, v1.irrep());
    EXPECT_EQ(2u, v1.num_irreps());

    EXPECT_EQ(5u, v2.dimension());
    EXPECT_EQ(3u, v2.dense_dimension());
    EXPECT_EQ(2u, v2.indexed_dimension());
    EXPECT_EQ((matrix<len_type>{{2, 3}, {1, 2}, {3, 1}, {0, 2}, {4, 0}}), v2.lengths());
    EXPECT_EQ((matrix<len_type>{{2, 3}, {1, 2}, {3, 1}}), v2.dense_lengths());
    EXPECT_EQ((std::vector<len_type>{2, 4}), v2.indexed_lengths());
    EXPECT_EQ(3u, v2.num_indices());
    EXPECT_EQ((std::vector<unsigned>{1, 0}), v2.indexed_irreps());
    EXPECT_EQ((matrix<len_type>{{0, 0}, {1, 0}, {1, 2}}), v2.indices());
    EXPECT_EQ(0u, v2.irrep());
    EXPECT_EQ(2u, v2.num_irreps());
}