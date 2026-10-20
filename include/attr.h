#ifndef ATTR_H_  /* trailing _ because colision... */
#define ATTR_H_

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

/* see comments in attr_size() */
#if defined(_MSC_VER) && !defined(__clang__)
#  include <intrin.h>
#endif

/* attributes are only A..Z
 *
 * because of that a bitset solution is not bad...
 * we can make all sorts of operations very fast with it
 *
 * i considered a graph solution but since the input is limited this
 * should minimize overhead, even though candidate keys can be exponential ;-;
 *
 * do i need them all? no, but it was fun to code and i may use in the
 * future idk
 *
 * did them all as static inline so that the compiler can inline it all
 * otherwise it would be a different translation unit blah blah
 * see: https://stackoverflow.com/questions/7762731/whats-the-difference-between-static-and-static-inline-function
 */

typedef uint32_t AttrSet;

#define ATTR_COUNT 26u

#define ATTR(i) ((AttrSet)1u << (i))
#define ATTR_A  ATTR(0)
#define ATTR_B  ATTR(1)
#define ATTR_C  ATTR(2)
#define ATTR_D  ATTR(3)
#define ATTR_E  ATTR(4)
#define ATTR_F  ATTR(5)
#define ATTR_G  ATTR(6)
#define ATTR_H  ATTR(7)
#define ATTR_I  ATTR(8)
#define ATTR_J  ATTR(9)
#define ATTR_K  ATTR(10)
#define ATTR_L  ATTR(11)
#define ATTR_M  ATTR(12)
#define ATTR_N  ATTR(13)
#define ATTR_O  ATTR(14)
#define ATTR_P  ATTR(15)
#define ATTR_Q  ATTR(16)
#define ATTR_R  ATTR(17)
#define ATTR_S  ATTR(18)
#define ATTR_T  ATTR(19)
#define ATTR_U  ATTR(20)
#define ATTR_V  ATTR(21)
#define ATTR_W  ATTR(22)
#define ATTR_X  ATTR(23)
#define ATTR_Y  ATTR(24)
#define ATTR_Z  ATTR(25)

#define ATTR_FULL  (ATTR(26) - 1)   /* mask 2^26 - 1: the 26 lsb set (A..Z) */
#define ATTR_EMPTY ((AttrSet) 0u)

/* convert char -> attr */
static inline AttrSet attr_char(char c) {
  return (c >= 'A' && c <= 'Z') ? ATTR((unsigned)(c - 'A')) : ATTR_EMPTY;
}

static inline AttrSet attr_union(AttrSet a, AttrSet b) { return a | b; }
static inline AttrSet attr_inter(AttrSet a, AttrSet b) { return a & b; }
static inline AttrSet attr_diff (AttrSet a, AttrSet b) { return a & ~b; }

static inline AttrSet attr_add(AttrSet a, unsigned int idx) {
  assert(idx < ATTR_COUNT);
  return a | ATTR(idx);
}
static inline AttrSet attr_del(AttrSet a, unsigned int idx) {
  assert(idx < ATTR_COUNT);
  return a & ~ATTR(idx);
}
static inline AttrSet attr_toggle(AttrSet a, unsigned int idx) {
  assert(idx < ATTR_COUNT);
  return a ^ ATTR(idx);
}

static inline bool attr_in(unsigned int idx, AttrSet a) {
  assert(idx < ATTR_COUNT);
  return (a & ATTR(idx)) != 0;
}
static inline bool attr_empty(AttrSet a)              { return a == ATTR_EMPTY; }
static inline bool attr_equal(AttrSet a, AttrSet b)   { return a == b; }
static inline bool attr_subset(AttrSet a, AttrSet b)  { return (a & ~b) == 0; }  /* b contains a (a <= b) */
static inline bool attr_psubset(AttrSet a, AttrSet b) {                          /* b strictly contains a (a < b) */
  return (a != b) && attr_subset(a, b);
}
static inline bool attr_disjoint(AttrSet a, AttrSet b) { return (a & b) == 0; }

static inline int attr_size(AttrSet x) { /* number of elements */
  /* rabbit-hole... things to note:
   *
   * - we want to count the number of ones in a 32bit number
   * - x86 has a instruction for that (of course)
   * - most compilers support using a builtin for that
   * - i left comments with sources to the docs
   * - i also added a hamming weight implementation in case you are not using
   *   a mainstream compiler... why do dis?
   */

  /* https://stackoverflow.com/questions/109023/count-the-number-of-set-bits-in-a-32-bit-integer#109025 */

  /* https://www.felixcloutier.com/x86/popcnt */

#if defined(__GNUC__) || defined(__clang__)

  /* https://gcc.gnu.org/onlinedocs/gcc/Bit-Operation-Builtins.html#index-_005f_005fbuiltin_005fpopcount */
  /* https://clang.llvm.org/docs/LanguageExtensions.html#builtin-popcountg */
  return __builtin_popcount(x);

#elif defined(_MSC_VER)

  /* https://learn.microsoft.com/en-us/cpp/intrinsics/popcnt16-popcnt-popcnt64?view=msvc-170 */
  return (int)__popcnt(x);

#else

  /* https://en.wikipedia.org/wiki/Hamming_weight */

  /* modified for 32bit */
  x -= (x >> 1) & 0x55555555u;
  x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
  x = (x + (x >> 4)) & 0x0F0F0F0Fu;
  return (int)((x * 0x01010101u) >> 24);
#endif
}

#endif /* ATTR_H_ */
