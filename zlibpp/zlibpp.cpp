#include "zlibpp.hpp"

#include <zlib.h>

namespace zlibpp {

const int NO_FLUSH = Z_NO_FLUSH;
const int PARTIAL_FLUSH = Z_PARTIAL_FLUSH;
const int SYNC_FLUSH = Z_SYNC_FLUSH;
const int FULL_FLUSH = Z_FULL_FLUSH;
const int FINISH = Z_FINISH;
const int BLOCK = Z_BLOCK;
const int TREES = Z_TREES;

const int OK = Z_OK;
const int STREAM_END = Z_STREAM_END;
const int NEED_DICT = Z_NEED_DICT;
const int ERRNO = Z_ERRNO;
const int STREAM_ERROR = Z_STREAM_ERROR;
const int DATA_ERROR = Z_DATA_ERROR;
const int MEM_ERROR = Z_MEM_ERROR;
const int BUF_ERROR = Z_BUF_ERROR;
const int VERSION_ERROR = Z_VERSION_ERROR;

const int NO_COMPRESSION = Z_NO_COMPRESSION;
const int BEST_SPEED = Z_BEST_SPEED;
const int BEST_COMPRESSION = Z_BEST_COMPRESSION;
const int DEFAULT_COMPRESSION = Z_DEFAULT_COMPRESSION;

struct stream_impl : stream {
    z_stream z;
};

void deflate_end(stream* p) {
    stream_impl* si = static_cast<stream_impl*>(p);
    deflateEnd(&si->z);
    delete si;
}

void inflate_end(stream* p) {
    stream_impl* si = static_cast<stream_impl*>(p);
    inflateEnd(&si->z);
    delete si;
}

void make_stream(stream_ptr& z, void (*f)(stream* p)) {
    stream_impl* si = new (std::nothrow) stream_impl();
    if (!si) return;
    z.reset(stream_ptr(si, f).release());
}

template<class F>
int do_zlib(const stream_ptr& sp, int flush, F f) {
    if (!sp) return Z_MEM_ERROR;

    stream_impl* si = static_cast<stream_impl*>(sp.get());

    si->z.next_in =
        const_cast<Bytef*>(static_cast<const Bytef*>(si->next_in));
    si->z.avail_in = static_cast<uInt>(si->avail_in);
    si->z.total_in = static_cast<uLong>(si->total_in);
    si->z.next_out = static_cast<Bytef*>(si->next_out);
    si->z.avail_out = static_cast<uInt>(si->avail_out);
    si->z.total_out = static_cast<uLong>(si->total_out);

    const int result = f(&si->z, flush);

    si->next_in = si->z.next_in;
    si->avail_in = static_cast<std::size_t>(si->z.avail_in);
    si->total_in = static_cast<std::size_t>(si->z.total_in);
    si->next_out = si->z.next_out;
    si->avail_out = static_cast<std::size_t>(si->z.avail_out);
    si->total_out = static_cast<std::size_t>(si->z.total_out);

    return result;
}

void deflate_init(stream_ptr& sp, int level) {
    make_stream(sp, deflate_end);
    if (!sp) return;
    stream_impl* si = static_cast<stream_impl*>(sp.get());
    deflateInit(&si->z, level);
}
int deflate(const stream_ptr& sp, int flush) {
    return do_zlib(sp, flush, ::deflate);
}

void inflate_init(stream_ptr& sp) {
    make_stream(sp, inflate_end);
    if (!sp) return;
    stream_impl* si = static_cast<stream_impl*>(sp.get());
    inflateInit(&si->z);
}
int inflate(const stream_ptr& sp, int flush) {
    return do_zlib(sp, flush, ::inflate);
}

}

