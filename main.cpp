#include <iostream>
#include <string>
#include "Directory.hpp"
#include "SistemaFicheiros.hpp"

void printCommands() {
    std::cout << "\nComandos disponíveis:\n";
    std::cout << "1. Criar diretória\n";
    std::cout << "2. Criar ficheiro\n";
    std::cout << "3. Mudar para diretória\n";
    std::cout << "4. Voltar à diretória pai\n";
    std::cout << "5. Listar conteúdo da diretória atual\n";
    std::cout << "6. Remover ficheiro\n";
    std::cout << "7. Remover diretória\n";
    std::cout << "8. Mostrar tamanho total da diretória atual\n";
    std::cout << "9. Mostrar comandos\n";
    std::cout << "10. Sair\n";
}

int main() {
    auto root = std::make_shared<Directory>("/");
    Directory* currentDir = root.get();
    std::string command;
    
    std::cout << "Bem-vindo ao Gestor de Diretórias!" << std::endl;
    printCommands();
    
    while (true) {
        std::cout << "\n" << currentDir->getName() << "> ";
        std::string cmd;
        std::cin >> cmd;
        
        if (cmd == "Sair") {
            break;
        }
        else if (cmd == "Mostrar comandos") {
            printCommands();
        }
        else if (cmd == "Criar diretória") {
            std::string name;
            std::cin >> name;
            currentDir->addSubdirectory(name);
            std::cout << "Diretória criada: " << name << "\n";
        }
        else if (cmd == "Criar ficheiro") {
            std::string name;
            size_t size;
            std::cin >> name >> size;
            currentDir->addFile(name, size);
            std::cout << "Ficheiro criado: " << name << "\n";
        }
        else if (cmd == "Mudar para diretória") {
            std::string name;
            std::cin >> name;
            if (name == "Voltar à diretória pai") {
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
        else if (cmd == "Listar conteúdo da diretória atual") {
            currentDir->listContents();
        }
        else if (cmd == "Remover ficheiro") {
            std::string name;
            std::cin >> name;
            currentDir->removeFile(name);
            std::cout << "Ficheiro removido: " << name << "\n";
        }
        else if (cmd == " Remover diretória") {
            std::string name;
            std::cin >> name;
            currentDir->removeSubdirectory(name);
            std::cout << "Diretória removida: " << name << "\n";
        }
        else if (cmd == "Mostrar tamanho total da diretória atual") {
            std::cout << "Tamanho total: " << currentDir->getTotalSize() << " bytes\n";
        }
        else {
            std::cout << "Comando inválido. Digite 'help' para ver os comandos disponíveis.\n";
        }
    }
    
    return 0;
}