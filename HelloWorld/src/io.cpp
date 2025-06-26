#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <initializer_list>
#include <string>
#include <fstream>

#include "tasBuilder.h"

#include "lexico.h"
#include "lr.h"
#include "linkedList.h"
// -------------------------------------------- CLASSES

// -------------------------------------------- MAIN BODY
/*
int main() {

    TAS_Builder testTAS{ input::testInput,input::refFirst };
    testTAS.tasBuilder();

    _getch();

    return 0;
}*/


int main() {
    // Generate TAS
    //TAS_Builder testTAS{ input::testInput,input::refFirst };
    //testTAS.tasBuilder();

    std::cout << "\n\n";

    std::ifstream file("data/input.warp");
    std::string fileContent;
    std::string line;
    while (std::getline(file, line)) {
        fileContent += line + "\n";
    }
    file.close();

    //std::string input{};
    //std::cout << " input: ";
    //std::getline(std::cin, input);

    parserLR0 mainParser{
        constant::TAS,
        constant::rules,
        fileContent
    };

    mainParser.analyzeInput();
    //"switch (hola) {}"

    //lexicAnalyzer analyze{ fileContent };
    //analyze.tokenTable();
    //mainParser.analyzer.tokenTable();

    std::cout << "\n\nEND";

    std::cin >> line;

    return 0;
}
