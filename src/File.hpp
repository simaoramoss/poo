
#ifndef FILE_HPP
#define FILE_HPP

/**
 * @file File.hpp
 * @brief Declara a classe File.
 */

#include <string>
#include <ctime>

/**
 * @class File
 * @brief Representa um ficheiro com nome, tamanho e data (YYYY|MM|DD).
 */
class File {
private:
    std::string name;
    size_t size;
    std::string date; 

public:
    /**
     * @brief Constrói um ficheiro; a data é inicializada com o dia atual.
     * @param name Nome do ficheiro.
     * @param size Tamanho em bytes.
     */
    File(const std::string& name, size_t size);
    
    /** @brief Obtém o nome do ficheiro. */
    std::string getName() const;
    /** @brief Obtém o tamanho (bytes). */
    size_t getSize() const;
    /** @brief Obtém a data no formato YYYY|MM|DD. */
    std::string getDate() const;
    
    /** @brief Atualiza o nome. */
    void setName(const std::string& newName);
    /**
     * @brief Define uma data específica.
     * @param newDate Data no formato YYYY|MM|DD.
     */
    void setDate(const std::string& newDate);
};

#endif
