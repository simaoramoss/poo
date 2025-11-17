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
    std::cout << "Diretoria: " << name << "\n";
    
    std::cout << "Subdiretorias:\n";
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

    for (const auto& file : files) total += file->getSize();
    for (const auto& dir : subdirectories) total += dir->getTotalSize();

    return total;
}

int Directory::getTotalFiles() const {
    int total = files.size();
    for (const auto& dir : subdirectories) total += dir->getTotalFiles();
    return total;
}

int Directory::getTotalDirectories() const {
    int total = 1;
    for (const auto& dir : subdirectories) total += dir->getTotalDirectories();
    return total;
}

int Directory::getElementCount() const {
    return subdirectories.size() + files.size();
}

std::pair<std::string, size_t> Directory::findLargestFileWithPath(const std::string& currentPath) const {
    std::string bestPath;
    size_t bestSize = 0;
    bool found = false;

    for (const auto& file : files) {
        size_t s = file->getSize();
        std::string p = currentPath.empty() ? file->getName() : currentPath + "\\" + file->getName();
        if (!found || s > bestSize) {
            found = true;
            bestSize = s;
            bestPath = p;
        }
    }

    for (const auto& dir : subdirectories) {
        std::string subPath = currentPath.empty() ? dir->getName() : currentPath + "\\" + dir->getName();
        auto [childPath, childSize] = dir->findLargestFileWithPath(subPath);
        if (!childPath.empty() && (!found || childSize > bestSize)) {
            found = true;
            bestSize = childSize;
            bestPath = childPath;
        }
    }

    return found ? std::make_pair(bestPath, bestSize) : std::make_pair("", 0);
}
void Directory::findAllDirectories(const std::string& name, std::list<std::string>& paths, const std::string& currentPath) {
    std::string newPath = currentPath.empty() ? name : currentPath + "\\" + name;
    if (this->name == name) {
        paths.push_back(newPath);
    }

    for (const auto& dir : subdirectories) {
        dir->findAllDirectories(name, paths, newPath);
    }
}