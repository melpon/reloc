#ifndef RELOC_RELOC_PTR_HPP_INCLUDED
#define RELOC_RELOC_PTR_HPP_INCLUDED

#include "detail/alloc_node.hpp"
#include "pinned_ptr.hpp"

namespace reloc {

class reloc_ptr {
public:
    typedef reloc_ptr this_type;
    typedef detail::alloc_node* pointer;

private:
    pointer p_;

public:
    reloc_ptr() : p_(0) { }
    explicit reloc_ptr(pointer p) : p_(p) { }
    pointer get() const { return p_; }
    pinned_ptr pin() const { return pinned_ptr(p_); }

    typedef pointer this_type::*unspecified_bool_type;
    operator unspecified_bool_type() const {
        return p_ == 0 ? 0 : &this_type::p_;
    }
};

inline bool operator==(const reloc_ptr& a, const reloc_ptr& b) {
    return a.get() == b.get();
}
inline bool operator!=(const reloc_ptr& a, const reloc_ptr& b) {
    return !(a == b);
}
inline bool operator<(const reloc_ptr& a, const reloc_ptr& b) {
    return a.get() < b.get();
}
inline bool operator>(const reloc_ptr& a, const reloc_ptr& b) {
    return b < a;
}
inline bool operator<=(const reloc_ptr& a, const reloc_ptr& b) {
    return !(b < a);
}
inline bool operator>=(const reloc_ptr& a, const reloc_ptr& b) {
    return !(a < b);
}

}

#endif // RELOC_RELOC_PTR_HPP_INCLUDED
