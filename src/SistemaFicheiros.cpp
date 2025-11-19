#include "SistemaFicheiros.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <queue>
#include <functional>
#include <sstream>
#include <iostream>
#include <optional>
#include <stack>
#include <vector>
#include <system_error>

namespace fs = std::filesystem;

SistemaFicheiros::SistemaFicheiros() : root(nullptr) {}
SistemaFicheiros::~SistemaFicheiros() { clearSystem(); }

void SistemaFicheiros::clearSystem() {
    root = nullptr;
}

bool SistemaFicheiros::Load(const std::string& pathStr) {
    try {
        fs::path basePath(pathStr);
        if (!fs::exists(basePath)) return false;

        clearSystem();
        root = std::make_shared<Directory>(basePath.filename().string());

        // Lista de diretórios e ficheiros a ignorar
        static const std::vector<std::string> ignoreDirs = { ".git", ".vscode", "bin", "obj", "build" };
        static const std::vector<std::string> ignoreFiles = { ".gitignore", ".DS_Store" };

        for (auto it = fs::recursive_directory_iterator(basePath, fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it)
        {
            const auto& entry = *it;
            fs::path entryPath = entry.path();
            std::string filename = entryPath.filename().string();

            // Ignorar diretórios inteiros (não entrar neles)
            if (entry.is_directory() &&
                std::find(ignoreDirs.begin(), ignoreDirs.end(), filename) != ignoreDirs.end())
            {
                it.disable_recursion_pending();
                continue;
            }

            // Ignorar ficheiros indesejados por nome/extensão
            if (!entry.is_directory()) {
                std::string ext = entryPath.extension().string();
                if (ext == ".exe" ||
                    std::find(ignoreFiles.begin(), ignoreFiles.end(), filename) != ignoreFiles.end())
                {
                    continue;
                }
            }

            // calcular caminho relativo de forma segura
            fs::path rel = entryPath.lexically_relative(basePath);
            if (rel.empty()) {
                continue;
            }

            if (entry.is_directory()) {
                // cria estrutura de diretórios (componentes de rel)
                auto dir = root;
                for (const auto& part : rel) {
                    std::string segment = part.string();
                    auto subdir = dir->findSubdirectory(segment);
                    if (!subdir) {
                        dir->addSubdirectory(segment);
                        subdir = dir->findSubdirectory(segment);
                    }
                    dir = subdir;
                }
            } else {
                // ficheiro: obter tamanho com error_code para evitar exceptions
                std::error_code ec;
                auto fileSize = fs::file_size(entryPath, ec);
                if (ec) {
                    continue;
                }

                // localizar/garantir a diretoria onde o ficheiro deve ser adicionado
                auto dir = root;
                fs::path parent = rel.parent_path();
                if (!parent.empty()) {
                    for (const auto& part : parent) {
                        std::string segment = part.string();
                        auto subdir = dir->findSubdirectory(segment);
                        if (!subdir) {
                            // Se faltar uma diretoria na árvore, cria-a.
                            dir->addSubdirectory(segment);
                            subdir = dir->findSubdirectory(segment);
                        }
                        dir = subdir;
                    }
                }
                // adiciona o ficheiro
                dir->addFile(entryPath.filename().string(), fileSize);
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao carregar sistema de ficheiros: " << e.what() << "\n";
        return false;
    } catch (...) {
        std::cerr << "Erro desconhecido ao carregar sistema de ficheiros\n";
        return false;
    }
}

int SistemaFicheiros::ContarFicheiros() const {
    if (!root) return 0;
    return root->getTotalFiles();
}

int SistemaFicheiros::ContarDirectorias() const {
    if (!root) return 0;
    return root->getTotalDirectories();
}

int SistemaFicheiros::Memoria() const {
    if (!root) return 0;
    return static_cast<int>(root->getTotalSize());
}

std::optional<std::string> SistemaFicheiros::DirectoriaMaisElementos() const {
    if (!root) return std::nullopt;

    std::shared_ptr<Directory> maxDir = root;
    int maxElements = root->getElementCount();

    std::queue<std::shared_ptr<Directory>> queue;
    queue.push(root);

    while (!queue.empty()) {
        auto current = queue.front(); queue.pop();
        int currentElements = current->getElementCount();
        if (currentElements > maxElements) {
            maxElements = currentElements;
            maxDir = current;
        }
        for (const auto& subdir : current->getSubdirectories()) {
            queue.push(subdir);
        }
    }

    return getAbsolutePath(maxDir.get());
}

std::optional<std::string> SistemaFicheiros::DirectoriaMenosElementos() const {
    if (!root) return std::nullopt;

    std::shared_ptr<Directory> minDir = root;
    int minElements = root->getElementCount();

    std::queue<std::shared_ptr<Directory>> queue;
    queue.push(root);

    while (!queue.empty()) {
        auto current = queue.front(); queue.pop();
        int currentElements = current->getElementCount();
        if (currentElements < minElements) {
            minElements = currentElements;
            minDir = current;
        }
        for (const auto& subdir : current->getSubdirectories()) {
            queue.push(subdir);
        }
    }

    return getAbsolutePath(minDir.get());
}

std::optional<std::string> SistemaFicheiros::DirectoriaMaisEspaco() const {
    if (!root) return std::nullopt;

    std::shared_ptr<Directory> bestDir = root;
    size_t bestSize = root->getTotalSize();

    std::queue<std::shared_ptr<Directory>> q;
    q.push(root);

    while (!q.empty()) {
        auto current = q.front(); q.pop();
        size_t curSize = current->getTotalSize();
        if (curSize > bestSize) {
            bestSize = curSize;
            bestDir = current;
        }
        for (const auto& subdir : current->getSubdirectories()) {
            q.push(subdir);
        }
    }

    std::string out = getAbsolutePath(bestDir.get()) + " (" + std::to_string(bestSize) + " bytes)";
    return out;
}

std::string SistemaFicheiros::getAbsolutePath(Directory* dir) const {
    if (!dir) return std::string();

   
    std::vector<std::string> parts;
    Directory* cur = dir;
    while (cur) {
        parts.push_back(cur->getName());
        cur = cur->getParent();
    }
   
    fs::path p;
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
        p /= *it;
    }
    return p.string();
}

std::optional<std::string> SistemaFicheiros::FicheiroMaior() const {
    if (!root) return std::nullopt;

    std::string maxPath;
    size_t maxSize = 0;
    bool found = false;

    std::queue<std::pair<std::shared_ptr<Directory>, fs::path>> queue;
    queue.push({root, fs::path(root->getName())});

    while (!queue.empty()) {
        auto [current, currentPath] = queue.front();
        queue.pop();

        for (const auto& file : current->getFiles()) {
            size_t sz = file->getSize();
            fs::path p = currentPath / file->getName();
            if (!found || sz > maxSize) {
                found = true;
                maxSize = sz;
                maxPath = p.string();
            }
        }
        for (const auto& subdir : current->getSubdirectories()) {
            fs::path childPath = currentPath / subdir->getName();
            queue.push({subdir, childPath});
        }
    }

    if (!found) return std::nullopt;

    std::string result = maxPath + " (" + std::to_string(maxSize) + " bytes)";
    return result;
}

void SistemaFicheiros::getAllDirectories(std::shared_ptr<Directory> dir,
                                         std::list<std::shared_ptr<Directory>>& dirs) const {
    if (!dir) return;
    dirs.push_back(dir);
    for (const auto& subdir : dir->getSubdirectories()) {
        getAllDirectories(subdir, dirs);
    }
}

void SistemaFicheiros::getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const {
    if (!dir) return;
    for (const auto& file : dir->getFiles()) {
        files.push_back(file);
    }
    for (const auto& subdir : dir->getSubdirectories()) {
        getAllFiles(subdir, files);
    }
}

bool SistemaFicheiros::RemoverAll(const std::string &s, const std::string &tipo) {
    if (!root) return false;

    bool removed = false;

    std::function<void(std::shared_ptr<Directory>)> dfs = [&](std::shared_ptr<Directory> dir) {
        if (!dir) return;

     
        if (tipo != "DIR") {
            while (dir->findFile(s)) {
                dir->removeFile(s);
                removed = true;
            }
        }
        auto subs = dir->getSubdirectories();
        for (const auto& sub : subs) {
            if (tipo == "DIR" && sub->getName() == s) {
                dir->removeSubdirectory(s);
                removed = true;
            } else {
                dfs(sub);
            }
        }
    };

    dfs(root);
    return removed;
}

bool SistemaFicheiros::MoveFicheiro(const std::string &Fich, const std::string &DirNova) {
    if (!root) return false;

    std::queue<std::shared_ptr<Directory>> q;
    q.push(root);
    std::shared_ptr<Directory> sourceDir = nullptr;
    std::shared_ptr<File> filePtr = nullptr;

    while (!q.empty() && !filePtr) {
        auto cur = q.front(); q.pop();
        for (const auto &f : cur->getFiles()) {
            if (f->getName() == Fich) {
                sourceDir = cur;
                filePtr = f;
                break;
            }
        }
        for (const auto &sub : cur->getSubdirectories()) q.push(sub);
    }

    if (!filePtr || !sourceDir) return false; 

    
    std::shared_ptr<Directory> destDir = nullptr;
    bool isPath = (DirNova.find('\\') != std::string::npos) || (DirNova.find('/') != std::string::npos);
    if (isPath) {
        std::vector<std::string> parts;
        std::string token;
        for (char c : DirNova) {
            if (c == '\\' || c == '/') {
                if (!token.empty()) { parts.push_back(token); token.clear(); }
            } else token.push_back(c);
        }
        if (!token.empty()) parts.push_back(token);

       
        std::shared_ptr<Directory> cur = root;
        for (const auto &p : parts) {
            auto next = cur->findSubdirectory(p);
            if (!next) { cur = nullptr; break; }
            cur = next;
        }
        destDir = cur;
    } else {
        std::queue<std::shared_ptr<Directory>> q2;
        q2.push(root);
        while (!q2.empty() && !destDir) {
            auto cur = q2.front(); q2.pop();
            if (cur->getName() == DirNova) {
                destDir = cur;
                break;
            }
            for (const auto &sub : cur->getSubdirectories()) q2.push(sub);
        }
    }

    if (!destDir) return false; // não encontrou

    // Não faz nada se tiver na mesma diretoria
    if (sourceDir.get() == destDir.get()) return false;

    // Se o destino já tiver um ficheiro com o mesmo nome, não faz nada.
    if (destDir->findFile(filePtr->getName())) return false;

    // Preserva tamanho e data
    destDir->addFile(filePtr->getName(), filePtr->getSize());
    auto added = destDir->findFile(filePtr->getName());
    if (added) added->setDate(filePtr->getDate());

    sourceDir->removeFile(filePtr->getName());

    return true;
}

bool SistemaFicheiros::MoverDirectoria(const std::string &DirOld, const std::string &DirNew) {
    if (!root) return false;

    std::shared_ptr<Directory> found = nullptr;
    std::shared_ptr<Directory> parentOfFound = nullptr;
    std::queue<std::pair<std::shared_ptr<Directory>, std::shared_ptr<Directory>>> q;
    q.push({root, nullptr});
    while (!q.empty() && !found) {
        auto [cur, parent] = q.front(); q.pop();
        if (cur->getName() == DirOld) {
            found = cur; parentOfFound = parent; break;
        }
        for (const auto &sub : cur->getSubdirectories()) q.push({sub, cur});
    }

    if (!found) return false; 

    if (!parentOfFound) {
        // Não mover raiz que é a DirOld
        return false;
    }

    // Encontra DirNew
    std::shared_ptr<Directory> dest = nullptr;
    bool isPath = (DirNew.find('\\') != std::string::npos) || (DirNew.find('/') != std::string::npos);
    if (isPath) {
        std::vector<std::string> parts;
        std::string token;
        for (char c : DirNew) {
            if (c == '\\' || c == '/') { if (!token.empty()) { parts.push_back(token); token.clear(); } }
            else token.push_back(c);
        }
        if (!token.empty()) parts.push_back(token);

        std::shared_ptr<Directory> cur = root;
        for (const auto &p : parts) {
            auto next = cur->findSubdirectory(p);
            if (!next) { cur = nullptr; break; }
            cur = next;
        }
        dest = cur;
    } else {
        std::queue<std::shared_ptr<Directory>> q2; q2.push(root);
        while (!q2.empty() && !dest) {
            auto cur = q2.front(); q2.pop();
            if (cur->getName() == DirNew) { dest = cur; break; }
            for (const auto &sub : cur->getSubdirectories()) q2.push(sub);
        }
    }

    if (!dest) return false;

    // Verifica que destino nao é sub da DirOld
    Directory* walker = dest.get();
    while (walker) {
        if (walker == found.get()) {
            return false;
        }
        walker = walker->getParent();
    }

   
    auto movedPtr = parentOfFound->takeSubdirectory(found->getName());
    if (!movedPtr) return false; 

    dest->addSubdirectoryPtr(movedPtr);
    return true;
}

void SistemaFicheiros::SetRoot(std::shared_ptr<Directory> r) {
    root = r;
}

std::shared_ptr<Directory> SistemaFicheiros::GetRoot() const {
    return root;
}

void SistemaFicheiros::Escrever_XML(const std::string &s) {
    if (!root) return;

    auto escapeXml = [](const std::string &in) {
        std::string out;
        out.reserve(in.size());
        for (char c : in) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&apos;"; break;
                default: out += c; break;
            }
        }
        return out;
    };

    std::ofstream ofs(s);
    if (!ofs.is_open()) return;

    ofs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    std::function<void(const std::shared_ptr<Directory>&, int)> writeDir;
    writeDir = [&](const std::shared_ptr<Directory> &dir, int indent) {
        std::string ind(indent, ' ');
        ofs << ind << "<Directory name=\"" << escapeXml(dir->getName()) << "\">\n";

        for (const auto &f : dir->getFiles()) {
            ofs << ind << "  <File name=\"" << escapeXml(f->getName())
                << "\" size=\"" << f->getSize() << "\" />\n";
        }

        for (const auto &sub : dir->getSubdirectories()) {
            writeDir(sub, indent + 2);
        }

        ofs << ind << "</Directory>\n";
    };

    writeDir(root, 0);
    ofs.close();
}

