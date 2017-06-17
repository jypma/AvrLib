#ifndef __AVR__
#include <type_traits>
#else

#ifndef _GLIBCXX_TYPE_TRAITS
#define _GLIBCXX_TYPE_TRAITS 1

namespace std
{
  typedef __SIZE_TYPE__ 	size_t;
  typedef __PTRDIFF_TYPE__	ptrdiff_t;

#if __cplusplus >= 201103L
  typedef decltype(nullptr)	nullptr_t;
#endif
}

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
  struct __is_integral_helper
  : public false_type { };

template<>
  struct __is_integral_helper<bool>
  : public true_type { };

template<>
  struct __is_integral_helper<char>
  : public true_type { };

template<>
  struct __is_integral_helper<signed char>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned char>
  : public true_type { };

#ifdef _GLIBCXX_USE_WCHAR_T
template<>
  struct __is_integral_helper<wchar_t>
  : public true_type { };
#endif

template<>
  struct __is_integral_helper<char16_t>
  : public true_type { };

template<>
  struct __is_integral_helper<char32_t>
  : public true_type { };

template<>
  struct __is_integral_helper<short>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned short>
  : public true_type { };

template<>
  struct __is_integral_helper<int>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned int>
  : public true_type { };

template<>
  struct __is_integral_helper<long>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned long>
  : public true_type { };

template<>
  struct __is_integral_helper<long long>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned long long>
  : public true_type { };

// Conditionalizing on __STRICT_ANSI__ here will break any port that
// uses one of these types for size_t.
#if defined(__GLIBCXX_TYPE_INT_N_0)
template<>
  struct __is_integral_helper<__GLIBCXX_TYPE_INT_N_0>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned __GLIBCXX_TYPE_INT_N_0>
  : public true_type { };
#endif
#if defined(__GLIBCXX_TYPE_INT_N_1)
template<>
  struct __is_integral_helper<__GLIBCXX_TYPE_INT_N_1>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned __GLIBCXX_TYPE_INT_N_1>
  : public true_type { };
#endif
#if defined(__GLIBCXX_TYPE_INT_N_2)
template<>
  struct __is_integral_helper<__GLIBCXX_TYPE_INT_N_2>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned __GLIBCXX_TYPE_INT_N_2>
  : public true_type { };
#endif
#if defined(__GLIBCXX_TYPE_INT_N_3)
template<>
  struct __is_integral_helper<__GLIBCXX_TYPE_INT_N_3>
  : public true_type { };

template<>
  struct __is_integral_helper<unsigned __GLIBCXX_TYPE_INT_N_3>
  : public true_type { };
#endif

/// is_integral
template<typename _Tp>
  struct is_integral
  : public __is_integral_helper<typename remove_cv<_Tp>::type>::type
  { };


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

template<typename>
  struct remove_reference;

/// remove_reference
template<typename _Tp>
  struct remove_reference
  { typedef _Tp   type; };

template<typename _Tp>
  struct remove_reference<_Tp&>
  { typedef _Tp   type; };

template<typename _Tp>
  struct remove_reference<_Tp&&>
  { typedef _Tp   type; };

// For several sfinae-friendly trait implementations we transport both the
// result information (as the member type) and the failure information (no
// member type). This is very similar to std::enable_if, but we cannot use
// them, because we need to derive from them as an implementation detail.

template<typename _Tp>
  struct __success_type
  { typedef _Tp type; };

struct __failure_type
{ };

/// is_array
template<typename>
  struct is_array
  : public false_type { };

template<typename _Tp, std::size_t _Size>
  struct is_array<_Tp[_Size]>
  : public true_type { };

template<typename _Tp>
  struct is_array<_Tp[]>
  : public true_type { };

template<typename>
  struct is_function;

template<typename>
  struct __is_member_object_pointer_helper
  : public false_type { };

template<typename _Tp, typename _Cp>
  struct __is_member_object_pointer_helper<_Tp _Cp::*>
  : public integral_constant<bool, !is_function<_Tp>::value> { };

/// is_member_object_pointer
template<typename _Tp>
  struct is_member_object_pointer
  : public __is_member_object_pointer_helper<
				typename remove_cv<_Tp>::type>::type
  { };

template<typename>
  struct __is_member_function_pointer_helper
  : public false_type { };

