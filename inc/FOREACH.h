#ifndef FOREACH_H_
#define FOREACH_H_

// source: http://stackoverflow.com/questions/14732803/preprocessor-variadic-for-each-macro-compatible-with-msvc10
#define SEMICOLON ;
#define NOTHING

#define EXPAND(x) x
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) EXPAND(FOR_EACH_ARG_N(__VA_ARGS__))
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define FOR_EACH_RSEQ_N() 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define CONCATENATE(x,y) x##y

#define FOR_EACH_1(what, x, ...) what(x)
#define FOR_EACH_2(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_1(what,  __VA_ARGS__))
#define FOR_EACH_3(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_2(what, sep, __VA_ARGS__))
#define FOR_EACH_4(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_3(what, sep, __VA_ARGS__))
#define FOR_EACH_5(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_4(what, sep, __VA_ARGS__))
#define FOR_EACH_6(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_5(what, sep, __VA_ARGS__))
#define FOR_EACH_7(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_6(what, sep, __VA_ARGS__))
#define FOR_EACH_8(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_7(what, sep, __VA_ARGS__))
#define FOR_EACH_9(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_8(what, sep, __VA_ARGS__))
#define FOR_EACH_10(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_9(what, sep, __VA_ARGS__))
#define FOR_EACH_11(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_10(what, sep, __VA_ARGS__))
#define FOR_EACH_12(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_11(what, sep, __VA_ARGS__))
#define FOR_EACH_13(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_12(what, sep, __VA_ARGS__))
#define FOR_EACH_14(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_13(what, sep, __VA_ARGS__))
#define FOR_EACH_15(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_14(what, sep, __VA_ARGS__))
#define FOR_EACH_16(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_15(what, sep, __VA_ARGS__))
#define FOR_EACH_17(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_16(what, sep, __VA_ARGS__))
#define FOR_EACH_18(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_17(what, sep, __VA_ARGS__))
#define FOR_EACH_19(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_18(what, sep, __VA_ARGS__))
#define FOR_EACH_20(what, sep, x, ...)\
  what(x) sep \
  EXPAND(FOR_EACH_19(what, sep, __VA_ARGS__))
#define FOR_EACH_(N, what, sep, ...) EXPAND(CONCATENATE(FOR_EACH_, N)(what, sep, __VA_ARGS__))

/**
 * Repeatedly invokes macro [what] on each of the variable arguments [...], separating each invocation
 * by [sep].
 */
#define FOR_EACH_SEP(sep, what, ...) FOR_EACH_(FOR_EACH_NARG(__VA_ARGS__), what, sep, __VA_ARGS__)

/**
 * Repeatedly invokes macro [what] on each of the variable arguments [...].
 */
#define FOR_EACH(what, ...) FOR_EACH_SEP(NOTHING, what, __VA_ARGS__)

// separated by COMMA
#define FOR_EACH_COMMA_1(what, x, ...) what(x)
#define FOR_EACH_COMMA_2(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_1(what,  __VA_ARGS__))
#define FOR_EACH_COMMA_3(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_2(what, __VA_ARGS__))
#define FOR_EACH_COMMA_4(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_3(what, __VA_ARGS__))
#define FOR_EACH_COMMA_5(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_4(what, __VA_ARGS__))
#define FOR_EACH_COMMA_6(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_5(what, __VA_ARGS__))
#define FOR_EACH_COMMA_7(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_6(what, __VA_ARGS__))
#define FOR_EACH_COMMA_8(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_7(what, __VA_ARGS__))
#define FOR_EACH_COMMA_9(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_8(what, __VA_ARGS__))
#define FOR_EACH_COMMA_10(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_9(what, __VA_ARGS__))
#define FOR_EACH_COMMA_11(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_10(what, __VA_ARGS__))
#define FOR_EACH_COMMA_12(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_11(what, __VA_ARGS__))
#define FOR_EACH_COMMA_13(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_12(what, __VA_ARGS__))
#define FOR_EACH_COMMA_14(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_13(what, __VA_ARGS__))
#define FOR_EACH_COMMA_15(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_14(what, __VA_ARGS__))
#define FOR_EACH_COMMA_16(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_15(what, __VA_ARGS__))
#define FOR_EACH_COMMA_17(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_16(what, __VA_ARGS__))
#define FOR_EACH_COMMA_18(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_17(what, __VA_ARGS__))
#define FOR_EACH_COMMA_19(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_18(what, __VA_ARGS__))
#define FOR_EACH_COMMA_20(what, x, ...)\
  what(x) , \
  EXPAND(FOR_EACH_COMMA_19(what, __VA_ARGS__))
#define FOR_EACH_COMMA_(N, what, ...) EXPAND(CONCATENATE(FOR_EACH_COMMA_, N)(what, __VA_ARGS__))
/**
 * Repeatedly invokes macro [what] on each of the variable arguments [...], separating each invocation
 * by a comma (,).
 */
#define FOR_EACH_SEP_COMMA(what, ...) FOR_EACH_COMMA_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#endif /* FOREACH_H_ */
