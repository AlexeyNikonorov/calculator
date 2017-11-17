#include "calculator.h"

int main() {
    string expr;
    while (getline(cin, expr)) {
        if (!expr.empty()) {
            try {
                cout << "result: " << calculate(expr) << '\n';
            } catch (exception& e) {
                cout << "error: " << e.what() << '\n';
            }
        }
    }
}