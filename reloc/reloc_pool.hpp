#ifndef RELOC_RELOC_POOL_HPP_INCLUDED
#define RELOC_RELOC_POOL_HPP_INCLUDED

#include <cstddef>
#include <cassert>
#include <utility>
#include <iterator>
#include "detail/type.hpp"
#include "detail/assoc_vector.hpp"
#include "detail/alloc_node.hpp"
#include "detail/free_node.hpp"
#include "detail/node_pred.hpp"
#include "detail/enable_if.hpp"
#include "std_traits.hpp"
#include "reloc_ptr.hpp"

namespace reloc {

template<std::size_t Alignment, class Traits = std_traits>
class reloc_pool {
public:
    typedef Traits traits_type;

private:
    typedef detail::byte byte;
    typedef detail::free_node free_node;
    typedef detail::alloc_node alloc_node;

    typedef detail::assoc_vector<free_node, detail::free_node_pred> free_list_t;
    typedef detail::assoc_vector<alloc_node*, detail::alloc_node_pred> alloc_list_t;

    byte* ptr_;
    std::size_t size_;
    free_list_t free_list_;
    alloc_list_t alloc_list_;

private:
    // T は std::size_t か byte* を渡される可能性があるが、
    // byte* を std::size_t に変換するのは保証のない操作なので、
    // 値が壊れそうなケースは enable_if で弾いておく
    template<class T>
    static T align_floor(T v,
        typename detail::enable_if<sizeof(T) <= sizeof(std::size_t)>::type* = 0) {

        return (T)((std::size_t)v / Alignment * Alignment);
    }
    template<class T>
    static T align_ceil(T v,
        typename detail::enable_if<sizeof(T) <= sizeof(std::size_t)>::type* = 0) {

        return (T)(((std::size_t)v + Alignment - 1) / Alignment * Alignment);
    }

public:
    reloc_pool(void* ptr, std::size_t size) {
        byte* p = static_cast<byte*>(ptr);
        ptr_ = align_ceil(p);
        std::size_t d = static_cast<std::size_t>(ptr_ - p);
        if (size < d) size = d;
        size_ = align_floor(size - d);
        if (size_ != 0) {
            free_node fn = { ptr_, size_ };
            free_list_.insert(fn);
        }
        validate();
    }
    ~reloc_pool() {
        // まだ解放されてないメモリがある
        assert(alloc_list_.size() == 0);
    }

    // プール全体のサイズ
    std::size_t size() const {
        return size_;
    }
    // 空き領域の合計値
    // ピンが１つもされていなければ、この値までのサイズが allocate 可能
    std::size_t total_free() const {
        std::size_t free = 0;
        for (free_list_t::const_iterator it = free_list_.begin(); it != free_list_.end(); ++it) {
            free += it->size;
        }
        return free;
    }
    // それぞれの空き領域の中の最大値
    // この値までのサイズならリロケートを起こすことなく allocate 可能
    std::size_t max_free() const {
        std::size_t max = 0;
        for (free_list_t::const_iterator it = free_list_.begin(); it != free_list_.end(); ++it) {
            if (max < it->size) max = it->size;
        }
        return max;
    }

    // このプールから確保した領域かどうか
    bool contains(const reloc_ptr& handle) const {
        const alloc_node* const p = handle.get();
        const bool contain = ptr_ <= p->ptr && p->ptr < ptr_ + size_;
        // p->ptr がプールの中を指しているのなら、それは必ず alloc_list_ の中にあるはず
        assert(!contain ||
                contain && alloc_list_.find(p->ptr) != alloc_list_.end());
        return contain;
    }

    class alloc_info_iterator {
        typedef alloc_info_iterator this_type;

        alloc_list_t::const_iterator it;
        alloc_info_iterator(alloc_list_t::const_iterator it) : it(it) { }
        friend class reloc_pool;

    public:
        typedef std::random_access_iterator_tag iterator_category;
        typedef const alloc_info_iterator       value_type;
        typedef std::ptrdiff_t                  difference_type;
        typedef const alloc_info_iterator*      pointer;
        typedef const alloc_info_iterator&      reference;

        alloc_info_iterator() { }
        alloc_info_iterator(const this_type& v) : it(v.it) { }
        void operator=(const this_type& v) { it = v.it; }

        this_type& operator++() { ++it; return *this; }
        this_type operator++(int) { this_type v(*this); ++it; return v; }
        this_type& operator--() { --it; return *this; }
        this_type operator--(int) { this_type v(*this); --it; return v; }

