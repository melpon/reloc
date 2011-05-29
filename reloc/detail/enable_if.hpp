#ifndef RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED
#define RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED

namespace reloc { namespace detail {

// いい加減な enable_if の実装
template<bool Cond>
struct enable_if;

template<>
struct enable_if<true> {
    typedef void type;
};

}}

#endif // RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED
