#include "SistemaFicheiros.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <queue>
#include <functional>
#include <sstream>
#include <iostream>
#include <map>
#include <cstdio>
#include <cctype>
#include <optional>
#include <stack>
#include <vector>
#include <system_error>

namespace fs = std::filesystem;

SistemaFicheiros::SistemaFicheiros() : root(nullptr) {}
// Libertamos referências à raiz para permitir nova carga ou encerramento limpo.
SistemaFicheiros::~SistemaFicheiros() { clearSystem(); }

void SistemaFicheiros::clearSystem() {
    root = nullptr;
}

// Constrói a árvore em memória a partir de uma pasta real do disco.
bool SistemaFicheiros::Load(const std::string& pathStr) {
    try {
        fs::path basePath(pathStr);
        if (!fs::exists(basePath)) return false;

        clearSystem();
        root = std::make_shared<Directory>(basePath.filename().string());

        // Alguns diretórios/ficheiros são ignorados para reduzir ruído.
        static const std::vector<std::string> ignoreDirs = { ".git", ".vscode", "bin", "obj", "build" };
        static const std::vector<std::string> ignoreFiles = { ".gitignore", ".DS_Store" };

        for (auto it = fs::recursive_directory_iterator(basePath, fs::directory_options::skip_permission_denied);
             it != fs::recursive_directory_iterator(); ++it)
        {
            const auto& entry = *it;
            fs::path entryPath = entry.path();
            std::string filename = entryPath.filename().string();

            if (entry.is_directory() &&
                std::find(ignoreDirs.begin(), ignoreDirs.end(), filename) != ignoreDirs.end())
            {
                it.disable_recursion_pending();
                continue;
            }

            if (!entry.is_directory()) {
                std::string ext = entryPath.extension().string();
                if (ext == ".exe" ||
                    std::find(ignoreFiles.begin(), ignoreFiles.end(), filename) != ignoreFiles.end())
                {
                    continue;
                }
            }

            fs::path rel = entryPath.lexically_relative(basePath);
            if (rel.empty()) continue;

            if (entry.is_directory()) {
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
                std::error_code ec;
                auto fileSize = fs::file_size(entryPath, ec);
                if (ec) continue;

                auto dir = root;
                fs::path parent = rel.parent_path();
                if (!parent.empty()) {
                    for (const auto& part : parent) {
                        std::string segment = part.string();
                        auto subdir = dir->findSubdirectory(segment);
                        if (!subdir) {
                            dir->addSubdirectory(segment);
                            subdir = dir->findSubdirectory(segment);
                        }
                        dir = subdir;
                    }
                }

                // Data do ficheiro
                auto ftime = fs::last_write_time(entryPath, ec);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
                std::string dateStr = std::asctime(std::localtime(&cftime));
                dateStr.erase(std::remove(dateStr.begin(), dateStr.end(), '\n'), dateStr.end());

                dir->addFile(entryPath.filename().string(), fileSize);
                auto fptr = dir->findFile(entryPath.filename().string());
                if (fptr) fptr->setDate(dateStr);
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
    return root ? root->getTotalFiles() : 0;
}

int SistemaFicheiros::ContarDirectorios() const {
    return root ? root->getTotalDirectories() : 0;
}

int SistemaFicheiros::Memoria() const {
    return root ? static_cast<int>(root->getTotalSize()) : 0;
}

// Percorre em largura e escolhe a diretoria com mais elementos (dirs+ficheiros).
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

// Percorre em largura e escolhe a diretoria com menos elementos.
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

// Encontra a diretoria que acumula mais espaço total (tamanho recursivo).
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
    return getAbsolutePath(bestDir.get()) + " (" + std::to_string(bestSize) + " bytes)";
}

// Constrói o caminho absoluto (da raiz até ao nó) a partir dos ponteiros parent.
std::string SistemaFicheiros::getAbsolutePath(Directory* dir) const {
    if (!dir) return std::string();
    std::vector<std::string> parts;
    Directory* cur = dir;
    while (cur) {
        parts.push_back(cur->getName());
        cur = cur->getParent();
    }
    fs::path p;
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) p /= *it;
    return p.string();
}

// Procura o ficheiro maior em toda a árvore e devolve caminho + tamanho.
std::optional<std::string> SistemaFicheiros::FicheiroMaior() const {
    if (!root) return std::nullopt;

    std::string maxPath;
    size_t maxSize = 0;
    bool found = false;

    std::queue<std::pair<std::shared_ptr<Directory>, fs::path>> queue;
    queue.push({root, fs::path(root->getName())});

    while (!queue.empty()) {
        auto [current, currentPath] = queue.front(); queue.pop();
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
            queue.push({subdir, currentPath / subdir->getName()});
        }
    }

    if (!found) return std::nullopt;
    return maxPath + " (" + std::to_string(maxSize) + " bytes)";
}

