#pragma once
#include "Tokens.h"
#include <vector>
#include <unordered_map>
#include <string>

class Lexer {
private:
    std::string codigo;
    size_t pos;
    int lineaActual;
    int colActual;

    // Variables dinamicas para columnas especiales
    int colAlpha;
    int colDigito;
    int colEspacio;
    int colSaltoLinea;
    int colOtros;

    std::vector<std::vector<int>> matriz;
    std::unordered_map<std::string, TipoToken> nombreATipo;
    std::unordered_map<int, TipoToken> estadoATipo;
    std::unordered_map<std::string, int> charAColumna; // Ahora es string para aceptar etiquetas
    std::unordered_map<std::string, TipoToken> diccionario;

    void cargarNombresTokens(const std::string& ruta);
    void cargarMatriz(const std::string& ruta);
    void cargarEstadosTokens(const std::string& ruta);
    void cargarCharColumnas(const std::string& ruta);
    void cargarKeywords(const std::string& ruta);

    int       obtenerColumna(char c);
    TipoToken obtenerTipoPorEstado(int estado);
    Token     siguienteToken();
    void      avanzarPosicion(size_t inicio, size_t fin);

public:
    Lexer(std::string fuente,
        std::string rutaMatriz,
        std::string rutaNombres,
        std::string rutaEstados,
        std::string rutaChars,
        std::string rutaKeywords);

    std::vector<Token> generarListaTokens();
};