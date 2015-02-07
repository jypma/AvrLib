/*
 * BitSet.hpp
 *
 *  Created on: Jan 18, 2015
 *      Author: jan
 */

#ifndef BITSET_HPP_
#define BITSET_HPP_


template<typename enumT>
class BitSet {
    typedef enumT enum_type;
    typedef __underlying_type(enum_type) store_type;

    const store_type flags_;
public:
    BitSet(): BitSet(store_type(0)) {}

    explicit BitSet(store_type value) :flags_(value) {}

    constexpr inline store_type underlying() const {
        return flags_;
    }

    constexpr inline bool operator [](enum_type flag) const {
        return (flags_ & static_cast<store_type>(flag));
    }
};

#endif /* BITSET_HPP_ */
