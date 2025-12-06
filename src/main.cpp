#include <iostream>
#include <string>
#include <queue>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <filesystem>
#include "Directory.hpp"
#include "SistemaFicheiros.hpp"

namespace fs = std::filesystem;

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
    std::cout << "13. search <nome> <0|1> - Procurar ficheiro (0) ou directoria (1) e devolver caminho completo\n";
    std::cout << "14. removerall <DIR|FILE> - Remover todas as diretorias ou todos os ficheiros\n";       
    std::cout << "15. exportarxml <ficheiro> - Exportar o sistema em memoria para XML (default: sistema.xml)\n";
    std::cout << "16. tree [<ficheiro>] - Listar arvore (ou gravar em ficheiro)\n";
    std::cout << "17. finddirs <nome> - Encontrar todas as diretorias com esse nome\n";
    std::cout << "18. findfiles <nome> - Encontrar todos os ficheiros com esse nome\n";
    std::cout << "19. renamefiles <old> <new> - Renomear ficheiros com nome <old> para <new>\n";
    std::cout << "20. dupfiles - Listar ficheiros duplicados (mesmo nome)\n";
    std::cout << "21. copybatch <padrao> <DirOrigem> <DirDestino> - Copiar ficheiros cujo nome contenha <padrao> da arvore de origem para a raiz do destino\n";
    std::cout << "help - Mostrar comandos\n";
    std::cout << "exit - Sair (guarda automaticamente em sistema_saved.xml)\n";
}

static std::string convertAsctimeToYMD(const std::string& asctimeStr) {
    // Formato típico: "Wed Jun 30 21:49:08 1993"
    std::istringstream iss(asctimeStr);
    std::tm tm = {};
    iss.str(asctimeStr);
    iss.clear();
    iss >> std::get_time(&tm, "%a %b %d %H:%M:%S %Y");
    if (iss.fail()) {
        return asctimeStr;
    }
    int year = tm.tm_year + 1900;
    int mon = tm.tm_mon + 1;
    int day = tm.tm_mday;
    return std::to_string(year) + "|" + std::to_string(mon) + "|" + std::to_string(day);
}

