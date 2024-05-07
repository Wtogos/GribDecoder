// Gribdecoder
#include "GribDecoder.h"
#include <iostream>
#include <fstream>

GribDecoder::GribDecoder(const std::string& filename) : filename(filename) {}

void GribDecoder::findPattern(std::ifstream& file, const std::string& pattern, std::streampos& position) {
    char byte;
    while (!file.eof()) {
        file.read(&byte, 1);
        if (byte == pattern[0]) {
            bool patternFound = true;
            for (size_t i = 1; i < pattern.size(); ++i) {
                file.read(&byte, 1);
                if (byte != pattern[i]) {
                    patternFound = false;
                    break;
                }
            }
            if (patternFound) {
                position = file.tellg();
                std::cout << pattern << " znajduje sie na pozycji: " << position << "\n";
                break;
            }
        }
    }
}

void GribDecoder::readMessageLength(std::ifstream& file) {
    char byte1, byte2, byte3;
    file.read(&byte1, 1);
    file.read(&byte2, 1);
    file.read(&byte3, 1);
    int message_length = ((int)byte1 << 16) | ((int)byte2 << 8) | (int)byte3 & 0xff;
    std::cout << "Dlugosc wiadomosci: " << message_length << "\n";
}

void GribDecoder::readSectionData(std::ifstream& file, std::streampos start, int length) {
    if (start != -1) {
        file.seekg(start);
        char* section_data = new char[length];
        file.read(section_data, length);
        std::cout << "==  Sekcja " << (start == start_position_of_section1 ? "1" : "2") << "  ==\n";
        std::cout << "Dlugosc sekcji " << (start == start_position_of_section1 ? "1" : "2") << ": " << length << "\n";
        delete[] section_data;
    }
    else {
        std::cout << "Blad: Poczatkowa pozycja sekcji nie zostala zapisana.\n";
    }
}

void GribDecoder::decode() {
    std::ifstream grib(filename, std::ios::binary | std::ios::in);

    if (!grib.is_open()) {
        std::cout << "Nie udalo sie otworzyc pliku!\n";
        return;
    }

    std::streampos position7777 = -1;
    std::streampos positionGRIB = -1;

    findPattern(grib, "GRIB", positionGRIB);
    findPattern(grib, "7777", position7777);

    if (positionGRIB != -1 && position7777 != -1) {
        int distance = position7777 - positionGRIB;
        std::cout << "Dystans miedzy 7777 a GRIB: " << distance << " bajt\n";
        start_position_of_section1 = positionGRIB;
    }
    else {
        std::cout << "Nie znaleziono wszystkich znacznikow.\n";
    }

    readMessageLength(grib);
    readSectionData(grib, start_position_of_section1, 100); // Assuming section 1 length is 100
    readSectionData(grib, -1, 0); // Placeholder for reading section 2

    grib.close();
}
