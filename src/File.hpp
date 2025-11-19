#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <ctime>

class File {
private:
    std::string name;
    size_t size;
    std::string date; 

public:
    File(const std::string& name, size_t size);
    
    std::string getName() const;
    size_t getSize() const;
    std::string getDate() const;
    
    void setName(const std::string& newName);
    void setDate(const std::string& newDate);
};

#endif