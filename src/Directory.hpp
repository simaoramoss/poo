#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include <string>
#include <vector>
#include <memory>
#include <list>
#include <ostream>
#include "File.hpp"

class Directory {
private:
    std::string name;
    std::vector<std::shared_ptr<Directory>> subdirectories;
    std::vector<std::shared_ptr<File>> files;
    Directory* parent;

public:
    Directory(const std::string& name, Directory* parent = nullptr);

    std::string getName() const;
    void setName(const std::string& newName);
    const std::vector<std::shared_ptr<Directory>>& getSubdirectories() const;
    const std::vector<std::shared_ptr<File>>& getFiles() const;
    Directory* getParent() const;

    void addSubdirectory(const std::string& name);
    void addSubdirectoryPtr(std::shared_ptr<Directory> dir);
    std::shared_ptr<Directory> takeSubdirectory(const std::string& name);
    void addFile(const std::string& name, size_t size);
    void addFilePtr(std::shared_ptr<File> fptr);
    void removeSubdirectory(const std::string& name);
    void removeFile(const std::string& name);
    std::shared_ptr<Directory> findSubdirectory(const std::string& name) const;
    void setParent(Directory* p);
    std::shared_ptr<File> findFile(const std::string& name) const;
    void listContents() const;
    size_t getTotalSize() const;
    int getTotalFiles() const;
    int getTotalDirectories() const;
    int getElementCount() const;
    std::shared_ptr<File> findLargestFile() const;
    std::pair<std::string, size_t> findLargestFileWithPath(const std::string& currentPath = "") const;
    void findAllDirectories(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    void findAllFiles(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    bool containsFile(const std::string& name) const;
    void generateTree(std::ostream& out, const std::string& prefix = "") const;
    bool isSubdirectoryOf(const Directory* other) const;
};

#endif // DIRECTORY_HPP
