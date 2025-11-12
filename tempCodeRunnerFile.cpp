#include "SistemaFicheiros.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <queue>
#include <sstream>
#include <iostream>
#include <map>
#include <regex>

namespace fs = std::filesystem;

SistemaFicheiros::SistemaFicheiros() : root(nullptr) {}

SistemaFicheiros::~SistemaFicheiros() {
    clearSystem();
}

void SistemaFicheiros::clearSystem() {
    root = nullptr;
}

bool SistemaFicheiros::Load(const std::string& path) {
    try {
        if (!fs::exists(path)) return false;
        
        clearSystem();
        root = std::make_shared<Directory>(fs::path(path).filename().string());
        
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            std::string currentPath = entry.path().string();
            std::string relativePath = currentPath.substr(path.length() + 1);
            
            if (entry.is_directory()) {
                // Create directory structure
                auto dir = root;
                std::istringstream pathStream(relativePath);
                std::string segment;
                
                while (std::getline(pathStream, segment, '\\')) {
                    auto subdir = dir->findSubdirectory(segment);
                    if (!subdir) {
                        dir->addSubdirectory(segment);
                        dir = dir->findSubdirectory(segment);
                    } else {
                        dir = subdir;
                    }
                }
            } else {
                // Add file to appropriate directory
                auto dir = root;
                fs::path filePath(relativePath);
                std::string fileName = filePath.filename().string();
                size_t fileSize = fs::file_size(entry.path());
                
                if (filePath.has_parent_path()) {
                    std::string dirPath = filePath.parent_path().string();
                    std::istringstream pathStream(dirPath);
                    std::string segment;
                    
                    while (std::getline(pathStream, segment, '\\')) {
                        dir = dir->findSubdirectory(segment);
                    }
                }
                
                dir->addFile(fileName, fileSize);
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

int SistemaFicheiros::ContarFicheiros() {
    if (!root) return 0;
    return root->getTotalFiles();
}

int SistemaFicheiros::ContarDirectorias() {
    if (!root) return 0;
    return root->getTotalDirectories();
}

int SistemaFicheiros::Memoria() {
    if (!root) return 0;
    return static_cast<int>(root->getTotalSize());
}

std::string* SistemaFicheiros::DirectoriaMaisElementos() {
    if (!root) return nullptr;
    
    std::shared_ptr<Directory> maxDir = root;
    int maxElements = root->getElementCount();
    
    std::queue<std::shared_ptr<Directory>> queue;
    queue.push(root);
    
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        
        int currentElements = current->getElementCount();
        if (currentElements > maxElements) {
            maxElements = currentElements;
            maxDir = current;
        }
        
        for (const auto& subdir : current->getSubdirectories()) {
            queue.push(subdir);
        }
    }
    
    return new std::string(getAbsolutePath(maxDir.get()));
}

// Implementação similar para outros métodos...

std::string SistemaFicheiros::getAbsolutePath(Directory* dir) const {
    std::string path;
    while (dir) {
        if (!path.empty()) {
            path = std::string ("\\") + path;
        }
        path = dir->getName() + path;
        dir = dir->getParent();
    }
    return path;
}

void SistemaFicheiros::getAllDirectories(std::shared_ptr<Directory> dir, 
                                       std::list<std::shared_ptr<Directory>>& dirs) const {
    if (!dir) return;
    
    dirs.push_back(dir);
    for (const auto& subdir : dir->getSubdirectories()) {
        getAllDirectories(subdir, dirs);
    }
}

void SistemaFicheiros::getAllFiles(std::shared_ptr<Directory> dir,
                                 std::list<std::shared_ptr<File>>& files) const {
    if (!dir) return;
    
    for (const auto& file : dir->getFiles()) {
        files.push_back(file);
    }
    
    for (const auto& subdir : dir->getSubdirectories()) {
        getAllFiles(subdir, files);
    }
}

