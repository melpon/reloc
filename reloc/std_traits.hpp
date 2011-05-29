#ifndef RELOC_STD_TRAITS_HPP_INCLUDED
#define RELOC_STD_TRAITS_HPP_INCLUDED

#include <algorithm>

namespace reloc {

struct std_traits {
    static void construct(void* p) { } // nothrow
    static void destroy(void* p) { } // nothrow

    // src �� dst �̃|�C���^�̗̈悪�I�[�o�[���b�v���Ă���ꍇ�� move�A
    // �����łȂ��ꍇ�� copy ���Ă΂��B
    // �i�ȉ��̕ۏ؂͌�ŏ����邩���j
    // copy, move �̍ۂɁAdst < src �ł��邱�Ƃ͕ۏ؂���Ă���B
    // �܂�̈�͍��i�A�h���X�̏��������j�ɂ����ړ����Ȃ�
    static void move(const void* src, std::size_t size, void* dst) { // nothrow
        std::copy(static_cast<const unsigned char*>(src), static_cast<const unsigned char*>(src) + size, static_cast<unsigned char*>(dst));
    }
    static void copy(const void* src, std::size_t size, void* dst) { // nothrow
        move(src, size, dst);
    }
};

}

#endif // RELOC_STD_TRAITS_HPP_INCLUDED
