#ifndef FOREACH_H_
#define FOREACH_H_

// source: http://stackoverflow.com/questions/14732803/preprocessor-variadic-for-each-macro-compatible-with-msvc10
#define SEMICOLON ;
#define NOTHING

#define EXPAND(x) x
#define FOR_EACH_NARG(...) FOR_EACH_NARG_(__VA_ARGS__, FOR_EACH_RSEQ_N())
#define FOR_EACH_NARG_(...) EXPAND(FOR_EACH_ARG_N(__VA_ARGS__))
#define FOR_EACH_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define FOR_EACH_RSEQ_N() 8, 7, 6, 5, 4, 3, 2, 1, 0
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
#define FOR_EACH_COMMA_(N, what, ...) EXPAND(CONCATENATE(FOR_EACH_COMMA_, N)(what, __VA_ARGS__))
/**
 * Repeatedly invokes macro [what] on each of the variable arguments [...], separating each invocation
 * by a comma (,).
 */
#define FOR_EACH_SEP_COMMA(what, ...) FOR_EACH_COMMA_(FOR_EACH_NARG(__VA_ARGS__), what, __VA_ARGS__)

#endif /* FOREACH_H_ */