// ----------------------------------------
// GetRoot e SetRoot
void SistemaFicheiros::SetRoot(std::shared_ptr<Directory> r) {
    root = r;
}

std::shared_ptr<Directory> SistemaFicheiros::GetRoot() const {
    return root;
}

// ----------------------------------------
// Métodos auxiliares para listar todos os diretórios/ficheiros de forma recursiva.
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

// ----------------------------------------
// Remover ficheiros ou diretórios
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

// Mover ficheiro
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

    if (!destDir) return false;

    if (sourceDir.get() == destDir.get()) return false;
    if (destDir->findFile(filePtr->getName())) return false;

    destDir->addFile(filePtr->getName(), filePtr->getSize());
    auto added = destDir->findFile(filePtr->getName());
    if (added) added->setDate(filePtr->getDate());

    sourceDir->removeFile(filePtr->getName());

    return true;
}


// Mover uma diretoria (e a sua subárvore) para outra diretoria.
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

    if (!found || !parentOfFound) return false;

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

    Directory* walker = dest.get();
    while (walker) {
        if (walker == found.get()) return false;
        walker = walker->getParent();
    }

    auto movedPtr = parentOfFound->takeSubdirectory(found->getName());
    if (!movedPtr) return false;

    dest->addSubdirectoryPtr(movedPtr);
    return true;
}

// ----------------------------------------
// XML
void SistemaFicheiros::Escrever_XML(const std::string &s) {
    if (!root) return;

    auto escapeXml = [](const std::string &in) {
        std::string out; out.reserve(in.size());
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
                << "\" size=\"" << f->getSize()
                << "\" date=\"" << escapeXml(f->getDate())
                << "\" />\n";
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
            std::string out; out.reserve(in.size());
            size_t pos = 0;
            while (pos < in.size()) {
                if (in[pos] == '&') {
                    if (in.substr(pos, 5) == "&amp;") { out += '&'; pos += 5; }
                    else if (in.substr(pos, 4) == "&lt;") { out += '<'; pos += 4; }
                    else if (in.substr(pos, 4) == "&gt;") { out += '>'; pos += 4; }
                    else if (in.substr(pos, 6) == "&quot;") { out += '"'; pos += 6; }
                    else if (in.substr(pos, 6) == "&apos;") { out += '\''; pos += 6; }
                    else { out += in[pos++]; }
                } else { out += in[pos++]; }
            }
            return out;
        };

        // Extrai o valor de um atributo na linha XML 
        auto extractAttribute = [](const std::string &line, const std::string &attr) -> std::string {
            std::string pattern = attr + "=\"";
            size_t start = line.find(pattern);
            if (start == std::string::npos) return "";
            start += pattern.size();
            size_t end = line.find("\"", start);
            if (end == std::string::npos) return "";
            return line.substr(start, end - start);
        };

        std::stack<std::shared_ptr<Directory>> stk;
        std::string line;
        while (std::getline(ifs, line)) {
            line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
            if (line.find("<Directory") != std::string::npos) {
                std::string name = unescapeXml(extractAttribute(line, "name"));
                auto dir = std::make_shared<Directory>(name);
                if (stk.empty()) root = dir;
                else stk.top()->addSubdirectoryPtr(dir);
                stk.push(dir);
            } else if (line.find("</Directory>") != std::string::npos) {
                if (!stk.empty()) stk.pop();
            } else if (line.find("<File") != std::string::npos) {
                std::string name = unescapeXml(extractAttribute(line, "name"));
                std::string sizeStr = extractAttribute(line, "size");
                std::string dateStr = unescapeXml(extractAttribute(line, "date"));
                size_t size = std::stoull(sizeStr);
                if (!stk.empty()) {
                    stk.top()->addFile(name, size);
                    auto fptr = stk.top()->findFile(name);
                    if (fptr) fptr->setDate(dateStr);
                }
            }
        }

        ifs.close();
        return true;
    } catch (...) { return false; }
}

