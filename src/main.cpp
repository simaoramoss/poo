#include <iostream>
#include <string>
#include "Directory.hpp"

void printCommands() {
    std::cout << "\nComandos disponíveis:\n";
    std::cout << "1. mkdir <nome> - Criar diretória\n";
    std::cout << "2. touch <nome> <tamanho> - Criar ficheiro\n";
    std::cout << "3. cd <nome> - Mudar para diretória\n";
    std::cout << "4. cd .. - Voltar à diretória pai\n";
    std::cout << "5. ls - Listar conteúdo da diretória atual\n";
    std::cout << "6. rm <nome> - Remover ficheiro\n";
    std::cout << "7. rmdir <nome> - Remover diretória\n";
    std::cout << "8. size - Mostrar tamanho total da diretória atual\n";
    std::cout << "9. help - Mostrar comandos\n";
    std::cout << "10. exit - Sair\n";
}

int main() {
    auto root = std::make_shared<Directory>("/");
    Directory* currentDir = root.get();
    std::string command;
    
    std::cout << "Bem-vindo ao Gestor de Diretórias!\n";
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
            std::cout << "Diretória criada: " << name << "\n";
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
                    std::cout << "Diretória não encontrada: " << name << "\n";
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
            std::cout << "Diretória removida: " << name << "\n";
        }
        else if (cmd == "size") {
            std::cout << "Tamanho total: " << currentDir->getTotalSize() << " bytes\n";
        }
        else {
            std::cout << "Comando inválido. Digite 'help' para ver os comandos disponíveis.\n";
        }
    }
    
    return 0;
}