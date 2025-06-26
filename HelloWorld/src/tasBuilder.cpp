#include <stdexcept>
#include <iostream>
#include <vector>
#include <fstream>

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
TAS_Builder::TAS_Builder(linkedList<architecture::rule> input, linkedList<architecture::first> firstList) : ruleSet{input}, firstList{firstList} {}
// =================================================================================================================================
//														TAS BUILDER
// =================================================================================================================================
void TAS_Builder::tasBuilder() {
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
			modified = false;
		}
	}
	transitionRef = lalrTransition;
	std::cout << "\n[OK] LALR created succesfully, number of transition rules generated -> " << transitionRef.getSize();
	// ================================ TAS Generation ================================
	std::cout << "\n =========================== TAS Creation Process:";
	printTransitions();
	TAS_Return.clear();
	int evaluatingToken{};
	column* columnToModify{};
	for (auto& transition : transitionRef) {
		// Shifts
		evaluatingToken = transition.subEntries[0]->ruleRef->rightSide[transition.subEntries[0]->pointer];		// Get token
		columnToModify = findToken(evaluatingToken);
		if (!columnToModify) columnToModify = &TAS_Return.append({ evaluatingToken,{} });
		for (auto& groupO : transition.groupOrigin) {
			columnToModify->entries.append({ groupO,static_cast<action>(transition.groupDestiny) });
		}
		// Reduce
		for (auto& rule : transition.subEntries) {
			if (!(rule->ruleRef->rightSide.getSize() <= rule->pointer + 1)) continue; // Unfit Rule to reduce
			
			columnToModify = findToken(rule->anticipationToken);
			if (!columnToModify) columnToModify = &TAS_Return.append({ rule->anticipationToken, {} });
			columnToModify->entries.append({ transition.groupDestiny,static_cast<action>(-rule->ruleRef->rule_ID) });
		}
	}
	std::cout << "\n[OK] TAS created succesfully, number of columns generated -> " << TAS_Return.getSize();

	exportTAS();
}
// =================================================================================================================================
//													Function Definitions
// =================================================================================================================================
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
/* Esta funcion recibe como parametro una lista de similares, y crea y agrega a la lista de referencia de transiciones
* la respectiva regla que generara un grupo N. Devuelve la id del grupo destino, y si la regla ya existe, entonces devuelve
* la id del grupo al que se iria.                                                               */
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
linkedList<int>& TAS_Builder::getFirstByID(const int id) {
	for (auto& first : firstList) if (first.leftSide_ID == id) return first.firstList;
	throw std::runtime_error("ID for leftside not found, check the source or code;");	// Error
}
bool TAS_Builder::ifValidStep(const architecture::groupEntry& target) {
	if (target.pointer >= target.ruleRef->rightSide.getSize()) return false;
	if (target.ruleRef->rightSide[target.pointer] < 0) return true;
	return false;
}
linkedList<int> TAS_Builder::getFirstFromRight(const architecture::groupEntry& target) {
	// Check first for end of line
	if (target.pointer + 1 >= target.ruleRef->rightSide.getSize()) return linkedList<int>{ target.anticipationToken };
	// Check for direct token continue
	int tokenRef{ target.ruleRef->rightSide[target.pointer + 1] };
	if (tokenRef >= 0) return linkedList<int>{ tokenRef };
	// Get firsts from not terminal
	linkedList<int> firstL{ getFirstByID(tokenRef) };
	// If firsts empty, return anticipation token
	if (firstL.getSize() == 0) return linkedList<int>{ target.anticipationToken };
	else return firstL;
}
linkedList< linkedList<architecture::groupEntry*>> TAS_Builder::getSimilars(const linkedList<architecture::groupEntry*>& group) {
	linkedList<d_subGroup> similars{};
	d_subGroup analyzeGroup{ group };	// Copy and iterate
	bool newEntry{ true };

	architecture::groupEntry* entrySave{};
	while (analyzeGroup.getSize() > 0) {
		newEntry = true;
		entrySave = analyzeGroup.pop();
		if (entrySave->pointer >= entrySave->ruleRef->rightSide.getSize()) continue;	// If rule invalid, continue
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
column* TAS_Builder::findToken(int token) {
	for (auto& columnValue : TAS_Return) {
		if (columnValue.token == token) return &columnValue;
	}
	return nullptr;
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
//bool ifInGroup(const linkedList<architecture::groupEntry>& source, const linkedList<architecture::groupEntry>& target) {
//	for 
//}
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
		std::cerr << "Failed to open file for writing." << std::endl;
		return;
	}

	outputFile << "{";
	for (auto& column : TAS_Return) {
		outputFile << "\n\t{ " << column.token << ", {";
		cellC = 0;
		for (auto& cell : column.entries) {
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