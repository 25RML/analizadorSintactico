#include <utility>
#include <string>

#include "linkedList.h"
#include "lexico.h"
#include "lr.h"

// ==================================================== Namespaces ====================================================



// ================================================= Class Definitions =================================================
// ----------------------------------------------------- CLASS: actionTable
actionTable::actionTable(linkedList<column> tableSet) : table{ tableSet } {}
actionTable::actionTable() : table{} {}
action actionTable::getAction(int token, int group) {
	for (int i{ 0 }; i < static_cast<int>(table.getSize()); ++i) {
		if (table[i].token == token) {
			for (int j{ 0 }; j < static_cast<int>(table[i].entries.getSize()); ++j) {
				if (table[i].entries[j].groupStart == group) return table[i].entries[j].action;	// this might be running in O(n^4) :c
			}
			break;
		}
	}
	return error; // error exit
}
// ----------------------------------------------------- CLASS: parserLR0
// ----------------------------------------------- Builders
parserLR0::parserLR0(actionTable tableInput, linkedList<pRules> rulesList, std::string input) :
	TAS{ tableInput }, stackList{ {0} }, analyzer{ input }, productionRules{rulesList} {}
// ----------------------------------------------- Analyze Inputs
void parserLR0::analyzeInput() {
	// Panic Mode (Temporal)
	linkedList<panicToken> saveState_tokens{};
	linkedList<linkedList<std::string>> saveState_display{};
	bool panicRetrieve{ true };
	// Usual Operations
	int currentToken{};
	action intersection{};
	bool takeNext{true};

	linkedList<std::string> printStack{ "0"};

	do {
		// ================================== Panic Mode
		int savedIndex{ analyzer.getIndex() };
		// ================================== Panic Mode
		if (takeNext) currentToken = analyzer.nextToken();
		if (currentToken == 900) continue;	// Skip comments

		intersection = TAS.getAction(currentToken, stackList[0]);
		// ================================== Panic Mode
		if (static_cast<int>(intersection) >= 1000)
		{
			if (panicRetrieve) {
				switch (intersection)
				{
				case S155_R3:	intersection = S155;  break;
				case S155_R61:	intersection = S155;  break;
				case S19_R73:	intersection = S19;	  break;
				default:		intersection = error; break;
				}
			}
			else {
				switch (intersection)
				{
				case S155_R3:	intersection = R3;  break;
				case S155_R61:	intersection = R61;  break;
				case S19_R73:	intersection = R73;	  break;
				default:		intersection = error; break;
				}
			}
			// Toggle Save State
			saveState_tokens.push({panicRetrieve,savedIndex,analyzer.getBuffer(),stackList});
			saveState_display.push(printStack);
			std::cout << "\n ===========================================================================";
			std::cout << "\n             GUARDADO DE ESTADO: Estado critico detectado";
			std::cout << "\n ===========================================================================";
		};
		// ================================== Panic Mode

		std::cout << "\n\n - Pila   : "; printStack.debug();			// Temp
		std::cout << "\n - Cadena :  "; analyzer.printIndex();
		// Debug
		//std::cout << "\n - Interseccion: " << static_cast<int>(intersection);
		std::cout << "\n - Token: " << currentToken;

		if (intersection == error) {
			// ================================== Panic Mode
			bool errorSet{ false };
			if (saveState_tokens.getSize() != 0) errorSet = saveState_tokens[0].review;
			if (!errorSet) {
				std::cout << "\n Error, token invalido para la secuencia ...";
				return;
			}
			analyzer.setIndex(saveState_tokens[0].savedIndex);	stackList.clear(); printStack.clear();
			analyzer.setBuffer(saveState_tokens[0].savedBuffer);
			stackList = saveState_tokens[0].saveState;
			printStack = saveState_display[0];
			saveState_tokens.pop();
			saveState_display.pop();
			panicRetrieve = false;
			takeNext = true;
			std::cout << "\n ===========================================================================";
			std::cout << "\n             MODO DE PANICO: Volviendo a un estado critico";
			std::cout << "\n ===========================================================================";
			continue;
			// ================================== Panic Mode
		}
		panicRetrieve = true;
		if (static_cast<int>(intersection) > 0) {				// Move
			stackList.push(currentToken);

			if (currentToken < 30) printStack.push(printToken(currentToken));	// printStack
			else printStack.push(analyzer.getBuffer());							//

			stackList.push(static_cast<int>(intersection));
			printStack.push(std::to_string(stackList[0]));						// printStack
			takeNext = true;
			

			std::cout << "     |     Desplazar " << static_cast<int>(intersection);
		}
		else if (static_cast<int>(intersection) < 0) {
			std::cout << intersection << "  ";
			std::pair<int, int> valueT{ numberOfRules(intersection) };
			stackList.pop(2 * valueT.first);
			printStack.pop(2 * valueT.first);
			int top{ stackList[0] };

			stackList.push(valueT.second);
			printStack.push(printNT(valueT.second));							// printStack

			stackList.push(static_cast<int>(TAS.getAction(stackList[0], top)));
			printStack.push(std::to_string(stackList[0]));						// printStack
			takeNext = false;

			std::cout << "     |     Reduce " << static_cast<int>(intersection) ; //printRule(intersection);
		}
		else if (intersection == ACCEPT) {
			std::cout << " - ACCEPT";
			return;
		}


	} while (1);
}
// ----------------------------------------------- Get Number of Rules
std::pair<int,int> parserLR0::numberOfRules(int index) {
	std::pair<int, int> returnValue{};
	for (int i{ 0 }; i < static_cast<int>(productionRules.getSize()); ++i) {
		if (productionRules[i].id == index) {
			return { productionRules[i].n_rightSide, productionRules[i].letter};
		}
	}
	return {-99,-99};
}
// ----------------------------------------------- Panic Mode
void parserLR0::panicMode() {
	//std::cout << "\n\n - Panic Mode!";
	//while (stackList.getSize() > 1) {
	//	stackList.pop();
	//}
	//stackList[0] = 0;
	//analyzer.resetBuffer();
	//std::cout << "\n - Pila: "; stackList.debug();
	//std::cout << "\n - Cadena: "; analyzer.printIndex();
	//std::cout << "\n - Fin del analisis.";
} // panicMode end



