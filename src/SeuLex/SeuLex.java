package SeuLex;

public class SeuLex {
    // 用特殊的 char 表示正则表达式中的词素，最高位为 1 则是运算符，否则是操作数

    // 操作数转为运算符
    char to_operator(char ch) {
        return (char) (ch|0x80);
    }
    // 运算符转为操作数
    char to_char(char ch) {
        return (char) (ch&0x7f);
    }
    // 判断是否是运算符
    int is_optr(char ch) {
        return ch&0x80;
    }
}
