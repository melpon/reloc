// :QuickRun "g++ -I. -I../zlib-1.2.5"

#include <zlibpp/zlibpp.hpp>

#include <zlibpp/zlibpp.cpp>

#include <cassert>

int main() {
    char* p = new char[100];

    zlibpp::deflate_stream ds(zlibpp::BEST_COMPRESSION);
    ds->next_in = "abc";
    ds->avail_in = 3;
    ds->next_out = p;
    ds->avail_out = 100;
    ds.deflate(zlibpp::FINISH);
    assert(ds->avail_in == 0);
    assert(ds->total_in == 3);
    assert(ds->total_out + ds->avail_out == 100);

    char buf[3];
    zlibpp::inflate_stream is;
    is->next_in = p;
    is->avail_in = ds->total_out;
    is->next_out = buf;
    is->avail_out = 3;
    is.inflate(zlibpp::FINISH);
    assert(is->avail_in == 0);
    assert(is->avail_out == 0);
    assert(is->total_in == ds->total_out);
    assert(buf[0] == 'a' && buf[1] == 'b' && buf[2] == 'c');

    delete[] p;
}