template<typename _Tp, typename _Cp>
  struct __is_member_function_pointer_helper<_Tp _Cp::*>
  : public integral_constant<bool, is_function<_Tp>::value> { };

/// is_member_function_pointer
template<typename _Tp>
  struct is_member_function_pointer
  : public __is_member_function_pointer_helper<
				typename remove_cv<_Tp>::type>::type
  { };

/// is_union
template<typename _Tp>
  struct is_union
  : public integral_constant<bool, __is_union(_Tp)>
  { };

/// is_function
template<typename>
  struct is_function
  : public false_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...)>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......)>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) volatile>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) volatile &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) volatile &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) volatile>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) volatile &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) volatile &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const volatile>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const volatile &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes...) const volatile &&>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const volatile>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const volatile &>
  : public true_type { };

template<typename _Res, typename... _ArgTypes>
  struct is_function<_Res(_ArgTypes......) const volatile &&>
  : public true_type { };

/// remove_extent
template<typename _Tp>
  struct remove_extent
  { typedef _Tp     type; };

template<typename _Tp, std::size_t _Size>
  struct remove_extent<_Tp[_Size]>
  { typedef _Tp     type; };

template<typename _Tp>
  struct remove_extent<_Tp[]>
  { typedef _Tp     type; };

/// is_object
template<typename _Tp>
  struct is_object
  : public __not_<__or_<is_function<_Tp>, is_reference<_Tp>,
                        is_void<_Tp>>>::type
  { };


// Utility to detect referenceable types ([defns.referenceable]).

template<typename _Tp>
  struct __is_referenceable
  : public __or_<is_object<_Tp>, is_reference<_Tp>>::type
  { };

template<typename _Res, typename... _Args>
  struct __is_referenceable<_Res(_Args...)>
  : public true_type
  { };

template<typename _Res, typename... _Args>
  struct __is_referenceable<_Res(_Args......)>
  : public true_type
  { };


// Pointer modifications.

/// add_pointer
template<typename _Tp, bool = __or_<__is_referenceable<_Tp>,
				      is_void<_Tp>>::value>
  struct __add_pointer_helper
  { typedef _Tp     type; };

template<typename _Tp>
  struct __add_pointer_helper<_Tp, true>
  { typedef typename remove_reference<_Tp>::type*     type; };

template<typename _Tp>
  struct add_pointer
  : public __add_pointer_helper<_Tp>
  { };

#if __cplusplus > 201103L
/// Alias template for remove_pointer
template<typename _Tp>
  using remove_pointer_t = typename remove_pointer<_Tp>::type;

/// Alias template for add_pointer
template<typename _Tp>
  using add_pointer_t = typename add_pointer<_Tp>::type;
#endif


// Decay trait for arrays and functions, used for perfect forwarding
// in make_pair, make_tuple, etc.
template<typename _Up,
	   bool _IsArray = is_array<_Up>::value,
	   bool _IsFunction = is_function<_Up>::value>
  struct __decay_selector;

// NB: DR 705.
template<typename _Up>
  struct __decay_selector<_Up, false, false>
  { typedef typename remove_cv<_Up>::type __type; };

template<typename _Up>
  struct __decay_selector<_Up, true, false>
  { typedef typename remove_extent<_Up>::type* __type; };

template<typename _Up>
  struct __decay_selector<_Up, false, true>
  { typedef typename add_pointer<_Up>::type __type; };

/// decay
template<typename _Tp>
  class decay
  {
    typedef typename remove_reference<_Tp>::type __remove_type;

  public:
    typedef typename __decay_selector<__remove_type>::__type type;
  };

template<typename _Tp>
  class reference_wrapper;

// Helper which adds a reference to a type when given a reference_wrapper
template<typename _Tp>
  struct __strip_reference_wrapper
  {
    typedef _Tp __type;
  };

template<typename _Tp>
  struct __strip_reference_wrapper<reference_wrapper<_Tp> >
  {
    typedef _Tp& __type;
  };

template<typename _Tp>
  struct __decay_and_strip
  {
    typedef typename __strip_reference_wrapper<
	typename decay<_Tp>::type>::__type __type;
  };

/// result_of
template<typename _Signature>
  class result_of;

// Sfinae-friendly result_of implementation:

#define __cpp_lib_result_of_sfinae 201210

