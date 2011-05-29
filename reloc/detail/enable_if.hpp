#ifndef RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED
#define RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED

namespace reloc { namespace detail {

// ���������� enable_if �̎���
template<bool Cond>
struct enable_if;

template<>
struct enable_if<true> {
    typedef void type;
};

}}

#endif // RELOC_DETAIL_ENABLE_IF_HPP_INCLUDED
