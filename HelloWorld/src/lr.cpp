#define DEBUG_LIST

#include <utility>
#include <string>
#include <fstream>
#include <sstream>

#include "linkedList.h"
#include "lexico.h"
#include "lr.h"

// ==================================================== Namespaces ====================================================



// ================================================= Class Definitions =================================================
// ----------------------------------------------------- CLASS: actionTable
actionTable::actionTable(linkedList<column> tableSet) : table{ tableSet } {}
actionTable::actionTable() : table{} {}
action actionTable::getAction(int token, int group) {

	for (auto& col : table) {
		if (col.token == token) {
			for (auto& entry : col.entries) {
				if (entry.groupStart == group) return entry.action;	// running in n^2 :D
			}
			break;
		}
	}
	return error; // error exit
}
// ----------------------------------------------------- CLASS: parserLR0
// ----------------------------------------------- Builders
parserLR0::parserLR0(actionTable tableInput, linkedList<pRules> rulesList, std::string input) :
	TAS{ tableInput }, stackList{ {0} }, analyzer{ input }, productionRules{ rulesList } {}
parserLR0::parserLR0(std::string input) : TAS{ importTAS() }, stackList{ {0} }, analyzer{input},productionRules{ importRULES() } {}
// ----------------------------------------------- Analyze Inputs
void parserLR0::analyzeInput() {
	importMap();
	importIDMap();
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
			//std::cout << intersection << "  ";
			std::pair<int, int> valueT{ numberOfRules(intersection) };
			stackList.pop(2 * valueT.first);
			printStack.pop(2 * valueT.first);
			int top{ stackList[0] };

			stackList.push(valueT.second);
			printStack.push(printNT(valueT.second));							// printStack

			stackList.push(static_cast<int>(TAS.getAction(stackList[0], top)));
			printStack.push(std::to_string(stackList[0]));						// printStack
			takeNext = false;

			std::cout << "     |     Reduce "; printRule(intersection); // << static_cast<int>(intersection) 
		}
		else if (intersection == ACCEPT) {
			std::cout << " - ACCEPT";
			return;
		}


	} while (1);
}
// ----------------------------------------------- Get Number of Rules
std::pair<int,int> parserLR0::numberOfRules(int index) {
	for (auto& rule : productionRules) {
		if (rule.id == index) {
			return { rule.n_rightSide, rule.letter };
		}
	}
	return {-999,-999};
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
void parserLR0::printRule(int id) {
	std::cout << rulesMap[-id];
}
std::string parserLR0::printNT(int idNT) {
	return idMap[idNT];
}
// ====================================================== IMPORT FUNCTIONS
actionTable importTAS() {
	linkedList<column> TAS_import;

	std::ifstream importF("data/import/tas.txt");
	if (!importF) {
		std::cerr << "Can't open 'data/import/tas.txt'";
		return TAS_import; 
	}

	std::cout << "\nImportando tas.txt...";
	[[maybe_unused]] int count{ 0 };

	std::string line{};
	while (std::getline(importF, line)) {
		std::istringstream iss(line);

		column columnApp{};
		int value{};
		int aux{};

		iss >> value;
		columnApp.token = value;
		while (iss >> value) {
			iss >> aux;
			columnApp.entries.append({ value,static_cast<action>(aux) });
		}

		//std::cout << "\nCargando entrada n" << ++count;
		TAS_import.append(columnApp);
	}
	std::cout << "\n[OK] Importacion realizada con exito";

	return actionTable{ TAS_import };
}
linkedList<pRules> importRULES() {
	linkedList<pRules> rulesImport;

	std::ifstream importF("data/import/rules.txt");
	if (!importF) {
		std::cerr << "Can't open 'data/import/rules.txt'";
		return rulesImport;
	}

	std::cout << "\nImportando rules.txt...";
	[[maybe_unused]]  int count{ 0 };

	std::string line{};
	while (std::getline(importF, line)) {
		std::istringstream iss(line);

		pRules ruleApp{};

		iss >> ruleApp.id;
		iss >> ruleApp.letter;
		iss >> ruleApp.n_rightSide;

		//std::cout << "\nCargando Regla " << ++count << " -> " << ruleApp.id << ' ' << ruleApp.letter << ' ' << ruleApp.n_rightSide << ' ';
		rulesImport.append(ruleApp);
	}

	std::cout << "\n[OK] Importacion realizada con exito";
	return rulesImport;
}

void parserLR0::importMap() {

	std::ifstream file("data/import/rules_map.txt");
	std::string line;

	while (getline(file, line)) {
		std::istringstream iss(line);
		int id;
		iss >> id;
		std::string restOfLine;
		getline(iss, restOfLine); 
		size_t firstChar = restOfLine.find_first_not_of(" \t");
		if (firstChar != std::string::npos) restOfLine = restOfLine.substr(firstChar);

		rulesMap[id] = restOfLine;
	}

	file.close();
}

void parserLR0::importIDMap() {

	std::ifstream file("data/import/id_map.txt");
	std::string line;

	while (getline(file, line)) {
		std::istringstream iss(line);
		int id;
		iss >> id;
		std::string restOfLine;
		getline(iss, restOfLine);
		size_t firstChar = restOfLine.find_first_not_of(" \t");
		if (firstChar != std::string::npos) restOfLine = restOfLine.substr(firstChar);

		idMap[id] = restOfLine;
	}

	file.close();
}