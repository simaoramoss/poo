#include "Directory.hpp"
#include <iostream>
#include <algorithm>

Directory::Directory(const std::string& name, Directory* parent)
    : name(name), parent(parent) {}

std::string Directory::getName() const {
    return name;
}


const std::vector<std::shared_ptr<Directory>>& Directory::getSubdirectories() const {
    return subdirectories;
}

const std::vector<std::shared_ptr<File>>& Directory::getFiles() const {
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

std::shared_ptr<Directory> Directory::findSubdirectory(const std::string& name) const {
    auto it = std::find_if(subdirectories.begin(), subdirectories.end(),
        [&name](const auto& dir) { return dir->getName() == name; });
    
    return (it != subdirectories.end()) ? *it : nullptr;
}

std::shared_ptr<File> Directory::findFile(const std::string& name) const {
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

int Directory::getTotalFiles() const {
    int total = static_cast<int>(files.size());
    for (const auto& dir : subdirectories) {
        total += dir->getTotalFiles();
    }
    return total;
}

int Directory::getTotalDirectories() const {
    int total = 1;  // Count the current directory itself
    for (const auto& dir : subdirectories) {
        total += dir->getTotalDirectories();
    }
    return total;
}

int Directory::getElementCount() const {
    return static_cast<int>(subdirectories.size() + files.size());
}

std::pair<std::string, size_t> Directory::findLargestFileWithPath(const std::string& currentPath) const {
    std::string bestPath;
    size_t bestSize = 0;
    bool found = false;

    // Check files in this directory
    for (const auto& file : files) {
        size_t s = file->getSize();
        std::string p = currentPath.empty() ? file->getName() : (currentPath + "\\" + file->getName());
        if (!found || s > bestSize) {
            found = true;
            bestSize = s;
            bestPath = p;
        }
    }

    // Recurse into subdirectories
    for (const auto& dir : subdirectories) {
        std::string subPath = currentPath.empty() ? dir->getName() : (currentPath + "\\" + dir->getName());
        auto [childPath, childSize] = dir->findLargestFileWithPath(subPath);
        if (!childPath.empty() && (!found || childSize > bestSize)) {
            found = true;
            bestSize = childSize;
            bestPath = childPath;
        }
    }

    if (!found) return {"", 0};
    return {bestPath, bestSize};
}