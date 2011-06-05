#include <cassert>
#include "misc/zlib_buffer.hpp"
#include "reloc/reloc_pool.hpp"

using namespace reloc;

void test1() {
    unsigned char* pd = new unsigned char[10 * 1024];
    reloc_pool<64> pool(pd, 10 * 1024);
    const char data[] = { 0, 1, 2, 3, 4, 5 };
    zlib_buffer<reloc_pool<64> > buf(pool, data, sizeof(data), false, true);
    assert(buf.compressed());
    buf.expand();
    assert(!buf.compressed());
    assert(buf.size() == sizeof(data));
    assert(std::equal(data, data + sizeof(data),
                      static_cast<unsigned char*>(buf.ptr().pin().get())));
}

int main() {
    test1();
}

