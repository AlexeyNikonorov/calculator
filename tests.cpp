#include "calculator.h"
#include <cassert>
#include <cstdlib>
#include <cmath>

/*
* Для удобства, операторы и скобки
* записаны в глобальные переменные
*
* A: "+"
* S: "-"
* M: "*"
* D: "/"
* L: "("
* R: ")"
*/
Token A(OPERATOR, ADD, 2), S(OPERATOR, SUB, 2),
      M(OPERATOR, MUL, 3), D(OPERATOR, DIV, 3),
      L(LEFT_BRACKET, 0, 1), R(RIGHT_BRACKET, 0, 1);

ostream& operator<<(ostream& os, const Token& t) {
    switch (t.type) {
        case OPERAND: os << t.value; break;
        case OPERATOR:
            switch (t.op_type) {
                case ADD: os << '+'; break;
                case SUB: os << '-'; break;
                case MUL: os << '*'; break;
                case DIV: os << '/'; break;
            }
            break;
        case LEFT_BRACKET: os << '('; break;
        case RIGHT_BRACKET: os << ')'; break;
    }
    return os;
}

ostream& operator<<(ostream& os,
                    const vector<Token>& tokens) {
    if (!tokens.empty()) {
        os << tokens[0];
        for (size_t i = 1; i < tokens.size(); ++i) {
            os << ' ' << tokens[i];
        }
    }
    return os;
}

bool operator==(const Token& a, const Token& b) {
    return a.type == b.type && 
            a.op_type == b.op_type && 
            a.precedence == b.precedence && 
            a.value == b.value;
}

/*
* Тест для функции read_operand.
* Функция применяется к каждому из входных выражений.
* Результат сравнивается с правильным ответом.
*   exprs - Массив тестовых формул.
*   answers - Массив с правильными ответами.
*/
void read_operand_test(const vector<string>& operands,
                       const vector<double>& answers) {
    for (size_t test = 0; test < operands.size(); ++test) {
        char_ptr beg = operands[test].begin(),
                 end = operands[test].end();

        double res = read_operand(beg, end, beg);

        assert(res == answers[test]);
    }
}

/*
* Тест для функции read_operand.
* Проверяется правильность преобразования строки в число.
* Генерируется случайное число, преобразуется в строку
* и передается в функцию read_operand.
* Возвращаемое значение сравнивается с исходным числом.
*   n_samples - Число испытаний.
*   max_real - Максимальное значение целой части.
*   max_dec - Максимальное значение дробной части.
*/
void read_operand_test_auto(size_t n_samples = 1000,
                            int max_real = 10000000,
                            int max_dec = 100) {
    double acc = 0.1 / max_dec;

    for (size_t i = 0; i < n_samples; ++i) {
        int a = rand() % max_real;
        int b = rand() % max_dec;
        double x = a + (double) b / max_dec;
        stringstream ss;
        ss << fixed << x;
        string s;
        ss >> s;
        char_ptr it = s.begin();
        double res = read_operand(it, s.end(), it);
        assert(abs(x - res) < acc);
        s[s.find('.')] = ',';
        it = s.begin();
        res = read_operand(it, s.end(), it);
        assert(abs(x - res) < acc);
    }
}

/*
* Тест для функции read_operand.
* Проверяется поведение при недопустимых значениях аргументов.
* Например: abc 10.0a a10.
*   operands - Массив с вводными строками.
*   expected_msgs - Массив с ожидаемыми сообщениями об ошибке.
*/
void read_operand_test_invalid_argument(const vector<string>& operands,
                                        const vector<string>& expected_msgs) {
    for (size_t test = 0; test < operands.size(); ++test) {
        char_ptr beg = operands[test].begin(),
                 end = operands[test].end();
        bool exception_thrown = false;

        try {
            read_operand(beg, end, beg);
        } catch (exception& e) {
            exception_thrown = true;
            assert(e.what() == expected_msgs[test]);
        }

        assert(exception_thrown);
    }
}

/*
* Тест для класса Tokenizer.
* Метод Tokenizer::next применяется к каждому из входных выражений.
* Результат сравнивается с правильным ответом.
*   exprs - Массив тестовых формул.
*   answers - Массив с правильными разбиениями формул на токены. (задается вручную)
*/
void tokenizer_test(const vector<string>& exprs,
                    const vector<vector<Token>>& answers) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        Tokenizer t(exprs[test]);
        Token token;
        vector<Token> res;

        while (t.next(token)) {
            res.push_back(token);
        }

        assert(res.size() == answers[test].size());
        for (size_t i = 0; i < res.size(); ++i) {
            assert(answers[test][i] == res[i]);
        }
    }
}

/*
* Тест для класса Tokenizer.
* Проверяется поведение при недопустимых значениях аргументов.
* Например: (1 + 2 * abc) , a1 + a2 , 1 + 10.0.0
*   exprs - Массив тестовых формул.
*   expected_msgs - Массив с ожидаемыми сообщениями об ошибке.
*/
void tokenizer_test_invalid_argument(const vector<string>& exprs,
                                     const vector<string>& expected_msgs) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        Tokenizer t(exprs[test]);
        Token token;
        bool exception_thrown = false;

        try {
            while (t.next(token))
                ;
        } catch (exception& e) {
            exception_thrown = true;
            assert(e.what() == expected_msgs[test]);
        }

        assert(exception_thrown);
    }
}

