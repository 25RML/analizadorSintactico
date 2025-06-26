#pragma once


#include <string>
#include <string_view>

#include "linkedList.h"

struct symbolEntry {
	std::string lexeme{};
	int token{};
	int type{}; // 100: int, 101: float
	double value{};
};


class symbolTable {
	linkedList<std::string> declarationStack{};
	linkedList<symbolEntry> entries{};
	void insert(const std::string& lexeme, int token, int type = 100, double value = 0.0f);
public:
	bool stackAppend(const std::string symbol);
	bool ifFind(const std::string_view lexeme);
	void moveFromStack(const int token, const int type = 100);
	void show();
};