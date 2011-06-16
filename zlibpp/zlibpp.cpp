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
    stream_impl() : z() { }
};

void deflate_end(stream* p) {
    stream_impl* si = static_cast<stream_impl*>(p);
    deflateEnd(si->z);
}

void make_stream(stream_ptr& z, void (*f)(stream* p)) {
    stream_impl* si = new (std::nothrow) stream_impl();
    if (!si) return;
    z.reset(stream_ptr(si, f).release());
}

int do_zlib(stream_ptr& sp, int flush, int (*f)(z_stream*, int)) {
    if (!sp) return ERRNO;

    sp->z.next_in =
        const_cast<Bytef*>(static_cast<const Bytef*>(sp->next_in));
    sp->z.avail_in = static_cast<uInt>(sp->avail_in);
    sp->z.total_in = static_cast<uLong>(sp->total_in);
    sp->z.next_out = static_cast<Bytef*>(sp->next_out);
    sp->z.avail_out = static_cast<uInt>(sp->avail_out);
    sp->z.total_out = static_cast<uLong>(sp->total_out);

    const int result = f(&sp->z, flush);

    sp->next_in = static_cast<byte*>(sp->z.next_in);
    sp->avail_in = static_cast<std::size_t>(sp->z.avail_in);
    sp->total_in = static_cast<std::size_t>(sp->z.total_in);
    sp->next_out = static_cast<byte*>(sp->z.next_out);
    sp->avail_out = static_cast<std::size_t>(sp->z.avail_out);
    sp->total_out = static_cast<std::size_t>(sp->z.total_out);
}

void deflate_init(stream_ptr& sp, int level) {
    make_stream(sp, f);
    if (!sp) return;
    deflateInit(sp->z);
}
int deflate(stream_ptr& sp, int flush) {
    return do_zlib(sp, flush, ::deflate);
}

void inflate_init(stream_ptr& sp, int level) {
    make_stream(sp, f);
    if (!sp) return;
    inflateInit(sp->z);
}
int inflate(stream_ptr& sp, int flush) {
    return do_zlib(sp, flush, ::inflate);
}

}

