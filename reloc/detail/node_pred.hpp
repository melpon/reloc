#ifndef RELOC_DETAIL_NODE_PRED_HPP_INCLUDED
#define RELOC_DETAIL_NODE_PRED_HPP_INCLUDED

#include <cstddef>
#include "type.hpp"
#include "alloc_node.hpp"
#include "free_node.hpp"

namespace reloc { namespace detail {

struct alloc_node_pred {
    typedef byte* key_type;
    typedef alloc_node* value_type;

    bool operator()(const key_type& a, const key_type& b) const {
        return a < b;
    }
    bool operator()(const key_type& a, const value_type& b) const {
        return a < b->ptr;
    }
    bool operator()(const value_type& a, const key_type& b) const {
        return a->ptr < b;
    }
    bool operator()(const value_type& a, const value_type& b) const {
        return a->ptr < b->ptr;
    }
};

struct free_node_pred {
    typedef byte* key_type;
    typedef free_node value_type;

    bool operator()(const key_type& a, const key_type& b) const {
        return a < b;
    }
    bool operator()(const key_type& a, const value_type& b) const {
        return a < b.ptr;
    }
    bool operator()(const value_type& a, const key_type& b) const {
        return a.ptr < b;
    }
    bool operator()(const value_type& a, const value_type& b) const {
        return a.ptr < b.ptr;
    }
};

}}

#endif // RELOC_DETAIL_FREE_NODE_HPP_INCLUDED
