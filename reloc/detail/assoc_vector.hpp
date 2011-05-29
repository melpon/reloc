#ifndef RELOC_DETAIL_ASSOC_VECTOR_HPP_INCLUDED
#define RELOC_DETAIL_ASSOC_VECTOR_HPP_INCLUDED

#include <vector>
#include <memory>
#include <utility>
#include <algorithm>

namespace reloc { namespace detail {

// ソート済み vector のいい加減な実装
template<class T, class C, class A = std::allocator<T> >
class assoc_vector {
    typedef std::vector<T, A> vector_t;
    vector_t v_;

public:
    typedef typename C::key_type key_type;
    typedef T value_type;
    typedef C compare_type;
    typedef typename vector_t::iterator iterator;
    typedef typename vector_t::const_iterator const_iterator;
    typedef typename vector_t::size_type size_type;

    assoc_vector() { }

    iterator begin() { return v_.begin(); }
    iterator end() { return v_.end(); }
    const_iterator begin() const { return v_.begin(); }
    const_iterator end() const { return v_.end(); }

    bool empty() const { return v_.empty(); }
    size_type size() const { return v_.size(); }

    size_type capacity() const { return v_.capacity(); }
    void reserve(size_type n) { v_.reserve(n); }

    std::pair<iterator, bool> insert(const value_type& v) {
        bool found = true;
        iterator it = lower_bound(v);
        if (it == end() || compare_type()(v, *it)) {
            it = v_.insert(it, v);
            found = false;
        }
        return std::make_pair(it, !found);
    }
    iterator insert(iterator pos, const value_type& v) {
        if ((pos == begin() || compare_type()(*(pos - 1), v)) &&
            (pos == end()   || compare_type()(v, *pos))) {
            return v_.insert(pos, v);
        }
        return insert(v).first;
    }

    void erase(iterator pos) { v_.erase(pos); }
    void erase(iterator first, iterator last) { v_.erase(first, last); }

    void swap(assoc_vector& v) {
        v_.swap(v.v_);
    }

    void clear() { v_.clear(); }

    iterator find(const key_type& key) {
        iterator it = lower_bound(key);
        return it != end() && compare_type()(key, *it) ? end() : it;
    }
    const_iterator find(const key_type& key) const {
        const_iterator it = lower_bound(key);
        return it != end() && compare_type()(key, *it) ? end() : it;
    }

    iterator lower_bound(const key_type& key) {
        return std::lower_bound(begin(), end(), key, compare_type());
    }
    const_iterator lower_bound(const key_type& key) const {
        return std::lower_bound(begin(), end(), key, compare_type());
    }
    iterator lower_bound(const value_type& value) {
        return std::lower_bound(begin(), end(), value, compare_type());
    }
    const_iterator lower_bound(const value_type& value) const {
        return std::lower_bound(begin(), end(), value, compare_type());
    }
    iterator upper_bound(const key_type& key) {
        return std::upper_bound(begin(), end(), key, compare_type());
    }
    const_iterator upper_bound(const key_type& key) const {
        return std::upper_bound(begin(), end(), key, compare_type());
    }
    iterator upper_bound(const value_type& value) {
        return std::upper_bound(begin(), end(), value, compare_type());
    }
    const_iterator upper_bound(const value_type& value) const {
        return std::upper_bound(begin(), end(), value, compare_type());
    }
    std::pair<iterator, iterator> equal_range(const key_type& key) {
        return std::equal_range(begin(), end(), key, compare_type());
    }
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
        return std::equal_range(begin(), end(), key, compare_type());
    }
    std::pair<iterator, iterator> equal_range(const value_type& value) {
        return std::equal_range(begin(), end(), value, compare_type());
    }
    std::pair<const_iterator, const_iterator> equal_range(const value_type& value) const {
        return std::equal_range(begin(), end(), value, compare_type());
    }
};

}}

#endif // RELOC_DETAIL_ASSOC_VECTOR_HPP_INCLUDED
