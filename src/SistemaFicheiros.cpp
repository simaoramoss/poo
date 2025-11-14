#include "SistemaFicheiros.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <queue>
#include <sstream>
#include <iostream>
#include <map>
#include <regex>
#include <climits>

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

        // Lista de diretórios e ficheiros a ignorar
        static const std::vector<std::string> ignoreDirs = { ".git", ".vscode", "bin", "obj", "build" };
        static const std::vector<std::string> ignoreFiles = { ".gitignore", ".DS_Store" };

        // Iterador recursivo
        for (auto it = fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it)
        {
            const auto& entry = *it;
            std::string currentPath = entry.path().string();
            std::string fileName = entry.path().filename().string();

            // Ignorar diretórios inteiros
            if (entry.is_directory() &&
                std::find(ignoreDirs.begin(), ignoreDirs.end(), fileName) != ignoreDirs.end())
            {
                it.disable_recursion_pending(); // não entra na pasta
                continue;
            }

            // Ignorar ficheiros indesejados
            if (!entry.is_directory() &&
                (entry.path().extension() == ".exe" ||
                 std::find(ignoreFiles.begin(), ignoreFiles.end(), fileName) != ignoreFiles.end()))
            {
                continue;
            }

            std::string relativePath = currentPath.substr(path.length() + 1);

            if (entry.is_directory()) {
                // Cria estrutura de diretórios
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
                // Adiciona ficheiro à diretoria correta
                auto dir = root.get();  // Começa na raiz
                fs::path filePath(relativePath);
                size_t fileSize = fs::file_size(entry.path());

                if (filePath.has_parent_path()) {
                    std::string dirPath = filePath.parent_path().string();
                    std::istringstream pathStream(dirPath);
                    std::string segment;

                    while (std::getline(pathStream, segment, '\\')) {
                        auto subdir = dir->findSubdirectory(segment);
                        if (subdir) {
                            dir = subdir.get();
                        } else {
                            dir = nullptr;
                            break; // diretório não encontrado, ignora ficheiro
                        }
                    }
                }

                if (dir != nullptr) {
                    dir->addFile(fileName, fileSize);
                }
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

std::string* SistemaFicheiros::DirectoriaMenosElementos() {
    if (!root) return nullptr;
    
    std::shared_ptr<Directory> minDir = root;  // Inicializa com root em vez de nullptr
    int minElements = root->getElementCount();
    
    std::queue<std::shared_ptr<Directory>> queue;
    queue.push(root);
    
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        
        int currentElements = current->getElementCount();
        if (currentElements < minElements) {
            minElements = currentElements;
            minDir = current;
        }
        
        for (const auto& subdir : current->getSubdirectories()) {
            queue.push(subdir);
        }
    }
    
    if (!minDir) return nullptr;  // Verificação de segurança
    return new std::string(getAbsolutePath(minDir.get()));
}


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

std::string* SistemaFicheiros::FicheiroMaior() {
    if (!root) return nullptr;

    std::string maxPath;
    size_t maxSize = 0;
    bool found = false;

    std::queue<std::pair<std::shared_ptr<Directory>, std::string>> queue;
    queue.push({root, root->getName()});

    while (!queue.empty()) {
        auto [current, currentPath] = queue.front();
        queue.pop();

        // Verifica todos os ficheiros desta diretoria
        for (const auto& file : current->getFiles()) {
            if (!found || file->getSize() > maxSize) {
                found = true;
                maxSize = file->getSize();
                maxPath = currentPath + "\\" + file->getName();
            }
        }

        // Percorrer subdiretorias
        for (const auto& subdir : current->getSubdirectories()) {
            queue.push({subdir, currentPath + "\\" + subdir->getName()});
        }
    }

    if (!found) return nullptr;  // Não há ficheiros

    // Construir string final
    std::string result = maxPath + " (" + std::to_string(maxSize) + " bytes)";
    return new std::string(result);
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

