#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define number_is_ok 0
#define number_is_plus_inf 1
#define number_is_minus_inf 2
#define nubmer_is_nan 3

typedef struct {
    int bits[4];
} s21_decimal;

union float_value {
    unsigned int int_view;
    float float_view;
};

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_get_bit(s21_decimal number, int byte);
int s21_equals_intmax(int num);
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
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub_res(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_helping_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
void from_float_to_string(float src, char* float_bin_buff);
void int_set_bit(unsigned int* number, int byte, int n);
int int_get_bit(unsigned int number, int byte);
void int_shift_left(unsigned int* num, int shift, int n);
void mul_by_10(s21_decimal* num);
void s21_set_scale(s21_decimal* num, int scale);
int s21_negate(s21_decimal value, s21_decimal *result);
int get_higher_bit(s21_decimal value);
int get_scale(s21_decimal value);
void s21_set_equal_scale(s21_decimal* num1, s21_decimal* num2);
void s21_set_scale(s21_decimal* num, int scale);
int s21_div_by_10(s21_decimal* number);
int s21_truncate(s21_decimal value, s21_decimal* result);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
void s21_set_equal_scale_back(s21_decimal* num1, s21_decimal* num2);
void s21_shift_decimal_left(s21_decimal* number);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);
int s21_div(s21_decimal value1, s21_decimal value2, s21_decimal* result);
int s21_from_decimal_to_float(s21_decimal src, float *dst);
void div_processing(s21_decimal *value_1, s21_decimal *value_2,
                  s21_decimal *result);
                  int initDecimalMinuend(s21_decimal *minuend, s21_decimal value_1,
                       s21_decimal value_2);
int sizeOfNumber(s21_decimal d);
void initMinuend(unsigned int *minuend, s21_decimal d, int begin);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
void getBinaryFromFloat(float src, char *float_bin_buff);
int getScaleFloat(const char *src);
void writeMantissaToDecimal(s21_decimal *d, char *float_bin_buff, int exp);
int s21_is_greater_or_equal(s21_decimal num1 , s21_decimal num2);
int s21_is_not_equal(s21_decimal num1, s21_decimal num2);
void nullSignAndScale(s21_decimal *d1, s21_decimal *d2);

#endif  //  SRC_S21_DECIMAL_H_
