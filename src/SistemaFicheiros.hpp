#pragma once
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <optional>
#include <queue>
#include <stack>
#include <functional>
#include "Directory.hpp"
#include "File.hpp"

class SistemaFicheiros {
private:
    std::shared_ptr<Directory> root;

public:
    SistemaFicheiros();
    ~SistemaFicheiros();

    // ----------------------------------------
    // Gestão do sistema
    void clearSystem();
    bool Load(const std::string& pathStr);

    // ----------------------------------------
    // Contagens e memória
    int ContarFicheiros() const;
    int ContarDirectorios() const;
    int Memoria() const;

    // ----------------------------------------
    // Diretórios
    std::optional<std::string> DirectoriaMaisElementos() const;
    std::optional<std::string> DirectoriaMenosElementos() const;
    std::optional<std::string> DirectoriaMaisEspaco() const;

    // ----------------------------------------
    // Ficheiros
    std::optional<std::string> FicheiroMaior() const;
    std::optional<std::string> DataFicheiro(const std::string &Fich) const;

    // ----------------------------------------
    // Get / Set root
    void SetRoot(std::shared_ptr<Directory> r);
    std::shared_ptr<Directory> GetRoot() const;

    // ----------------------------------------
    // Operações sobre ficheiros / diretórios
    bool RemoverAll(const std::string &s, const std::string &tipo);
    bool MoveFicheiro(const std::string &Fich, const std::string &DirNova);
    bool MoverDirectoria(const std::string &DirOld, const std::string &DirNew);

    // ----------------------------------------
    // XML
    void Escrever_XML(const std::string &s);
    bool Ler_XML(const std::string &s);

private:
    // ----------------------------------------
    // Funções auxiliares
    std::string getAbsolutePath(Directory* dir) const;

    void getAllDirectories(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<Directory>>& dirs) const;
    void getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const;
};
