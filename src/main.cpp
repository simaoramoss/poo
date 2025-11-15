#include <iostream>
#include <string>
#include <queue>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#include "Directory.hpp"
#include "SistemaFicheiros.hpp"

void printCommands() {
    std::cout << "\nComandos disponíveis:\n";
    std::cout << "1. mkdir <nome> - Criar diretoria\n";
    std::cout << "2. touch <nome> <tamanho> - Criar ficheiro\n";
    std::cout << "3. cd <nome> - Mudar para diretoria\n";
    std::cout << "4. cd .. - Voltar à diretoria pai\n";
    std::cout << "5. ls - Listar conteúdo da diretoria atual\n";
    std::cout << "6. rm <nome> - Remover ficheiro\n";
    std::cout << "7. rmdir <nome> - Remover diretoria\n";
    std::cout << "8. size - Mostrar tamanho total da diretoria atual\n";
    std::cout << "9. maior - Mostrar o ficheiro que ocupa mais espaço (caminho)\n";
    std::cout << "10. dirmais - Mostrar diretoria com mais elementos (a partir da diretoria atual)\n";
    std::cout << "11. dirmenos - Mostrar diretoria com menos elementos (a partir da diretoria atual)\n";
    std::cout << "12. maisespaco - Mostrar diretoria que ocupa mais espaço (a partir da raiz do sistema)\n";
    std::cout << "13. removerall <DIR|FILE> - Remover todas as diretorias ou todos os ficheiros\n";
    std::cout << "14. exportarxml <ficheiro> - Exportar o sistema em memoria para XML (default: sistema.xml)\n";
    std::cout << "15. search <nome> <0|1> - Procurar ficheiro (0) ou directoria (1) e devolver caminho completo\n";        std::cout << "16. help - Mostrar comandos\n";
    std::cout << "17. exit - Sair\n";
}

