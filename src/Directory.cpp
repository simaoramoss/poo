#include "Directory.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <sstream>

Directory::Directory(const std::string& name, Directory* parent)
    : name(name), parent(parent) {}

std::string Directory::getName() const {
    return name;
}

void Directory::setName(const std::string& newName) {
    name = newName;
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

void Directory::addSubdirectoryPtr(std::shared_ptr<Directory> dir) {
    if (!dir) return;
    dir->setParent(this);
    subdirectories.push_back(dir);
}

std::shared_ptr<Directory> Directory::takeSubdirectory(const std::string& name) {
    auto it = std::find_if(subdirectories.begin(), subdirectories.end(),
        [&name](const auto& dir) { return dir->getName() == name; });
    if (it == subdirectories.end()) return nullptr;
    std::shared_ptr<Directory> ptr = *it;
    subdirectories.erase(it);
    return ptr;
}

void Directory::addFile(const std::string& name, size_t size) {
    auto newFile = std::make_shared<File>(name, size);
    files.push_back(newFile);
}

void Directory::addFilePtr(std::shared_ptr<File> fptr) {
    if (!fptr) return;
    files.push_back(fptr);
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

void Directory::setParent(Directory* p) {
    parent = p;
}

std::shared_ptr<File> Directory::findFile(const std::string& name) const {
    auto it = std::find_if(files.begin(), files.end(),
        [&name](const auto& f) { return f->getName() == name; });
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
        std::cout << "  " << file->getName() << " (" << file->getSize() << " bytes)";
        if (!file->getDate().empty()) std::cout << " - " << file->getDate();
        std::cout << "\n";
    }
}

size_t Directory::getTotalSize() const {
    size_t total = 0;
    for (const auto& file : files) total += file->getSize();
    for (const auto& dir : subdirectories) total += dir->getTotalSize();
    return total;
}

int Directory::getTotalFiles() const {
    int total = static_cast<int>(files.size());
    for (const auto& dir : subdirectories) total += dir->getTotalFiles();
    return total;
}

int Directory::getTotalDirectories() const {
    int total = 1; // conta-se a própria
    for (const auto& dir : subdirectories) total += dir->getTotalDirectories();
    return total;
}

int Directory::getElementCount() const {
    return static_cast<int>(subdirectories.size() + files.size());
}

std::shared_ptr<File> Directory::findLargestFile() const {
    std::shared_ptr<File> best = nullptr;
    size_t bestSize = 0;
    for (const auto& f : files) {
        if (!best || f->getSize() > bestSize) {
            best = f;
            bestSize = f->getSize();
        }
    }
    for (const auto& d : subdirectories) {
        auto cand = d->findLargestFile();
        if (cand && (!best || cand->getSize() > bestSize)) {
            best = cand;
            bestSize = cand->getSize();
        }
    }
    return best;
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

    return found ? std::make_pair(bestPath, bestSize) : std::make_pair(std::string(), 0u);
}

void Directory::findAllDirectories(const std::string& name, std::list<std::string>& paths, const std::string& currentPath) {
    std::string newPath = currentPath.empty() ? this->name : currentPath + "\\" + this->name;
    if (this->name == name) {
        paths.push_back(newPath);
    }
    for (const auto& d : subdirectories) {
        d->findAllDirectories(name, paths, newPath);
    }
}

void Directory::findAllFiles(const std::string& name, std::list<std::string>& paths, const std::string& currentPath) {
    std::string base = currentPath.empty() ? this->name : currentPath + "\\" + this->name;
    for (const auto& f : files) {
        if (f->getName() == name) {
            paths.push_back(base + "\\" + f->getName());
        }
    }
    for (const auto& d : subdirectories) {
        d->findAllFiles(name, paths, base);
    }
}

bool Directory::containsFile(const std::string& name) const {
    for (const auto& f : files) if (f->getName() == name) return true;
    for (const auto& d : subdirectories) if (d->containsFile(name)) return true;
    return false;
}

void Directory::generateTree(std::ostream& out, const std::string& prefix) const {
    out << prefix << getName() << "/\n";
    std::string childPrefix = prefix + "  ";
    for (const auto& f : files) {
        out << childPrefix << f->getName() << " (" << f->getSize() << ")\n";
    }
    for (const auto& d : subdirectories) {
        d->generateTree(out, childPrefix);
    }
}

bool Directory::isSubdirectoryOf(const Directory* other) const {
    const Directory* cur = parent;
    while (cur) {
        if (cur == other) return true;
        cur = cur->getParent();
    }
    return false;
}
