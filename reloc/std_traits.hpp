#ifndef RELOC_STD_TRAITS_HPP_INCLUDED
#define RELOC_STD_TRAITS_HPP_INCLUDED

#include <algorithm>

namespace reloc {

struct std_traits {
    static void construct(void* p) { } // nothrow
    static void destroy(void* p) { } // nothrow

    // src と dst のポインタの領域がオーバーラップしている場合は move_left または move_right、
    // そうでない場合は copy が呼ばれる。
    // オーバーラップしている場合で、dst < src である場合には move_left が、
    // src < dst である場合には move_right が呼ばれる。
    static void move_left(const void* src, std::size_t size, void* dst) { // nothrow
        std::copy(static_cast<const unsigned char*>(src), static_cast<const unsigned char*>(src) + size, static_cast<unsigned char*>(dst));
    }
    static void move_right(const void* src, std::size_t size, void* dst) { // nothrow
        std::copy_backward(static_cast<const unsigned char*>(src), static_cast<const unsigned char*>(src) + size, static_cast<unsigned char*>(dst) + size);
    }
    static void copy(const void* src, std::size_t size, void* dst) { // nothrow
        move_left(src, size, dst);
    }
};

}

#endif // RELOC_STD_TRAITS_HPP_INCLUDED