// Obtém a data guardada para um ficheiro pelo seu nome.
std::optional<std::string> SistemaFicheiros::DataFicheiro(const std::string &Fich) const {
    if (!root) return std::nullopt;
    std::queue<std::shared_ptr<Directory>> q;
    q.push(root);

    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        for (const auto &f : cur->getFiles()) {
            if (f->getName() == Fich) return f->getDate();
        }
        for (const auto &sub : cur->getSubdirectories()) q.push(sub);
    }
    return std::nullopt;
}


// Pesquisa por diretoria (Tipo=1) ou ficheiro (Tipo=0) e devolve caminho.
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

// ----------------------------------------
// Tree
void SistemaFicheiros::Tree(const std::string *fich) {
    if (!root) return;
    if (!fich) {
        root->generateTree(std::cout, "");
        return;
    }
    std::ofstream ofs(*fich);
    if (!ofs.is_open()) return;
    root->generateTree(ofs, "");
    ofs.close();
}

// ----------------------------------------
// Pesquisar todas as diretorias com nome <dir>
void SistemaFicheiros::PesquisarAllDirectorias(std::list<std::string> &lres, const std::string &dir) {
    if (!root) return;
    root->findAllDirectories(dir, lres, "");
}

// ----------------------------------------
// Pesquisar todos os ficheiros com nome <file>
void SistemaFicheiros::PesquisarAllFicheiros(std::list<std::string> &lres, const std::string &file) {
    if (!root) return;
    root->findAllFiles(file, lres, "");
}

// ----------------------------------------
// Implementação do CopyBatch: cópia em lote de ficheiros cujo nome contém um padrão
static std::string toLower(const std::string &s) {
    std::string out; out.reserve(s.size());
    for (char c : s) out.push_back(std::tolower((unsigned char)c));
    return out;
}

