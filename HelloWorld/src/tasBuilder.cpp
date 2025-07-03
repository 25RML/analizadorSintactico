#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <unordered_map>
#include <conio.h>

#include "lexico.h"
#include "tasBuilder.h"
#include "linkedList.h"
#include "lr.h"

// =================================================================================================================================
//														   Using
// =================================================================================================================================
using d_subEntry = architecture::groupEntry*;
using d_subGroup = linkedList<d_subEntry>;
// =================================================================================================================================
//														Constructor
// =================================================================================================================================
TAS_Builder::TAS_Builder(linkedList<architecture::rule> input) : ruleSet{input} {}
TAS_Builder::TAS_Builder() : ruleSet{} {}
// =================================================================================================================================
//														TAS BUILDER
// =================================================================================================================================
void TAS_Builder::tasBuilder() {
	ruleSet.clear();
	generateRules();
	// ================================ Firsts List Generation ================================
	std::cout << "\n =========================== Creating Firsts:";
	generateFirstList();
	std::cout << "\n[OK] Finished creation of firsts ";
	//printFirstList();
	// ================================ Groups Generation ================================
	architecture::groupEntry startRule{ initRule(input::g_startRule) };

	d_subGroup newGroup{};
	newGroup.append(generateGroupFrom({ startRule }));

	groupList.clear();
	groupList.append(newGroup);
	groupPending.clear();
	groupPending.append(&groupList.back());

	std::cout << "\n =========================== Group Creation Process:";
	while (groupPending.getSize() > 0) {
		std::cout << "\n ====== Analizing group " << groupAnalizing;
		linkedList<architecture::groupEntry*>* savePop{ groupPending.pop() };
		linkedList<d_subGroup> saveSimilars{ getSimilars(*savePop) };
		std::cout << "\nFound " << saveSimilars.getSize() << " similar groups to move";
		for (auto& element : saveSimilars) moveAndTransition(element);
		std::cout << "\n[OK] Finished analisys of group " << groupAnalizing;
		++groupAnalizing;
	}
	// All groups assembled, now procceed with the lalr
	//printGroups();
	std::cout << "\n Groups created with no issues, clearing...";
	groupList.clear();
	std::cout << "\n[OK] Groups cleared with no issues, proceeding...";
	// ================================ LALR Generation ================================
	//printTransitions();
	std::cout << "\n =========================== LALR Creation Process:";
	linkedList<architecture::groupTransition> lalrTransition{};
	bool modified{ false };
	int saveDestiny{ 0 };
	while (transitionRef.getSize() > 0) {
		architecture::groupTransition transitionSave{ transitionRef.pop() };
		for (auto& toEval : lalrTransition) {
			if (isIncludedOnTransition(transitionSave, toEval)) {
				saveDestiny = combineTransitions(transitionSave, toEval);
				modified = true;
				break;
			}
		}
		if (!modified) lalrTransition.append(transitionSave);
		else {
			updateGroupOrigin(transitionRef, transitionSave.groupDestiny, saveDestiny);
			updateGroupOrigin(lalrTransition, transitionSave.groupDestiny, saveDestiny);
			updateGroupOrigin(entryVoidList, transitionSave.groupDestiny, saveDestiny);
			modified = false;
		}
	}
	transitionRef = lalrTransition;
	std::cout << "\n[OK] LALR created succesfully, number of transition rules generated -> " << transitionRef.getSize();
	// ================================ TAS Generation ================================
	std::cout << "\n =========================== TAS Creation Process:";
	//printTransitions();
	TAS_Return.clear();
	int evaluatingToken{};
	column* columnToModify{};
	for (auto& transition : transitionRef) {
		// Shifts
		evaluatingToken = transition.subEntries[0]->ruleRef->rightSide[transition.subEntries[0]->pointer];		// Get token
		columnToModify = findToken(evaluatingToken);
		if (!columnToModify) columnToModify = &TAS_Return.append({ evaluatingToken,{} });
		for (auto& groupO : transition.groupOrigin) {
			if (detectAmbiguity(columnToModify, groupO)) std::cout << "\n [ADVERTENCIA] :: Entrada ambigua creada en el token " << columnToModify->token << " con estado " << groupO;
			columnToModify->entries.append({ groupO,static_cast<action>(transition.groupDestiny) });
		}
		// Reduce
		for (auto& rule : transition.subEntries) {
			if (!(rule->ruleRef->rightSide.getSize() <= rule->pointer + 1)) continue; // Unfit Rule to reduce
			
			columnToModify = findToken(rule->anticipationToken);
			if (!columnToModify) columnToModify = &TAS_Return.append({ rule->anticipationToken, {} });
			if (detectAmbiguity(columnToModify, transition.groupDestiny)) std::cout << "\n [ADVERTENCIA] :: Entrada ambigua creada en el token " << columnToModify->token << " con estado " << transition.groupDestiny;
			columnToModify->entries.append({ transition.groupDestiny,static_cast<action>(-rule->ruleRef->rule_ID) });
		}
	}
	// Special : Void groups (Ej: A -> )
	for (auto& voidE : entryVoidList) {
		columnToModify = findToken(voidE.entryRef->anticipationToken);
		if (!columnToModify) columnToModify = &TAS_Return.append({ evaluatingToken,{} });

		
		for (auto& origin : voidE.groupOrigin) {
			std::cout << "\nCreated void entry -> " << origin << " : R" << voidE.entryRef->ruleRef->rule_ID;
			if (detectAmbiguity(columnToModify, origin)) std::cout << "\n [ADVERTENCIA] :: Entrada ambigua creada en el token " << columnToModify->token << " con estado " << origin;
			columnToModify->entries.append({ origin,static_cast<action>(-voidE.entryRef->ruleRef->rule_ID) });
		}
	}

	std::cout << "\n[OK] TAS creada exitosamente, numero de columnas generadas -> " << TAS_Return.getSize();
	std::cout << "\n Pulse cualquier tecla para continuar... "; _getch();
	exportTAS();
	exportFinal();
}
// =================================================================================================================================
//													Function Definitions
// =================================================================================================================================

