#include "reloc/reloc_pool.hpp"
#include "reloc/reloc_ptr.hpp"
#include "reloc/pinned_ptr.hpp"

using namespace reloc;

typedef unsigned char uint8;

void test1() {
    reloc_pool<1> pool(0, 100);
    reloc_ptr p = pool.allocate(50);
    pool.deallocate(p);
}

void assert_range(void* buf, int size, int n) {
    uint8* p = (uint8*)buf;
    for (int i = 0; i < size; i++) {
        assert(p[i] == n + i);
    }
}

void test2() {
    uint8* p = new uint8[100];
    for (int i = 0; i < 100; i++) {
        p[i] = static_cast<uint8>(i);
    }

    reloc_pool<1> pool(p, 100);
    reloc_ptr p1 = pool.allocate(40);
    reloc_ptr p2 = pool.allocate(10);
    reloc_ptr p3 = pool.allocate(40);
    reloc_ptr p4 = pool.allocate(10);
    assert(p1.pin().get() == p + 0);
    assert(p2.pin().get() == p + 40);
    assert(p3.pin().get() == p + 50);
    assert(p4.pin().get() == p + 90);
    pool.deallocate(p2);
    pool.deallocate(p4);
    reloc_ptr p5 = pool.allocate(20);
    assert(p1.pin().get() == p + 0);
    assert_range(p1.pin().get(), 40, 0);
    assert(p3.pin().get() == p + 40);
    assert_range(p3.pin().get(), 40, 50);
    assert(p5.pin().get() == p + 80);

    pool.deallocate(p1);
    pool.deallocate(p3);
    pool.deallocate(p5);

    delete[] p;
}

void test3() {
    uint8* p = new uint8[100];
    for (int i = 0; i < 100; i++) {
        p[i] = static_cast<uint8>(i);
    }

    reloc_pool<1> pool(p, 100);
    reloc_ptr p1 = pool.allocate(10);
    reloc_ptr p2 = pool.allocate(40);
    reloc_ptr p3 = pool.allocate(10);
    reloc_ptr p4 = pool.allocate(40);
    assert(p1.pin().get() == p + 0);
    assert(p2.pin().get() == p + 10);
    assert(p3.pin().get() == p + 50);
    assert(p4.pin().get() == p + 60);
    pool.deallocate(p1);
    pool.deallocate(p3);
    reloc_ptr p5 = pool.allocate(20);
    assert(p2.pin().get() == p + 0);
    assert_range(p2.pin().get(), 40, 10);
    assert(p4.pin().get() == p + 60);
    assert_range(p4.pin().get(), 40, 60);
    assert(p5.pin().get() == p + 40);

    pool.deallocate(p2);
    pool.deallocate(p4);
    pool.deallocate(p5);

    delete[] p;
}

void test4() {
    uint8* p = new uint8[100];
    for (int i = 0; i < 100; i++) {
        p[i] = static_cast<uint8>(i);
    }

    reloc_pool<1> pool(p, 100);
    reloc_ptr p1 = pool.allocate(5);
    reloc_ptr p2 = pool.allocate(30);
    reloc_ptr p3 = pool.allocate(10);
    reloc_ptr p4 = pool.allocate(40);
    reloc_ptr p5 = pool.allocate(10);
    pool.deallocate(p1);
    pool.deallocate(p3);
    pool.deallocate(p5);
    reloc_ptr p6 = pool.allocate(20);
    assert(p2.pin().get() == p + 5);
    assert(p4.pin().get() == p + 35);
    assert(p6.pin().get() == p + 75);

    pool.deallocate(p2);
    pool.deallocate(p4);
    pool.deallocate(p6);

    delete[] p;
}

void test5() {
    uint8* p = new uint8[100];
    for (int i = 0; i < 100; i++) {
        p[i] = static_cast<uint8>(i);
    }

    reloc_pool<1> pool(p, 100);
    reloc_ptr p1 = pool.allocate(10);
    reloc_ptr p2 = pool.allocate(10);
    reloc_ptr p3 = pool.allocate(10);
    reloc_ptr p4 = pool.allocate(10);
    reloc_ptr p5 = pool.allocate(10);
    reloc_ptr p6 = pool.allocate(10);
    reloc_ptr p7 = pool.allocate(10);
    reloc_ptr p8 = pool.allocate(10);
    reloc_ptr p9 = pool.allocate(10);
    pool.deallocate(p3);
    pool.deallocate(p7);
    reloc_ptr p10 = pool.allocate(30);
    assert(p1.pin().get() == p + 0);
    assert(p2.pin().get() == p + 10);
    assert(p4.pin().get() == p + 20);
    assert(p5.pin().get() == p + 30);
    assert(p6.pin().get() == p + 40);
    assert(p8.pin().get() == p + 50);
    assert(p9.pin().get() == p + 60);
    assert(p10.pin().get() == p + 70);

    pool.deallocate(p2);
    pool.deallocate(p6);
    pool.deallocate(p8);
    pool.deallocate(p10);

    reloc_ptr p11 = pool.allocate(35);

    assert(p1.pin().get() == p + 0);
    assert(p4.pin().get() == p + 20);
    assert(p5.pin().get() == p + 30);
    assert(p9.pin().get() == p + 40);
    assert(p11.pin().get() == p + 50);

    pool.deallocate(p1);
    pool.deallocate(p4);
    pool.deallocate(p5);
    pool.deallocate(p9);
    pool.deallocate(p11);

    delete[] p;
}

void test6() {
    uint8* p = new uint8[100];
    for (int i = 0; i < 100; i++) {
        p[i] = static_cast<uint8>(i);
    }

    reloc_pool<1> pool(p, 100);
    reloc_ptr p1 = pool.allocate(10);
    reloc_ptr p2 = pool.allocate(10);
    reloc_ptr p3 = pool.allocate(10);
    reloc_ptr p4 = pool.allocate(10);
    reloc_ptr p5 = pool.allocate(10);
    reloc_ptr p6 = pool.allocate(10);
    reloc_ptr p7 = pool.allocate(10);
    reloc_ptr p8 = pool.allocate(10);
    reloc_ptr p9 = pool.allocate(10);
    pinned_ptr pin4 = p4.pin();
    pool.deallocate(p2);
    pool.deallocate(p3);
    pool.deallocate(p5);
    pool.deallocate(p7);
    pool.deallocate(p9);

    reloc_ptr p10 = pool.allocate(30);

    assert(p1.pin().get() == p + 0);
    assert(p4.pin().get() == p + 30);
    assert(p6.pin().get() == p + 50);
    assert(p8.pin().get() == p + 60);
    assert(p10.pin().get() == p + 70);

    pool.deallocate(p1);
    pin4.reset();
    pool.deallocate(p4);
    pool.deallocate(p6);
    pool.deallocate(p8);
    pool.deallocate(p10);

    delete[] p;
}

void test7() {
    uint8* p = new uint8[64 * 128];

    reloc_pool<64> pool(p, 64 * 128);
    reloc_ptr p1 = pool.allocate(10);
    assert(p1.pin().get() == (void*)((((size_t)p + 0) + 63) & ~0x3f));
    reloc_ptr p2 = pool.allocate(10);
    assert(p2.pin().get() == (void*)((((size_t)p + 64) + 63) & ~0x3f));

    pool.deallocate(p1);
    pool.deallocate(p2);

    delete[] p;
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
}
