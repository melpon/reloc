#ifndef RELOC_DETAIL_ALLOC_NODE_HPP_INCLUDED
#define RELOC_DETAIL_ALLOC_NODE_HPP_INCLUDED

#include <cstddef>
#include "type.hpp"

namespace reloc { namespace detail {

struct alloc_node {
    byte* ptr;
    std::size_t size;
    std::size_t pinned;
};

}}

#endif // RELOC_DETAIL_ALLOC_NODE_HPP_INCLUDED
