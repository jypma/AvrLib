#ifndef __AVR__
#include <type_traits>
#else

#ifndef _GLIBCXX_TYPE_TRAITS
#define _GLIBCXX_TYPE_TRAITS 1

// For some reason, avr-gcc does not bundle type_traits.h. We copy some of its definitions here.

namespace std {

// Primary template.
/// Define a member typedef @c type only if a boolean constant is true.
template<bool, typename _Tp = void>
  struct enable_if
  { };

// Partial specialization for true.
template<typename _Tp>
  struct enable_if<true, _Tp>
  { typedef _Tp type; };

/// integral_constant
template<typename _Tp, _Tp __v>
  struct integral_constant
  {
    static constexpr _Tp                  value = __v;
    typedef _Tp                           value_type;
    typedef integral_constant<_Tp, __v>   type;
    constexpr operator value_type() const { return value; }
#if __cplusplus > 201103L

#define __cpp_lib_integral_constant_callable 201304

    constexpr value_type operator()() const { return value; }
#endif
  };

template<typename _Tp, _Tp __v>
  constexpr _Tp integral_constant<_Tp, __v>::value;

/// The type used as a compile-time boolean with true value.
typedef integral_constant<bool, true>     true_type;

/// The type used as a compile-time boolean with false value.
typedef integral_constant<bool, false>    false_type;

/// is_enum
template<typename _Tp>
  struct is_enum
  : public integral_constant<bool, __is_enum(_Tp)>
  { };

/// is_class
template<typename _Tp>
  struct is_class
  : public integral_constant<bool, __is_class(_Tp)>
  { };

/// The underlying type of an enum.
template<typename _Tp>
  struct underlying_type
  {
    typedef __underlying_type(_Tp) type;
  };

template<typename, typename>
    struct is_same;

template<typename, typename>
    struct is_same
  : public false_type { };

template<typename _Tp>
    struct is_same<_Tp, _Tp>
  : public true_type { };

/// remove_const
template<typename _Tp>
  struct remove_const
  { typedef _Tp     type; };

template<typename _Tp>
  struct remove_const<_Tp const>
  { typedef _Tp     type; };

/// remove_volatile
template<typename _Tp>
  struct remove_volatile
  { typedef _Tp     type; };

template<typename _Tp>
  struct remove_volatile<_Tp volatile>
  { typedef _Tp     type; };

/// remove_cv
template<typename _Tp>
  struct remove_cv
  {
    typedef typename
    remove_const<typename remove_volatile<_Tp>::type>::type     type;
  };


template<typename>
    struct __is_pointer_helper
    : public false_type { };

  template<typename _Tp>
    struct __is_pointer_helper<_Tp*>
    : public true_type { };

  /// is_pointer
  template<typename _Tp>
    struct is_pointer
    : public __is_pointer_helper<typename remove_cv<_Tp>::type>::type
    { };

  template<typename, typename>
       struct is_base_of;

  template<typename _Base, typename _Derived>
        struct is_base_of
        : public integral_constant<bool, __is_base_of(_Base, _Derived)>
        { };

  template<bool, typename, typename>
          struct conditional;

  // Primary template.
     /// Define a member typedef @c type to one of two argument types.
     template<bool _Cond, typename _Iftrue, typename _Iffalse>
        struct conditional
        { typedef _Iftrue type; };

      // Partial specialization for false.
      template<typename _Iftrue, typename _Iffalse>
        struct conditional<false, _Iftrue, _Iffalse>
        { typedef _Iffalse type; };

     template<typename...>
       struct __or_;

     template<>
       struct __or_<>
       : public false_type
       { };

     template<typename _B1>
       struct __or_<_B1>
       : public _B1
       { };

     template<typename _B1, typename _B2>
       struct __or_<_B1, _B2>
       : public conditional<_B1::value, _B1, _B2>::type
       { };

     template<typename _B1, typename _B2, typename _B3, typename... _Bn>
       struct __or_<_B1, _B2, _B3, _Bn...>
       : public conditional<_B1::value, _B1, __or_<_B2, _B3, _Bn...>>::type
      { };


        template<typename...>
          struct __and_;

        template<>
          struct __and_<>
          : public true_type
          { };

        template<typename _B1>
          struct __and_<_B1>
          : public _B1
          { };

        template<typename _B1, typename _B2>
          struct __and_<_B1, _B2>
          : public conditional<_B1::value, _B2, _B1>::type
          { };

        template<typename _B1, typename _B2, typename _B3, typename... _Bn>
          struct __and_<_B1, _B2, _B3, _Bn...>
          : public conditional<_B1::value, __and_<_B2, _B3, _Bn...>, _B1>::type
          { };

        template<typename _Pp>
          struct __not_
          : public integral_constant<bool, !_Pp::value>
          { };

        template<typename>
             struct remove_cv;

           template<typename>
             struct __is_void_helper
             : public false_type { };

           template<>
             struct __is_void_helper<void>
             : public true_type { };

           /// is_void
           template<typename _Tp>
             struct is_void
             : public integral_constant<bool, (__is_void_helper<typename
                               remove_cv<_Tp>::type>::value)>
             { };

  template<typename>
    struct add_rvalue_reference;

  template<typename _Tp>
    typename add_rvalue_reference<_Tp>::type declval() noexcept;

  /// is_lvalue_reference
       template<typename>
         struct is_lvalue_reference
         : public false_type { };

       template<typename _Tp>
         struct is_lvalue_reference<_Tp&>
         : public true_type { };

       /// is_rvalue_reference
       template<typename>
         struct is_rvalue_reference
         : public false_type { };

       template<typename _Tp>
         struct is_rvalue_reference<_Tp&&>
         : public true_type { };

  /// is_reference
      template<typename _Tp>
        struct is_reference
        : public __or_<is_lvalue_reference<_Tp>,
                       is_rvalue_reference<_Tp>>::type
         { };

  template<typename _Tp,
              bool = __and_<__not_<is_reference<_Tp>>,
                            __not_<is_void<_Tp>>>::value>
       struct __add_rvalue_reference_helper
       { typedef _Tp   type; };

     template<typename _Tp>
       struct __add_rvalue_reference_helper<_Tp, true>
       { typedef _Tp&&   type; };

     /// add_rvalue_reference
     template<typename _Tp>
       struct add_rvalue_reference
       : public __add_rvalue_reference_helper<_Tp>
       { };

     template<typename>
       struct remove_cv;

template<typename _Tp, typename>
struct __remove_pointer_helper
{ typedef _Tp     type; };

template<typename _Tp, typename _Up>
struct __remove_pointer_helper<_Tp, _Up*>
{ typedef _Up     type; };

/// remove_pointer
template<typename _Tp>
struct remove_pointer
: public __remove_pointer_helper<_Tp, typename remove_cv<_Tp>::type>
{ };

}


#endif
#endif

