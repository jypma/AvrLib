#ifndef ENUM_HPP_
#define ENUM_HPP_

#ifndef AVR
#include <iostream>
#endif

template<typename def, typename inner = typename def::type>
class Enum : public def
{
public:
    typedef inner type;
private:
    inner val;
public:
    constexpr Enum(type v) : val(v) {}
    constexpr type underlying() const { return val; }
    constexpr type *underlying_ptr() { return &val; }

    friend constexpr bool operator == (const Enum & lhs, const Enum & rhs) { return lhs.val == rhs.val; }
    friend constexpr bool operator == (const Enum & lhs, const inner rhs) { return lhs.val == rhs; }
    friend constexpr bool operator == (const inner lhs, const Enum &rhs) { return lhs == rhs.val; }
    friend constexpr bool operator != (const Enum & lhs, const Enum & rhs) { return lhs.val != rhs.val; }
    friend constexpr bool operator != (const Enum & lhs, const inner rhs) { return lhs.val != rhs; }
};

#ifndef AVR
template<typename def, typename inner = typename def::type>
inline ::std::ostream& operator<<(::std::ostream& os, const Enum<def,inner>& that) {
    return os << that.underlying();
}
#endif


#endif /* ENUM_HPP_ */
