#pragma once
#include "Tokens.h"
#include <vector>
#include <unordered_map>

class Lexer {
private:
    std::string codigo;
    size_t pos;

    // Matriz dinámica en memoria
    std::vector<std::vector<int>> matriz;

    // Diccionario mapeado a tokens específicos
    std::unordered_map<std::string, TipoToken> diccionario = {
        {"entero", KW_ENTERO}, {"clase", KW_CLASE}, {"interruptor", KW_INTERRUPTOR},
        {"caso", KW_CASO}, {"caracter", KW_CARACTER}, {"cadena", KW_CADENA_TIPO},
        {"si", KW_SI}, {"de_lo_contrario", KW_SINO}, {"demas", KW_DEMAS},
        {"mientras", KW_MIENTRAS}, {"hacer", KW_HACER}, {"quebrar", KW_QUEBRAR},
        {"verdadero", KW_VERDADERO}, {"falso", KW_FALSO}, {"bool", KW_BOOL}, {"doble", KW_DOBLE},
        {"Imprimir", FUNCION_PROPIA}, {"Leer", FUNCION_PROPIA}, {"LimpiarPantalla", FUNCION_PROPIA}
    };

    int obtenerColumna(char c);
    TipoToken obtenerTipoPorEstado(int estado);
    void cargarMatriz(const std::string& rutaArchivo);

public:
    Lexer(std::string fuente, std::string rutaMatriz);
    Token siguienteToken();
};