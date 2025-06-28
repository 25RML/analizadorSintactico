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
    //TAS_Builder testTAS{ input::testInput };
    TAS_Builder testTAS{};
    testTAS.tasBuilder();

    std::cout << "\n\n";

    std::ifstream file("data/input.warp");
    std::string fileContent;
    std::string line;
    while (std::getline(file, line)) {
        fileContent += line + "\n";
    }
    file.close();


    parserLR0 mainParser{
        fileContent
    };

    mainParser.analyzeInput();


    std::cout << "\n\nEND";
    std::cin >> line;

    return 0;
}
