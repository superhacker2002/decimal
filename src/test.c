#include <stdio.h>
#include <math.h>
#include "s21_decimal.h"

void float_print(float num1);
void int_print(int num1);
// void s21_set_bit(int* num, int n, int byte);
// void s21_set_bit(unsigned int *source_number, int bit_position, int bit);
union float_value {
        int int_view;
        float float_view;
};

int main() {
    // float num1 = 7000.25435425;
    // union float_value number;
    // number.int_view = 0;
    // number.float_view = num1;
    // int float_number_sign = (int)(number.int_view & 0b00010000000000000000000000000000);
    // float_number_sign = (number.int_view&1<<31) ? 1 : 0;

    // int exponent = 0;
    // float_print(num1);
    // printf("\n");

    // for (int i = 30; i >= 23; i--) {
    //     int bit = (*(unsigned int*)(&num1)>>i&1);
    //     exponent = exponent << 1;
    //     int_set_bit(&exponent, 0, bit);
    // }

    // // int_print(exponent);
    // printf("%d", exponent);
    printf("%f", -1.0/-0.0);
    // int mask = 1 << 4;
    // int_print(mask);
    // mask = 3 << 3;
    // int_print(mask);
    return 0;
}

void float_print(float num1) {
    for (int i = 31; i >= 0; i--) {
        if (*(unsigned int*)(&num1)&1<<i) printf("1");
        else printf("0");
        if (i == 23 || i == 31) {
            printf(" ");
        }
    }
    printf("\n");
}

void int_print(int num1) {
    for (int i = 31; i >= 0; i--) {
        if (num1&1<<i) printf("1");
        else printf("0");
        if (i == 23 || i == 31) {
            printf(" ");
        }
    }
    printf("\n");
}

// void s21_set_bit(int* num, int n, int byte) {
//     if (*num&n<<byte != n) {
//         *num^=1<<byte;
//     }
// }

// void s21_set_bit(unsigned int *source_number, int bit_position, int bit) {
//     unsigned int mask = 1;
//     mask <<= bit_position;

//     if (bit == 0) {
//         mask = ~mask;
//         *source_number &= mask;
//     }

//     if (bit == 1) {
//         *source_number |= mask;
//     }
// }