#pragma once
/**
 * @file SistemaFicheiros.hpp
 * @brief Interface do serviço que gere as operações sobre a árvore em memória.
 */
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

/**
 * @class SistemaFicheiros
 * @brief Serviço de alto nível para gerir diretórios/ficheiros em memória.
 */
class SistemaFicheiros {
private:
    std::shared_ptr<Directory> root;

public:
    /** @brief Construtor padrão. */
    SistemaFicheiros();
    /** @briefLiberta a raiz ao limpar o sistema. */
    ~SistemaFicheiros();

    // ----------------------------------------
    // Gestão do sistema
    /** @brief Limpa o sistema (desfaz referência à raiz). */
    void clearSystem();
    /** @brief Carrega a árvore a partir de uma pasta real do disco. */
    bool Load(const std::string& pathStr);

    // ----------------------------------------
    // Contagens e memória
    /** @brief Conta todos os ficheiros. */
    int ContarFicheiros() const;
    /** @brief Conta todas as diretorias (inclui a raiz). */
    int ContarDirectorios() const;
    /** @brief Soma do tamanho de todos os ficheiros. */
    int Memoria() const;

    // ----------------------------------------
    // Diretórios
    /** @brief Diretoria com mais elementos. */
    std::optional<std::string> DirectoriaMaisElementos() const;
    /** @brief Diretoria com menos elementos. */
    std::optional<std::string> DirectoriaMenosElementos() const;
    /** @brief Diretoria com mais espaço total ocupado. */
    std::optional<std::string> DirectoriaMaisEspaco() const;

    // ----------------------------------------
    // Ficheiros
    /** @brief Ficheiro maior (caminho + tamanho). */
    std::optional<std::string> FicheiroMaior() const;
    /** @brief Data associada a um ficheiro. */
    std::optional<std::string> DataFicheiro(const std::string &Fich) const;

    // ----------------------------------------
    // Get / Set root
    /** @brief Define a raiz usada pelo serviço. */
    void SetRoot(std::shared_ptr<Directory> r);
    /** @brief Obtém a raiz atual. */
    std::shared_ptr<Directory> GetRoot() const;

    // ----------------------------------------
    // Operações sobre ficheiros / diretórios
    /** @brief Remove ficheiros/diretorias com nome alvo em toda a árvore. */
    bool RemoverAll(const std::string &s, const std::string &tipo);
    /** @brief Move um ficheiro para outra diretoria. */
    bool MoveFicheiro(const std::string &Fich, const std::string &DirNova);
    /** @brief Move uma diretoria (com subárvore) para outra diretoria. */
    bool MoverDirectoria(const std::string &DirOld, const std::string &DirNew);
    /** @brief Pesquisa por diretoria (1) ou ficheiro (0) e devolve o caminho. */
    std::optional<std::string> Search(const std::string &s, int Tipo) const;

    // ----------------------------------------
    // XML
    /** @brief Exporta a árvore em XML. */
    void Escrever_XML(const std::string &s);
    /** @brief Importa a árvore a partir de XML. */
    bool Ler_XML(const std::string &s);

    // ----------------------------------------
    // Tree (imprime a arvore em consola ou grava para ficheiro)
    /** @brief Imprime a árvore ou grava num ficheiro se indicado. */
    void Tree(const std::string *fich = nullptr);

    // ----------------------------------------
    // Pesquisar todas as diretorias com nome <dir> e colocar caminhos em <lres>
    /** @brief Lista caminhos de diretorias com determinado nome. */
    void PesquisarAllDirectorias(std::list<std::string> &lres, const std::string &dir);
    // ----------------------------------------
    // Pesquisar todos os ficheiros com nome <file> e colocar caminhos em <lres>
    /** @brief Lista caminhos de ficheiros com determinado nome. */
    void PesquisarAllFicheiros(std::list<std::string> &lres, const std::string &file);
    // ----------------------------------------
    // Copiar em batch: copia ficheiros cujo nome contenha <padrao> a partir de DirOrigem (incluindo sub-directorias) para a raiz de DirDestino.
    /** @brief Copia ficheiros do padrão (case-insensitive) da origem para a raiz do destino. */
    bool CopyBatch(const std::string &padrao, const std::string &DirOrigem, const std::string &DirDestino);

    // ----------------------------------------
    // Renomear todos os ficheiros com nome <fich_old> para <fich_new>
    /** @brief Renomeia todos os ficheiros com nome antigo para o novo. */
    void RenomearFicheiros(const std::string &fich_old, const std::string &fich_new);
    // ----------------------------------------
    // Verificar se existem ficheiros duplicados (mesmo nome)
    /** @brief Indica se existem ficheiros duplicados (mesmo nome). */
    bool FicheiroDuplicados() const;
    /** @brief Lista formatada dos ficheiros duplicados e respetivos caminhos. */
    std::vector<std::string> GetFicheirosDuplicados() const;

private:
    // ----------------------------------------
    // Funções auxiliares
    /** @brief Constrói o caminho absoluto de uma diretoria. */
    std::string getAbsolutePath(Directory* dir) const;

    /** @brief Preenche uma lista com todas as diretorias da subárvore. */
    void getAllDirectories(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<Directory>>& dirs) const;
    /** @brief Preenche uma lista com todos os ficheiros da subárvore. */
    void getAllFiles(std::shared_ptr<Directory> dir, std::list<std::shared_ptr<File>>& files) const;
};
