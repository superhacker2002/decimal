#include "s21_decimal.h"
#include <float.h>
// bits[0] -

// Складывает два числа, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
// 1 inf, 2 -inf
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    s21_decimal_init(result);
    s21_set_equal_scale(&value_2, &value_1);
    if (s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // -x + (-y)
        s21_set_bit(&value_1, 127, 0);
        s21_set_bit(&value_2, 127, 0);
        if (s21_helping_add(value_1, value_2, result) == 1) res = 2;
        s21_set_bit(result, 127, 1);
    } else if (s21_get_bit(value_1, 127) && !s21_get_bit(value_2, 127)) {  // -x + y
        s21_set_bit(&value_1, 127, 0);
        s21_sub(value_2, value_1, result);
    } else if (!s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // x + (-y)
        s21_set_bit(&value_2, 127, 0);
        s21_sub(value_1, value_2, result);
    } else if (!s21_get_bit(value_1, 127) && !s21_get_bit(value_2, 127)) {  // x + y
        res = s21_helping_add(value_1, value_2, result);
    }
    s21_set_scale(result, get_scale(value_1));
    return res;
}

// 1- (+/-)inf, 2- nan
int s21_helping_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    int n = 0;
    int tmp = 0;
    while (n < 96) {
        int bits = s21_get_bit(value_1, n) + s21_get_bit(value_2, n) + tmp;
        if (bits == 1) {
            s21_shift_left(result, 1, n);   // сдвигаем нный байт на противоложный, то бишь на 1
            tmp = 0;
        } else if (bits == 2) {
            tmp = 1;
        } else if (bits == 3) {
            s21_shift_left(result, 1, n);  // сдвигаем нный байт на противоложный, то бишь на 1
            tmp = 1;
        }
        n++;
    }
    if (tmp == 1) {
        res = 1;
    }
    return res;
}

// Умножает два числа, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int mul_res = 0;
    s21_decimal tmp = {{0, 0, 0, 0}};
    s21_decimal_init(result);
    int sign1 = s21_get_bit(value_1, 127), sign2 = s21_get_bit(value_2, 127);
    int scale1 = get_scale(value_1), scale2 = get_scale(value_2);
    // скейлы будут складываться, числа умножаться друг на друга

    for (int n = 0; n < 96 && mul_res == 0; n++) {  // умножаем числа
        s21_decimal_init(&tmp);
        if (s21_get_bit(value_2, n)) {
            tmp.bits[0] = value_1.bits[0] & 0xFFFFFFFF;
            tmp.bits[1] = value_1.bits[1] & 0xFFFFFFFF;
            tmp.bits[2] = value_1.bits[2] & 0xFFFFFFFF;
        } else {
            tmp.bits[0] = value_1.bits[0] & 0;
            tmp.bits[1] = value_1.bits[1] & 0;
            tmp.bits[2] = value_1.bits[2] & 0;
        }
        for (int i = 0; i < n; i++)
            s21_shift_decimal_left(&tmp);
        if (s21_add(*result, tmp, result)) {
            mul_res = sign1 + sign2 == 0 ? 1 : sign1 + sign2 == 1 ? 2 : 1; 
        }
    }

    if (scale1 + scale2 <= 28) {
        for (int n = 112, tmp = 0; n <= 119; n++) {  // суммируем скейлы
            int bits = s21_get_bit(value_1, n) + s21_get_bit(value_2, n) + tmp;
            if (bits == 1) {
                s21_set_bit(result, n, 1);
                tmp = 0;
            } else if (bits == 2) {
                tmp = 1;
            } else if (bits == 3) {
                s21_set_bit(result, n, 1);
                tmp = 1;
            }
        }
    } else {
        mul_res = sign1 + sign2 == 0 ? 1 : sign1 + sign2 == 1 ? 2 : 1;
    }

    // ставим знак
    if (s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // if val1 < 0 && val2 < 0 -> +res
        s21_set_bit(result, 127, 0);
    } else if (s21_get_bit(value_1, 127) && !s21_get_bit(value_2, 127)) {  // if val1 < 0 && val2 > 0 -> -res
        s21_set_bit(result, 127, 1);
    } else if (!s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // if val1 > 0 && val2 < 0 -> -res
        s21_set_bit(result, 127, 1);
    }
    return mul_res;
}

