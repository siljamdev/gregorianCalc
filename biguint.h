#ifndef BIGUINT_H
#define BIGUINT_H

#include <stdint.h>

typedef struct{
	size_t len;
	uint32_t *ptr;
} BigUint;

BigUint biguintNew(uint32_t value);
BigUint biguintClone(BigUint *b);
void biguintFree(BigUint *b);
BigUint biguintSum(BigUint *a, BigUint *b);
BigUint biguintSubtract(BigUint *a, BigUint *b);
BigUint biguintMultiply(BigUint *a, BigUint *b);
bool biguintGreater(BigUint *a, BigUint *b);
char *biguintToStr(BigUint *b);

#endif