struct __invoke_memfun_ref { };
struct __invoke_memfun_deref { };
struct __invoke_memobj_ref { };
struct __invoke_memobj_deref { };
struct __invoke_other { };

// Associate a tag type with a specialization of __success_type.
template<typename _Tp, typename _Tag>
  struct __result_of_success : __success_type<_Tp>
  { using __invoke_type = _Tag; };

// [func.require] paragraph 1 bullet 1:
struct __result_of_memfun_ref_impl
{
  template<typename _Fp, typename _Tp1, typename... _Args>
    static __result_of_success<decltype(
    (std::declval<_Tp1>().*std::declval<_Fp>())(std::declval<_Args>()...)
    ), __invoke_memfun_ref> _S_test(int);

  template<typename...>
    static __failure_type _S_test(...);
};

template<typename _MemPtr, typename _Arg, typename... _Args>
  struct __result_of_memfun_ref
  : private __result_of_memfun_ref_impl
  {
    typedef decltype(_S_test<_MemPtr, _Arg, _Args...>(0)) type;
  };

// [func.require] paragraph 1 bullet 2:
struct __result_of_memfun_deref_impl
{
  template<typename _Fp, typename _Tp1, typename... _Args>
    static __result_of_success<decltype(
    ((*std::declval<_Tp1>()).*std::declval<_Fp>())(std::declval<_Args>()...)
    ), __invoke_memfun_deref> _S_test(int);

  template<typename...>
    static __failure_type _S_test(...);
};

template<typename _MemPtr, typename _Arg, typename... _Args>
  struct __result_of_memfun_deref
  : private __result_of_memfun_deref_impl
  {
    typedef decltype(_S_test<_MemPtr, _Arg, _Args...>(0)) type;
  };

// [func.require] paragraph 1 bullet 3:
struct __result_of_memobj_ref_impl
{
  template<typename _Fp, typename _Tp1>
    static __result_of_success<decltype(
    std::declval<_Tp1>().*std::declval<_Fp>()
    ), __invoke_memobj_ref> _S_test(int);

  template<typename, typename>
    static __failure_type _S_test(...);
};

template<typename _MemPtr, typename _Arg>
  struct __result_of_memobj_ref
  : private __result_of_memobj_ref_impl
  {
    typedef decltype(_S_test<_MemPtr, _Arg>(0)) type;
  };

// [func.require] paragraph 1 bullet 4:
struct __result_of_memobj_deref_impl
{
  template<typename _Fp, typename _Tp1>
    static __result_of_success<decltype(
    (*std::declval<_Tp1>()).*std::declval<_Fp>()
    ), __invoke_memobj_deref> _S_test(int);

  template<typename, typename>
    static __failure_type _S_test(...);
};

template<typename _MemPtr, typename _Arg>
  struct __result_of_memobj_deref
  : private __result_of_memobj_deref_impl
  {
    typedef decltype(_S_test<_MemPtr, _Arg>(0)) type;
  };

template<typename _MemPtr, typename _Arg>
  struct __result_of_memobj;

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, _Arg>
  {
    typedef typename remove_cv<typename remove_reference<
      _Arg>::type>::type _Argval;
    typedef _Res _Class::* _MemPtr;
    typedef typename conditional<__or_<is_same<_Argval, _Class>,
      is_base_of<_Class, _Argval>>::value,
      __result_of_memobj_ref<_MemPtr, _Arg>,
      __result_of_memobj_deref<_MemPtr, _Arg>
    >::type::type type;
  };

template<typename _MemPtr, typename _Arg, typename... _Args>
  struct __result_of_memfun;

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, _Arg, _Args...>
  {
    typedef typename remove_cv<typename remove_reference<
      _Arg>::type>::type _Argval;
    typedef _Res _Class::* _MemPtr;
    typedef typename conditional<__or_<is_same<_Argval, _Class>,
      is_base_of<_Class, _Argval>>::value,
      __result_of_memfun_ref<_MemPtr, _Arg, _Args...>,
      __result_of_memfun_deref<_MemPtr, _Arg, _Args...>
    >::type::type type;
  };

