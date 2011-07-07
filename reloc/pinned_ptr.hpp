#ifndef RELOC_PINNED_PTR_HPP_INCLUDED
#define RELOC_PINNED_PTR_HPP_INCLUDED

#include <algorithm>
#include <utility>
#include "detail/alloc_node.hpp"

namespace reloc {

class pinned_ptr {
public:
    typedef pinned_ptr this_type;
    typedef detail::alloc_node* pointer;

private:
    pointer p_;

public:
    pinned_ptr() : p_(0) { }
    pinned_ptr(const pinned_ptr& p) : p_(p.p_) {
        if (p_) ++p_->pinned;
    }
    explicit pinned_ptr(pointer p, bool own = true) : p_(p) {
        if (p_ && own) ++p_->pinned;
    }
    ~pinned_ptr() {
        if (p_) --p_->pinned;
    }
    pinned_ptr& operator=(const pinned_ptr& p) {
        this_type(p).swap(*this);
        return *this;
    }
    void swap(pinned_ptr& p) {
        std::swap(p_, p.p_);
    }
    void reset(pointer p = 0) {
        this_type(p).swap(*this);
    }

    void* get() const { return p_->ptr; }

    typedef pointer this_type::*unspecified_bool_type;
    operator unspecified_bool_type() const {
        return p_ == 0 ? 0 : &this_type::p_;
    }
};

inline bool operator==(const pinned_ptr& a, const pinned_ptr& b) {
    return a.get() == b.get();
}
inline bool operator!=(const pinned_ptr& a, const pinned_ptr& b) {
    return !(a == b);
}
inline bool operator<(const pinned_ptr& a, const pinned_ptr& b) {
    return a.get() < b.get();
}
inline bool operator>(const pinned_ptr& a, const pinned_ptr& b) {
    return b < a;
}
inline bool operator<=(const pinned_ptr& a, const pinned_ptr& b) {
    return !(b < a);
}
inline bool operator>=(const pinned_ptr& a, const pinned_ptr& b) {
    return !(a < b);
}

}

#endif // RELOC_PINNED_PTR_HPP_INCLUDED