bool SistemaFicheiros::CopyBatch(const std::string &padrao, const std::string &DirOrigem, const std::string &DirDestino) {
    if (!root) return false;
    // localizar a diretoria de origem
    std::shared_ptr<Directory> src = nullptr;
    bool isPath = (DirOrigem.find('\\') != std::string::npos) || (DirOrigem.find('/') != std::string::npos);
    if (isPath) {
        std::shared_ptr<Directory> cur = root;
        std::string token;
        for (size_t i=0;i<DirOrigem.size();++i) {
            char c = DirOrigem[i];
            if (c=='\\' || c=='/') { if (!token.empty()) { auto next = cur->findSubdirectory(token); if (!next) { cur = nullptr; break; } cur = next; token.clear(); } }
            else token.push_back(c);
        }
        if (!token.empty() && cur) { auto next = cur->findSubdirectory(token); if (next) cur = next; else cur = nullptr; }
        src = cur;
    } else {
        std::queue<std::shared_ptr<Directory>> q; q.push(root);
        while (!q.empty() && !src) {
            auto cur = q.front(); q.pop();
            if (cur->getName() == DirOrigem) { src = cur; break; }
            for (const auto &s : cur->getSubdirectories()) q.push(s);
        }
    }
    if (!src) return false;

    // localizar a diretoria de destino
    std::shared_ptr<Directory> dst = nullptr;
    isPath = (DirDestino.find('\\') != std::string::npos) || (DirDestino.find('/') != std::string::npos);
    if (isPath) {
        std::shared_ptr<Directory> cur = root;
        std::string token;
        for (size_t i=0;i<DirDestino.size();++i) {
            char c = DirDestino[i];
            if (c=='\\' || c=='/') { if (!token.empty()) { auto next = cur->findSubdirectory(token); if (!next) { cur = nullptr; break; } cur = next; token.clear(); } }
            else token.push_back(c);
        }
        if (!token.empty() && cur) { auto next = cur->findSubdirectory(token); if (next) cur = next; else cur = nullptr; }
        dst = cur;
    } else {
        std::queue<std::shared_ptr<Directory>> q; q.push(root);
        while (!q.empty() && !dst) {
            auto cur = q.front(); q.pop();
            if (cur->getName() == DirDestino) { dst = cur; break; }
            for (const auto &s : cur->getSubdirectories()) q.push(s);
        }
    }
    if (!dst) return false;

    // recolher todos os ficheiros da sub-árvore de origem
    std::list<std::shared_ptr<File>> files;
    std::function<void(std::shared_ptr<Directory>)> collect = [&](std::shared_ptr<Directory> d){
        if (!d) return;
        for (const auto &f : d->getFiles()) files.push_back(f);
        for (const auto &s : d->getSubdirectories()) collect(s);
    };
    collect(src);

    std::string patternLow = toLower(padrao);
    int copied = 0;
    for (const auto &f : files) {
        std::string name = f->getName();
        if (toLower(name).find(patternLow) == std::string::npos) continue;

        // garantir nome único no destino (adiciona sufixo _NNN quando necessário)
        std::string base = name;
        std::string ext;
        size_t pos = name.find_last_of('.');
        if (pos != std::string::npos) { base = name.substr(0,pos); ext = name.substr(pos); }

        std::string destName = name;
        int seq = 1;
        while (dst->containsFile(destName)) {
            char buf[64]; sprintf(buf, "_%03d", seq);
            destName = base + buf + ext;
            seq++;
        }

        dst->addFile(destName, f->getSize());
        auto added = dst->findFile(destName);
        if (added) added->setDate(f->getDate());
        copied++;
    }

    return copied > 0;
}

// ----------------------------------------
// Renomear ficheiros
void SistemaFicheiros::RenomearFicheiros(const std::string &fich_old, const std::string &fich_new) {
    if (!root) return;
    std::queue<std::shared_ptr<Directory>> q; q.push(root);
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        auto files = cur->getFiles();
        for (const auto &f : files) {
            if (f->getName() == fich_old) f->setName(fich_new);
        }
        for (const auto &s : cur->getSubdirectories()) q.push(s);
    }
}

// ----------------------------------------
// Duplicados
bool SistemaFicheiros::FicheiroDuplicados() const {
    if (!root) return false;
    std::map<std::string,int> count;
    std::queue<std::shared_ptr<Directory>> q; q.push(root);
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        for (const auto &f : cur->getFiles()) count[f->getName()]++;
        for (const auto &s : cur->getSubdirectories()) q.push(s);
    }
    for (const auto &p : count) if (p.second > 1) return true;
    return false;
}

std::vector<std::string> SistemaFicheiros::GetFicheirosDuplicados() const {
    std::vector<std::string> out;
    if (!root) return out;
    std::map<std::string, std::vector<std::string>> mapPaths;
    std::queue<std::shared_ptr<Directory>> q; q.push(root);
    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        std::string dirPath = getAbsolutePath(cur.get());
        for (const auto &f : cur->getFiles()) {
            mapPaths[f->getName()].push_back(dirPath + "\\" + f->getName());
        }
        for (const auto &s : cur->getSubdirectories()) q.push(s);
    }
    for (const auto &p : mapPaths) {
        if (p.second.size() > 1) {
            std::ostringstream oss;
            oss << p.first << ": ";
            for (size_t i=0;i<p.second.size();++i) {
                if (i) oss << ", ";
                oss << p.second[i];
            }
            out.push_back(oss.str());
        }
    }
    return out;
}
