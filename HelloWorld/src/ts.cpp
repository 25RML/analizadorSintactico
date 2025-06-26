#include <string_view>
#include <iostream>

#include "ts.h"
#include "linkedList.h"


// ----------------------------------------------------- CLASS: symbolTable
void symbolTable::insert(const std::string& lexeme, int token, int type, double value) {
	entries.append(symbolEntry{ lexeme, token, type, value });
}

bool symbolTable::ifFind(const std::string_view lexeme) {
	for (auto& entry : declarationStack) if (entry == lexeme) return true;
	for (auto& entry : entries) if (entry.lexeme == lexeme) return true;
	return false;
}

void symbolTable::moveFromStack(const int token, const int type) {
	while (declarationStack.getSize() > 0) {
		insert(declarationStack.pop(), token, type);
	}
}

void symbolTable::show() {
	for (const auto& entry : entries) {
		std::cout << "Token: " << entry.token
			<< ", Lexema: " << entry.lexeme
			<< ", Tipo: " << entry.type
			<< ", Valor: " << entry.value << '\n';
	}
}

bool symbolTable::stackAppend(const std::string symbol) {
	if (ifFind(symbol)) return false;	// Already exists
	declarationStack.push(symbol);
	return true;
}