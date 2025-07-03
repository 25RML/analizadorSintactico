#pragma once
#include <unordered_map>

#include "linkedList.h"
#include "lr.h"

namespace architecture {
	struct rule {
		int rule_ID{};
		int leftSide_ID{};
		linkedList<int> rightSide{};
	};

	struct groupEntry {
		const rule* ruleRef{};
		int anticipationToken{};
		int pointer{};
	};

	struct first {
		int leftSide_ID{};
		linkedList<int> firstList{};
	};
	
	struct groupTransition {
		linkedList<int> groupOrigin{};
		linkedList<architecture::groupEntry*> subEntries{};
		int groupDestiny{};
	};

	struct entryVoid {
		linkedList<int> groupOrigin{};
		const groupEntry* entryRef{};
	};
}

namespace input {
	inline const architecture::rule g_startRule{ 0, 0, {-1} };

	inline const linkedList<architecture::rule> testInput{
		{ 1, -1, {-1, 707, -2}	},		// A -> A + B
		{ 2, -1, {-2}			},		// A -> B
		{ 3, -2, {-2, 707, -3}	},		// B -> B + C
		{ 4, -2, {-3}			},		// B -> C
		{ 5, -3, {802, -1, 803}	},		// C -> ( A )
		{ 6, -3, {600}			}		// C -> id
	};

	inline const linkedList<architecture::first> refFirst{
		{-1,{802,600} },	// A -> (, id
		{-2,{802,600} },	// B -> (, id
		{-3,{802,600} }		// C -> (, id
	};
}

class TAS_Builder {
	// Attributes
	linkedList<architecture::rule> ruleSet{};
	linkedList<architecture::first> firstList{};
	linkedList<architecture::groupEntry> groupStackRef{ {} };
	linkedList<linkedList<architecture::groupEntry*>> groupList{ {} };
	linkedList<linkedList<architecture::groupEntry*>*> groupPending{ {} };
	linkedList<architecture::groupTransition> transitionRef{};
	linkedList<architecture::entryVoid> entryVoidList{};
	int groupAt{ 0 };
	int groupAnalizing{ 0 };
	linkedList<column> TAS_Return{};

	std::unordered_map<int, bool> cacheIfEmpty;
public:
	// Constructor
	TAS_Builder(linkedList<architecture::rule> input);
	TAS_Builder();
	// Methods
	void tasBuilder();
	architecture::groupEntry initRule(const architecture::rule& target);
	linkedList<architecture::groupEntry> initRule(const architecture::groupEntry& target);
	architecture::groupEntry* insertOrFindSubGroup(const architecture::groupEntry& target);
	int moveAndTransition(const linkedList<architecture::groupEntry*>& similarGroup);
	bool isIncludedOnTransition(const architecture::groupTransition& source, const architecture::groupTransition& target);
	// Auxiliary
	bool ifValidStep(const architecture::groupEntry& target);
	linkedList<int>& getFirstByID(const int id);
	linkedList<int> getFirstFromRight(const architecture::groupEntry& target);
	linkedList<linkedList<architecture::groupEntry*>> getSimilars(const linkedList<architecture::groupEntry*>& group);
	linkedList<architecture::groupEntry*> generateGroupFrom(linkedList<architecture::groupEntry> groupSource);
	architecture::groupTransition* transitionTo(const linkedList<architecture::groupEntry*>& ref);
	int combineTransitions(const architecture::groupTransition& toCombine, architecture::groupTransition& combineTo);
	column* findToken(int token);
	void generateFirstList();
	architecture::first* insertFirst(int id);
	linkedList<int>& generateFirst(int id);
	bool isEmptyProduction(int id);
	void generateRules();

	void exportFinal();
	void appendVoidRule(const architecture::entryVoid& entry);
	// Auxiliary ++ (DEBUG FUNCTIONS)
private:
	void printGroups();
	void printTransitions();
	void exportTAS();
	void printFirstList();
};

void updateGroupOrigin(linkedList<architecture::groupTransition>& stack, int toKeep, int toReplace);
void updateGroupOrigin(linkedList<architecture::entryVoid>& stack, int toKeep, int toReplace);
linkedList<int> appendSmart(linkedList<int> toModify, linkedList<int> toAppend);
void appendNoRepeat(linkedList<int>& source, int value);
void appendNoRepeat(linkedList<int>& source, linkedList<int> value);
bool detectAmbiguity(const column* target, const int group);