        this_type& operator+=(difference_type n) { it += n; return *this; }
        this_type& operator-=(difference_type n) { it -= n; return *this; }
        friend this_type operator+(const this_type& a, difference_type n) { this_type v(a); v += n; return v; }
        friend this_type operator+(difference_type n, const this_type& a) { this_type v(a); v += n; return v; }
        friend this_type operator-(const this_type& a, difference_type n) { this_type v(a); v -= n; return v; }
        friend difference_type operator-(const this_type& a, const this_type& b) { return a.it - b.it; }

        reference operator*() const { return *this; }
        pointer operator->() const { return this; }

        bool operator==(const this_type& a) const { return it == a.it; }
        bool operator!=(const this_type& a) const { return it != a.it; }
        bool operator<(const this_type& a) const { return it < a.it; }
        bool operator>(const this_type& a) const { return it > a.it; }
        bool operator<=(const this_type& a) const { return it <= a.it; }
        bool operator>=(const this_type& a) const { return it >= a.it; }

        reference operator[](difference_type n) const { return it[n]; }

        const void* ptr() const { return (*it)->ptr; }
        bool pinned() const { return (*it)->pinned != 0; }
        std::size_t size() const { return (*it)->size; }
    };
    typedef std::pair<alloc_info_iterator, alloc_info_iterator> alloc_info_range;

    alloc_info_range alloc_info() const {
        return alloc_info_range(
                alloc_info_iterator(alloc_list_.begin()),
                alloc_info_iterator(alloc_list_.end()));
    }

private:
    // noncopyable
    reloc_pool();
    reloc_pool(const reloc_pool&);
    reloc_pool& operator=(const reloc_pool&);

public:
    reloc_ptr allocate(std::size_t size) {
        if (size == 0) size = 1;
        size = align_ceil(size);

        // ここで reserve しておくことで例外安全になる
        alloc_list_.reserve(alloc_list_.size() + 1);
        // free_list_ の reserve 数は半分ぐらいでも大丈夫そうな気がするのだけど、
        // 確証が得られないのでとりあえず考えられる最大の数を入れておく
        free_list_.reserve(alloc_list_.size() + 2);
        // ここで例外が発生すると、reserve によって alloc_list_ と free_list_ の
        // 内部状態が変わってしまうが、外部から見える状態は変わらないので問題ない
        std::auto_ptr<alloc_node> an(new alloc_node());

        reloc_ptr rh = allocate_free_list(size, an);
        if (rh) return rh;

        // リロケートして再度確保する
        free_list_t::iterator it = relocate(size);
        if (it != free_list_.end()) {
            rh = allocate_free_node(it, size, an);
            assert(rh);
        }
        return rh;
    }

    reloc_ptr reallocate(const reloc_ptr& handle, std::size_t size) {
        alloc_node* const p = handle.get();
        if (!p) return allocate(size);

        // handle は解放される可能性があるのでピンされていてはならない
        assert(p->pinned == 0);

        if (size == 0) size = 1;
        size = align_ceil(size);

        if (p->size == size) return handle;

        byte* const rp = p->ptr + p->size;
        // このブロックの右側が空いてるかどうかを調べる
        free_list_t::iterator it = free_list_.lower_bound(rp);
        const bool free_right = it != free_list_.end() && it->ptr == rp;

        if (size < p->size) {
            // 現在のブロックより小さいブロックを要求された
            const std::size_t cs = p->size - size;
            if (free_right) {
                it->ptr -= cs;
                it->size += cs;
                p->size -= cs;
                validate();
                return reloc_ptr(p);
            } else {
                free_node fn = { p->ptr + size, cs };
                free_list_.insert(it, fn); // throwable
                p->size -= cs;
                validate();
                return reloc_ptr(p);
            }
        } else {
            // 現在のブロックより大きいブロックを要求された
            const std::size_t cs = size - p->size;
            if (free_right && it->size >= cs) {
                // 全ての領域を使ったので削除する
                if (it->size == cs) {
                    free_list_.erase(it);
                } else {
                    it->ptr += cs;
                    it->size -= cs;
                }
                p->size += cs;
                validate();
                return reloc_ptr(p);
            } else {
                // リアロケートする必要がある
                const reloc_ptr p2 = allocate(size); // throwable
                if (!p2) return p2;

                const pinned_ptr pin = p2.pin();
                // リアロケートによるコピーであることを Traits に伝えた方がいいかもしれない
                copy_as_possible(p->ptr, p->size, static_cast<byte*>(pin.get()));
                deallocate(handle);

                validate();
                return p2;
            }
        }
    }

private:
    // フリーリストから単純に探す
    reloc_ptr allocate_free_list(std::size_t size, std::auto_ptr<alloc_node>& an) {
        for (free_list_t::iterator it = free_list_.begin(); it != free_list_.end(); ++it) {
            if (it->size >= size) {
                return allocate_free_node(it, size, an);
            }
        }
        return reloc_ptr();
    }
    // it の位置でアロケートする
    reloc_ptr allocate_free_node(free_list_t::iterator it, std::size_t size, std::auto_ptr<alloc_node>& an) {
        assert(it->size >= size);
        byte* p = it->ptr;
        // 全ての領域を使ったので削除する
        if (it->size == size) {
            free_list_.erase(it);
        } else {
            it->ptr += size;
            it->size -= size;
        }
        an->ptr = p;
        an->size = size;
        an->pinned = 0;

        assert(alloc_list_.size() < alloc_list_.capacity());
        alloc_node* const pan = an.release();
        alloc_list_.insert(pan); // nothrow のはず

        traits_type::construct(p);

        validate();
        return reloc_ptr(pan);
    }

private:
    struct reloc_cand {
        bool valid;
        std::size_t alloc_size;
        free_list_t::iterator first;
        free_list_t::iterator last;

