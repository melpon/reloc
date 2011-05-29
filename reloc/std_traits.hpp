#ifndef RELOC_STD_TRAITS_HPP_INCLUDED
#define RELOC_STD_TRAITS_HPP_INCLUDED

#include <algorithm>

namespace reloc {

struct std_traits {
    static void construct(void* p) { } // nothrow
    static void destroy(void* p) { } // nothrow
    static void move(const void* src, std::size_t size, void* dst) { // nothrow
        std::copy(static_cast<const unsigned char*>(src), static_cast<const unsigned char*>(src) + size, static_cast<unsigned char*>(dst));
    }
};

}

#endif // RELOC_STD_TRAITS_HPP_INCLUDED