// Crea una regla de tipo groupEntry a partir de una sola regla comun.
// Esta funcion ya casi no se usa en el cosigo								
architecture::groupEntry TAS_Builder::initRule(const architecture::rule& target) {
	return architecture::groupEntry{ &target };
}

linkedList<architecture::groupEntry> TAS_Builder::initRule(const architecture::groupEntry& target) {
	// Variables
	bool cautionFlag{ target.pointer == 0 && (target.ruleRef->rightSide[0] == target.ruleRef->leftSide_ID) };
	linkedList<architecture::groupEntry> groupStack{};
	// Get the firsts from the Leftside ID (Letter)
	int target_ID{ target.ruleRef->rightSide[target.pointer] };
	linkedList<int> firstL{ getFirstFromRight(target) };

	// Iterate through every element that matches the leftSide_ID from target.pointer
	for (auto& rule : ruleSet) {
		// Check for matches
		if (rule.leftSide_ID != target_ID) continue;
		if (cautionFlag) if (rule.leftSide_ID == rule.rightSide[0]) continue;
		// IF match add for every first in firstL
		if (firstL.getSize() == 0) groupStack.append({ &rule,target.anticipationToken,0 });
		else for (auto& firstValue : firstL) groupStack.append({ &rule,firstValue, 0 });
	}

	// Return stack
	return groupStack;
}
/* Esta funcion se encarga de agregar un nuevo elemento a la lista de sub grupos(sin repetir una entrada ya puesta)
 Devolvera un puntero del subgrupo, en caso de repitencia, devolvera el puntero del sub-grupo hallado
 Complejidad: O(n)																				*/
