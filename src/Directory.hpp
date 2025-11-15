#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <string>
#include <vector>
#include <memory>
#include <list>
#include "File.hpp"

class Directory {
private:
    std::string name;
    std::vector<std::shared_ptr<Directory>> subdirectories;
    std::vector<std::shared_ptr<File>> files;
    Directory* parent;

public:
    Directory(const std::string& name, Directory* parent = nullptr);
    
    // Getters
    std::string getName() const;
    void setName(const std::string& newName);
    const std::vector<std::shared_ptr<Directory>>& getSubdirectories() const;
    const std::vector<std::shared_ptr<File>>& getFiles() const;
    Directory* getParent() const;
    
    // Basic Operations
    void addSubdirectory(const std::string& name);
    void addFile(const std::string& name, size_t size);
    void removeSubdirectory(const std::string& name);
    void removeFile(const std::string& name);
    std::shared_ptr<Directory> findSubdirectory(const std::string& name) const;
    std::shared_ptr<File> findFile(const std::string& name) const;
    void listContents() const;
    
    // Advanced Operations
    size_t getTotalSize() const;
    int getTotalFiles() const;
    int getTotalDirectories() const;
    int getElementCount() const; // conta arquivos + diretorios (sem contar subdiretorios)
    std::shared_ptr<File> findLargestFile() const;
    // Retorna o caminho (relativo a esta diretoria) e tamanho do maior ficheiro
    std::pair<std::string, size_t> findLargestFileWithPath(const std::string& currentPath = "") const;
    void findAllDirectories(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    void findAllFiles(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    bool containsFile(const std::string& name) const;
    void generateTree(std::ostream& out, const std::string& prefix = "") const;
    bool isSubdirectoryOf(const Directory* other) const;
};

#endif