// Возвращает 1 если на byte-ом байте в числе number стоит 1
int s21_get_bit(s21_decimal number, int byte) {
    int bits = byte / 32;
    byte = byte % 32;
    return (number.bits[bits] & 1 << byte) ? 1 : 0;
}

// Устанавливает byte-й бит в числе number на n=(0/1)
void s21_set_bit(s21_decimal* number, int byte, int n) {
    if (s21_get_bit(*number, byte) != n)
        s21_shift_left(number, 1, byte);
}

// Сдвигает влево n-ный байт на shift байтов в числе number
void s21_shift_left(s21_decimal* number, int shift, int n) {
    int byte = n / 32;
    n = n % 32;
    number->bits[byte] ^= shift << n;
}

void s21_shift_decimal_left(s21_decimal* number) {
    int low_last_bit = s21_get_bit(*number, 31);
    int mid_last_bit = s21_get_bit(*number, 63);

    number->bits[0] <<= 1;
    number->bits[1] <<= 1;
    number->bits[2] <<= 1;

    s21_set_bit(number, 32, low_last_bit);
    s21_set_bit(number, 64, mid_last_bit);
}

void int_set_bit(unsigned int* number, int byte, int n) {
    if (int_get_bit(*number, byte) != n)
        int_shift_left(number, 1, byte);
}

int int_get_bit(unsigned int number, int byte) {
    return (number & 1 << byte) ? 1 : 0;
}

void int_shift_left(unsigned int* num, int shift, int n) {
    *num ^= shift << n;
}

//  выводит число в двоичном представлении
void print_decimal(s21_decimal number) {
    printf("+/-\t    SCALE\t\t\t\t\tHIGH\t\t\t\tMID\t\t\t\tLOW\n");
    for (int i = 127; i >= 0; i--) {
        printf("%d", s21_get_bit(number, i));
        if (i == 32 || i == 64 || i == 96 || i == 112 || i == 120 || i == 127) printf(" ");
    }
    printf("\n");
}

//  конвертация из int в s21_decimal. Возвращает 0 при успехе и 1 при ошибке
int s21_from_int_to_decimal(int src, s21_decimal *dst) {
    int res = 0;
    if (dst) {
        s21_decimal_init(dst);
        if (src & 1 << 31) {
            s21_set_bit(dst, 127, 1);
            src *= -1;
        }
        int n = 0;
        while (n < 31) {
            if (src & 1 << n) s21_set_bit(dst, n, 1);
            n++;
        }
    } else {
        res = 1;
    }
    return res;
}

void s21_decimal_init(s21_decimal* num) {
    num->bits[0] = 0;
    num->bits[1] = 0;
    num->bits[2] = 0;
    num->bits[3] = 0;
}

void s21_set_equal_scale(s21_decimal* num1, s21_decimal* num2) {
    int scale_1 = get_scale(*num1), scale_2 = get_scale(*num2);
    int scale_diff;
    s21_decimal num1_tmp = *num1;
    s21_decimal num2_tmp = *num2;
    int overflow_flag = 0;
    if (scale_1 < scale_2) {
        scale_diff = scale_2 - scale_1;
        s21_set_scale(&num1_tmp, scale_2);
        for (; scale_diff > 0; scale_diff--) {
            if (s21_get_bit(num1_tmp, 93) == 0
            && s21_get_bit(num1_tmp, 94) == 0
            && s21_get_bit(num1_tmp, 95) == 0) {
                mul_by_10(&num1_tmp);
            } else {
                overflow_flag = 1;
                s21_set_equal_scale_back(num1, num2);
                break;
            }
        }
    } else if (scale_1 > scale_2) {
        scale_diff = scale_1 - scale_2;
        s21_set_scale(&num2_tmp, scale_1);
        for (; scale_diff > 0; scale_diff--) {
            if (s21_get_bit(num2_tmp, 93) == 0
            && s21_get_bit(num2_tmp, 94) == 0
            && s21_get_bit(num2_tmp, 95) == 0) {
                mul_by_10(&num2_tmp);
            } else {
                overflow_flag = 1;
                s21_set_equal_scale_back(num1, num2);
                break;
            }
        }
    }
    if (overflow_flag == 0) {
        *num1 = num1_tmp;
        *num2 = num2_tmp;
    }
}