architecture::groupEntry* TAS_Builder::insertOrFindSubGroup(const architecture::groupEntry& target) {
	for (auto& entry : groupStackRef) if (target.ruleRef == entry.ruleRef)
		if (target.anticipationToken == entry.anticipationToken && target.pointer == entry.pointer)
			return &entry;
	return &groupStackRef.append(target);
}
// Esta funcion recibe como parametro una lista de similares, y crea y agrega a la lista de referencia de transiciones
// la respectiva regla que generara un grupo N. Devuelve la id del grupo destino, y si la regla ya existe, entonces devuelve
// la id del grupo al que se iria.                                                              
int TAS_Builder::moveAndTransition(const linkedList<architecture::groupEntry*>& similarGroup) {

	architecture::groupTransition* transitionTarget{ transitionTo(similarGroup) };
	if (transitionTarget) {
		// Debug (And Fancy)
		std::cout << "\nModified Transition Rule: [ "; for (auto& entry : transitionTarget->groupOrigin) std::cout << entry << ' '; std::cout << "] -> " << transitionTarget->groupDestiny << " to [ ";
		transitionTarget->groupOrigin.append(groupAnalizing);
		for (auto& entry : transitionTarget->groupOrigin) std::cout << entry << ' '; std::cout << "] -> " << transitionTarget->groupDestiny;
		return transitionTarget->groupDestiny;
	}
	//int groupTarget{ transitionTo(similarGroup) };
	//if (groupTarget) return groupTarget;

	transitionRef.append({ {groupAnalizing}, similarGroup, ++groupAt });
	std::cout << "\nGenerated transition rule: " << groupAnalizing << " -> " << groupAt;
	linkedList<architecture::groupEntry> movedGroup{};
	for (auto& match : similarGroup) {
		movedGroup.append(*match);
		++movedGroup[-1].pointer;
	}
	groupPending.append(&groupList.append(generateGroupFrom(movedGroup)));
	std::cout << "\nGenerated group " << groupAt;
	return groupAt;
}
// Esta funcion verifica entre 2 grupos de transicion (source y target) si todos los elementos de un grupo estan en el otro y viceversa
bool TAS_Builder::isIncludedOnTransition(const architecture::groupTransition& source, const architecture::groupTransition& target) {
	int matchFound{ false };
	if (source.subEntries.getSize() > target.subEntries.getSize()) {
		for (auto& t_ruleS : source.subEntries) {
			matchFound = false;
			for (auto& t_ruleT : target.subEntries) {
				if (t_ruleS->ruleRef == t_ruleT->ruleRef && t_ruleS->pointer == t_ruleT->pointer) {
					matchFound = true;
					break;
				}
			}
			if (!matchFound) return false;
		}
	}
	else {
		for (auto& t_ruleT : target.subEntries) {
			matchFound = false;
			for (auto& t_ruleS : source.subEntries) {
				if (t_ruleS->ruleRef == t_ruleT->ruleRef && t_ruleS->pointer == t_ruleT->pointer) {
					matchFound = true;
					break;
				}
			}
			if (!matchFound) return false;
		}
	}
	return true;
}
// =================================================================================================================================
//														 Auxiliary
// =================================================================================================================================
// Obtiene los primeros de una regla de production buscando usando la id de la letra que genera dicha produccion
// Tira una excepcion de no encontrar ni una, ya que si esta funcion falla, la TAS no podra generarse
linkedList<int>& TAS_Builder::getFirstByID(const int id) {
	for (auto& first : firstList) if (first.leftSide_ID == id) return first.firstList;
	throw std::runtime_error("ID for leftside not found, check the source or code;");	// Error
}
// Verifica si una regla de produccion (con puntero) permite la creacion de similares desde su estado actual. Ejemplo:
// A -> 0B id  (true)   |  A -> B 0id (false) 
bool TAS_Builder::ifValidStep(const architecture::groupEntry& target) {
	if (target.pointer >= target.ruleRef->rightSide.getSize()) return false;
	if (target.ruleRef->rightSide[target.pointer] < 0) return true;
	return false;
}
// Esta funcion obtiene los tokens de anticipacion en base a una posicion de una regla de produccion con puntero
// La funcion debe recibir una regla de produccion con puntero que haya sido validada por la funcion ifValidStep, comportamiento
// inesperado puede ocurrir de no hacerse asi.
linkedList<int> TAS_Builder::getFirstFromRight(const architecture::groupEntry& target) {
	// Check first for end of line
	if (target.pointer + 1 >= target.ruleRef->rightSide.getSize()) return linkedList<int>{ target.anticipationToken };
	// Check for direct token continue
	int tokenRef{ target.ruleRef->rightSide[target.pointer + 1] };
	if (tokenRef >= 0) return linkedList<int>{ tokenRef };
	// Get firsts from not terminal
	linkedList<int> firstL{ getFirstByID(tokenRef) };
	if (isEmptyProduction(tokenRef)) {		// Iterate if at least one entry is repeated
		appendNoRepeat(firstL, getFirstFromRight({ target.ruleRef,target.anticipationToken,target.pointer + 1 }));
	}
	// If firsts empty, return anticipation token
	if (firstL.getSize() == 0) return linkedList<int>{ target.anticipationToken };
	else return firstL;
}
// Esta funcion obtiene el conjunto de similares que se extraen de un grupo de reglas de produccion con puntero
// No devolvera similares que hayan llegando al final de la regla (Ej: A -> id0)
linkedList< linkedList<architecture::groupEntry*>> TAS_Builder::getSimilars(const linkedList<architecture::groupEntry*>& group) {
	linkedList<d_subGroup> similars{};
	d_subGroup analyzeGroup{ group };	// Copy and iterate
	bool newEntry{ true };

	architecture::groupEntry* entrySave{};
	while (analyzeGroup.getSize() > 0) {
		newEntry = true;
		entrySave = analyzeGroup.pop();
		if (entrySave->pointer >= entrySave->ruleRef->rightSide.getSize()) {		// If at the end of rule, dont continue
			if (entrySave->ruleRef->rightSide.getSize() == 0) appendVoidRule({ {groupAnalizing},entrySave });
			continue;
		}
		for (auto& groupSim : similars) {
			if (groupSim[0]->ruleRef->rightSide[groupSim[0]->pointer] == entrySave->ruleRef->rightSide[entrySave->pointer]) {
				groupSim.append(entrySave);
				newEntry = false;
				break;
			}
		}
		if (newEntry) similars.append({ entrySave });
	}
	return similars;
}
// Esta funcion genera un conjunto de reglas de produccion con puntero en base a un grupo de similares.
linkedList<architecture::groupEntry*> TAS_Builder::generateGroupFrom(linkedList<architecture::groupEntry> groupSource) {
	// Variable Declaration
	d_subGroup groupReturn{};
	d_subEntry saveReturn{};
	bool skipLoop{false};
	while (groupSource.getSize() > 0) {
		skipLoop = false;
		saveReturn = insertOrFindSubGroup(groupSource.pop());
		for (auto& entry : groupReturn) {
			if (saveReturn == entry) {
				skipLoop = true;
				break;
			}
		}
		if (skipLoop) continue;
		groupReturn.append(saveReturn);
		if (ifValidStep(*saveReturn)) groupSource.append(initRule(*saveReturn));
	}
	// End
	return groupReturn;
}
// Esta funcion retorna el puntero de la regla de transicion en la referencia global que sea equivalente
// a la regla de transicion pasada por referencia, de no encontrar nada, devuelve nullptr
architecture::groupTransition* TAS_Builder::transitionTo(const linkedList<architecture::groupEntry*>& ref) {
	int matchCount{};
	int matchReq{};
	for (auto& entry : transitionRef) {
		if (entry.subEntries.getSize() != ref.getSize()) continue;
		matchReq = static_cast<int>(entry.subEntries.getSize());
		matchCount = 0;
		for (auto& match : ref) {
			for (auto& t_Rule : entry.subEntries) {
				if (match == t_Rule) {
					++matchCount;
					break;
				}
			}
		}
		if (matchCount == matchReq) return &entry;
	}		
	return nullptr;
}
// Esta funcion combina reglas de transicion similares.
// El conjunto destino por defecto de esta regla combinada sera igual al que poseia el parametro combineTo
// Devuelve un int equivalente al grupo destino de toCombine
int TAS_Builder::combineTransitions(const architecture::groupTransition& toCombine, architecture::groupTransition& combineTo) {
	bool isAbleTo{ true };
	for (auto& toEval : toCombine.subEntries) {
		isAbleTo = true;
		for (auto& entry : combineTo.subEntries) {
			if (toEval == entry) {
				isAbleTo = false;
				break;
			}
		}
		if (isAbleTo) combineTo.subEntries.append(toEval);
	}
	// DEBUG
	std::cout << "\n Combined Groups -> " << toCombine.groupDestiny << " and " << combineTo.groupDestiny << " into " << combineTo.groupDestiny;
	//for (auto& sub : combineTo.subEntries) {
	//	std::cout << '\n' << sub->ruleRef->leftSide_ID << " :";
	//	for (auto& ruleEntry : sub->ruleRef->rightSide) std::cout << ' ' << ruleEntry;
	//	std::cout << "  \t\t| AT: " << sub->anticipationToken << "  |  P: " << sub->pointer;
	//}
	combineTo.groupOrigin = appendSmart(combineTo.groupOrigin, toCombine.groupOrigin);
	return toCombine.groupDestiny;
}
// Esta funcion devuelve un puntero hacia la columna de la TAS generada con un token igual al pasado por parametro
column* TAS_Builder::findToken(int token) {
	for (auto& columnValue : TAS_Return) {
		if (columnValue.token == token) return &columnValue;
	}
	return nullptr;
}
// Esta funcion genera la lista de referencia de todos los primeros en base a las reglas de produccion de referencia
void TAS_Builder::generateFirstList() {
	for (auto& rule : ruleSet) {
		generateFirst(rule.leftSide_ID);
	}
}
// Returns First List of ID, if ID doesn't has one, generates one
linkedList<int>& TAS_Builder::generateFirst(int id) {
	// Found Similarity
	architecture::first* firstTarget{ insertFirst(id) };
	if (firstTarget->firstList.getSize() > 0) return firstTarget->firstList;
	// Recursive until find
	for (auto& ruleEntry : ruleSet) {
		if (ruleEntry.leftSide_ID == id) {
			for (auto& token : ruleEntry.rightSide) {
				if (token == id) break;						// Token = id, then skip / Ej: A -> A ...
				if (token > 0) {							// Token = terminal, add / Ej: A -> a ...
					appendNoRepeat(firstTarget->firstList, token);
					break;
				}
				else {										// Token = N-terminal, add all first of NT / Ej: A -> B
					appendNoRepeat(firstTarget->firstList, generateFirst(token));
					if (!isEmptyProduction(token)) break;	// If token has at least 1 empty production, continue analizing
				}
			}
		}
	}
	if (firstTarget->firstList.getSize() == 0) throw std::runtime_error("Couldn't find any valid first value");
	else return firstTarget->firstList;
}
// Esta funcion inserta una entrada de conjunto de primeros vacia a la lista de referencia, pero si existe una,
// devuelve un puntero hacia esa entrada. Por defecto, devuelve un puntero hacia la entrada recien generada
architecture::first* TAS_Builder::insertFirst(int id) {
	for (auto& firstRule : firstList) if (firstRule.leftSide_ID == id) return &firstRule;
	return &firstList.append({ id,{} });
}
// Esta funcion devuelve verdadero si existe al menos una regla de produccion vacia con la id del parametro,
// caso contrario, devuelve falso (cuando todas las producciones generan al menos un token)
bool TAS_Builder::isEmptyProduction(int id) {
	// Returns cached result if it existed before
	auto i = cacheIfEmpty.find(id);
	if (i != cacheIfEmpty.end()) {
		return i->second;
	}
	// Calculate if not
	for (auto& rule : ruleSet) {
		if (rule.leftSide_ID == id) if (rule.rightSide.getSize() == 0) {
			cacheIfEmpty[id] = true;
			return true;
		}
	}
	cacheIfEmpty[id] = false;
	return false;
}
// Esta funcion genera las reglas de produccion de referencia en base a lo introducido en el archivo "rules_definition.txt"
// Emplea un analizador lexico para detectar cada regla y bota excepciones en caso de errores detectados
void TAS_Builder::generateRules() {
	std::map<std::string, int> mapReference;
	int saveToken{};
	int letterID{ 0 };
	int ruleID{ 0 };
	std::string saveBuffer{};

	std::ofstream output("data/import/rules_map.txt");
	std::ofstream outputID("data/import/id_map.txt");
	std::ifstream file("data/rules_definition.txt");
	std::string fileContent;
	std::string line;

	architecture::rule ruleApp{};
	while (std::getline(file, line)) {
		ruleApp.rightSide.clear();
		lexicAnalyzer ruleAnalysis{ line };
		saveToken = ruleAnalysis.nextToken();
		if (saveToken == 0 || saveToken == 900) continue;
		if (saveToken == 600) {	// If token matches id definition
			saveBuffer = ruleAnalysis.getBuffer();
			if (saveBuffer == "id") throw std::runtime_error("Can't place 'id' on leftside");
			auto it = mapReference.find(saveBuffer);
			if (it == mapReference.end()) {
				mapReference[saveBuffer] = --letterID;
				outputID << letterID << ' ' << saveBuffer << '\n';
			}

			ruleApp.rule_ID = ++ruleID;
			ruleApp.leftSide_ID = mapReference[saveBuffer];
		}
		else throw std::runtime_error("Invalid leftside in rules_definition.txt");
		if (ruleAnalysis.nextToken() != 807) throw std::runtime_error("Couldn't find ':' in rule definition in rules_definition.txt");
		// Append all tokens
		while ((saveToken = ruleAnalysis.nextToken()) != 0) {
			if (saveToken == -1) throw std::runtime_error("Couldn't find token conversion for " + ruleAnalysis.getBuffer());
			// Detected token ID
			if (saveToken == 600 && (saveBuffer = ruleAnalysis.getBuffer()) != "id") {
				auto it = mapReference.find(saveBuffer);
				if (it == mapReference.end()) {
					mapReference[saveBuffer] = --letterID; // id doesn't exists
					outputID << letterID << ' ' << saveBuffer << '\n';
				}

				saveToken = mapReference[saveBuffer];
			}
			ruleApp.rightSide.append(saveToken);
		}
		std::cout << "\nImported Rule: " << ruleApp.rule_ID << ' ' << ruleApp.leftSide_ID << ' '; for (auto& entry : ruleApp.rightSide) std::cout << entry << ' ';
		ruleSet.append(ruleApp);
		output << ruleID << ' ' << line << '\n';
	}
	file.close();
	output.close();
	outputID.close();
}

