
#include "arref/arref.hpp"
#include <iostream>
#include <vector>
#include <numeric>
#include <cassert>

template <typename T, size_t D>
struct Block {
    arref::arref<T, D> data;
    std::array<size_t, D> dims;
};

void f2(const arref::arref<int, 2>& r) {
    //r += arref::dim<0>{1};
}

int main () {
    std::vector<int> v(10 * 10 * 10);
    std::iota(v.begin(), v.end(), 0);

    auto r = arref::make_arref(v.data(), 100u, 10u, 1u);

    for (int m = 0; m < 10; ++m) {
        for (int n = 0; n < 10; ++n) {
            for (int o = 0; o < 10; ++o) {
                assert(r[m][n][o] == v[m*100 + n*10 +o]);
            }
        }
    }

    using arref::dim;
    auto r2 = r + dim<2>{1} + dim<1>{-4} - dim<0>{2};
    for (int m = 0; m < 10; ++m) {
        for (int n = 0; n < 10; ++n) {
            for (int o = 0; o < 10; ++o) {
                assert(r2[m-1][n+4][o+2] == v[m*100 + n*10 +o]);
            }
        }
    }
    r2[4][5][6] = -1;

    auto r4 = r;
    r4 = std::move(r2);
    const auto r3 = arref::make_arref<const int>(v.data(), 100u, 10u, 1u);
    auto cr = r3;
    auto sub = cr[5];
    //sub[5][5] = -1;

    auto str = sub.transpose(0, 1);
    for (int m = 0; m < 10; ++m) {
        for (int n = 0; n < 10; ++n) {
            assert(str[n][m] == v[5*100 + m*10 +n]);
        }
    }
    f2(r[3].transpose(0, 1));
    f2(r2[1]);

    auto tr = r.transpose(1, 0, 2);
    for (int m = 0; m < 10; ++m) {
        for (int n = 0; n < 10; ++n) {
            for (int o = 0; o < 10; ++o) {
                assert(tr[n][o][m] == v[m*100 + n*10 +o]);
            }
        }
    }

}
