#include <stdio.h>
#include <limits.h>

#define number_is_ok 0
#define number_is_plus_inf 1
#define number_is_minus_inf 2
#define nubmer_is_nan 3

typedef struct {
    int bits[4];
} s21_decimal;

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_bit(int number, int byte);
int s21_equals_intmax(int num);
void s21_shift(int* num, int shift, int n);