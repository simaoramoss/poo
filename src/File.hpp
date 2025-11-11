#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <ctime>

class File {
private:
    std::string name;
    size_t size;
    std::string date; // formato: "YYYY|MM|DD"

public:
    File(const std::string& name, size_t size);
    
    // Getters
    std::string getName() const;
    size_t getSize() const;
    std::string getDate() const;
    
    // Setters
    void setName(const std::string& newName);
    void setDate(const std::string& newDate);
};

#endif