#ifndef RELOC_DETAIL_FREE_NODE_HPP_INCLUDED
#define RELOC_DETAIL_FREE_NODE_HPP_INCLUDED

#include <cstddef>
#include "type.hpp"

namespace reloc { namespace detail {

struct free_node {
    byte* ptr;
    std::size_t size;
};

}}

#endif // RELOC_DETAIL_FREE_NODE_HPP_INCLUDED
