#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <cmath>

using namespace std;
using char_ptr = string::const_iterator;

enum {
	OPERAND = 0, OPERATOR = 1,
	LEFT_BRACKET = 2, RIGHT_BRACKET = 3,
	ADD = 0, MUL = 1, SUB = 2, DIV = 3,
};

/*
* В качестве алгоритма разбора формул использован алгоритм сортировочной
* станции (shunting yard algorithm). При его реализации я опирался на статьи:
* 	https://en.wikipedia.org/wiki/Shunting_yard_algorithm
* 	https://en.wikipedia.org/wiki/Reverse_Polish_notation
*/

/*
* Класс Token представляет токен в формуле.
* 	type - Тип токена (OPERAND, OPERATOR, LEFT/RIGHT_BRACKET)
*	op_type - Тип оператора (ADD, SUB, MUL, DIV).
*				Используется только для операторов.
*	precedence - Приоритет операции (1, 2, 3).
*				Используется только для операторов и скобок.
*	value - Численное значение токена.
*			Используется только для операндов.
*/
struct Token {
	int type;
	int op_type;
	int precedence;
	double value;

	Token() {}
	Token(double value)
		: type(OPERAND), op_type(0), precedence(0), value(value) {}
	Token(int type, int op_type, int precedence)
		: type(type), op_type(op_type), precedence(precedence), value(0) {}
};

/*
* Вспомогательные функции.
*/
double evaluate_operation(const Token& op,
						  const Token& a,
						  const Token& b) {
	switch (op.op_type) {
		case ADD:
			return a.value + b.value;
		case SUB:
			return a.value - b.value;
		case MUL:
			return a.value * b.value;
		case DIV:
			if (b.value == 0.0)
				throw invalid_argument("division by zero");
			return a.value / b.value;
		default:
			throw domain_error("unknown operation");
	}
}

bool is_operand(char c) {
	switch (c) {
		case ' ': case '+': case '-':
		case '*': case '/': case '(': case ')':
			return false;
	}
	return true;
}

/*
* Функция read_operand преобразует вводную строку в число с
* плавающей точкой или выбрасывает исключение, если этого не удается.
* 	start - Итератор, указывающий на начало строки.
*	end - Итератор, указывающий на конец строки.
*	it - После завершения выполнения функции, этот итератор
*			будет указывать на первый символ после считанного числа.
*/
double read_operand(char_ptr start,
					char_ptr end,
					char_ptr& it) {
	stringstream ss;
	for (it = start; it != end && is_operand(*it); ++it) {
		ss << (*it == ',' ? '.' : *it);
	}
	double res; ss >> res;
	if (ss.fail() || !ss.eof()) {
		throw invalid_argument("bad token " + string(start, it));
	}
	return res;
}

/*
* Класс Tokenizer используется для разбиения формулы на токены.
*	cur - Текущая позиция в строке.
*	end - Конец строки.
*	allow_negation - Если следующий токен - оператор "-",
						интерпретировать его, как оператор отрицания.
*/
struct Tokenizer {
	char_ptr cur, end;
	bool allow_negation;

	Tokenizer(const string& expr)
		: cur(expr.begin()), end(expr.end()),
		  allow_negation(true) {}

	bool next(Token& token) {
		while (cur != end && *cur == ' ') {
			++cur;
		}
		if (cur == end) {
			return false;
		}
		switch (*(cur++)) {
			case '+':
				token = Token(OPERATOR, ADD, 2);
				break;
			case '-':
				if (allow_negation)
					token = -read_operand(cur, end, cur);
				else
					token = Token(OPERATOR, SUB, 2);
				break;
			case '*':
				token = Token(OPERATOR, MUL, 3);
				break;
			case '/':
				token = Token(OPERATOR, DIV, 3);
				break;
			case '(':
				token = Token(LEFT_BRACKET, 0, 1);
				break;
			case ')':
				token = Token(RIGHT_BRACKET, 0, 1);
				break;
			default:
				token = read_operand(cur - 1, end, cur);
		}
		allow_negation = token.type != OPERAND && 
						 token.type != RIGHT_BRACKET;
		return true;
	}
};

/*
* Функция to_postfix_notation преобразует формулу
* в обратную поольскую нотацию
*/
vector<Token> to_postfix_notation(Tokenizer& tokenizer) {
	vector<Token> postfix;
	vector<Token> op_stack;
	Token token;

	while (tokenizer.next(token)) {
		switch (token.type) {
			case OPERAND:
				postfix.push_back(token);
				break;
			case OPERATOR:
				while (!op_stack.empty()) {
					Token op = op_stack.back();
					if (token.precedence <= op.precedence) {
						postfix.push_back(op);
						op_stack.pop_back();
					} else break;
				}
				op_stack.push_back(token);
				break;
			case LEFT_BRACKET:
				op_stack.push_back(token);
				break;
			case RIGHT_BRACKET:
				while (!op_stack.empty()) {
					Token op = op_stack.back();
					if (op.type != LEFT_BRACKET) {
						postfix.push_back(op);
						op_stack.pop_back();
					} else break;
				}
				if (op_stack.empty()) {
					throw invalid_argument("mismatched parentheses");
				}
				op_stack.pop_back();
				break;
		}
	}

	for (auto op = op_stack.rbegin(); op != op_stack.rend(); ++op) {
		if (op->type == LEFT_BRACKET) {
			throw invalid_argument("mismatched parentheses");
		}
		postfix.push_back(*op);
	}
	return postfix;
}

/*
* Функция evaluate_postfix_notation вычисляет выражение,
* представленное в обратной польской нотации.
*/
double evaluate_postfix_notation(const vector<Token>& postfix) {
	vector<Token> stack;
	for (const Token& token: postfix) {
		if (token.type == OPERATOR) {
			if (stack.empty()) {
				throw invalid_argument("bad input expression");
			}
			Token op2 = stack.back();
			stack.pop_back();
			if (stack.empty()) {
				throw invalid_argument("bad input expression");
			}
			Token op1 = stack.back();
			stack.pop_back();
			Token result = evaluate_operation(token, op1, op2);
			stack.push_back(result);
		} else if (token.type == OPERAND) {
			stack.push_back(token);
		}
	}
	if (stack.size() != 1) {
		throw invalid_argument("bad input expression");
	}
	return round(stack.back().value * 100) / 100.0;
}

double calculate(const string& expr) {
	Tokenizer tokenizer(expr);
	vector<Token> postfix = to_postfix_notation(tokenizer);
	return evaluate_postfix_notation(postfix);
}