#include "Directory.hpp"
#include <iostream>
#include <algorithm>

Directory::Directory(const std::string& name, Directory* parent)
    : name(name), parent(parent) {}

std::string Directory::getName() const {
    return name;
}

std::vector<std::shared_ptr<Directory>> Directory::getSubdirectories() const {
    return subdirectories;
}

std::vector<std::shared_ptr<File>> Directory::getFiles() const {
    return files;
}

Directory* Directory::getParent() const {
    return parent;
}

void Directory::addSubdirectory(const std::string& name) {
    auto newDir = std::make_shared<Directory>(name, this);
    subdirectories.push_back(newDir);
}

void Directory::addFile(const std::string& name, size_t size) {
    auto newFile = std::make_shared<File>(name, size);
    files.push_back(newFile);
}

void Directory::removeSubdirectory(const std::string& name) {
    auto it = std::find_if(subdirectories.begin(), subdirectories.end(),
        [&name](const auto& dir) { return dir->getName() == name; });
    
    if (it != subdirectories.end()) {
        subdirectories.erase(it);
    }
}

void Directory::removeFile(const std::string& name) {
    auto it = std::find_if(files.begin(), files.end(),
        [&name](const auto& file) { return file->getName() == name; });
    
    if (it != files.end()) {
        files.erase(it);
    }
}

std::shared_ptr<Directory> Directory::findSubdirectory(const std::string& name) {
    auto it = std::find_if(subdirectories.begin(), subdirectories.end(),
        [&name](const auto& dir) { return dir->getName() == name; });
    
    return (it != subdirectories.end()) ? *it : nullptr;
}

std::shared_ptr<File> Directory::findFile(const std::string& name) {
    auto it = std::find_if(files.begin(), files.end(),
        [&name](const auto& file) { return file->getName() == name; });
    
    return (it != files.end()) ? *it : nullptr;
}

void Directory::listContents() const {
    std::cout << "Diretória: " << name << "\n";
    std::cout << "Subdiretórias:\n";
    for (const auto& dir : subdirectories) {
        std::cout << "  " << dir->getName() << "/\n";
    }
    
    std::cout << "Ficheiros:\n";
    for (const auto& file : files) {
        std::cout << "  " << file->getName() << " (" << file->getSize() << " bytes)\n";
    }
}

size_t Directory::getTotalSize() const {
    size_t total = 0;
    
    // Sum files sizes
    for (const auto& file : files) {
        total += file->getSize();
    }
    
    // Recursively sum subdirectories sizes
    for (const auto& dir : subdirectories) {
        total += dir->getTotalSize();
    }
    
    return total;
}