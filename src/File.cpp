
#include "File.hpp"
#include <ctime>
#include <sstream>
#include <iomanip>

// Ao criar um ficheiro, registamos também a data (YYYY|MM|DD) do momento.
File::File(const std::string& name, size_t size) 
    : name(name), size(size) {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    
    std::stringstream ss;
    ss << (1900 + ltm->tm_year) << "|" 
       << std::setw(2) << std::setfill('0') << (1 + ltm->tm_mon) << "|"
       << std::setw(2) << std::setfill('0') << ltm->tm_mday;
    date = ss.str();
}

std::string File::getName() const {
    return name;
}

size_t File::getSize() const {
    return size;
}

std::string File::getDate() const {
    return date;
}

void File::setName(const std::string& newName) {
    name = newName;
}

void File::setDate(const std::string& newDate) {
    // Útil quando importamos de XML ou ficamos com a data do disco.
    date = newDate;
}