/*
* Тест для функции to_postfix_notation.
* Функция применяется к каждому из входных выражений.
* Результат сравнивается с правильным ответом.
*   exprs - Массив тестовых формул.
*   answers - Массив с правильными ответами.
*/
void to_postfix_notation_test(const vector<string>& exprs,
                              const vector<string>& answers) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        Tokenizer t(exprs[test]);
        
        stringstream ss;
        ss << to_postfix_notation(t);

        assert(ss.str() == answers[test]);
    }
}

/*
* Тест для функции to_postfix_notation.
* Проверяется поведение при разборе формулы, содержащей несовпадающие скобки.
*   exprs - Массив тестовых формул.
*   expected_msg - ожидаемое сообщение об ошибке.
*/
void to_postfix_notation_test_invalid_argument(const vector<string>& exprs,
                                               const string& expected_msg) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        Tokenizer t(exprs[test]);
        bool exception_thrown = false;
        
        try {
            stringstream ss;
            ss << to_postfix_notation(t);
        } catch (exception& e) {
            exception_thrown = true;
            assert(e.what() == expected_msg);
        }

        assert(exception_thrown);
    }
}

/*
* Тест для функции evaluate_postfix_notation.
* Функция применяется к каждому из входных выражений.
* Результат сравнивается с правильным ответом.
*   exprs - Массив тестовых формул (разбитых на токены).
*   answers - Массив с правильными ответами.
*/
void evaluate_postfix_notation_test(const vector<vector<Token>>& exprs,
                                    const vector<double>& answers) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        double res = evaluate_postfix_notation(exprs[test]);
        assert(res == answers[test]);
    }
}

/*
* Тест для функции evaluate_postfix_notation.
* Проверяется поведение при вычислении формулы
* с недопустимой последовательностью операторов.
* Например: 1 2 + * , (что соответствует 1 * + 2). 
*   exprs - Массив тестовых формул (разбитых на токены).
*   answers - Массив с правильными ответами.
*/
void evaluate_postfix_notation_test_invalid_argument(const vector<vector<Token>>& exprs,
                                                     const vector<string>& expected_msgs) {
    for (size_t test = 0; test < exprs.size(); ++test) {
        bool exception_thrown = false;

        try {
            evaluate_postfix_notation(exprs[test]);
        } catch (exception& e) {
            exception_thrown = true;
            assert(e.what() == expected_msgs[test]);
        }
        assert(exception_thrown);
    }
}

int main() {
    read_operand_test_auto();

    read_operand_test(
        {
            "1",
            "1.12",
            "1,12",
            "1000000.12"
        },
        {
            1,
            1.12,
            1.12,
            1000000.12,
        }
    );

    read_operand_test_invalid_argument(
        {
            "10.1a",
            "10.10.10",
            "abc",
            "abc10",
            "abc+",
        },
        {
            "bad token 10.1a",
            "bad token 10.10.10",
            "bad token abc",
            "bad token abc10",
            "bad token abc",
        }
    );

    tokenizer_test(
        {
            "   1   +  2*3  ", // игнорируются ли пробелы
            "1 + -2", // проверить отрицание
            "-1 + 5 - 3", // пример из описания задания
            "-10 + (8 * 2.5) - (3 / 1,5)",// пример из описания задания
            "1 + (2 * (2.5 + 2.5 + (3 - 2))) - (3 / 1.5)",// пример из описания задания
        },
        {
            {1,A,2,M,3},
            {1,A,-2},
            {-1,A,5,S,3,},
            {-10,A,L,8,M,2.5,R,S,L,3,D,1.5,R,},
            {1,A,L,2,M,L,2.5,A,2.5,A,L,3,S,2,R,R,R,S,L,3,D,1.5,R,},
        }
    );

    tokenizer_test_invalid_argument(
        {
            "1.1 + 2.1 + abc", // пример из описания задания (токен в конце формулы)
            "1 + 3 * a1 + 8", // токен в середине формулы
            "a1 + 4 + 7", // токен в начале формулы
            "1 + (a1*(1 + 2)) * 3", // токен не отделен пробелами
        },
        {
            "bad token abc",
            "bad token a1",
            "bad token a1",
            "bad token a1",
        }
    );
    
    to_postfix_notation_test(
        {
            "", // тривиальный тест
            "1", // тривиальный тест
            "1 + 2",
            "1 + 2 * 3",
            "1 + 2 * (3 + 4)",
            "1 + 2 * (3 + 4) + 5",
            "((15 / (7 - (1 + 1))) * 3) - (2 + (1 + 1))",
        },
        {
            "",
            "1",
            "1 2 +",
            "1 2 3 * +",
            "1 2 3 4 + * +",
            "1 2 3 4 + * + 5 +",
            "15 7 1 1 + - / 3 * 2 1 1 + + -",
        }
    );

    to_postfix_notation_test_invalid_argument(
        {
            "(1 + 2 * 3 * 4", // нет правой скобки
            "1 + 2) * 3 * 4)", // нет левой скобки
        },
        "mismatched parentheses"
    );

    evaluate_postfix_notation_test(
        {
            {1}, // тривиальный тест
            {1,3,D}, // 1 / 3, округляется ли результат
            {1,2,A}, // 1 + 2
            {15,7,1,1,A,S,D,3,M,2,1,1,A,A,S},
        },
        {
            1,
            0.33,
            3,
            5,
        }
    );

    evaluate_postfix_notation_test_invalid_argument(
        {
            {}, // при вводе пустого выражения выбрасывается исключение
            {1,0,D}, // 1 / 0, деление на ноль
            {1,2,A,M}, // 1 + * 2
            {1,M}, // * 1
        },
        {
            "bad input expression",
            "division by zero",
            "bad input expression",
            "bad input expression",
        }
    );
}