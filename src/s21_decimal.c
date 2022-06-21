#include "s21_decimal.h"
// bits[0] - 

int main() {
    s21_decimal num1 = {{0,0,0,0}};
    s21_decimal num2 = {{0,0,0,0}};
    s21_from_int_to_decimal(-2, &num1);
    s21_from_int_to_decimal(-3, &num2);
    s21_decimal res;
    s21_mul(num1, num2, &res);
    print_decimal(num1);
    printf("\n");
    print_decimal(res);
    // s21_from_int_to_decimal(101, &num1);
    // s21_from_int_to_decimal(100, &num2);
    
    // printf(s21_is_less_or_equal(num1, num2) ? "menshe ili ravni" : "net");
}

// Складывает два числа, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int n = 0; // итератор для битов чисел
    int tmp = 0; // в уме
    s21_decimal_init(result);
    while(n < 96) {
        // if (s21_equals_intmax(result->bits[i])) i++;
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
    return 0;
}

// Умножает два числа, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    s21_decimal tmp = {{0,0,0,0}};
    s21_decimal_init(result);
    // скейлы будут складываться, числа умножаться друг на друга
    for (int n = 16, tmp = 0; n <= 23; n++) {  // суммируем скейлы
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

    for (int n = 0; n < 96; n++) {  // умножаем числа
        s21_decimal_init(&tmp);
        if (s21_get_bit(value_2, n)) {
            tmp.bits[0] = value_1.bits[0] & s21_int_max;
            tmp.bits[1] = value_1.bits[1] & s21_int_max;
            tmp.bits[2] = value_1.bits[2] & s21_int_max;
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

//  Сдвигает вправо n-ный байт на shift байтов в числе number
void s21_shift_right(s21_decimal* number, int shift, int n) {
    int byte = n / 32;
    n = n % 32;
    number->bits[byte]^=shift>>n;
}

//  выводит число в двоичном представлении
void print_decimal(s21_decimal number) {
    for (int i = 127; i >= 0; i--) {
        printf("%d", s21_get_bit(number, i));
        if (i == 32 || i == 64 || i == 96) printf(" ");
    }
}

// void get_decimal(s21_decimal* num, char* nums) {
//     for (int i = 0, j = strlen(nums); i <= strlen(nums); i++, j--) {
//         if (nums[i] == '0') {
//             s21_set_bit(num, j, 0);
//         } else if (nums[i] == '1') {
//             s21_set_bit(num, j, 1);
//         }
//     }
// }

//  конвертация из int в s21_decimal. Возвращает 0 при успехе и 1 при ошибке
int s21_from_int_to_decimal(int src, s21_decimal *dst) {
    int res = 0;
    if (dst != NULL) {
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

//  конвертация из float в s21_decimal. Возвращает 0 при успехе и 1 при ошибке
// int s21_from_float_to_decimal(float src, s21_decimal *dst) {
//     int res = 0;
//     // if (dst) {
//     //     int scale = 0;  // будущий показатель степени в decimal
//     //     /*
//     //         пока src не станет целым числом или scale не привысит 28
//     //         умножаем src на 10 и scale++. Для того чтобы не записывать лишний мусор числа 
//     //         с плавающей точкой - ограничиваем scale до 28
//     //     */
//     //     if (src < 0) {
//     //         src*= -1;
//     //         s21_set_bit(dst, 127, 1);
//     //     }
//     //     for (; src != (int)src && scale <= 28; src *=10, scale++) {;} 
//     //     int n = 0;
//     //     while (n < 64) {
//     //         if ((int)src&1<<n) 
//     //     }
//     // } else {
//     //     res = 1;
//     // }
//     return res;
// }

void s21_decimal_init(s21_decimal* num) {
    num->bits[0] = 0;
    num->bits[1] = 0;
    num->bits[2] = 0;
    num->bits[3] = 0;
}

void s21_set_equal_scale(s21_decimal* num1, s21_decimal* num2) {
    int n = 16;
    int scale_equal_flag = 1;  // equals
    while (n <= 23 || scale_equal_flag == 1) {  // проверяем равны ли scale у двух чисел
        if (s21_get_bit(*num1, n) != s21_get_bit(*num2, n)) scale_equal_flag = 0;  // not equals
        n++;
    }
    if (!scale_equal_flag) {
        
    }
}

// проверяет что число num1 меньше чем число num2. Возвращает 1 - TRUE, 0 - FALSE.
int s21_is_less(s21_decimal num1, s21_decimal num2) {
    //s21_set_equal_scale(&num1, &num2); // приравнием скейл чисел, что проверять только bits[0-2]
    int result = 0;
    if (s21_get_bit(num1, 127) && !s21_get_bit(num2, 127)) {  // если num1 отрицательное, а num2 положительное
        result = 1;
    } else if (!s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {
        result = 0;
    } else {
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
    //s21_set_equal_scale(&num1, &num2); // приравнием скейл чисел, что проверять только bits[0-2]
    int result = 0;
    if (s21_get_bit(num1, 127) && !s21_get_bit(num2, 127)) {  // если num1 отрицательное, а num2 положительное
        result = 0;
    } else if (!s21_get_bit(num1, 127) && s21_get_bit(num2, 127)) {
        result = 1;
    } else {
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
    // s21_set_equal_scale(&num1, &num2);  // приравнять scale 
    int result = 1;
    if (s21_get_bit(num1, 127) != s21_get_bit(num2, 127)) {
        result = 0;
    } else {
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