int main() {
    auto root = std::make_shared<Directory>("/");
    Directory* currentDir = root.get();
    std::string command;
    
    std::cout << "Bem-vindo ao Gestor de Diretorias!" << std::endl;
    printCommands();
    
    while (true) {
        std::cout << "\n" << currentDir->getName() << "> ";
        std::string cmd;
        std::cin >> cmd;
        
        if (cmd == "exit") {
            break;
        }
        else if (cmd == "help") {
            printCommands();
        }
        else if (cmd == "mkdir") {
            std::string name;
            std::cin >> name;
            currentDir->addSubdirectory(name);
            std::cout << "Diretoria criada: " << name << "\n";
        }
        else if (cmd == "touch") {
            std::string name;
            size_t size;
            std::cin >> name >> size;
            currentDir->addFile(name, size);
            std::cout << "Ficheiro criado: " << name << "\n";
        }
        else if (cmd == "cd") {
            std::string name;
            std::cin >> name;
            if (name == "..") {
                if (currentDir->getParent() != nullptr) {
                    currentDir = currentDir->getParent();
                }
            }
            else {
                auto dir = currentDir->findSubdirectory(name);
                if (dir) {
                    currentDir = dir.get();
                }
                else {
                    std::cout << "Diretoria nao encontrada: " << name << "\n";
                }
            }
        }
        else if (cmd == "ls") {
            currentDir->listContents();
        }
        else if (cmd == "rm") {
            std::string name;
            std::cin >> name;
            currentDir->removeFile(name);
            std::cout << "Ficheiro removido: " << name << "\n";
        }
        else if (cmd == "rmdir") {
            std::string name;
            std::cin >> name;
            currentDir->removeSubdirectory(name);
            std::cout << "Diretoria removida: " << name << "\n";
        }
        else if (cmd == "size") {
            std::cout << "Tamanho total: " << currentDir->getTotalSize() << " bytes\n";
        }
        else if (cmd == "maior") {
            // Procurar o maior ficheiro a partir da diretoria atual (incluindo subdiretorias)
            auto [path, size] = currentDir->findLargestFileWithPath("");
            if (path.empty()) {
                std::cout << "Nenhum ficheiro encontrado nesta diretoria ou subdiretorias.\n";
            } else {
                std::cout << "Ficheiro maior: " << path << " (" << size << " bytes)\n";
            }
        }
        else if (cmd == "dirmais") {
            // Encontrar a diretoria (a partir da atual) com mais elementos (arquivos + subdiretorias)
            Directory* bestDir = currentDir;
            int bestCount = currentDir->getElementCount();
            std::queue<std::pair<Directory*, std::string>> q;
            q.push({currentDir, currentDir->getName()});

            while (!q.empty()) {
                auto [d, path] = q.front(); q.pop();
                int cnt = d->getElementCount();
                if (cnt > bestCount) {
                    bestCount = cnt;
                    bestDir = d;
                }
                for (const auto& sub : d->getSubdirectories()) {
                    q.push({sub.get(), path + "\\" + sub->getName()});
                }
            }

            // Mostrar resultado (caminho relativo à diretoria atual)
            // Se a diretoria atual for a raiz '/', a string conterá nomes separados por '\\'
            std::string resultPath = bestDir->getName();
            // Reconstruir caminho relativo usando parent chain até currentDir
            Directory* it = bestDir;
            std::string rel;
            while (it && it != currentDir->getParent()) {
                if (rel.empty()) rel = it->getName(); else rel = it->getName() + std::string("\\") + rel;
                it = it->getParent();
                if (it == nullptr) break;
                if (it == currentDir->getParent()) break;
            }
            if (!rel.empty()) resultPath = rel;

            std::cout << "Diretoria com mais elementos: " << resultPath << " (" << bestCount << " elementos)\n";
        }
        else if (cmd == "dirmenos") {
            // Encontrar a diretoria (a partir da atual) com menos elementos (arquivos + subdiretorias)
            Directory* minDir = currentDir;
            int minCount = currentDir->getElementCount();
            std::queue<std::pair<Directory*, std::string>> q2;
            q2.push({currentDir, currentDir->getName()});

            while (!q2.empty()) {
                auto [d, path] = q2.front(); q2.pop();
                int cnt = d->getElementCount();
                if (cnt < minCount) {
                    minCount = cnt;
                    minDir = d;
                }
                for (const auto& sub : d->getSubdirectories()) {
                    q2.push({sub.get(), path + "\\" + sub->getName()});
                }
            }

            // Reconstruir caminho relativo à diretoria atual
            std::string rel2;
            Directory* it2 = minDir;
            while (it2 && it2 != currentDir->getParent()) {
                if (rel2.empty()) rel2 = it2->getName(); else rel2 = it2->getName() + std::string("\\") + rel2;
                it2 = it2->getParent();
                if (it2 == nullptr) break;
                if (it2 == currentDir->getParent()) break;
            }
            std::string resultPath2 = rel2.empty() ? minDir->getName() : rel2;
            std::cout << "Diretoria com menos elementos: " << resultPath2 << " (" << minCount << " elementos)\n";
        }
        else if (cmd == "maisespaco") {
            // Encontrar, entre as subdiretorias imediatas da diretoria atual,
            // qual ocupa mais espaço (incluindo o conteúdo recursivo dessas subdiretorias).
            const auto& subs = currentDir->getSubdirectories();
            if (subs.empty()) {
                std::cout << "Nao existem subdiretorias na diretoria atual.\n";
            } else {
                size_t bestSize = 0;
                Directory* bestDir = nullptr;
                for (const auto& s : subs) {
                    size_t sz = s->getTotalSize();
                    if (!bestDir || sz > bestSize) {
                        bestDir = s.get();
                        bestSize = sz;
                    }
                }
                if (bestDir) {
                    std::cout << "Diretoria que ocupa mais espaco: " << bestDir->getName()
                              << " (" << bestSize << " bytes)\n";
                } else {
                    std::cout << "Nenhuma diretoria encontrada.\n";
                }
            }
        }
        else if (cmd == "removerall") {
            std::string tipo;
            if (!(std::cin >> tipo)) {
                std::cout << "Uso: removerall <DIR|FILE>\n";
                continue;
            }
            // normalize tipo to uppercase
            std::transform(tipo.begin(), tipo.end(), tipo.begin(), [](unsigned char c){ return std::toupper(c); });

            bool removed = false;
            // collect all directories (pointers) in the tree
            std::vector<Directory*> allDirs;
            std::queue<Directory*> q;
            q.push(root.get());
            while (!q.empty()) {
                Directory* d = q.front(); q.pop();
                allDirs.push_back(d);
                for (const auto& sub : d->getSubdirectories()) q.push(sub.get());
            }

            if (tipo == "DIR") {
                // remove all subdirectories from every directory
                for (Directory* d : allDirs) {
                    auto subs = d->getSubdirectories();
                    for (const auto& s : subs) {
                        d->removeSubdirectory(s->getName());
                        removed = true;
                    }
                }
            } else if (tipo == "FILE") {
                // remove all files from every directory
                for (Directory* d : allDirs) {
                    auto files = d->getFiles();
                    for (const auto& f : files) {
                        d->removeFile(f->getName());
                        removed = true;
                    }
                }
            } else {
                std::cout << "Tipo invalido. Use DIR ou FILE." << "\n";
                continue;
            }

            if (removed) std::cout << "Remocao concluida." << "\n";
            else std::cout << "Nenhuma ocorrencia encontrada para remover." << "\n";
        }
        else if (cmd == "exportarxml") {
            std::string path;
            // read optional path; if none provided, use default
            if (!(std::cin >> path)) {
                path = "sistema.xml";
            }

            SistemaFicheiros sf;
            sf.SetRoot(root);
            sf.Escrever_XML(path);
            std::cout << "Sistema exportado para: " << path << "\n";
        }
        else if (cmd == "search") {
            std::string nome;
            int tipo;
            if (!(std::cin >> nome >> tipo)) {
                std::cout << "Uso: search <nome> <0|1>  (0=ficheiro, 1=directoria)\n";
                continue;
            }

            SistemaFicheiros sf;
            sf.SetRoot(root);
            std::string* res = sf.Search(nome, tipo);
            if (res == nullptr) {
                std::cout << "Nao encontrado: " << nome << "\n";
            } else {
                std::cout << "Encontrado: " << *res << "\n";
                delete res;
            }
        }
        else {
            std::cout << "Comando invalido. Digite 'help' para ver os comandos disponíveis.\n";
        }
    }
    
    return 0;
}