// _GLIBCXX_RESOLVE_LIB_DEFECTS
// 2219.  INVOKE-ing a pointer to member with a reference_wrapper
//        as the object expression

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, reference_wrapper<_Arg>>
  : __result_of_memobj_ref<_Res _Class::*, _Arg&>
  { };

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, reference_wrapper<_Arg>&>
  : __result_of_memobj_ref<_Res _Class::*, _Arg&>
  { };

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, const reference_wrapper<_Arg>&>
  : __result_of_memobj_ref<_Res _Class::*, _Arg&>
  { };

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, reference_wrapper<_Arg>&&>
  : __result_of_memobj_ref<_Res _Class::*, _Arg&>
  { };

template<typename _Res, typename _Class, typename _Arg>
  struct __result_of_memobj<_Res _Class::*, const reference_wrapper<_Arg>&&>
  : __result_of_memobj_ref<_Res _Class::*, _Arg&>
  { };

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, reference_wrapper<_Arg>, _Args...>
  : __result_of_memfun_ref<_Res _Class::*, _Arg&, _Args...>
  { };

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, reference_wrapper<_Arg>&,
			      _Args...>
  : __result_of_memfun_ref<_Res _Class::*, _Arg&, _Args...>
  { };

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, const reference_wrapper<_Arg>&,
			      _Args...>
  : __result_of_memfun_ref<_Res _Class::*, _Arg&, _Args...>
  { };

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, reference_wrapper<_Arg>&&,
			      _Args...>
  : __result_of_memfun_ref<_Res _Class::*, _Arg&, _Args...>
  { };

template<typename _Res, typename _Class, typename _Arg, typename... _Args>
  struct __result_of_memfun<_Res _Class::*, const reference_wrapper<_Arg>&&,
			      _Args...>
  : __result_of_memfun_ref<_Res _Class::*, _Arg&, _Args...>
  { };

template<bool, bool, typename _Functor, typename... _ArgTypes>
  struct __result_of_impl
  {
    typedef __failure_type type;
  };

template<typename _MemPtr, typename _Arg>
  struct __result_of_impl<true, false, _MemPtr, _Arg>
  : public __result_of_memobj<typename decay<_MemPtr>::type, _Arg>
  { };

template<typename _MemPtr, typename _Arg, typename... _Args>
  struct __result_of_impl<false, true, _MemPtr, _Arg, _Args...>
  : public __result_of_memfun<typename decay<_MemPtr>::type, _Arg, _Args...>
  { };

// [func.require] paragraph 1 bullet 5:
struct __result_of_other_impl
{
  template<typename _Fn, typename... _Args>
    static __result_of_success<decltype(
    std::declval<_Fn>()(std::declval<_Args>()...)
    ), __invoke_other> _S_test(int);

  template<typename...>
    static __failure_type _S_test(...);
};

template<typename _Functor, typename... _ArgTypes>
  struct __result_of_impl<false, false, _Functor, _ArgTypes...>
  : private __result_of_other_impl
  {
    typedef decltype(_S_test<_Functor, _ArgTypes...>(0)) type;
  };

template<typename _Functor, typename... _ArgTypes>
  struct result_of<_Functor(_ArgTypes...)>
  : public __result_of_impl<
      is_member_object_pointer<
        typename remove_reference<_Functor>::type
      >::value,
      is_member_function_pointer<
        typename remove_reference<_Functor>::type
      >::value,
	    _Functor, _ArgTypes...
    >::type
  { };

template<std::size_t _Len>
  struct __aligned_storage_msa
  {
    union __type
    {
	unsigned char __data[_Len];
	struct __attribute__((__aligned__)) { } __align;
    };
  };

/**
 *  @brief Alignment type.
 *
 *  The value of _Align is a default-alignment which shall be the
 *  most stringent alignment requirement for any C++ object type
 *  whose size is no greater than _Len (3.9). The member typedef
 *  type shall be a POD type suitable for use as uninitialized
 *  storage for any object whose size is at most _Len and whose
 *  alignment is a divisor of _Align.
*/
template<std::size_t _Len, std::size_t _Align =
	   __alignof__(typename __aligned_storage_msa<_Len>::__type)>
  struct aligned_storage
  {
    union type
    {
	unsigned char __data[_Len];
	struct __attribute__((__aligned__((_Align)))) { } __align;
    };
  };

template <typename... _Types>
  struct __strictest_alignment
  {
    static const size_t _S_alignment = 0;
    static const size_t _S_size = 0;
  };

