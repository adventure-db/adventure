#ifndef ADV_UTIL_MATH_H
#define ADV_UTIL_MATH_H

// Round up to nearest power of 2 (64-bit input)
// http://graphics.stanford.edu/~seander/bithacks.html
// TODO: double check and make portable
static inline unsigned long next_pow_2(unsigned long n)
{
	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	n++;
	return n;
}

// TODO: this is heavily platform dependent (and requires gcc/clang)
static inline unsigned long log2(unsigned long n)
{
	return (8*sizeof(unsigned long) - __builtin_clzl(n))-1;
}

#endif
