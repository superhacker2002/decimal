#include "s21_decimal.h"
#include <float.h>
// bits[0] - 

int main() {
    s21_decimal num1 = {{0,0,0,0}};
    s21_decimal num2 = {{0,0,0,0}};
    s21_decimal res;
    s21_from_float_to_decimal(4.1, &num1);
    s21_from_float_to_decimal(7.75, &num2);
    s21_add(num1, num2, &res);
    print_decimal(res);
}

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
    while(n < 96) {
        int bits = s21_get_bit(value_1, n) + s21_get_bit(value_2, n) + tmp;
        if (bits == 1) {
            s21_shift_left(result, 1, n);  // сдвигаем нный байт на противоложный, то бишь на 1
            tmp = 0;
        } else if (bits == 2) {
            tmp = 1;
        } else if (bits == 3) {
            s21_shift_left(result, 1, n); // сдвигаем нный байт на противоложный, то бишь на 1
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
    s21_decimal tmp = {{0,0,0,0}};
    s21_decimal_init(result);
    // скейлы будут складываться, числа умножаться друг на друга

    for (int n = 0; n < 96; n++) {  // умножаем числа
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
        tmp.bits[0] <<= n;
        tmp.bits[1] <<= n;
        tmp.bits[2] <<= n;
        s21_add(*result, tmp, result);
    }

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

    // ставим знак
    if (s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // if val1 < 0 && val2 < 0 -> +res
        s21_set_bit(result, 127, 0);
    } else if (s21_get_bit(value_1, 127) && !s21_get_bit(value_2, 127)) {  // if val1 < 0 && val2 > 0 -> -res
        s21_set_bit(result, 127, 1);
    } else if (!s21_get_bit(value_1, 127) && s21_get_bit(value_2, 127)) {  // if val1 > 0 && val2 < 0 -> -res
        s21_set_bit(result, 127, 1);
    }
    return 0;
}

// Возвращает 1 если на byte-ом байте в числе number стоит 1
int s21_get_bit(s21_decimal number, int byte) {
    int bits = byte / 32;
    byte = byte % 32;
    return (number.bits[bits]&1<<byte) ? 1 : 0;
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
    number->bits[byte]^=shift<<n;
}

void s21_shift_decimal_left (s21_decimal* number) {
    int low_last_bit = s21_get_bit(*number, 31);
    int mid_last_bit = s21_get_bit(*number, 63);

    number->bits[0] <<= 1;
    number->bits[1] <<= 1;
    number->bits[2] <<= 1;

    s21_set_bit(number, 32, low_last_bit);
    s21_set_bit(number, 64, mid_last_bit);
}

void s21_shift_decimal_right (s21_decimal* number) {
    int mid_first_bit = s21_get_bit(*number, 32);
    int high_first_bit = s21_get_bit(*number, 64);

    number->bits[0] >>= 1;
    number->bits[1] >>= 1;
    number->bits[2] >>= 1;

    s21_set_bit(number, 31, high_first_bit);
    s21_set_bit(number, 63, mid_first_bit);
}

//  Сдвигает вправо n-ный байт на shift байтов в числе number
void s21_shift_right(s21_decimal* number, int shift, int n) {
    int byte = n / 32;
    n = n % 32;
    number->bits[byte]^=shift>>n;
}

void int_set_bit(int* number, int byte, int n) {
    if (int_get_bit(*number, byte) != n)
        int_shift_left(number, 1, byte);
}

int int_get_bit(int number, int byte) {
    return (number&1<<byte) ? 1 : 0;
}

void int_shift_left(int* num, int shift, int n) {
    *num^=shift<<n;
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
        s21_decimal_init(dst);  // зануляем число, чтобы избежать ошибок 
        if (src&1<<31) {  // if src < 0, dst.bits[3][31] = 1
            s21_set_bit(dst, 127, 1);
            src *= -1;
        }
        int n = 0;
        while (n < 31) {  // идем от 0 до 30 бита (берем 31 бит), поскольку 32-й бит содержит в себе знак, который нам не нужен
            if (src&1<<n) s21_set_bit(dst, n, 1);
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
    if (scale_1 < scale_2) {
        scale_diff = scale_2 - scale_1; // 2
        s21_set_scale(num1, scale_2);
        for (; scale_diff > 0; scale_diff--) {
            mul_by_10(num1);
        }
    } else if (scale_1 > scale_2) {
        scale_diff = scale_1 - scale_2;
        s21_set_scale(num2, scale_1);
        for (; scale_diff > 0; scale_diff--) {
            mul_by_10(num2);
        }
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
        s21_set_equal_scale(&num1, &num2); // приравнием скейл чисел, что проверять только bits[0-2]
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
        s21_set_equal_scale(&num1, &num2); // приравнием скейл чисел, что проверять только bits[0-2]
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
        s21_set_equal_scale(&num1, &num2); // приравнием скейл чисел, что проверять только bits[0-2]
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

// Вычитает value1 из value2, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int res = 0;
    s21_set_equal_scale(&value_1, &value_2);
    if (s21_is_less(value_1, value_2)) { // если x < y
        if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 1) {  // если -x - (-y) -> -3 - (-1) -> 3 - 1  и "-")
            s21_set_bit(&value_1, 127, 0);
            s21_set_bit(&value_2, 127, 0);
            s21_sub_res(value_1, value_2, result);
            s21_set_bit(result, 127, 1);
        } else if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 0)  { // если -х - (y) -> -3 - 1 -> "-" 3 + 1  complete!
            if (s21_add(value_1, value_2, result) == 1) res = 2;
            s21_set_bit(result, 127, 1);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 0) {  // если х - у complete!
            s21_sub_res(value_2, value_1, result);
            s21_set_bit(result, 127, 1);
        }
    } else if (s21_is_greater(value_1, value_2)) { // если х > у
        if (s21_get_bit(value_1, 127) == 1 && s21_get_bit(value_2, 127) == 1) {  // если -x - (-y) (-1) - (-3)  -> 3 - 1 ) complete!
            s21_set_bit(&value_1, 127, 0);
            s21_set_bit(&value_2, 127, 0);
            s21_sub_res(value_2, value_1, result);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 1) {  // если x - (-y) (19 - (-11) ) complete!
            res = s21_add(value_1, value_2, result);
        } else if (s21_get_bit(value_1, 127) == 0 && s21_get_bit(value_2, 127) == 0) {  // если х - у complete!
            s21_sub_res(value_1, value_2, result);
        }
}
    return res;
}

int s21_sub_res(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    s21_decimal_init(result);
    int i = 0;
    int tmp = 0;
        while (i < 96) {
            int bits = s21_get_bit(value_1, i) - tmp - s21_get_bit(value_2, i); // 1 - 0 - 1
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
// (*(unsigned int*)(&src)&1<<31)
int s21_from_float_to_decimal(float src, s21_decimal *dst) {
    int converting_res = 0;
    if (dst == NULL || !isfinite(src)) {
        converting_res = 1;
    } else {
        s21_decimal_init(dst);
        // Считываем знак
        int float_sign = (*(unsigned int*)(&src)&1<<31) ? 1 : 0;
        //
        if (src < 0) src *= -1;
        float tmp_fl = src;
        // считываем экспоненту
        int exponent = 0;
        for (int i = 30; i >= 23; i--) {
            int bit = (*(unsigned int*)(&tmp_fl)>>i&1);
            exponent = exponent << 1;
            int_set_bit((int *) &exponent, 0, bit);
        }
        exponent -= 127;
        //

        // обработка экспоненты и нормализация числа
        if (exponent > 95) {
            converting_res = 1;
        } else if (exponent > -95) {
            int scale = 0;
            float_normalising(&tmp_fl, &scale);
            double result = 1;
            
            // возводим все биты мантиссы в отрицательную степень двойки
            for (int i = 1, j = 22; j >= 0; i++, j--) {
                if ((*(unsigned int*)(&src)&1<<j))
                    result += pow(2, -i);
            }
            
            // приводим число к 8-9 знакам перед запятой
            result *= pow(2, exponent);
            result *= pow(10, 8 + scale);

            if (scale > 0) {
                while (result < 10000000) {
                    result *= 10;
                }
            }

            long int tmp = round(result);
            int remainder = 0;

            while (tmp >= 10000000) {
                remainder = tmp % 10;
                tmp = round(tmp);
                tmp /= 10;
            }

            while (scale + 7 > 29) {
                remainder = tmp % 10;
                tmp /= 10;
                scale--;
            }

            if (remainder > 4) {
                tmp++;
            }

            while (tmp % 10 == 0) {
                tmp /= 10;
                scale--;
            }

            s21_from_int_to_decimal(tmp, dst);

            while (scale + 7 <= 0) {
                mul_by_10(dst), scale++;
            }

            s21_set_bit(dst, 127, float_sign); // устанавливаем знак
            s21_set_scale(dst, scale + 6); // устанавливаем скейл
        }
    }
    return converting_res;
}

void float_normalising (float* num, int* scale) {
    while ((unsigned long int)*num < 1) {
        *num *= 10;
        *scale += 1;
    }
    while ((unsigned long int)*num > 10) {
        *num /= 10;
        *scale -= 1;
    }
}

void mul_by_10(s21_decimal* num) {
    s21_decimal value2 = {{10,0,0,0}};
    s21_mul(*num, value2, num);
}

void s21_set_scale (s21_decimal* num, int scale) {
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
        if (s21_get_bit(value, 127) == 1) s21_set_bit(result, 127, 0);
        else s21_set_bit(result, 127, 1);
    } else {
        res = 1;
    }
    return res;
}

int equals_zero(s21_decimal value) {
    s21_decimal zero = {{0,0,0,0}};
    return s21_is_equal(value, zero);
}

// 0 - OK
// 1 - число слишком велико или равно бесконечности
// 2 - число слишком мало или равно отрицательной бесконечности
// 3 - деление на 0

int s21_div(s21_decimal value1, s21_decimal value2, s21_decimal* result) {
    int div_res = 0;
    if (equals_zero(value1) && equals_zero(value2)) {
        div_res = 3;
    } else if (equals_zero(value2)) {
        int sign1 = s21_get_bit(value1, 127);
        int sign2 = s21_get_bit(value2, 127);
        div_res = sign1 + sign2 == 0 ? 1 : sign1 + sign2 == 1 ? 2 : 1;
    } else {
        s21_decimal_init(result);
        s21_decimal mask = {{1,0,0,0}};
        int higher_bit = get_higher_bit(value1);
        s21_decimal tmp = {{0,0,0,0}};
        int i = 1;
        int j = higher_bit;
        while (s21_is_less(tmp, value2)) {
            for (int n = 0; n < j; n++) {  // получаем нужную маску
               s21_shift_decimal_left(&mask); 
            }

            tmp.bits[0] = value1.bits[0] & mask.bits[0];  // получаем tmp переменную которую 
            tmp.bits[1] = value1.bits[1] & mask.bits[1];  // нужно сдвинуть до младших битов для
            tmp.bits[2] = value1.bits[2] & mask.bits[2];  // сравнения с value2

            for (int n = 0; n < j; n++) {  // сдвигаем tmp для сравнения
               s21_shift_decimal_right(&tmp); 
            }

            i += 2;  // итерация
            j--;
            s21_from_int_to_decimal(i, &mask);
        }
    }
    return div_res;
}

int get_higher_bit (s21_decimal value) {
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

void s21_div_by_10 (s21_decimal* number) {
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
}