void TAS_Builder::appendVoidRule(const architecture::entryVoid& entry) {
	for (auto& ref : entryVoidList) {
		if (ref.entryRef == entry.entryRef) {
			appendNoRepeat(ref.groupOrigin, entry.groupOrigin);
			return;
		}
	}
	entryVoidList.append(entry);
}

bool detectAmbiguity(const column* target, const int group) {
	for (auto& entry : target->entries) {
		if (entry.groupStart == group) return true;
	}
	return false;
}

void updateGroupOrigin(linkedList<architecture::groupTransition>& stack, int toKeep, int toReplace) {
	bool foundKeep{};
	int indexErase{ -1 };
	int trueErase{ -1 };
	for (auto& evaluate : stack) {
		foundKeep = false;
		indexErase = -1;
		trueErase = -1;
		for (auto& group : evaluate.groupOrigin) {
			++indexErase;
			if (group == toKeep) foundKeep = true;
			if (group == toReplace) {
				if (foundKeep) {
					trueErase = indexErase;
					break;
				}
				else {
					group = toKeep;
					break;
				}
			}
		}
		if (trueErase >= 0) evaluate.groupOrigin.erase(trueErase);
		if (evaluate.groupDestiny == toReplace) evaluate.groupDestiny = toKeep;
	}
}