        reloc_cand() : valid(false) { }
        reloc_cand(std::size_t a, free_list_t::iterator b, free_list_t::iterator c)
            : valid(true), alloc_size(a), first(b), last(c) { }

        void set_if_min(const reloc_cand& rc) {
            if (!rc.valid) return;
            if (!valid || rc.alloc_size < alloc_size) {
                valid = true;
                alloc_size = rc.alloc_size;
                first = rc.first;
                last = rc.last;
            }
        }
    };

    free_list_t::iterator relocate(std::size_t size) {
        reloc_cand rc = find_relocatable_range(size);
        if (!rc.valid) return free_list_.end();

        return do_relocate(rc.first, rc.last);
    }
    reloc_cand find_relocatable_range(std::size_t size) {
        reloc_cand rc;
        // alloc_node::pinned になっているデータ単位で分け、
        // それぞれの領域を find_relocatable_range する
        free_list_t::iterator fit = free_list_.begin();
        for (alloc_list_t::iterator it = alloc_list_.begin(); it != alloc_list_.end(); ++it) {
            if ((*it)->pinned != 0) {
                free_list_t::iterator fit2 = free_list_.lower_bound((*it)->ptr);
                rc.set_if_min(find_relocatable_range(size, fit, fit2));
                fit = fit2;
            }
        }
        rc.set_if_min(find_relocatable_range(size, fit, free_list_.end()));
        return rc;
    }
    reloc_cand find_relocatable_range(std::size_t size, free_list_t::iterator first, free_list_t::iterator last) {
        reloc_cand rc;
        if (std::distance(first, last) < 2) return rc;

        free_list_t::iterator it1 = first;
        free_list_t::iterator it2 = it1 + 1;
        std::size_t fs = it1->size;
        std::size_t as = 0;
        while (it2 != last) {
            fs += it2->size;
            as += it2->ptr - ((it2 - 1)->ptr + (it2 - 1)->size);
            while (fs >= size) {
                // リロケーション候補に追加
                rc.set_if_min(reloc_cand(as, it1, it2));
                fs -= it1->size;
                as -= (it1 + 1)->ptr - (it1->ptr + it1->size);
                ++it1;
            }
            ++it2;
        }
        return rc;
    }
    // [first, last] の範囲にある alloc_node を移動する
    free_list_t::iterator do_relocate(free_list_t::iterator first, free_list_t::iterator last) {
        assert(last != free_list_.end());
        assert(std::distance(first, last) >= 1);

        free_list_t::iterator it = first;
        byte* ptr = it->ptr;
        std::size_t free_size = 0;
        while (it != last) {
            alloc_list_t::iterator af = alloc_list_.lower_bound(it->ptr);
            alloc_list_t::iterator al = alloc_list_.upper_bound((it + 1)->ptr);
            while (af != al) {
                assert(ptr < (*af)->ptr);
                copy_as_possible((*af)->ptr, (*af)->size, ptr);
                (*af)->ptr = ptr;
                ptr += (*af)->size;
                ++af;
            }
            free_size += it->size;
            ++it;
        }
        first->ptr = ptr;
        first->size = free_size + last->size;
        free_list_.erase(first + 1, last + 1);
        validate();
        return first;
    }

