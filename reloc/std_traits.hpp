#ifndef RELOC_STD_TRAITS_HPP_INCLUDED
#define RELOC_STD_TRAITS_HPP_INCLUDED

#include <algorithm>

namespace reloc {

struct std_traits {
    static void construct(void* p) { } // nothrow
    static void destroy(void* p) { } // nothrow

    // src と dst のポインタの領域がオーバーラップしている場合は move、
    // そうでない場合は copy が呼ばれる。
    // （以下の保証は後で除けるかも）
    // copy, move の際に、dst < src であることは保証されている。
    // つまり領域は左（アドレスの小さい方）にしか移動しない
    static void move(const void* src, std::size_t size, void* dst) { // nothrow
        std::copy(static_cast<const unsigned char*>(src), static_cast<const unsigned char*>(src) + size, static_cast<unsigned char*>(dst));
    }
    static void copy(const void* src, std::size_t size, void* dst) { // nothrow
        move(src, size, dst);
    }
};

}

#endif // RELOC_STD_TRAITS_HPP_INCLUDED
