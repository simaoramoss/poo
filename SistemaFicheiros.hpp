#ifndef SISTEMA_FICHEIROS_HPP
#define SISTEMA_FICHEIROS_HPP

#include <string>
#include <list>
#include <memory>
#include "Directory.hpp"

class SistemaFicheiros {
private:
    std::shared_ptr<Directory> root;
    
    // Métodos auxiliares privados
    void clearSystem();
    std::shared_ptr<Directory> findDirectory(const std::string& path);
    std::string getAbsolutePath(Directory* dir) const;
    void getAllDirectories(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<Directory>>& dirs) const;
    void getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const;

public:
    SistemaFicheiros();
    ~SistemaFicheiros();

    // Métodos principais conforme especificação
    bool Load(const std::string& path);
    int ContarFicheiros();
    int ContarDirectorias();
    int Memoria();
    std::string* DirectoriaMaisElementos();
    std::string* DirectoriaMenosElementos();
    std::string* FicheiroMaior();
    std::string* DirectoriaMaisEspaco();
    std::string* Search(const std::string& s, int Tipo);
    bool RemoverAll(const std::string& s, const std::string& tipo);
    void Escrever_XML(const std::string& s);
    bool Ler_XML(const std::string& s);
    bool MoveFicheiro(const std::string& Fich, const std::string& DirNova);
    bool MoverDirectoria(const std::string& DirOld, const std::string& DirNew);
    std::string* DataFicheiro(const std::string& ficheiro);
    void Tree(const std::string* fich = nullptr);
    void PesquisarAllDirectorias(std::list<std::string>& lres, const std::string& dir);
    void PesquisarAllFicheiros(std::list<std::string>& lres, const std::string& file);
    void RenomearFicheiros(const std::string& fich_old, const std::string& fich_new);
    bool FicheiroDuplicados();
    bool CopyBatch(const std::string& padrao, const std::string& DirOrigem, const std::string& DirDestino);
};

#endif