void updateGroupOrigin(linkedList<architecture::entryVoid>& stack, int toKeep, int toReplace) {
	bool foundKeep{};
	int indexErase{ -1 };
	int trueErase{ -1 };
	for (auto& evaluate : stack) {
		foundKeep = false;
		indexErase = -1;
		trueErase = -1;
		for (auto& group : evaluate.groupOrigin) {
			++indexErase;
			if (group == toKeep) foundKeep = true;
			if (group == toReplace) {
				if (foundKeep) {
					trueErase = indexErase;
					break;
				}
				else {
					group = toKeep;
					break;
				}
			}
		}
		if (trueErase >= 0) evaluate.groupOrigin.erase(trueErase);
	}
}

linkedList<int> appendSmart(linkedList<int> toModify, linkedList<int> toAppend) {
	linkedList<int> returnList{};

	while (toModify.getSize() > 0 && toAppend.getSize() > 0) {
		if (toModify[0] == toAppend[0]) {
			returnList.append(toModify.pop());
			toAppend.erase(0);
		}
		else if (toModify[0] < toAppend[0]) returnList.append(toModify.pop());
		else if (toModify[0] > toAppend[0]) returnList.append(toAppend.pop());
	}
	if (toModify.getSize() > 0) returnList.append(toModify);
	else if (toAppend.getSize() > 0) returnList.append(toAppend);
	return returnList;
}

