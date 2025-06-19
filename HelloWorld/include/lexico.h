#pragma once

// ----------------------------------- 'FORWARD' CLASS DECLARATION
class lexicAnalyzer {
	std::string buffer{};
	std::string input{};
	int index{};
	// ---------------------------------------- CONSTANTS
	static inline constexpr std::pair<std::string_view, int> reservedWordList[32]{
		{"class",	100},
		{"summon",	101},
		{"int",		200},
		{"real",	201},
		{"prec",	202},
		{"flag",	203},
		{"char",	204},
		{"byte",	205},
		{"chain",	206},
		{"vector",	207},
		{"last",	300},
		{"public",	301},
		{"private", 302},
		{"static",	303},
		{"define",	400},
		{"inspect", 401},
		{"select",	402},
		{"jump",	403},
		{"none",	404},
		{"when",	405},
		{"then",	406},
		{"but",		407},
		{"repeat",	408},
		{"loop",	409},
		{"main",	410},
		{"true",	905},
		{"false",	905},
		{"throw",	413},
		{"if",		414},
		{"say",		500},
		{"read",	501},
		{"empty",	601}
	};
	constexpr int charToken(const char& c);
	// ---------------------------------------- BUILDER & COMMON
public:
	lexicAnalyzer(std::string input = "");
	constexpr int reservedToken(std::string_view word);
	int nextToken();			// Gets token (Reader/Scanner)
	void tokenTable();			// Debug, probably, idk
	void printIndex(bool both = true);
	std::string getBuffer();
	int getIndex();
	void setIndex(int newIndex);
	void setBuffer(std::string_view bufferSave);
};