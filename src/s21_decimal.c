#include "s21_decimal.h"
// bits[0] - 

int main() {
    s21_decimal x = {2147483647,12,0,0}; // 2147483647
    s21_decimal y = {7,0,3,0};
    s21_decimal res;
    s21_add(x, y, &res);
    printf("%d %d %d %d\n", res.bits[0], res.bits[1], res.bits[2], res.bits[3]);
    print_decimal(res);
    // printf("%d %d %d %d", x.bits[0], x.bits[1], x.bits[2], x.bits[3]);
    // int a = 236;
    // int b = 567;
    // int res = 1;
    // // printf("%d\n", res);
    // // res^=1<<0; // сдвигаем нулевой бит на 1
    // // res^=1<<1; // сдвигаем первый бит на 1
    // // res^=1<<2; // сдвигаем второй бит на 1
    // // printf("%d\n", res);
    // // res = ~res;
    // // res^=1<<0;
    // // printf("%d\n", res);
    // if (res&1<<31) printf("число отрицательное"); // если в 32 бите стоит 1
    // else printf("число положительное");

    // if (s21_bit(res,31)) printf("включен");
    // else printf("не включен");
}

// Складывает два числа, результат записывается в result. Возвращает 0 если число ок, 1-3 если число inf/nan
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int n = 0; // итератор для битов чисел
    int tmp = 0; // в уме
    result->bits[0] = 0;
    result->bits[1] = 0;
    result->bits[2] = 0;
    result->bits[3] = 0;
    // while(n < 31) {
    //     if (!s21_bit(value_1.bits[0], n) && !s21_bit(value_2.bits[0],n) && tmp == 0) { // 000
    //         // res[0] не меняем остается нулём
    //     } else if (!s21_bit(value_1.bits[0], n) && !s21_bit(value_2.bits[0],n) && tmp == 1) { // 001
    //         result->bits[0]^=1<<n; // сдвигаем нный байт на противоложный, то бишь на 1
    //         tmp = 0;
    //     } else if (!s21_bit(value_1.bits[0], n) && s21_bit(value_2.bits[0],n) && tmp == 0) { // 010
    //         result->bits[0]^=1<<n; // сдвигаем нный байт на противоложный, то бишь на 1
    //     } else if (!s21_bit(value_1.bits[0], n) && s21_bit(value_2.bits[0],n) && tmp == 1) { // 011
    //         // res[0] не меняем остается нулём так как 1 + 1 = 10
    //         tmp = 1;
    //     } else if (s21_bit(value_1.bits[0], n) && !s21_bit(value_2.bits[0],n) && tmp == 0) { // 100
    //         result->bits[0]^=1<<n; // сдвигаем нный байт на противоложный, то бишь на 1
    //     } else if (s21_bit(value_1.bits[0], n) && !s21_bit(value_2.bits[0],n) && tmp == 1) { // 101
    //         // res[0] не меняем остается нулём так как 1 + 1 = 10
    //         tmp = 1;
    //     } else if (s21_bit(value_1.bits[0], n) && s21_bit(value_2.bits[0],n) && tmp == 0) { // 110
    //         // res[0] не меняем остается нулём так как 1 + 1 = 10
    //         tmp = 1;
    //     } else if (s21_bit(value_1.bits[0], n) && s21_bit(value_2.bits[0],n) && tmp == 1) { // 111
    //         result->bits[0]^=1<<n;
    //         tmp = 1;
    //     }
    //     n++;
    // }
    // int i = 0;
    while(n < 96) {
        // if (s21_equals_intmax(result->bits[i])) i++;

        int bits = s21_bit(value_1, n) + s21_bit(value_2, n) + tmp;
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

// Возвращает 1 если на byte-ом байте в числе number стоит 1
// int s21_bit(int number, int byte) {
//     return (number&1<<byte) ? 1 : 0;
// }

// Возвращает 1 если на byte-ом байте в числе number стоит 1
int s21_bit(s21_decimal number, int byte) {
    int bits = byte / 32;
    byte = byte % 32;
    return (number.bits[bits]&1<<byte) ? 1 : 0;
}

// Сдвигает влево n-ный байт на shift байтов в числе number
void s21_shift_left(s21_decimal* number, int shift, int n) {
    int byte = n / 32;
    n = n % 32;
    number->bits[byte]^=shift<<n;
    // *num^=shift<<n;
}

// Сдвигает вправо n-ный байт на shift байтов в числе number
void s21_shift_right(int* num, int shift, int n) {
    *num^=shift>>n;
}

// Возвращает 1 если число равно INT_MAX и 0 если число не равно INT_MAX 
// int s21_equals_intmax(s21_decimal num) {
//     int flag = 1;
//     int n = 1;
//     if (!s21_bit(num, 0)) {
//         while (n < 31 && flag == 1) {
//         if (!s21_bit(num, n)) flag = 0;
//         n++;
//         }
//     }
//     return flag;
// }

// выводит число в двоичном представлении
void print_decimal(s21_decimal number) {
    for (int i = 95; i >= 0; i--) {
        if (s21_bit(number, i)) printf("1");
        else printf("0");
        if (i == 32 || i == 64) printf(" ");
    }
}
