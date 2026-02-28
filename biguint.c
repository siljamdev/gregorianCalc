#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "biguint.h"

//New with a set length
BigUint newLen(size_t len){
	BigUint b;
	
	b.len = len;
	if(len == 0){
		b.ptr = NULL;
		return b;
	}
	
	b.ptr = malloc(len * sizeof(uint32_t));
	if(b.ptr == NULL){ //Fail
		b.len = 0;
		return b;
	}
	
	//Initialize to 0
	for(size_t i = 0; i < len; i++){
		b.ptr[i] = 0;
	}
	
	return b;
}

//New with a value
BigUint biguintNew(uint32_t value){
	BigUint b;
	
	b.len = 1;
	b.ptr = malloc(sizeof(uint32_t));
	if(b.ptr == NULL){ //Fail
		b.len = 0;
		return b;
	}
	*b.ptr = value;
	return b;
}

BigUint biguintClone(BigUint *b){
	BigUint r;
	
	r.len = b->len;
	if(b->len == 0){
		r.ptr = NULL;
		return r;
	}
	
	r.ptr = malloc(r.len * sizeof(uint32_t));
	if(r.ptr == NULL){ //Fail
		r.len = 0;
		return r;
	}
	
	memcpy(r.ptr, b->ptr, r.len * sizeof(uint32_t)); //Copy memory
	
	return r;
}

void biguintFree(BigUint *b){
	if(b->ptr != NULL){
		free(b->ptr);
		b->ptr = NULL;
	}
	
	b->len = 0;
}

//Shrink to smallest possible length
void shrink(BigUint *b){	
	//Find out real length
	size_t realLen = b->len;
	while(realLen > 0 && b->ptr[realLen - 1] == 0){
		realLen--;
	}
	
	if(realLen > 0){ //Realloc with 0 bytes apparently fills the pointer with garbage
		uint32_t *new = realloc(b->ptr, realLen * sizeof(uint32_t));
		if(new == NULL){ //Fail reallocation
			return;
		}
		
		b->ptr = new;
		b->len = realLen;
	}else{
		free(b->ptr);
		b->ptr = NULL;
		b->len = 0;
	}
}

//Ensure it has allocated length
void ensureLen(BigUint *b, size_t len){
	if(b->len >= len){
		return;
	}
	
	uint32_t *new = realloc(b->ptr, len * sizeof(uint32_t));
	if(new == NULL){
		return;
	}
	
	b->ptr = new;
	
	//Initialize memory
	for(size_t i = b->len; i < len; i++){
		b->ptr[i] = 0;
	}
	
	b->len = len;
}

BigUint biguintSum(BigUint *a, BigUint *b){
	size_t len = a->len > b->len ? a->len : b->len;
	BigUint r = newLen(len);
	
	uint64_t carry = 0;
	
	for(size_t i = 0; i < len; i++){
		uint32_t s1 = i < a->len ? a->ptr[i] : 0;
		uint32_t s2 = i < b->len ? b->ptr[i] : 0;
		
		uint64_t s3 = (uint64_t) s1 + s2 + carry;
		r.ptr[i] = (uint32_t) s3;
		carry = s3 >> 32;
	}
	
	if(carry > 0){
		ensureLen(&r, len + 1); //Expand as needed
		r.ptr[len] = (uint32_t) carry;
	}
	
	return r;
}

//Assume a is bigger than b
BigUint biguintSubtract(BigUint *a, BigUint *b){
	size_t len = a->len;
	BigUint r = newLen(len);
	
	uint64_t borrow = 0;
	
	for(size_t i = 0; i < len; i++){
		uint32_t s1 = a->ptr[i];
		uint32_t s2 = i < b->len ? b->ptr[i] : 0;
		
		uint64_t s3 = (uint64_t) s1 - s2 - borrow;
		
		if(s1 < s2 + borrow){
            borrow = 1;
            s3 += ((uint64_t) 1 << 32);  //Add base
        }else{
            borrow = 0;
        }
		
		r.ptr[i] = (uint32_t) s3;
	}
	
	shrink(&r);
	return r;
}

BigUint biguintMultiply(BigUint *a, BigUint *b){
	BigUint r = newLen(a->len + b->len);
	
	for(size_t i = 0; i < a->len; i++){
		uint64_t carry = 0;
		for(size_t j = 0; j < b->len; j++){
			uint64_t sum = (uint64_t) a->ptr[i] * b->ptr[j] + r.ptr[i + j] + carry;
			r.ptr[i + j] = (uint32_t) (sum & 0xFFFFFFFF);
			carry = sum >> 32;
		}
		
		if(carry > 0){
            r.ptr[i + b->len] += (uint32_t) carry;
        }
	}
	
	//Shrink because used length might be smaller
	shrink(&r);
	
	return r;
}

bool biguintGreater(BigUint *a, BigUint *b){
	size_t len = a->len > b->len ? a->len : b->len;
	
	for(size_t i = len; i-- > 0;){
		uint32_t s1 = i < a->len ? a->ptr[i] : 0;
		uint32_t s2 = i < b->len ? b->ptr[i] : 0;
		
		if(s1 > s2){
			return true;
		}else if(s1 < s2){
			return false;
		}
	}
	
	return false; //Equal
}

//returns remainder (modulus)
uint32_t divideBy10(BigUint *b){
    uint64_t r = 0; //Remainder
	
    for(size_t i = b->len; i-- > 0;){
        uint64_t c = (r << 32) | b->ptr[i];
        b->ptr[i] = (uint32_t) (c / 10);
        r = c % 10;
    }
	
	//Remove last 0s
    while(b->len > 0 && b->ptr[b->len - 1] == 0){
        b->len--;
    }
	
	return (uint32_t) r;
}

char toDigit(uint32_t d){
	switch(d){
		case 0: return '0';
		case 1: return '1';
		case 2: return '2';
		case 3: return '3';
		case 4: return '4';
		case 5: return '5';
		case 6: return '6';
		case 7: return '7';
		case 8: return '8';
		case 9: return '9';
		default: return '\0';
	}
}

char *biguintToStr(BigUint *b){
	BigUint n = biguintClone(b); //We need to modify an instance
	
	size_t len = 11 * n.len + 1;
	char *chars = malloc(len * sizeof(char));
	size_t i = len - 1;
	chars[i--] = '\0';
	
	while(n.len > 0){
		uint32_t toPrint = divideBy10(&n);
		
		chars[i--] = toDigit(toPrint);
	}
	
	char *usedPtr = chars + i + 1;
	size_t usedLen = len - i - 1;
	memmove(chars, usedPtr, usedLen);
	
	char *temp = realloc(chars, usedLen);
	if(temp != NULL){
		chars = temp;
	}
	
	biguintFree(&n);
	
	return chars;
}

void printDebug(BigUint *b){
	printf("%zu: ", b->len);
	printf("%p\n", (void*) b->ptr);
	printf("[");
	
	for(size_t i = 0; i < b->len; i++){
		if(i > 0){
			printf(", ");
		}
		printf("%u", b->ptr[i]);
	}
	
	printf("]\n");
}