template <typename _Tp, typename... _Types>
  struct __strictest_alignment<_Tp, _Types...>
  {
    static const size_t _S_alignment =
      alignof(_Tp) > __strictest_alignment<_Types...>::_S_alignment
	? alignof(_Tp) : __strictest_alignment<_Types...>::_S_alignment;
    static const size_t _S_size =
      sizeof(_Tp) > __strictest_alignment<_Types...>::_S_size
	? sizeof(_Tp) : __strictest_alignment<_Types...>::_S_size;
  };

/**
 *  @brief Provide aligned storage for types.
 *
 *  [meta.trans.other]
 *
 *  Provides aligned storage for any of the provided types of at
 *  least size _Len.
 *
 *  @see aligned_storage
 */
template <size_t _Len, typename... _Types>
  struct aligned_union
  {
  private:
    static_assert(sizeof...(_Types) != 0, "At least one type is required");

    using __strictest = __strictest_alignment<_Types...>;
    static const size_t _S_len = _Len > __strictest::_S_size
	? _Len : __strictest::_S_size;
  public:
    /// The value of the strictest alignment of _Types.
    static const size_t alignment_value = __strictest::_S_alignment;
    /// The storage.
    typedef typename aligned_storage<_S_len, alignment_value>::type type;
  };

template <size_t _Len, typename... _Types>
  const size_t aligned_union<_Len, _Types...>::alignment_value;

/// common_type
template<typename... _Tp>
  struct common_type;

// Sfinae-friendly common_type implementation:

struct __do_common_type_impl
{
  template<typename _Tp, typename _Up>
    static __success_type<typename decay<decltype
			    (true ? std::declval<_Tp>()
			     : std::declval<_Up>())>::type> _S_test(int);

  template<typename, typename>
    static __failure_type _S_test(...);
};

template<typename _Tp, typename _Up>
  struct __common_type_impl
  : private __do_common_type_impl
  {
    typedef decltype(_S_test<_Tp, _Up>(0)) type;
  };

struct __do_member_type_wrapper
{
  template<typename _Tp>
    static __success_type<typename _Tp::type> _S_test(int);

  template<typename>
    static __failure_type _S_test(...);
};

template<typename _Tp>
  struct __member_type_wrapper
  : private __do_member_type_wrapper
  {
    typedef decltype(_S_test<_Tp>(0)) type;
  };

template<typename _CTp, typename... _Args>
  struct __expanded_common_type_wrapper
  {
    typedef common_type<typename _CTp::type, _Args...> type;
  };

template<typename... _Args>
  struct __expanded_common_type_wrapper<__failure_type, _Args...>
  { typedef __failure_type type; };

template<typename _Tp>
  struct common_type<_Tp>
  { typedef typename decay<_Tp>::type type; };

template<typename _Tp, typename _Up>
  struct common_type<_Tp, _Up>
  : public __common_type_impl<_Tp, _Up>::type
  { };

template<typename _Tp, typename _Up, typename... _Vp>
  struct common_type<_Tp, _Up, _Vp...>
  : public __expanded_common_type_wrapper<typename __member_type_wrapper<
             common_type<_Tp, _Up>>::type, _Vp...>::type
  { };

#if __cplusplus > 201103L
  /// Alias template for aligned_storage
  template<size_t _Len, size_t _Align =
	    __alignof__(typename __aligned_storage_msa<_Len>::__type)>
    using aligned_storage_t = typename aligned_storage<_Len, _Align>::type;

  template <size_t _Len, typename... _Types>
    using aligned_union_t = typename aligned_union<_Len, _Types...>::type;

  /// Alias template for decay
  template<typename _Tp>
    using decay_t = typename decay<_Tp>::type;

  /// Alias template for enable_if
  template<bool _Cond, typename _Tp = void>
    using enable_if_t = typename enable_if<_Cond, _Tp>::type;

  /// Alias template for conditional
  template<bool _Cond, typename _Iftrue, typename _Iffalse>
    using conditional_t = typename conditional<_Cond, _Iftrue, _Iffalse>::type;

  /// Alias template for common_type
  template<typename... _Tp>
    using common_type_t = typename common_type<_Tp...>::type;

  /// Alias template for underlying_type
  template<typename _Tp>
    using underlying_type_t = typename underlying_type<_Tp>::type;

  /// Alias template for result_of
  template<typename _Tp>
    using result_of_t = typename result_of<_Tp>::type;
#endif

}


#endif
#endif

