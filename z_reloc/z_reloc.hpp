#ifndef Z_RELOC_Z_RELOC_HPP_INCLUDED
#define Z_RELOC_Z_RELOC_HPP_INCLUDED

#include <cstddef>
#include <utility>
#include <new>
#include <exception>

#include <zlibpp/zlibpp.hpp>
#include <reloc/reloc_ptr.hpp>

namespace z_reloc {

template<class Pool>
class scoped_ptr {
    Pool& pool_;
    buffer_t buf_;

    // noncopyable
    scoped_ptr(const scoped_ptr&);
    scoped_ptr& operator=(const scoped_ptr&);

public:
    scoped_ptr(Pool& pool) : pool_(pool) { }
    scoped_ptr(Pool& pool, const buffer_t& buf)
        : pool_(pool), buf_(buf) { }
    ~scoped_ptr() {
        pool_.deallocate(buf_.first);
    }

    void reset(const buffer_t& buf) {
        if (buf_.first != buf.first) {
            pool_.deallocate(buf_.first);
        }
        buf_ = buf;
    }
    reloc::reloc_ptr get() const { return buf_.first; }
    std::size_t size() const { return buf_.second; }
    buffer_t release() {
        buffer_t b = buf_;
        buf_.first = reloc_ptr();
        return b;
    }
    Pool& pool() { return pool_; }
};

/*
class z_reloc {
private:
    // この関数で例外が発生した場合、Poolの状態が変わっている可能性があることに注意（基本的な保証）
    template<class Pool, class Init, class Func, class End>
    static buffer_t zlib(Pool& pool,
        const void* in, std::size_t in_size, std::size_t out_init_size, float rate,
        Init init, Func func, End end) {

        typedef unsigned char byte;

        z_stream z = { };
        init(&z);
        struct guard {
            z_stream& z;
            End end;
            ~guard() { end(&z); }
        } g = { z, end };

        scoped_ptr<Pool> p(pool, buffer_t(allocexp::alloc(pool, out_init_size), out_init_size));
        pinned_ptr pin = p.get().pin();

        z.next_in = const_cast<byte*>(static_cast<const byte*>(in));
        z.avail_in = in_size;
        z.next_out = static_cast<byte*>(pin.get());
        z.avail_out = p.size();

        while (true) {
            const int result = func(&z, z.avail_in == 0 ? Z_FINISH : Z_NO_FLUSH);
            if (result == Z_STREAM_END) break;

            if (result != Z_OK) {
                // TODO: ちゃんとした例外に差し替える
                throw std::exception();
            }
            if (z.avail_out == 0) {
                const std::size_t oldsize = p.size();
                const std::size_t newsize = static_cast<std::size_t>(p.size() * rate);
                pin.reset();
                p.reset(buffer_t(allocexp::realloc(pool, p.get(), newsize), newsize));
                pin = p.get().pin();
                z.next_out = static_cast<byte*>(pin.get()) + oldsize;
                z.avail_out = p.size() - oldsize;
            }
        }
        pin.reset();
        p.reset(buffer_t(allocexp::realloc(pool, p.get(), z.total_out), z.total_out));
        return p.release();
    }

private:
    struct deflate_init {
        int level;
        deflate_init(int level) : level(level) { }
        void operator()(z_stream* p) const { deflateInit(p, level); }
    };

    struct inflate_init {
        void operator()(z_stream* p) const { inflateInit(p); }
    };

public:
    template<class Pool>
    static buffer_t deflate(Pool& pool,
        const void* in, std::size_t in_size, std::size_t out_init_size, float rate) {
        return zlib(pool, in, in_size, out_init_size, rate,
                    deflate_init(Z_DEFAULT_COMPRESSION), ::deflate, deflateEnd);
    }

    template<class Pool>
    static buffer_t inflate(Pool& pool,
        const void* in, std::size_t in_size, std::size_t out_init_size, float rate) {
        return zlib(pool, in, in_size, out_init_size, rate,
                    inflate_init(), ::inflate, inflateEnd);
    }
};
*/

}

#endif // Z_RELOC_Z_RELOC_HPP_INCLUDED
