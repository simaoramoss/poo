#pragma once
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <optional>
#include "Directory.hpp"
#include "File.hpp"

class SistemaFicheiros {
private:
    std::shared_ptr<Directory> root;

public:
    SistemaFicheiros();
    ~SistemaFicheiros();

    void clearSystem();

    bool Load(const std::string& pathStr);

    int ContarFicheiros() const;
    int ContarDirectorias() const;
    int Memoria() const;

    std::optional<std::string> DirectoriaMaisElementos() const;
    std::optional<std::string> DirectoriaMenosElementos() const;
    std::optional<std::string> DirectoriaMaisEspaco() const;
    std::optional<std::string> FicheiroMaior() const;

    void getAllDirectories(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<Directory>>& dirs) const;
    void getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const;

    bool RemoverAll(const std::string &s, const std::string &tipo);

    void SetRoot(std::shared_ptr<Directory> r);
    std::shared_ptr<Directory> GetRoot() const;

    void Escrever_XML(const std::string &s);
    bool Ler_XML(const std::string &s);

    std::optional<std::string> Search(const std::string &s, int Tipo) const;

private:
    std::string getAbsolutePath(Directory* dir) const;
};