void appendNoRepeat(linkedList<int>& source, int value) {
	for (auto& i : source) if (i == value) return;
	source.append(value);
}
void appendNoRepeat(linkedList<int>& source, linkedList<int> value) {
	while (value.getSize() > 0) {
		int toAppend{ value.pop() };
		for (auto& i : source) if (toAppend == i) continue;
		source.append(toAppend);
	}
}

// =================================================================================================================================
//													Auxiliary ++ (DEBUG)
// =================================================================================================================================
void TAS_Builder::printGroups() {
	int groupNum{ 0 };
	for (auto& group : groupList) {
		std::cout << "\n Group " << groupNum++ << ": ";
		for (auto& sub : group) {
			std::cout << '\n' << sub->ruleRef->leftSide_ID << " :";
			for (auto& ruleEntry : sub->ruleRef->rightSide) std::cout << ' ' << ruleEntry;
			std::cout << "  \t\t| AT: " << sub->anticipationToken << "  |  P: " << sub->pointer;
		}
	}
}
void TAS_Builder::printTransitions() {
	for (auto& transition : transitionRef) {
		std::cout << "\n Transition [ ";
		for (auto origin : transition.groupOrigin) std::cout << origin << " ";
		std::cout << "] -> " << transition.groupDestiny;
		for (auto& sub : transition.subEntries) {
			std::cout << '\n' << sub->ruleRef->leftSide_ID << " :";
			for (auto& ruleEntry : sub->ruleRef->rightSide) std::cout << ' ' << ruleEntry;
			std::cout << "  \t\t| AT: " << sub->anticipationToken << "  |  P: " << sub->pointer;
		}
	}
}
void TAS_Builder::exportTAS() {
	int cellC{ 0 };
	int columnC{ 0 };

	std::ofstream outputFile("data/TAS.txt");
	if (!outputFile) {
		std::cerr << "Can't open data/TAS.txt" << std::endl;
		return;
	}

	outputFile << "{";
	for (const auto& column : TAS_Return) {
		outputFile << "\n\t{ " << column.token << ", {";
		cellC = 0;
		for (const auto& cell : column.entries) {
			outputFile << "{ " << cell.groupStart << ", ";
			int actionINT{ static_cast<int>(cell.action) };
			if (actionINT > 0) outputFile << 'S' << actionINT;
			else if (actionINT < 0) outputFile << 'R' << -actionINT;
			else outputFile << "ACCEPT";
			outputFile << " }";
			if (!(++cellC == column.entries.getSize())) outputFile << ", ";
		}
		outputFile << "}}";
		if (!(++columnC == TAS_Return.getSize())) outputFile << ',';
	}
	outputFile << "\n}";

	std::cout << "\nTAS.txt succefully generated";
	outputFile.close();
}
void TAS_Builder::printFirstList() {
	int count{ 0 };
	for (const auto& first : firstList) {
		std::cout << '\n' << ++count << " | " << first.leftSide_ID << " : "; for (auto& entry : first.firstList) std::cout << entry << ' ';
	}
}

void TAS_Builder::exportFinal() {
	int lines{ 0 };
	// Generate TAS
	std::ofstream output("data/import/tas.txt");
	if (!output) {
		std::cerr << "Can't open data/import/tas.txt" << std::endl;
		return;
	}
	for (const auto& column : TAS_Return) {
		output << column.token;
		for (const auto& cell : column.entries) {
			output << ' ' << cell.groupStart << ' ' << static_cast<int>(cell.action);
		}
		if (!(++lines >= TAS_Return.getSize())) output << '\n';
	}
	output.close();
	lines = 0;
	// Generate RULES
	output.open("data/import/rules.txt");
	if (!output) {
		std::cerr << "Can't open data/import/rules.txt" << std::endl;
		return;
	}
	for (const auto& rule : ruleSet) {
		output << -rule.rule_ID << ' ' << rule.leftSide_ID << ' ' << rule.rightSide.getSize();
		if (!(++lines >= ruleSet.getSize())) output << '\n';
	}
	output.close();
}