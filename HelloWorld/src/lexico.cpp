#define DEBUG_LIST

#include <string>
#include <string_view>
#include <iostream>

#include "lexico.h"
#include "linkedList.h"

// ==================================================== Namespaces ====================================================

namespace {
	namespace constant {
		constexpr std::string_view whitespace{ " \n\r\t\f\v" };
	}

	std::string trim(std::string& object) {
		std::size_t ind{ object.find_first_not_of(constant::whitespace) };
		if (ind == std::string::npos) return "";
		else object = object.substr(ind);
		ind = object.find_last_not_of(constant::whitespace);
		if (ind == std::string::npos) return "";
		else object = object.substr(0, ind + 1);

		return object;
	}

	bool isWhiteSpace(char c) {
		return std::isspace(static_cast<unsigned char>(c));
	}
}

// ================================================= Class Definitions =================================================
// ---------------------------------------- CHAR -> TOKEN (kinda) 
constexpr int lexicAnalyzer::charToken(const char& c) {
	switch (c)
	{
	case '<': return 700;
	case '>': return 701;
	case '=': return 704;
	case '+': return 707;
	case '-': return 708;
	case '*': return 709;	// OK
	case '/': return 710;
	case '^': return 712;	// OK
	case '%': return 713;	// OK
	case '|': return 716;
	case '&': return 717;
	case '!': return 718;
	case '[': return 800;	// OK
	case ']': return 801;	// OK
	case '(': return 802;	// OK
	case ')': return 803;	// OK
	case '{': return 804;	// OK
	case '}': return 805;	// OK
	case ';': return 806;	// OK
	case ':': return 807;	// OK
	case ',': return 808;	// OK
	case '.': return 809;	// OK
	case '#': return 900;
	case '"': return 901;	
	case '\'': return 902;	
	default: return -1;
	}
}
// ---------------------------------------- BUILDER
lexicAnalyzer::lexicAnalyzer(std::string input) : buffer{ "" }, input{ input + " $" }, index{ 0 } {}
// ---------------------------------------- WORD SEARCH
constexpr int lexicAnalyzer::reservedToken(std::string_view word) {
	for (const auto& [key, token] : reservedWordList) {
		if (word == key) return token;
	}
	return 600;
}
// ---------------------------------------- READER
int lexicAnalyzer::nextToken() {
	buffer = "";
	int state{ 0 };
	bool exit{ true };
	char current{};

	bool flag{ false };
	int saveIndex{0};

	//std::cout << "\n Calling Token -------------------------------";
	while (1) {
		current = input.at(static_cast<std::size_t>(index));
		//std::cout << "\n nextToken():: Evaluating " << current;

		if (current == '$') {
			if (exit) return 0;	// End of stream

			switch (state) {
			case 1: return reservedToken(buffer);		// ID | Reserved Word
			case 2: return 600;
			case 3: return -1;

			case 12:	// Error Return (String)
				buffer = "\"";
				[[fallthrough]];
			case 13:	// Error Return (char)
			case 14:
			{
				index = saveIndex + 1;
				return -1;
			}
			case 15: return 900;						// Comment
			}
		}
		else {
			trim(buffer);

			//std::cout << " | Buffer: " << buffer << " | State: " << state;

			switch (state) {
			case 0:				// Case 0 : Start
			{
				buffer += current;
				if (isalpha(current) || current == '_') {
					state = 1;									// to Identifier
					exit = false;
				}
				else if (isdigit(current)) {
					state = 2;									// to Number (Natural)
					exit = false;
				}
				else if (charToken(current) != -1) {
					switch (charToken(current))
					{
					case 700: state = 3; break;					// Check <
					case 701: state = 5; break;					// Check >
					case 704: state = 6; break;					// Check =
					case 707: state = 7; break;					// Check +
					case 708: state = 8; break;					// Check -
					case 710: state = 9; break;					// Check /
					case 716: state = 10; break;				// Check |
					//case 717: state = 11; break;				// Check &
					case 718: state = 11; break;				// Check !
					case 900: state = 15; break;				// Check Comment
					case 901:									// Check String
						saveIndex = index;
						state = 12; 
						break;				
					case 902:									// Check char
						saveIndex = index;
						state = 13;
						break;				
					default:
						++index;
						return charToken(current);
						break;
					}
					exit = false;
				}
				else if (!isWhiteSpace(current)) return -1;			// Error
				break;
			}
			case 1:			// Case 1 : Identifier
			{
				if (isalnum(current) || current == '_') buffer += current;
				else return reservedToken(buffer);				// Reserved or Identifier
				break;
			}
			case 2:			// Case 2 : Number (Natural)
			{
				if (isdigit(current)) buffer += current;
				else if (current == '.') state = 4;				// Goes to state 4 "to see" if there is another number
				else return 904;
				break;
			}
			case 3:			// Case 3 : Check Operator (<)
			{
				if (current != '=') return 700;					// Operator (<)
				buffer += current; ++index;
				return 702;										// Operator (<=)
			}
			case 4:			// Case 4 : Real Check
			{
				if (isdigit(current)) {
					if (!flag) {
						buffer += '.';
						flag = true;
					}
					buffer += current;
				}
				else {
					if (!flag) {
						--index;
						return 904;
					}
					else return 903;
				}
				break;
			}
			case 5:			// Case 5 : Check Operator (>)
			{
				if (current != '=') return 701;					// Operator (>)
				buffer += current; ++index;
				return 703;										// Operator (>=)
			}
			case 6:			// Case 6 : Check Operator (=)
			{
				if (current != '=') return 704;					// Operator (=)
				buffer += current; ++index;
				return 705;										// Operator (==)
			}
			case 7:			// Case 7 : Check Operator (+)
			{
				if (current != '+') return 707;					// Operator (+)
				buffer += current; ++index;
				return 714;										// Operator (++)
			}
			case 8:			// Case 8 : Check Operator (-)
			{
				if (current != '-') return 708;					// Operator (-)
				buffer += current; ++index;
				return 715;										// Operator (--)
			}
			case 9:			// Case 9 : Check Operator (/)
			{
				if (current != '/') return 710;					// Operator (/)
				buffer += current; ++index;
				return 711;										// Operator (//)
			}
			case 10:		// Case 10 : Check Operator (|)
			{
				if (current != '|') return -1;					// Error
				buffer += current; ++index;
				return 716;										// Operator (||)
			}
			case 11:		// Case 11 : Check Operator (!)
			{
				if (current != '=') return 718;					// Operator (!)
				buffer += current; ++index;
				return 706;										// Operator (!=)
			}
			case 12:		// Case 12 : Check String
			{
				if (current == '\n') {
					buffer = "\"";
					index = saveIndex + 1;
					return -1;
				}
				buffer += current;
				if (current == '"') {
					++index;
					return 901;
				}
				break;
			}
			case 13:		// Case 13 : Check Char (Part 1)
			{
				if (current == '\n' || current == '\'') return -1;	// Error
				buffer += current;
				state = 14;
				break;
			}
			case 14:		// Case 14 : Check Char (Part 2)
			{
				if (current == '\'') {
					buffer += current;
					++index;
					return 902;										// Char
				}
				buffer = "'";
				index = saveIndex + 1;
				return -1;											// Error
			}
			case 15:		// Case 15 : Check Comment
			{
				if (current == '\n') {
					++index;
					return 900;					// Comment
				}
				buffer += current;
				break;
			}
			}
			++index;
		}
	}
}
// ---------------------------------------- TABLE  (debug only)
void lexicAnalyzer::tokenTable() {
	linkedList<std::string> tableAll{};				
	linkedList<std::string> tableId{};

	std::string line{};

	index = 0;
	do {
		int token{ nextToken() };
		std::cout << "\n Token Called: " << token << " | Buffer: " << buffer;
		if (token < 0) line = "\n " + std::to_string(token) + "\tError : " + buffer;
		else if (token == 0) {
			std::cout << "\n\n ======================================\n Program Finished";
			std::cout << "\n - Table of all Symbols: "; tableAll.printAll();
			std::cout << "\n - Table of only Identifiers: "; tableId.printAll();
			return;
		}
		else if (token >= 100 && token <= 413) line = "\n " + std::to_string(token) + "\tReserved Word : " + buffer;
		else if (token >= 700 && token <= 718) line = "\n " + std::to_string(token) + "\tOperator : " + buffer;
		else if (token >= 800 && token <= 809) line = "\n " + std::to_string(token) + "\tDelimitador : " + buffer;
		else if (token == 900) line = "\n " + std::to_string(token) + "\tComentario : " + buffer;
		else if (token >= 901) line = "\n " + std::to_string(token) + "\tLiteral : " + buffer;
		else if (token == 600) {
			line = "\n " + std::to_string(token) + "\tIdentifier : " + buffer;
			tableId.append(line);
		}
		else if (token == 31) line = "\n " + std::to_string(token) + "\tNumber : " + buffer;
		tableAll.append(line);

	} while (1);
}

void lexicAnalyzer::printIndex(bool both) {
	if (both) std::cout << buffer;
	std::cout << input.substr(static_cast<std::size_t>(index)) ;
	return;
}

std::string lexicAnalyzer::getBuffer() { return buffer; }
void lexicAnalyzer::setBuffer(std::string_view bufferSave) { buffer = bufferSave; }

int lexicAnalyzer::getIndex() { return index; }
void lexicAnalyzer::setIndex(int newIndex) {
	if (newIndex < 0) return;
	index = newIndex;
}