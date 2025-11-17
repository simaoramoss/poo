// SistemaFicheiros.cpp
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

        // Iterador recursivo com skip_permission_denied
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
                // entry é exatamente a basePath (a raiz carregada) — ignorar
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
                    // Não conseguimos ler o tamanho — simplesmente ignorar este ficheiro
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
                            // Se faltar uma diretoria na árvore, cria-a (mais robusto)
                            dir->addSubdirectory(segment);
                            subdir = dir->findSubdirectory(segment);
                        }
                        dir = subdir;
                    }
                }
                // adiciona o ficheiro
                dir->addFile(entryPath.filename().string(), fileSize);
            }
        } // fim for iterator

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
    // cast seguro: se te preocupa overflow, muda a assinatura para size_t
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

    // recolher nomes até à raiz
    std::vector<std::string> parts;
    Directory* cur = dir;
    while (cur) {
        parts.push_back(cur->getName());
        cur = cur->getParent();
    }
    // construir caminho na ordem correta (da raiz para a folha)
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

    // BFS with path tracking using fs::path (no in-place mutation)
    std::queue<std::pair<std::shared_ptr<Directory>, fs::path>> queue;
    queue.push({root, fs::path(root->getName())});

    while (!queue.empty()) {
        auto [current, currentPath] = queue.front();
        queue.pop();

        // Check files in current directory (without modifying currentPath)
        for (const auto& file : current->getFiles()) {
            size_t sz = file->getSize();
            fs::path p = currentPath / file->getName();
            if (!found || sz > maxSize) {
                found = true;
                maxSize = sz;
                maxPath = p.string();
            }
        }

        // Enqueue subdirectories with their paths
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

bool SistemaFicheiros::RemoverAll(const std::string &s, const std::string &tipo) {
    if (!root) return false;

    bool removed = false;

    std::function<void(std::shared_ptr<Directory>)> dfs = [&](std::shared_ptr<Directory> dir) {
        if (!dir) return;

        // Remove files with name == s in this directory (if tipo != "DIR")
        if (tipo != "DIR") {
            while (dir->findFile(s)) {
                dir->removeFile(s);
                removed = true;
            }
        }

        // Iterate over a copy of subdirectories to allow mutating the container
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

        // files (name + size)
        for (const auto &f : dir->getFiles()) {
            ofs << ind << "  <File name=\"" << escapeXml(f->getName())
                << "\" size=\"" << f->getSize() << "\" />\n";
        }

        // subdirectories
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

        // Limpar o sistema de ficheiros existente
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

        // Ler o ficheiro completo
        std::string xmlContent;
        std::string line;
        while (std::getline(ifs, line)) {
            xmlContent += line + "\n";
        }
        ifs.close();

        // Stack para rastrear diretórios
        std::stack<Directory*> dirStack;
        size_t pos = 0;

        while (pos < xmlContent.length()) {
            size_t tagStart = xmlContent.find('<', pos);
            if (tagStart == std::string::npos) break;

            size_t tagEnd = xmlContent.find('>', tagStart);
            if (tagEnd == std::string::npos) break;

            std::string tag = xmlContent.substr(tagStart + 1, tagEnd - tagStart - 1);

            // Ignorar XML declaration
            if (!tag.empty() && tag[0] == '?') {
                pos = tagEnd + 1;
                continue;
            }

            // Fechar diretoria </Directory>
            if (!tag.empty() && tag[0] == '/') {
                if (!dirStack.empty()) dirStack.pop();
                pos = tagEnd + 1;
                continue;
            }

            // Ficheiro <File .../>
            if (tag.rfind("File", 0) == 0) {
                std::string name = unescapeXml(extractAttribute(tag, "name"));
                std::string sizeStr = extractAttribute(tag, "size");

                if (!name.empty() && !sizeStr.empty() && !dirStack.empty()) {
                    size_t fileSize = 0;
                    try { fileSize = std::stoull(sizeStr); }
                    catch (...) { fileSize = 0; }
                    dirStack.top()->addFile(name, fileSize);
                }
                pos = tagEnd + 1;
                continue;
            }

            // Abrir diretoria <Directory name="...">
            if (tag.rfind("Directory", 0) == 0) {
                std::string dirName = unescapeXml(extractAttribute(tag, "name"));

                if (!dirName.empty()) {
                    if (root == nullptr) {
                        root = std::make_shared<Directory>(dirName);
                        dirStack.push(root.get());
                    } else {
                        Directory *currentDir = dirStack.top();
                        currentDir->addSubdirectory(dirName);
                        auto newDir = currentDir->findSubdirectory(dirName);
                        if (newDir) dirStack.push(newDir.get());
                        else {
                            // Shouldn't happen, but avoid crash
                            pos = tagEnd + 1;
                            continue;
                        }
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
        // BFS for directory
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
        // search for file
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