// Normal
std::string printToken(int token) {
	switch (token) {
	case 100: return "class";
	case 101: return "summon";
	case 200: return "int";
	case 201: return "real";
	case 202: return "prec";
	case 203: return "flag";
	case 204: return "char";
	case 205: return "byte";
	case 206: return "chain";
	case 207: return "vector";
	case 300: return "last";
	case 301: return "public";
	case 302: return "private";
	case 303: return "static";
	case 400: return "define";
	case 401: return "inspect";
	case 402: return "select";
	case 403: return "jump";
	case 404: return "none";
	case 405: return "when";
	case 406: return "then";
	case 407: return "but";
	case 408: return "repeat";
	case 409: return "loop";
	case 410: return "main";
	case 411: return "true";
	case 412: return "false";
	case 413: return "throw";
	case 500: return "say";
	case 501: return "read";
	case 601: return "empty";
	default: return "";
	}
}
void printRule(int id) {
	switch (id) {
	case -1: std::cout << "A -> switch (id) {C} B"; break;
	case -2: std::cout << "B -> A"; break;
	case -3: std::cout << "B -> "; break;
	case -4: std::cout << "C -> case num D : B break ; C"; break;
	case -5: std::cout << "C -> "; break;
	case -6: std::cout << "D -> .. num"; break;
	case -7: std::cout << "D -> "; break;
			
	}
}
std::string printNT(int idNT) {
	switch (idNT) {
	case -1: return "A";
	case -2: return "B";
	case -3: return "C";
	case -4: return "D";
	case -5: return "E";
	case -6: return "F";
	case -7: return "G";
	case -8: return "H";
	case -9: return "I";
	case -10: return "J";
	case -11: return "K";
	case -12: return "L";
	case -13: return "M";
	case -14: return "N";
	case -15: return "O";
	case -16: return "P";
	case -17: return "Q";
	case -18: return "R";
	case -19: return "S";
	case -20: return "T";
	case -21: return "U";
	case -22: return "V";
	case -23: return "W";
	case -24: return "X";
	case -25: return "Y";
	case -26: return "Z";
	case -27: return "AA";
	case -28: return "AB";
	case -29: return "AC";
	case -30: return "AD";
	case -31: return "AE";
	case -32: return "AF";
	case -33: return "AG";
	case -34: return "AH";
	case -35: return "AI";
	case -36: return "AJ";
	case -37: return "AK";
	case -38: return "AL";
	case -39: return "AM";
	case -40: return "AN";
	case -41: return "AO";
	case -42: return "AP";
	case -43: return "AQ";
	case -44: return "AR";
	case -45: return "AS";
	case -46: return "AT";
	case -47: return "AU";
	case -48: return "AV";
	case -49: return "AW";
	case -50: return "AX";
	case -51: return "AY";
	case -52: return "AZ";
	case -53: return "BA";
	case -54: return "BB";
	case -55: return "BC";
	case -56: return "BD";
	case -57: return "BE";
	case -58: return "BF";
	case -59: return "BG";
	case -60: return "BH";
	case -61: return "BI";
	default: return "";
	}
}