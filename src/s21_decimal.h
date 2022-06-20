#include <stdio.h>
#include <limits.h>
#include <string.h>

#define number_is_ok 0
#define number_is_plus_inf 1
#define number_is_minus_inf 2
#define nubmer_is_nan 3

typedef struct {
    int bits[4];
} s21_decimal;

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
// int s21_bit(int number, int byte);
int s21_get_bit(s21_decimal number, int byte);
int s21_equals_intmax(int num);
// void s21_shift_left(int* num, int shift, int n);
void s21_shift_left(s21_decimal* number, int shift, int n);
void print_decimal(s21_decimal number);
int s21_from_int_to_decimal(int src, s21_decimal *dst);
void s21_decimal_init(s21_decimal* num);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_is_less(s21_decimal num1, s21_decimal num2);
void s21_set_bit(s21_decimal* number, int byte, int n);
int s21_is_greater(s21_decimal num1, s21_decimal num2);
int s21_is_less(s21_decimal num1, s21_decimal num2);
int s21_is_equal(s21_decimal num1, s21_decimal num2);
int s21_is_less_or_equal(s21_decimal num1 , s21_decimal num2);