int main() {
    // Arranque: criamos uma árvore vazia com raiz "/".
    auto root = std::make_shared<Directory>("/");
    Directory* currentDir = root.get();

    // Criamos o serviço que gere as operações sobre a árvore.
    SistemaFicheiros sf;
    sf.SetRoot(root);
    // Recupera o estado anterior, se existir, para continuar onde ficámos.
    if (sf.Ler_XML("sistema_saved.xml")) {
        root = sf.GetRoot();
        currentDir = root.get();
        std::cout << "Sistema carregado de sistema_saved.xml" << std::endl;
    }

    std::cout << "Bem-vindo ao Gestor de Diretorias!" << std::endl;
    printCommands();

    while (true) {
        std::cout << "\n" << currentDir->getName() << "> ";
        std::string cmd;
        if (!(std::cin >> cmd)) break;

        if (cmd == "exit") {
            // Antes de sair, guardamos o estado para poder retomar depois.
            sf.SetRoot(root);
            sf.Escrever_XML("sistema_saved.xml");
            std::cout << "Sistema guardado em sistema_saved.xml. A sair...\n";
            break;
        }
        else if (cmd == "help") {
            printCommands();
        }
        else if (cmd == "mkdir") {
            // Cria uma subdiretoria diretamente na diretoria atual.
            std::string name;
            std::cin >> name;
            currentDir->addSubdirectory(name);
            std::cout << "Diretoria criada: " << name << "\n";
        }
        else if (cmd == "load") {
            // Varre uma pasta real do disco e constrói a árvore em memória.
            std::string path;
            if (!(std::cin >> path)) { std::cout << "Uso: load <path>\n"; continue; }
            bool ok = sf.Load(path);
            if (ok) {
                root = sf.GetRoot();
                currentDir = root.get();
                std::cout << "Diretoria carregada em memoria: " << path << "\n";
            } else {
                std::cout << "Falha ao carregar a diretoria: " << path << "\n";
            }
        }
        else if (cmd == "touch") {
            // Cria um ficheiro simples na diretoria atual com o tamanho indicado.
            std::string name;
            size_t size;
            std::cin >> name >> size;
            currentDir->addFile(name, size);
            std::cout << "Ficheiro criado: " << name << "\n";
        }
        else if (cmd == "cd") {
            // Navegação: entra numa subdiretoria ou volta para o pai com `..`.
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
            // Mostra subdiretorias e ficheiros da diretoria atual.
            currentDir->listContents();
        }
        else if (cmd == "rm") {
            // Remove um ficheiro pelo nome na diretoria atual.
            std::string name;
            std::cin >> name;
            currentDir->removeFile(name);
            std::cout << "Ficheiro removido: " << name << "\n";
        }
        else if (cmd == "rmdir") {
            // Remove uma subdiretoria.
            std::string name;
            std::cin >> name;
            currentDir->removeSubdirectory(name);
            std::cout << "Diretoria removida: " << name << "\n";
        }
        else if (cmd == "size") {
            // Soma recursivamente o tamanho de todos os ficheiros sob a diretoria atual.
            std::cout << "Tamanho total: " << currentDir->getTotalSize() << " bytes\n";
        }
        else if (cmd == "maior") {
            // Procura o ficheiro maior e imprime o caminho completo.
            auto [path, size] = currentDir->findLargestFileWithPath("");
            if (path.empty()) {
                std::cout << "Nenhum ficheiro encontrado nesta diretoria ou subdiretorias.\n";
            } else {
                std::cout << "Ficheiro maior: " << path << " (" << size << " bytes)\n";
            }
        }
        else if (cmd == "directoriamaiselementos") {
            // Calcula a diretoria (em toda a árvore) com mais elementos (dirs+ficheiros).
            sf.SetRoot(root);
            auto res = sf.DirectoriaMaisElementos();
            if (!res.has_value()) std::cout << "Nenhuma diretoria encontrada.\n";
            else std::cout << res.value() << "\n";
        }
        else if (cmd == "directoriamenoselementos") {
            // Calcula a diretoria (em toda a árvore) com menos elementos.
            sf.SetRoot(root);
            auto res = sf.DirectoriaMenosElementos();
            if (!res.has_value()) std::cout << "Nenhuma diretoria encontrada.\n";
            else std::cout << res.value() << "\n";
        }
        else if (cmd == "ficheiromaior") {
            // Versão via `SistemaFicheiros` que devolve também o tamanho formatado.
            sf.SetRoot(root);
            auto res = sf.FicheiroMaior();
            if (!res.has_value()) std::cout << "Nenhum ficheiro encontrado.\n";
            else std::cout << res.value() << "\n";
        }
        else if (cmd == "directoriamaiespaco") {
            // Diretoria que ocupa mais espaço (soma recursiva dos ficheiros).
            sf.SetRoot(root);
            auto res = sf.DirectoriaMaisEspaco();
            if (!res.has_value()) std::cout << "Nenhuma diretoria encontrada.\n";
            else std::cout << res.value() << "\n";
        }
        else if (cmd == "contarficheiros") {
            // Quantos ficheiros existem no sistema, no total.
            sf.SetRoot(root);
            std::cout << sf.ContarFicheiros() << "\n";
        }
        else if (cmd == "contardirectorios") {
            // Quantas diretorias existem (conta inclui a raiz).
            sf.SetRoot(root);
            std::cout << sf.ContarDirectorios() << "\n";
        }
        else if (cmd == "memoria") {
            // Memória total ocupada (soma dos tamanhos dos ficheiros).
            sf.SetRoot(root);
            std::cout << sf.Memoria() << "\n";
        }
        else if (cmd == "dirmais") {
            // Versão local (partindo da diretoria atual) para diretoria com mais elementos.
            Directory* bestDir = currentDir;
            int bestCount = currentDir->getElementCount();
            std::queue<std::pair<Directory*, std::string>> q;
            q.push({ currentDir, currentDir->getName() });

            while (!q.empty()) {
                auto [d, path] = q.front(); q.pop();
                int cnt = d->getElementCount();
                if (cnt > bestCount) {
                    bestCount = cnt;
                    bestDir = d;
                }
                for (const auto& sub : d->getSubdirectories()) {
                    q.push({ sub.get(), path + "\\" + sub->getName() });
                }
            }

            std::cout << "Diretoria com mais elementos: "
                      << bestDir->getName() << " (" << bestCount << " elementos)\n";
        }
        else if (cmd == "dirmenos") {
            // Versão local (partindo da diretoria atual) para diretoria com menos elementos.
            Directory* minDir = currentDir;
            int minCount = currentDir->getElementCount();
            std::queue<std::pair<Directory*, std::string>> q;
            q.push({ currentDir, currentDir->getName() });

            while (!q.empty()) {
                auto [d, path] = q.front(); q.pop();
                int cnt = d->getElementCount();
                if (cnt < minCount) {
                    minCount = cnt;
                    minDir = d;
                }
                for (const auto& sub : d->getSubdirectories()) {
                    q.push({ sub.get(), path + "\\" + sub->getName() });
                }
            }

            std::cout << "Diretoria com menos elementos: "
                      << minDir->getName() << " (" << minCount << " elementos)\n";
        }
        else if (cmd == "maisespaco") {
            // Entre as subdiretorias diretas da atual, qual ocupa mais espaço.
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
                }
            }
        }
        else if (cmd == "removerall") {
            // Remove todos os ficheiros ou todas as diretorias com o nome indicado, na árvore toda.
            std::string tipo;
            if (!(std::cin >> tipo)) {
                std::cout << "Uso: removerall <DIR|FILE>\n";
                continue;
            }

            std::transform(tipo.begin(), tipo.end(), tipo.begin(),
                [](unsigned char c) { return std::toupper(c); });

            bool removed = false;

            std::vector<Directory*> allDirs;
            std::queue<Directory*> q;
            q.push(root.get());
            while (!q.empty()) {
                Directory* d = q.front(); q.pop();
                allDirs.push_back(d);
                for (const auto& sub : d->getSubdirectories()) q.push(sub.get());
            }

            if (tipo == "DIR") {
                for (Directory* d : allDirs) {
                    auto subs = d->getSubdirectories();
                    for (const auto& s : subs) {
                        d->removeSubdirectory(s->getName());
                        removed = true;
                    }
                }
            }
            else if (tipo == "FILE") {
                for (Directory* d : allDirs) {
                    auto files = d->getFiles();
                    for (const auto& f : files) {
                        d->removeFile(f->getName());
                        removed = true;
                    }
                }
            }
            else {
                std::cout << "Tipo invalido. Use DIR ou FILE." << "\n";
                continue;
            }

            if (removed) std::cout << "Remocao concluida.\n";
            else std::cout << "Nenhuma ocorrencia encontrada para remover.\n";
        }
        else if (cmd == "exportarxml") {
            // Exporta a árvore para um XML simples (útil para persistência/consulta).
            std::string path;
            if (!(std::cin >> path)) {
                path = "sistema.xml";
            }
            sf.SetRoot(root);
            sf.Escrever_XML(path);
            std::cout << "Sistema exportado para: " << path << "\n";
        }
        else if (cmd == "lerxml") {
            // Lê a árvore a partir de um XML gerado previamente.
            std::string path;
            if (!(std::cin >> path)) {
                std::cout << "Uso: lerxml <ficheiro>\n";
                continue;
            }

            bool sucesso = sf.Ler_XML(path);
            if (sucesso) {
                root = sf.GetRoot();
                currentDir = root.get();
                std::cout << "Sistema carregado com sucesso a partir de: " << path << "\n";
                std::cout << "Resumo: " << sf.ContarDirectorios() << " diretorias, "
                          << sf.ContarFicheiros() << " ficheiros, " << sf.Memoria() << " bytes" << std::endl;
            }
            else {
                std::cout << "Falha ao carregar o sistema a partir de: " << path << "\n";
            }
        }
        else if (cmd == "tree") {
            // Desenha a árvore em texto; se indicar ficheiro, grava em vez de imprimir.
            std::string arg;
            // optional filename
            if (std::getline(std::cin, arg)) {
                // trim
                auto trim = [](std::string &s){ size_t a=0; while(a<s.size() && isspace((unsigned char)s[a])) a++; size_t b=s.size(); while(b>a && isspace((unsigned char)s[b-1])) b--; s = s.substr(a,b-a); };
                trim(arg);
            }
            sf.SetRoot(root);
            if (arg.empty()) sf.Tree(nullptr);
            else sf.Tree(&arg);
        }
        else if (cmd == "finddirs") {
            // Procura diretorias com o nome dado e lista os caminhos.
            std::string name;
            if (!(std::cin >> name)) { std::cout << "Uso: finddirs <nome>\n"; continue; }
            sf.SetRoot(root);
            std::list<std::string> results;
            sf.PesquisarAllDirectorias(results, name);
            if (results.empty()) std::cout << "Nenhuma diretoria encontrada com o nome: " << name << "\n";
            else { std::cout << "Diretorias encontradas:\n"; for (auto &p: results) std::cout << "  " << p << "\n"; }
        }
        else if (cmd == "copybatch") {
            // Copia para a raiz do destino os ficheiros cujo nome contém o padrão.
            std::string padrao, dirOrig, dirDest;
            if (!(std::cin >> padrao >> dirOrig >> dirDest)) { std::cout << "Uso: copybatch <padrao> <DirOrigem> <DirDestino>\n"; continue; }
            sf.SetRoot(root);
            bool ok = sf.CopyBatch(padrao, dirOrig, dirDest);
            if (ok) std::cout << "CopyBatch concluido (ficheiros copiados para a raiz de " << dirDest << ").\n";
            else std::cout << "CopyBatch falhou (origem/destino nao encontrado ou nenhum ficheiro corresponde ao padrao).\n";
        }
        else if (cmd == "findfiles") {
            // Procura ficheiros com o nome dado e lista os caminhos.
            std::string name;
            if (!(std::cin >> name)) { std::cout << "Uso: findfiles <nome>\n"; continue; }
            sf.SetRoot(root);
            std::list<std::string> results;
            sf.PesquisarAllFicheiros(results, name);
            if (results.empty()) std::cout << "Nenhum ficheiro encontrado com o nome: " << name << "\n";
            else { std::cout << "Ficheiros encontrados:\n"; for (auto &p: results) std::cout << "  " << p << "\n"; }
        }
        else if (cmd == "renamefiles") {
            // Renomeia todos os ficheiros com o nome antigo para o novo.
            std::string oldName, newName;
            if (!(std::cin >> oldName >> newName)) { std::cout << "Uso: renamefiles <old> <new>\n"; continue; }
            sf.SetRoot(root);
            sf.RenomearFicheiros(oldName, newName);
            std::cout << "Renomeacao concluida: " << oldName << " -> " << newName << " (onde aplicavel)\n";
        }
        else if (cmd == "dupfiles") {
            // Sinaliza ficheiros duplicados (mesmo nome) e lista onde estão.
            sf.SetRoot(root);
            auto duplicates = sf.GetFicheirosDuplicados();
            if (duplicates.empty()) std::cout << "Nao foram encontrados ficheiros duplicados.\n";
            else { std::cout << "Ficheiros duplicados encontrados:\n"; for (auto &d: duplicates) std::cout << "  " << d << "\n"; }
        }
        else if (cmd == "search") {
            // Pesquisa global por diretoria (1) ou ficheiro (0) e devolve o caminho completo.
            std::string nome;
            int tipo;
            if (!(std::cin >> nome >> tipo)) {
                std::cout << "Uso: search <nome> <0|1>\n";
                continue;
            }

            sf.SetRoot(root);
            auto res = sf.Search(nome, tipo);
            if (!res.has_value()) {
                std::cout << "Nao encontrado: " << nome << "\n";
            } else {
                std::cout << "Encontrado: " << res.value() << "\n";
            }
        }
        else if (cmd == "movefile") {
            std::string nome, dir;
            if (!(std::cin >> nome >> dir)) {
                std::cout << "Uso: movefile <nome> <dir>\n";
                continue;
            }

            sf.SetRoot(root);
            bool ok = sf.MoveFicheiro(nome, dir);
            if (ok) std::cout << "Ficheiro movido: " << nome << " -> " << dir << "\n";
            else std::cout << "Falha ao mover ficheiro (nao encontrado, destino inexistente, duplicado ou ja na pasta destino)\n";
        }
        else if (cmd == "movedir") {
            std::string oldName, newName;
            if (!(std::cin >> oldName >> newName)) {
                std::cout << "Uso: movedir <DirOld> <DirNew>\n";
                continue;
            }

            sf.SetRoot(root);
            bool ok = sf.MoverDirectoria(oldName, newName);
            if (ok) std::cout << "Directoria movida: " << oldName << " -> " << newName << "\n";
            else std::cout << "Falha ao mover directoria (nao encontrada, destino inexistente, ou destino dentro de origem)\n";
        }
        else if (cmd == "getdate") {
            std::string fname;
            if (!(std::cin >> fname)) {
                std::cout << "Uso: getdate <nome_ficheiro>\n";
                continue;
            }

            sf.SetRoot(root);
            auto pdate = sf.DataFicheiro(fname);
            if (!pdate.has_value()) {
                std::cout << "Ficheiro nao encontrado: " << fname << "\n";
            } else {
                std::string stored = pdate.value();
                std::string out = convertAsctimeToYMD(stored);
                std::cout << "Data de " << fname << ": " << out << "\n";
            }
        }
        else {
            std::cout << "Comando invalido. Digite 'help' para ver os comandos disponíveis.\n";
        }
    }

    return 0;
}
