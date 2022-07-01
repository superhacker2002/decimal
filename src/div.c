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
        s21_decimal one = {{1, 0, 0, 0}};
        s21_decimal mask = {{1, 0, 0, 0}};
        int higher_bit = get_higher_bit(value1);
        s21_decimal tmp = {{0, 0, 0, 0}};
        s21_decimal remainder = {{0, 0, 0, 0}};
        int scale = 0;
        int iteration = 0;
        while (scale < 28 && !s21_get_bit(*result, 95) /* && пока нет остатка*/) {
            int i = 1;
            int j = higher_bit;
            while (s21_is_less(tmp, value2) && j > 0) {  // поиск первого подчисла
                for (int n = 0; n < j; n++) {  // получаем нужную маску
                    s21_shift_decimal_left(&mask); // 1 << 4
                }

                tmp.bits[0] = value1.bits[0] & mask.bits[0];  // получаем tmp переменную которую
                tmp.bits[1] = value1.bits[1] & mask.bits[1];  // нужно сдвинуть до младших битов для
                tmp.bits[2] = value1.bits[2] & mask.bits[2];  // сравнения с value2

                for (int n = 0; n < j; n++) {  // сдвигаем tmp для сравнения
                    s21_shift_decimal_right(&tmp);
                }
                i += 2;  // итерация
                j--;  // hb - j - 1
                s21_from_int_to_decimal(i, &mask);
                print_decimal(tmp);
            }
            if (equals_zero(tmp)) {
                scale++;
            } else {
                s21_sub(tmp, value2, &remainder);
                s21_add(*result, one, result);
            }
            while (s21_is_less(remainder, value2)) {
                
            }
        }
    }
    return div_res;
}