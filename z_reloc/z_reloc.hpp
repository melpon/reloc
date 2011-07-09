#ifndef Z_RELOC_Z_RELOC_HPP_INCLUDED
#define Z_RELOC_Z_RELOC_HPP_INCLUDED

#include <cstddef>
#include <utility>
#include <new>
#include <exception>

#include <zlibpp/zlibpp.hpp>
#include <reloc/reloc_ptr.hpp>

// reloc_pool を使って zlib 圧縮を行うクラス

namespace z_reloc {

struct sized_ptr {
    reloc::reloc_ptr ptr;
    std::size_t size;
};

namespace detail {

template<class Stream, class StreamFunc, class Pool>
sized_ptr zlib_reloc(Pool& pool,
    const void* in, std::size_t in_size, std::size_t out_init_size, float rate,
    Stream& s, StreamFunc func) {

    assert(rate > 1.0f);

    class scoped_ptr {
        Pool& pool_;
        reloc::reloc_ptr p_;

    public:
        scoped_ptr(Pool& pool, reloc::reloc_ptr p)
            : pool_(pool), p_(p) { }
        ~scoped_ptr() {
            pool_.deallocate(p_);
        }

        void reset(reloc::reloc_ptr p) {
            if (p_ != p) {
                pool_.deallocate(p_);
                p_ = p;
            }
        }
        void reallocate(std::size_t size) {
            p_ = pool_.reallocate(p_, size);
        }
        reloc::reloc_ptr get() const { return p_; }
        reloc::reloc_ptr release() {
            reloc::reloc_ptr p = p_;
            p_ = reloc::reloc_ptr();
            return p;
        }
    };

    scoped_ptr p(pool, pool.allocate(out_init_size));
    if (!p.get()) return sized_ptr();

    reloc::pinned_ptr pin = p.get().pin();

    s->next_in = in;
    s->avail_in = in_size;
    s->next_out = pin.get();
    s->avail_out = out_init_size;

    typedef unsigned char byte;

    while (true) {
        const int result = (s.*func)(s->avail_in == 0 ? zlibpp::FINISH : zlibpp::NO_FLUSH);
        if (result == zlibpp::STREAM_END) break;

        if (result != zlibpp::OK) {
            throw result;
        }

        if (s->avail_out == 0) {
            const std::size_t oldsize = s->total_out;
            std::size_t newsize = static_cast<std::size_t>(oldsize * rate);
            if (oldsize == newsize) ++newsize;

            pin.reset();
            p.reallocate(newsize);
            if (!p.get()) return sized_ptr();
            pin = p.get().pin();

            s->next_out = static_cast<byte*>(pin.get()) + oldsize;
            s->avail_out = newsize - oldsize;
        }
    }
    pin.reset();
    p.reallocate(s->total_out);
    sized_ptr sp = { p.release(), s->total_out };
    return sp;
}

}

template<class Pool>
sized_ptr deflate(Pool& pool, const void* in, std::size_t in_size,
    std::size_t out_init_size = 8, float rate = 1.5f, int level = zlibpp::BEST_COMPRESSION) {

    zlibpp::deflate_stream ds(level);
    return detail::zlib_reloc(pool, in, in_size, out_init_size, rate, ds, &zlibpp::deflate_stream::deflate);
}

template<class Pool>
sized_ptr inflate(Pool& pool, const void* in, std::size_t in_size,
    std::size_t out_init_size = 8, float rate = 1.5f) {

    zlibpp::inflate_stream is;
    return detail::zlib_reloc(pool, in, in_size, out_init_size, rate, is, &zlibpp::inflate_stream::inflate);
}

}

#endif // Z_RELOC_Z_RELOC_HPP_INCLUDED