void s21_set_equal_scale_back(s21_decimal* num1, s21_decimal* num2) {
    int scale_1 = get_scale(*num1), scale_2 = get_scale(*num2);
    int scale_diff;
    if (scale_1 > scale_2) {
        scale_diff = scale_1 - scale_2;
        s21_set_scale(num1, scale_2);
        for (; scale_diff > 0; scale_diff--)
                s21_div_by_10(num1);
    } else if (scale_2 > scale_1) {
        scale_diff = scale_2 - scale_1;
        s21_set_scale(num2, scale_1);
        for (; scale_diff > 0; scale_diff--)
            s21_div_by_10(num2);
    }
}

// проверяет что число num1 меньше чем число num2. Возвращает 1 - TRUE, 0 - FALSE.
int s21_is_less(s21_decimal num1, s21_decimal num2) {
    int result = 0;
    if (s21_get_bit(num1, 127) && !s21_get_bit(num2, 127)) {  // если num1 отрицательное, а num2 положительное
        result = 1;
    } else if (!s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {
        result = 0;
    } else {
        s21_set_equal_scale(&num1, &num2);  // приравнием скейл чисел, что проверять только bits[0-2]
        int n = 95;
        while (n >= 0) {
            if (s21_get_bit(num1, n) < s21_get_bit(num2, n)) {
                if (s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {  // если оба числа меньше 0
                    result = 0;
                } else {
                result = 1;
                }
                break;
            } else if (s21_get_bit(num1, n) > s21_get_bit(num2, n)) {
                if (s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {  // если оба числа меньше 0
                    result = 1;
                } else {
                result = 0;
                }
                break;
            } else {
                n--;
            }
        }
    }
    return result;
}

// проверяет что число num1 больше чем число num2. Возвращает 1 - TRUE, 0 - FALSE.
int s21_is_greater(s21_decimal num1, s21_decimal num2) {
    int result = 0;
    if (s21_get_bit(num1, 127) && !s21_get_bit(num2, 127)) {  // если num1 отрицательное, а num2 положительное
        result = 0;
    } else if (!s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {
        result = 1;
    } else {
        s21_set_equal_scale(&num1, &num2);  // приравнием скейл чисел, что проверять только bits[0-2]
        int n = 95;
        while (n >= 0) {
            if (s21_get_bit(num1, n) > s21_get_bit(num2, n)) {
                if (s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {  // если оба числа меньше 0
                    result = 0;
                } else {
                    result = 1;
                }
                break;
            } else if (s21_get_bit(num1, n) < s21_get_bit(num2, n)) {
                if (s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {  // если оба числа меньше 0
                    result = 1;
                } else {
                    result = 0;
                }
                break;
            } else {
                n--;
            }
        }
    }
    return result;
}

// Возвращает 1 - если num1 == num2, иначе - 0
int s21_is_equal(s21_decimal num1, s21_decimal num2) {
    int result = 1;
    if (s21_get_bit(num1, 127) != s21_get_bit(num2, 127)) {
        result = 0;
    } else {
        s21_set_equal_scale(&num1, &num2);  // приравнием скейл чисел, что проверять только bits[0-2]
        int n = 0;
        while (n < 96) {
            if (s21_get_bit(num1, n) != s21_get_bit(num2, n)) {
                result = 0;
                break;
            }
            n++;
        }
    }
    return result;
}

// Возвращает 1 - если num1 <= num2, иначе - 0
int s21_is_less_or_equal(s21_decimal num1 , s21_decimal num2) {
    return s21_is_less(num1, num2) || s21_is_equal(num1, num2) ? 1 : 0;
}

// Возвращает 1 - если num1 >= num2, иначе - 0
int s21_is_greater_or_equal(s21_decimal num1 , s21_decimal num2) {
    return s21_is_greater(num1, num2) || s21_is_equal(num1, num2) ? 1 : 0;
}

// Возвращает 1 - если num1 != num2, иначе - 0
int s21_is_not_equal(s21_decimal num1, s21_decimal num2) {
    return s21_is_less(num1, num2) || s21_is_greater(num1, num2) ? 1 : 0;
}

// Вычитает value1 из value2, результат записывается в result.
// Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    s21_set_equal_scale(&value_1, &value_2);
    if (s21_is_less(value_1, value_2)) {
        if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 1) {
            s21_set_bit(&value_1, 127, 0);
            s21_set_bit(&value_2, 127, 0);
            s21_sub_res(value_1, value_2, result);
            s21_set_bit(result, 127, 1);
        } else if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 0)  {
            s21_set_bit(&value_1, 127, 0);
            if (s21_add(value_1, value_2, result) == 1) res = 2;
            s21_set_bit(result, 127, 1);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 0) {
            s21_sub_res(value_2, value_1, result);
            s21_set_bit(result, 127, 1);
        }
    } else if (s21_is_greater(value_1, value_2)) {
        if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 1) {
            s21_set_bit(&value_1, 127, 0);
            s21_set_bit(&value_2, 127, 0);
            s21_sub_res(value_2, value_1, result);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 1) {
            s21_set_bit(&value_2, 127, 0);
            res = s21_add(value_1, value_2, result);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 0) {
            s21_sub_res(value_1, value_2, result);
        }
    }
    s21_set_scale(result, get_scale(value_1));
    return res;
}

int s21_sub_res(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    s21_decimal_init(result);
    int i = 0;
    int tmp = 0;
        while (i < 96) {
            int bits = s21_get_bit(value_1, i) - tmp - s21_get_bit(value_2, i);  // 1 - 0 - 1
            if (bits == 1) {
                s21_shift_left(result, 1, i);
                tmp = 0;
            } else if (bits == -1) {
                s21_shift_left(result, 1, i);
                tmp = 1;
            } else if (bits == 0) {
                tmp = 0;
            }
            i++;
        }
    return 0;
}

//  конвертация из float в s21_decimal. Возвращает 0 при успехе и 1 при ошибке
int s21_from_float_to_decimal(float src, s21_decimal *dst) {
    int status = 0;
    if (src == 1 / 0.0 && src == 0 / 0.0 && src == -1 / 0.0 && src == -0 / 0.0)
        status = 1;
    s21_decimal_init(dst);
    if (src != 0) {
        s21_decimal_init(dst);
        if (dst && !status) {
            int is_negative = 0;
            if (src < 0) {
                is_negative = 1;
                src *= -1;
            }
            double dbl = src;
            char float_bin_buff[32] = "";
            getBinaryFromFloat((float)dbl, float_bin_buff);
            int exp = getScaleFloat(float_bin_buff);
            int scale = 0;
            while (scale < 28 && (int)dbl / (int)pow(2, 21) == 0) {
                dbl *= 10;
                scale++;
            }
            dbl = round(dbl);
            if (scale <= 28 && (exp > -94 && exp < 96)) {
                while (fmod(dbl, 10) == 0 && scale > 0) {
                    dbl = dbl / 10;
                    scale--;
                }
                getBinaryFromFloat((float)dbl, float_bin_buff);
                exp = getScaleFloat(float_bin_buff);
                s21_set_bit(dst, exp, 1);
                writeMantissaToDecimal(dst, float_bin_buff, exp);
                s21_set_scale(dst, scale);
                s21_set_bit(dst, 127, is_negative);
            } else {
                status = 1;
            }
        }
    }
    return status;
}


void getBinaryFromFloat(float src, char *float_bin_buff) {
    union float_value float_number_bits;
    float_number_bits.float_view = src;
    for (unsigned int mask = 0x80000000; mask; mask >>= 1) {
        *float_bin_buff = !!(float_number_bits.int_view & mask);
        float_bin_buff++;
    }
}

int getScaleFloat(const char *src) {
    int result = 0, base = 1;
    for (int i = 8; i > 0; i--) {
        result += src[i] * base;
        base = base * 2;
    }
    return (result - 127);
}

void writeMantissaToDecimal(s21_decimal *d, char *float_bin_buff, int exp) {
    for (int i = exp - 1, j = 9; i >= 0 && j < 32; i--, j++) {
        if (float_bin_buff[j]) s21_set_bit(d, i, 1);
    }
}

void mul_by_10(s21_decimal* num) {
    s21_decimal value2 = {{10, 0, 0, 0}};
    s21_mul(*num, value2, num);
}

void s21_set_scale(s21_decimal* num, int scale) {
    int sign = s21_get_bit(*num, 127);
    num->bits[3] = scale;
    num->bits[3] <<= 16;
    s21_set_bit(num, 127, sign);
}

int s21_negate(s21_decimal value, s21_decimal *result) {
    int res = 0;
    if (result) {
        s21_decimal_init(result);
        for (int i = 0; i < 4; i++)
            result->bits[i] = value.bits[i];
        if (s21_get_bit(value, 127) == 1)
            s21_set_bit(result, 127, 0);
        else
            s21_set_bit(result, 127, 1);
    } else {
        res = 1;
    }
    return res;
}

int equals_zero(s21_decimal value) {
    s21_decimal zero = {{0, 0, 0, 0}};
    return s21_is_equal(value, zero);
}

int get_higher_bit(s21_decimal value) {
    int i = 95;
    for (; i >= 0; i--) {
        if (s21_get_bit(value, i)) {
            break;
        }
    }
    return i;
}

int get_scale(s21_decimal value) {
    int mask = 0x7F8000;
    int scale = value.bits[3] & mask;
    scale >>= 16;
    return scale;
}

int s21_div_by_10(s21_decimal* number) {
    unsigned int buffer = 0;
    int scale_of_number = get_scale(*number);
    s21_decimal result = {{0, 0, 0, 0}};
    int highest_bit_pos = get_higher_bit(*number);
    for (int i = 0; i < highest_bit_pos + 2; i++) {
        if (buffer >= 10) {
            s21_shift_decimal_left(&result);
            s21_set_bit(&result, 0, 1);
            buffer -= 10;
            int new_bit = s21_get_bit(*number, highest_bit_pos - i);
            buffer = buffer << 1;
            int_set_bit(&buffer, 0, new_bit);
        } else {
            s21_shift_decimal_left(&result);
            s21_set_bit(&result, 0, 0);
            int new_bit = s21_get_bit(*number, highest_bit_pos - i);
            buffer = buffer << 1;
            int_set_bit(&buffer, 0, new_bit);
        }
    }
    buffer = buffer << 1;
    *number = result;
    s21_set_scale(number, scale_of_number);
    return buffer;
}

int s21_truncate(s21_decimal value, s21_decimal* result) {
    int scale = get_scale(value);
    int sign = s21_get_bit(value, 127);

    for (int i = 0; i < scale; i++) {
        s21_div_by_10(&value);
    }

    *result = value;
    s21_set_bit(result, 127, sign);
    s21_set_scale(result, 0);

    return 0;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
    int res = 0;
    if (dst) {
        *dst = 0;  // avoiding errors
        int sign = s21_get_bit(src, 127);
        s21_decimal tmp = {{0, 0, 0, 0}};
        s21_truncate(src, &tmp);
        if (get_higher_bit(tmp) < 31) {
            *dst = tmp.bits[0];
            if (sign == 1) *dst *= -1;
        } else {
            res = 1;
        }
    } else {
        res = 1;
    }
    return res;
}

int s21_round(s21_decimal value, s21_decimal *result) {
    int is_less_then_zero = 0;
    s21_decimal tmp_value;
    s21_decimal half;
    s21_decimal diff;
    s21_decimal one = {{1, 0, 0, 0}};
    s21_from_float_to_decimal(0.5, &half);
    s21_decimal zero = {{0, 0, 0, 0}};
    if (s21_is_less(value, zero)) {
        is_less_then_zero = 1;
        s21_negate(value, &value);
    }
    s21_truncate(value, &tmp_value);
    s21_sub(value, tmp_value, &diff);
    if (s21_is_greater_or_equal(diff, half)) {
        s21_add(tmp_value, one, result);
    } else {
        *result = tmp_value;
    }
    if (is_less_then_zero) {
        s21_negate(*result, result);
    }
    return 0;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
    s21_decimal zero = {{0, 0, 0, 0}};
    s21_decimal one = {{1, 0, 0, 0}};
    s21_truncate(value, result);
    if (s21_is_less(value, zero)) {
        s21_sub(*result, one, result);
    }
    return 0;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
    int ret = 0;
    if (!dst || get_scale(src) > 28) {
        ret = 1;
    } else {
        *dst = 0.0;
        int scale = get_scale(src);
        int last_bit = s21_get_bit(src, 127);
        int sign = s21_get_bit(src, 127);
        if (sign) s21_set_bit(&src, 127, 1);
        s21_set_scale(&src, scale);
        double tmp = *dst;
        if (last_bit == 1) s21_set_bit(&src, 127, 1);
        unsigned long base = 1;
        for (int i = 0; i < 96; i++) {
            tmp += s21_get_bit(src, i) * base;
            base = base * 2;
        }
        while (scale != 0) {
            tmp = tmp / 10;
            scale--;
        }
        if (s21_get_bit(src, 127)) tmp *= -1;
        *dst = tmp;
    }
    return ret;
}

// -----------------------division------------------------------------------

// 0 - OK
// 1 - число слишком велико или равно бесконечности
// 2 - число слишком мало или равно отрицательной бесконечности
// 3 - деление на 0

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int status = 0;
    int sign1 = s21_get_bit(value_1, 127);
    int sign2 = s21_get_bit(value_2, 127);

    if (equals_zero(value_2) == 1) {
        status = 3;
    } else if (equals_zero(value_1) == 1) {
        s21_decimal_init(result);
    } else {
        s21_set_equal_scale(&value_1, &value_2);
        if (equals_zero(value_1) == 1) {
            status = 2;
        } else if (equals_zero(value_2) == 1) {
            status = 1;
            if (sign1 != sign2) {
                status = 2;
            }
        } else {
            div_processing(&value_1, &value_2, result);
            if (equals_zero(*result) == 1) {
                status = 2;
            }
        }
        if (sign1 != sign2) {
            s21_set_bit(result, 127, 1);
        }
    }
    return status;
}

void div_processing(s21_decimal *value_1, s21_decimal *value_2,
                  s21_decimal *result) {
    s21_decimal minuend, resultOfDevide;
    int i, j, status_of_mul;
    int firstCycle = 0;
    s21_decimal ten = {{10, 0, 0, 0}};
    int wholeNumber = 0;

    do {
        status_of_mul = 0;
        s21_decimal_init(&minuend);
        s21_decimal_init(&resultOfDevide);
        if (s21_is_less(*value_1, *value_2)) {  // делимое меньше делителя
            minuend = *value_1;  // уменьшаемое value1
            i = -1;
            if (firstCycle == 1) {  // не первая итерация
                wholeNumber--;
            }
        } else {
            i = initDecimalMinuend(&minuend, *value_1, *value_2);  
        }

        j = 95;
        int check = 0;
        for (; j >= 0 && check == 0; i--, j--) {
            if (s21_is_greater_or_equal(minuend, *value_2)) {
                s21_set_bit(&resultOfDevide, j, 1);
                s21_sub_res(minuend, *value_2, &minuend);
                if (i >= 0) {
                    s21_shift_decimal_left(&minuend);
                    s21_set_bit(&minuend, 0, s21_get_bit(*value_1, i));
                }
            } else {
                s21_set_bit(&resultOfDevide, j, 0);
                if (i >= 0) {
                    s21_shift_decimal_left(&minuend);
                    s21_set_bit(&minuend, 0, s21_get_bit(*value_1, i));
                }
            }
            if (s21_is_less(minuend, *value_2) && i == -1 && firstCycle == 0) {
                check = 1;
                if (equals_zero(resultOfDevide) != 1) {
                    s21_decimal_init(result);
                    int tmp = 95;
                    for (int k = 95 - j; k >= 0; k--, tmp--) {
                        s21_set_bit(result, k, s21_get_bit(resultOfDevide, tmp));
                    }
                    wholeNumber = sizeOfNumber(*result);
                    s21_decimal_init(result);
                }
                firstCycle = 1;
            }
            if (s21_is_less(minuend, *value_2) && i == -1 && firstCycle == 1) {
                check = 1;
            }
        }
        status_of_mul = s21_mul(*value_1, ten, value_1);
    } while (equals_zero(minuend) != 1 && status_of_mul == 0);
    s21_decimal_init(result);
    int tmp = 95;
    for (i = 94 - j; i >= 0; i--, tmp--) {
        s21_set_bit(result, i, s21_get_bit(resultOfDevide, tmp));
    }
    int fractionNumber = sizeOfNumber(*result);
    int Scale = fractionNumber - wholeNumber;
    s21_set_scale(result, Scale);
}

// получение уменьшаемого которое >= делителю
int initDecimalMinuend(s21_decimal *minuend, s21_decimal value_1,
                       s21_decimal value_2) {
    int i = get_higher_bit(value_1);
    s21_decimal tmp_minuend = *minuend;
    s21_decimal_init(&tmp_minuend);
    while (s21_is_less(tmp_minuend, value_2)) {
        s21_shift_decimal_left(&tmp_minuend);
        s21_set_bit(&tmp_minuend, 0, s21_get_bit(value_1, i));
        i--;
    }
    *minuend = tmp_minuend;
    return i;
}

int sizeOfNumber(s21_decimal d) {
    s21_decimal resultOfDevide;
    unsigned int minuend;
    int size = 0;

    while (equals_zero(d) != 1) {
        s21_decimal_init(&resultOfDevide);
        int begin = get_higher_bit(d);

        minuend = 0;
        initMinuend(&minuend, d, begin);
        int i = 95;
        int check = 0;

        for (int j = begin - 3; j >= 0 && check == 0; j--, i--) {
            if (minuend >= 10u) {
                s21_set_bit(&resultOfDevide, i, 1);
                minuend -= 10u;
                minuend <<= 1;
                int_set_bit(&minuend, 0, s21_get_bit(d, j - 1));
            } else {
                s21_set_bit(&resultOfDevide, i, 0);
                minuend <<= 1;
                if (j != 0) {
                    int_set_bit(&minuend, 0, s21_get_bit(d, j - 1));
                }
            }
            if (minuend < 10u && j == 0) {
                check = 1;
            }
        }
        s21_decimal_init(&d);
        int tmp = 95;
        for (int j = 94 - i; j >= 0; j--, tmp--) {
            s21_set_bit(&d, j, s21_get_bit(resultOfDevide, tmp));
        }
        size++;
    }
    return size;
}

void initMinuend(unsigned int *minuend, s21_decimal d, int begin) {
    int i = 3;
    for (int j = 0; i >= 0; j++, i--) {
        int_set_bit(minuend, j, s21_get_bit(d, begin - i));
    }
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    s21_decimal div_res;
    if (s21_div(value_1, value_2, &div_res) == 0) {
        print_decimal(div_res);
        s21_truncate(div_res, &div_res);  // 332
        print_decimal(div_res);
        s21_mul(div_res, value_2, &div_res);  // 664
        print_decimal(div_res);
        s21_sub(value_1, div_res, result);
        print_decimal(*result);
    } else {
        res = 1;
    }
    return res;
}