bool SistemaFicheiros::Ler_XML(const std::string &s) {
    try {
        std::ifstream ifs(s);
        if (!ifs.is_open()) return false;

        clearSystem();

        auto unescapeXml = [](const std::string &in) {
            std::string out;
            out.reserve(in.size());
            size_t pos = 0;
            while (pos < in.size()) {
                if (in[pos] == '&') {
                    if (in.substr(pos, 5) == "&amp;") { out += '&'; pos += 5; }
                    else if (in.substr(pos, 4) == "&lt;") { out += '<'; pos += 4; }
                    else if (in.substr(pos, 4) == "&gt;") { out += '>'; pos += 4; }
                    else if (in.substr(pos, 6) == "&quot;") { out += '"'; pos += 6; }
                    else if (in.substr(pos, 6) == "&apos;") { out += '\''; pos += 6; }
                    else { out += in[pos++]; }
                } else {
                    out += in[pos++];
                }
            }
            return out;
        };

        auto extractAttribute = [](const std::string &line, const std::string &attr) -> std::string {
            std::string pattern = attr + "=\"";
            size_t start = line.find(pattern);
            if (start == std::string::npos) return "";
            start += pattern.length();
            size_t end = line.find("\"", start);
            if (end == std::string::npos) return "";
            return line.substr(start, end - start);
        };


        std::string xmlContent;
        std::string line;
        while (std::getline(ifs, line)) {
            xmlContent += line + "\n";
        }
        ifs.close();

        std::stack<std::shared_ptr<Directory>> dirStack;
        size_t pos = 0;

        while (pos < xmlContent.length()) {
            size_t tagStart = xmlContent.find('<', pos);
            if (tagStart == std::string::npos) break;

            size_t tagEnd = xmlContent.find('>', tagStart);
            if (tagEnd == std::string::npos) break;

            std::string tag = xmlContent.substr(tagStart + 1, tagEnd - tagStart - 1);

     
            if (!tag.empty() && tag[0] == '?') {
                pos = tagEnd + 1;
                continue;
            }

        
            if (!tag.empty() && tag[0] == '/') {
                if (!dirStack.empty()) dirStack.pop();
                pos = tagEnd + 1;
                continue;
            }

         
            if (tag.rfind("File", 0) == 0) {
                std::string name = unescapeXml(extractAttribute(tag, "name"));
                std::string sizeStr = extractAttribute(tag, "size");
                std::string dateStr = extractAttribute(tag, "date");

                if (!name.empty() && !sizeStr.empty() && !dirStack.empty()) {
                    size_t fileSize = 0;
                    try { fileSize = std::stoull(sizeStr); }
                    catch (...) { fileSize = 0; }

                    auto parentDir = dirStack.top();
                    parentDir->addFile(name, fileSize);
                    if (!dateStr.empty()) {
                        auto fptr = parentDir->findFile(name);
                        if (fptr) fptr->setDate(unescapeXml(dateStr));
                    }
                }
                pos = tagEnd + 1;
                continue;
            }
            if (tag.rfind("Directory", 0) == 0) {
                std::string dirName = unescapeXml(extractAttribute(tag, "name"));

                if (!dirName.empty()) {
                    if (root == nullptr) {
                        root = std::make_shared<Directory>(dirName);
                        dirStack.push(root);
                    } else if (!dirStack.empty()) {
                        auto currentDir = dirStack.top();
                        currentDir->addSubdirectory(dirName);
                        auto newDir = currentDir->findSubdirectory(dirName);
                        if (newDir) dirStack.push(newDir);
                        else {
                            // Evita crash
                            pos = tagEnd + 1;
                            continue;
                        }
                    } else {
                        auto dummyParent = root;
                        dummyParent->addSubdirectory(dirName);
                        auto newDir = dummyParent->findSubdirectory(dirName);
                        if (newDir) dirStack.push(newDir);
                    }
                }
                pos = tagEnd + 1;
                continue;
            }

            pos = tagEnd + 1;
        }

        return root != nullptr;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao ler XML: " << e.what() << "\n";
        return false;
    } catch (...) {
        std::cerr << "Erro desconhecido ao ler XML\n";
        return false;
    }
}

std::optional<std::string> SistemaFicheiros::Search(const std::string &s, int Tipo) const {
    if (!root) return std::nullopt;

    // Tipo: 1 = diretoria, 0 = ficheiro
    if (Tipo == 1) {
        std::queue<std::shared_ptr<Directory>> q;
        q.push(root);
        while (!q.empty()) {
            auto cur = q.front(); q.pop();
            if (cur->getName() == s) {
                return getAbsolutePath(cur.get());
            }
            for (const auto &sub : cur->getSubdirectories()) q.push(sub);
        }
        return std::nullopt;
    } else {
       
        std::queue<std::pair<std::shared_ptr<Directory>, fs::path>> q;
        q.push({root, fs::path(root->getName())});
        while (!q.empty()) {
            auto [cur, curPath] = q.front(); q.pop();
            for (const auto &file : cur->getFiles()) {
                if (file->getName() == s) {
                    fs::path p = curPath / file->getName();
                    return p.string();
                }
            }
            for (const auto &sub : cur->getSubdirectories()) {
                q.push({sub, curPath / sub->getName()});
            }
        }
        return std::nullopt;
    }
}
