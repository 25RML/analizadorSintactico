#pragma once

#include <utility>
#include <string>

#include "lexico.h"
#include "linkedList.h"


enum action {
	R140 = -140,
	R139, R138, R137, R136, R135, R134, R133, R132, R131, R130,
	R129, R128, R127, R126, R125, R124, R123, R122, R121, R120,
	R119, R118, R117, R116, R115, R114, R113, R112, R111, R110,
	R109, R108, R107, R106, R105, R104, R103, R102, R101, R100,
	R99, R98, R97, R96, R95, R94, R93, R92, R91, R90,
	R89, R88, R87, R86, R85, R84, R83, R82, R81, R80,
	R79, R78, R77, R76, R75, R74, R73, R72, R71, R70,
	R69, R68, R67, R66, R65, R64, R63, R62, R61, R60,
	R59, R58, R57, R56, R55, R54, R53, R52, R51, R50,
	R49, R48, R47, R46, R45, R44, R43, R42, R41, R40,
	R39, R38, R37, R36, R35, R34, R33, R32, R31, R30,
	R29, R28, R27, R26, R25, R24, R23, R22, R21, R20,
	R19, R18, R17, R16, R15, R14, R13, R12, R11, R10,
	R9, R8, R7, R6, R5, R4, R3, R2, R1,
	ACCEPT = 0,
	S1 = 1, S2, S3, S4, S5, S6, S7, S8, S9,
	S10, S11, S12, S13, S14, S15, S16, S17, S18, S19,
	S20, S21, S22, S23, S24, S25, S26, S27, S28, S29,
	S30, S31, S32, S33, S34, S35, S36, S37, S38, S39,
	S40, S41, S42, S43, S44, S45, S46, S47, S48, S49,
	S50, S51, S52, S53, S54, S55, S56, S57, S58, S59,
	S60, S61, S62, S63, S64, S65, S66, S67, S68, S69,
	S70, S71, S72, S73, S74, S75, S76, S77, S78, S79,
	S80, S81, S82, S83, S84, S85, S86, S87, S88, S89,
	S90, S91, S92, S93, S94, S95, S96, S97, S98, S99,
	S100, S101, S102, S103, S104, S105, S106, S107, S108, S109,
	S110, S111, S112, S113, S114, S115, S116, S117, S118, S119,
	S120, S121, S122, S123, S124, S125, S126, S127, S128, S129,
	S130, S131, S132, S133, S134, S135, S136, S137, S138, S139,
	S140, S141, S142, S143, S144, S145, S146, S147, S148, S149,
	S150, S151, S152, S153, S154, S155, S156, S157, S158, S159,
	S160, S161, S162, S163, S164, S165, S166, S167, S168, S169,
	S170, S171, S172, S173, S174, S175, S176, S177, S178, S179,
	S180, S181, S182, S183, S184, S185, S186, S187, S188, S189,
	S190, S191, S192, S193, S194, S195, S196, S197, S198, S199,
	S200, S201, S202, S203, S204, S205, S206, S207, S208, S209,
	S210, S211, S212, S213, S214, S215, S216, S217, S218, S219,
	S220, S221, S222, S223, S224, S225, S226, S227, S228, S229,
	S230, S231, S232, S233, S234, S235, S236, S237, S238, S239,
	S240, S241, S242, S243, S244, S245, S246, S247, S248, S249,
	S250, S251, S252, S253, S254, S255, S256, S257, S258, S259,
	S260, S261, S262, S263, S264, S265, S266, S267, S268, S269,
	S270, S271, S272, S273, S274, S275, S276, S277, S278, S279,
	S280, S281, S282, S283, S284, S285, S286, S287, S288, S289,
	S290, S291, S292, S293, S294, S295, S296, S297, S298, S299,
	S300, S301, S302, S303, S304, S305, S306, S307, S308, S309,
	S310, S311, S312, S313, S314, S315, S316, S317, S318, S319,
	S320, S321, S322, S323, S324, S325, S326, S327, S328, S329,
	S330, S331, S332, S333, S334, S335, S336, S337, S338, S339,
	S340, S341, S342, S343, S344, S345, S346, S347, S348, S349,
	S350, S351, S352, S353, S354, S355, S356, S357, S358, S359,
	S360, S361, S362, S363, S364, S365, S366, S367, S368, S369,
	S370, S371, S372, S373, S374, S375, S376, S377, S378, S379,
	S380, S381, S382, S383, S384, S385, S386, S387, S388, S389,
	S390, S391, S392, S393, S394, S395, S396, S397, S398, S399,

	error = 999,
	
	S155_R3 =  15503,
	S155_R61 = 15561,
	S19_R73 =   1973
};
struct entry {
	int groupStart{};
	action action{};
};
struct column {
	int token{};
	linkedList<entry> entries{};
};
struct pRules {
	int id{};
	int letter{};
	int n_rightSide{};
};
// ---------------------- PANIC MODE
struct panicToken {
	bool review{ false };
	int savedIndex{};
	std::string savedBuffer{};
	linkedList<int> saveState{};
};
// ---------------------- PANIC MODE
namespace constant {
	inline const linkedList<column> TAS{
	{ -1, {{ 0, S1 }, { 4, S8 }}},
	{ 0, {{ 1, ACCEPT }, { 2, R2 }, { 3, R4 }, { 5, R6 }, { 13, R1 }, { 14, R3 }, { 15, R5 }}},
	{ -2, {{ 0, S2 }, { 4, S2 }, { 6, S13 }}},
	{ 707, {{ 2, R2 }, { 3, R4 }, { 5, R6 }, { 1, S6 }, { 8, S6 }, { 2, S7 }, { 13, S7 }, { 14, R3 }, { 15, R5 }}},
	{ 803, {{ 2, R2 }, { 3, R4 }, { 5, R6 }, { 13, R1 }, { 14, R3 }, { 8, S15 }, { 15, R5 }}},
	{ -3, {{ 0, S3 }, { 4, S3 }, { 6, S3 }, { 7, S14 }}},
	{ 802, {{ 0, S4 }, { 4, S4 }, { 6, S4 }, { 7, S4 }}},
	{ 600, {{ 0, S5 }, { 4, S5 }, { 6, S5 }, { 7, S5 }}}
};
	inline const linkedList<pRules> rules{
		{-1, -1, 3},
		{-2, -1, 1},
		{-3, -2, 3},
		{-4, -2, 1},
		{-5, -3, 3},
		{-6, -3, 1}
	};
}
// ----------------------------------------------------- CLASS: actionTable
class actionTable {
	linkedList<column> table{};
public:
	actionTable(linkedList<column> tableSet);
	actionTable();

	action getAction(int token, int group);
};
// ----------------------------------------------------- CLASS: parserLR0
class parserLR0 {
	actionTable TAS{};
	linkedList<int> stackList{};
	linkedList<pRules> productionRules{};
public:
	lexicAnalyzer analyzer{};
	// ----------------------------------------------- Builders
	parserLR0(actionTable tableInput, linkedList<pRules> rulesList, std::string input = "");	// Builder
	parserLR0(std::string input = "");
	// ----------------------------------------------- Analyze Inputs
	void analyzeInput();
	std::pair<int,int> numberOfRules(int index);
	void panicMode();
};

std::string printToken(int token);
void printRule(int id);
std::string printNT(int idNT);

actionTable importTAS();
linkedList<pRules> importRULES();