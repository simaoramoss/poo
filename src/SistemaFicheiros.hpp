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
    std::optional<std::string> Search(const std::string &s, int Tipo) const;

    // ----------------------------------------
    // XML
    void Escrever_XML(const std::string &s);
    bool Ler_XML(const std::string &s);

    // ----------------------------------------
    // Tree (imprime a arvore em consola ou grava para ficheiro)
    void Tree(const std::string *fich = nullptr);

    // ----------------------------------------
    // Pesquisar todas as diretorias com nome <dir> e colocar caminhos em <lres>
    void PesquisarAllDirectorias(std::list<std::string> &lres, const std::string &dir);
    // ----------------------------------------
    // Pesquisar todos os ficheiros com nome <file> e colocar caminhos em <lres>
    void PesquisarAllFicheiros(std::list<std::string> &lres, const std::string &file);
    // ----------------------------------------
    // Copiar em batch: copia ficheiros cujo nome contenha <padrao> a partir de
    // DirOrigem (incluindo sub-directorias) para a raiz de DirDestino.
    bool CopyBatch(const std::string &padrao, const std::string &DirOrigem, const std::string &DirDestino);

    // ----------------------------------------
    // Renomear todos os ficheiros com nome <fich_old> para <fich_new>
    void RenomearFicheiros(const std::string &fich_old, const std::string &fich_new);
    // ----------------------------------------
    // Verificar se existem ficheiros duplicados (mesmo nome)
    bool FicheiroDuplicados() const;
    // Retorna uma lista formatada com os ficheiros duplicados e os seus caminhos
    std::vector<std::string> GetFicheirosDuplicados() const;

private:
    // ----------------------------------------
    // Funções auxiliares
    std::string getAbsolutePath(Directory* dir) const;

    void getAllDirectories(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<Directory>>& dirs) const;
    void getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const;
};
