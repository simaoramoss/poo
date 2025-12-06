#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

/**
 * @file Directory.hpp
 * @brief Declara a classe Directory (nó da árvore de diretórios).
 */

#include <string>
#include <vector>
#include <memory>
#include <list>
#include <ostream>
#include "File.hpp"

/**
 * @class Directory
 * @brief Nó da árvore: guarda subdiretorias, ficheiros e ponteiro para o pai.
 */
class Directory {
private:
    std::string name;
    std::vector<std::shared_ptr<Directory>> subdirectories;
    std::vector<std::shared_ptr<File>> files;
    Directory* parent;

public:
    /**
     * @brief Constrói uma diretoria.
     * @param name Nome da diretoria.
     * @param parent Ponteiro para a diretoria pai (opcional).
     */
    Directory(const std::string& name, Directory* parent = nullptr);

    /** @brief Obtém o nome da diretoria. */
    std::string getName() const;
    /** @brief Atualiza o nome da diretoria. */
    void setName(const std::string& newName);
    /** @brief Lista de subdiretorias diretas. */
    const std::vector<std::shared_ptr<Directory>>& getSubdirectories() const;
    /** @brief Lista de ficheiros diretos. */
    const std::vector<std::shared_ptr<File>>& getFiles() const;
    /** @brief Ponteiro para o pai (ou nullptr se raiz). */
    Directory* getParent() const;

    /** @brief Adiciona uma subdiretoria com o nome dado. */
    void addSubdirectory(const std::string& name);
    /** @brief Liga uma subdiretoria já existente (shared_ptr) a este nó. */
    void addSubdirectoryPtr(std::shared_ptr<Directory> dir);
    /**
     * @brief Remove dos filhos e devolve o ponteiro para reanexar noutro lado.
     * @param name Nome da subdiretoria a retirar.
     * @return shared_ptr para a subdiretoria removida (ou nullptr se não existir).
     */
    std::shared_ptr<Directory> takeSubdirectory(const std::string& name);
    /** @brief Cria e adiciona um ficheiro com nome e tamanho. */
    void addFile(const std::string& name, size_t size);
    /** @brief Adiciona um ficheiro já existente (shared_ptr) a esta diretoria. */
    void addFilePtr(std::shared_ptr<File> fptr);
    /** @brief Remove uma subdiretoria pelo nome. */
    void removeSubdirectory(const std::string& name);
    /** @brief Remove um ficheiro pelo nome. */
    void removeFile(const std::string& name);
    /** @brief Procura uma subdiretoria pelo nome. */
    std::shared_ptr<Directory> findSubdirectory(const std::string& name) const;
    /** @brief Atualiza o ponteiro para o pai. */
    void setParent(Directory* p);
    /** @brief Procura um ficheiro pelo nome. */
    std::shared_ptr<File> findFile(const std::string& name) const;
    /** @brief Imprime subdiretorias e ficheiros desta diretoria. */
    void listContents() const;
    /** @brief Soma recursiva dos tamanhos dos ficheiros sob esta diretoria. */
    size_t getTotalSize() const;
    /** @brief Conta recursivamente todos os ficheiros. */
    int getTotalFiles() const;
    /** @brief Conta recursivamente todas as diretorias (inclui a própria). */
    int getTotalDirectories() const;
    /** @brief Quantos elementos diretos tem (subdirs + ficheiros). */
    int getElementCount() const;
    /** @brief Procura o ficheiro maior na subárvore. */
    std::shared_ptr<File> findLargestFile() const;
    /**
     * @brief Procura o maior ficheiro e devolve também o caminho absoluto.
     * @param currentPath Prefixo de caminho usado na recursão.
     * @return Par (caminho, tamanho). Se não existir, caminho vazio e tamanho 0.
     */
    std::pair<std::string, size_t> findLargestFileWithPath(const std::string& currentPath = "") const;
    /** @brief Lista todos os caminhos de diretorias com o nome indicado. */
    void findAllDirectories(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    /** @brief Lista todos os caminhos de ficheiros com o nome indicado. */
    void findAllFiles(const std::string& name, std::list<std::string>& paths, const std::string& currentPath = "");
    /** @brief Indica se existe um ficheiro com o nome dado na subárvore. */
    bool containsFile(const std::string& name) const;
    /** @brief Gera uma representação textual em árvore com indentação. */
    void generateTree(std::ostream& out, const std::string& prefix = "") const;
    /** @brief Verifica se esta diretoria é descendente de outra. */
    bool isSubdirectoryOf(const Directory* other) const;
};

#endif // DIRECTORY_HPP