    static void copy_as_possible(const byte* src, std::size_t size, byte* dst) {
        assert(src != dst);

        if (dst < src && dst + size > src) {
            traits_type::move_left(src, size, dst);
        } else if (src < dst && src + size > dst) {
            traits_type::move_right(src, size, dst);
        } else {
            traits_type::copy(src, size, dst);
        }
    }

public:
    void deallocate(const reloc_ptr& handle) { // nothrow
        alloc_node* const p = handle.get();
        if (!p) return;

        assert(free_list_.find(p->ptr) == free_list_.end());
        assert(alloc_list_.find(p->ptr) != alloc_list_.end());
        assert(p->pinned == 0);

        // アロケーションデータをフリーリストへ追加
        free_list_t::iterator it = free_list_.lower_bound(p->ptr);
        // p->ptr の左側が空き領域であるか
        const bool free_left = it != free_list_.begin() && (it - 1)->ptr + (it - 1)->size == p->ptr;
        // p->ptr の右側が空き領域であるか
        const bool free_right = it != free_list_.end() && p->ptr + p->size == it->ptr;
        if (free_left && free_right) {
            (it - 1)->size += p->size + it->size;
            free_list_.erase(it);
        } else if (!free_left && free_right) {
            it->ptr = p->ptr;
            it->size += p->size;
        } else if (free_left && !free_right) {
            (it - 1)->size += p->size;
        } else {
            assert(free_list_.size() < free_list_.capacity());
            // allocate 時に、想定される最大数でフリーリストを reserve しているので、
            // insert で例外が発生することはない
            free_node fn = { p->ptr, p->size };
            free_list_.insert(it, fn); // nothrow のはず
        }

        // アロケーションリストからの解放
        alloc_list_.erase(alloc_list_.find(p->ptr));
        traits_type::destroy(p->ptr);
        delete p;

        validate();
    }

private:
    template<class T>
    static bool aligned(T v) {
        return align_floor(v) == align_ceil(v);
    }
    template<class Iterator, class Comp>
    static bool is_sorted(Iterator first, Iterator last, Comp comp) {
        if (first == last) return true;

        Iterator next = first;
        for (++next; next != last; first = next, ++next)
            if (comp(*next, *first))
                return false;
        return true;
    }

    // データの整合性チェック
    bool check_validation() const {
        // ptr_, size_
        // free_list_t::ptr, free_list_t::size,
        // alloc_list_t::ptr, alloc_list_t::size が、
        // Alignment でアライメントされているされているかチェックする
        if (!aligned(ptr_)) return false;
        if (!aligned(size_)) return false;
        for (free_list_t::const_iterator it = free_list_.begin(); it != free_list_.end(); ++it) {
            if (!aligned(it->ptr)) return false;
            if (!aligned(it->size)) return false;
        }
        for (alloc_list_t::const_iterator it = alloc_list_.begin(); it != alloc_list_.end(); ++it) {
            if (!aligned((*it)->ptr)) return false;
            if (!aligned((*it)->size)) return false;
        }

        // free_list_t::size, alloc_list_t::size が 0 より大きいかチェックする
        for (free_list_t::const_iterator it = free_list_.begin(); it != free_list_.end(); ++it) {
            if (it->size == 0) return false;
        }
        for (alloc_list_t::const_iterator it = alloc_list_.begin(); it != alloc_list_.end(); ++it) {
            if ((*it)->size == 0) return false;
        }

        // free_list_.begin(), free_list_.end() が昇順になっているかチェックする
        if (!is_sorted(free_list_.begin(), free_list_.end(), detail::free_node_pred())) return false;

        // alloc_list_.begin(), alloc_list_.end() が昇順になっているかチェックする
        if (!is_sorted(alloc_list_.begin(), alloc_list_.end(), detail::alloc_node_pred())) return false;

        // free_list_t の空き領域のデータが連続していないかチェックする
        if (!free_list_.empty()) {
            for (free_list_t::const_iterator it = free_list_.begin(); it != free_list_.end() - 1; ++it) {
                if (it->ptr + it->size == (it + 1)->ptr) return false;
            }
        }

        free_list_t::const_iterator fit = free_list_.begin();
        free_list_t::const_iterator flast = free_list_.end();
        alloc_list_t::const_iterator ait = alloc_list_.begin();
        alloc_list_t::const_iterator alast = alloc_list_.end();
        byte* ptr = ptr_;
        while (fit != flast || ait != alast) {
            // 必ずどちらかの ptr と一致している必要がある
            if (!(fit != flast && fit->ptr == ptr ||
                  ait != alast && (*ait)->ptr == ptr)) return false;

            if (fit != flast && fit->ptr == ptr) {
                ptr += fit->size;
                ++fit;
            } else if (ait != alast && (*ait)->ptr == ptr) {
                ptr += (*ait)->size;
                ++ait;
            } else {
                return false;
            }
        }
        if (ptr != ptr_ + size_) return false;
        return true;
    }
    void validate() const {
        assert(check_validation());
    }
};

}

#endif // RELOC_RELOC_POOL_HPP_INCLUDED
