#include <iostream>
#include <conio.h>
#include <initializer_list>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>

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
namespace fs = std::filesystem;

// =============================== PARA REVISAR SI EL ARCHIVO RULES_DEFINITION.TXT CAMBIO EN LA ULTIMA EJECUCION
std::time_t to_time_t(fs::file_time_type ftime);
std::time_t loadSavedTimestamp(const std::string& metaPath);
void saveTimestamp(const std::string& metaPath, std::time_t time);
bool fileChanged(const std::string& dataPath, const std::string& metaPath);

int main() {
    // Generate TAS
    if (fileChanged("data/rules_definition.txt", "data/last.meta")) {
        TAS_Builder testTAS{};
        testTAS.tasBuilder();
        
        std::time_t newTime = to_time_t(fs::last_write_time("data/rules_definition.txt"));
        saveTimestamp("data/last.meta", newTime);
    }


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


// =============================== PARA REVISAR SI EL ARCHIVO RULES_DEFINITION.TXT CAMBIO EN LA ULTIMA EJECUCION

std::time_t to_time_t(fs::file_time_type ftime) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + system_clock::now()
    );
    return system_clock::to_time_t(sctp);
}

std::time_t loadSavedTimestamp(const std::string& metaPath) {
    std::ifstream in(metaPath, std::ios::binary);
    if (!in) return 0;

    std::time_t saved;
    in.read(reinterpret_cast<char*>(&saved), sizeof(saved));
    return saved;
}

void saveTimestamp(const std::string& metaPath, std::time_t time) {
    std::ofstream out(metaPath, std::ios::binary);
    out.write(reinterpret_cast<const char*>(&time), sizeof(time));
}

bool fileChanged(const std::string& dataPath, const std::string& metaPath) {
    if (!fs::exists(dataPath)) {
        std::cerr << "File does not exist: " << dataPath << "\n";
        return false;
    }

    auto currentTime = to_time_t(fs::last_write_time(dataPath));
    auto savedTime = loadSavedTimestamp(metaPath);

    return currentTime != savedTime;
}