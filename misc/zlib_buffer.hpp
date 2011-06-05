#ifndef ZLIB_BUFFER_HPP_INCLUDED
#define ZLIB_BUFFER_HPP_INCLUDED

#include <cstddef>
#include "../reloc/reloc_ptr.hpp"
#include "zlib_helper.hpp"

// 名前空間は後で考える

template<class Pool>
class zlib_buffer {
private:
    scoped_ptr<Pool> p_;
    bool compressed_;

public:
    zlib_buffer(Pool& pool, const void* ptr, std::size_t size, bool ptr_cp, bool store_cp) : p_(pool) {
        if (ptr_cp == store_cp) {
            // 単純コピー
            p_.reset(buffer_t(allocexp::alloc(pool, size), size));
            // ptr は Pool の外にあるポインタなので、
            // Pool::traits_type を使うのは良くないかも
            Pool::traits_type::copy(ptr, size, p_.get().pin().get());
        } else {
            if (store_cp) {
                p_.reset(zlib_helper::deflate(pool, ptr, size, 128, 1.5f));
            } else {
                p_.reset(zlib_helper::inflate(pool, ptr, size, 128, 1.5f));
            }
        }
        compressed_ = store_cp;
    }
    bool compressed() const { return compressed_; }
    void compress() {
        if (!compressed()) {
            buffer_t buf = zlib_helper::deflate(p_.pool(), p_.get().pin().get(), p_.size(), 128, 1.5f);
            p_.reset(buf);
            compressed_ = true;
        }
    }
    void expand() {
        if (compressed()) {
            buffer_t buf = zlib_helper::inflate(p_.pool(), p_.get().pin().get(), p_.size(), 128, 1.5f);
            p_.reset(buf);
            compressed_ = false;
        }
    }
    reloc::reloc_ptr ptr() const { return p_.get(); }
    std::size_t size() const { return p_.size(); }
    // 展開済みデータを返す
    reloc::reloc_ptr eptr() {
        expand();
        return p_;
    }
    // 圧縮済みデータを返す
    reloc::reloc_ptr cptr() {
        compress();
        return p_;
    }

};

#endif // ZLIB_BUFFER_HPP_INCLUDED
