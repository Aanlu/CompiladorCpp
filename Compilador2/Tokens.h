#pragma once
#include <string>

// 1. Expansión estructural: Cada palabra clave tiene su propio identificador
enum TipoToken {
    ID, ENTERO, DECIMAL, CADENA, MAS, INCREMENTO, MENOS, DECREMENTO,
    MULTIPLICACION, DIVISION, ASIGNACION, IGUALDAD, DESIGUALDAD,
    SIMBOLO, FUNCION_PROPIA, ERROR_LEX, FIN, COMENTARIO_LINEA, COMENTARIO_BLOQUE,

    // Identificadores unívocos para palabras reservadas
    KW_ENTERO, KW_CLASE, KW_INTERRUPTOR, KW_CASO, KW_CARACTER, KW_CADENA_TIPO,
    KW_SI, KW_SINO, KW_DEMAS, KW_MIENTRAS, KW_HACER, KW_QUEBRAR,
    KW_VERDADERO, KW_FALSO, KW_BOOL, KW_DOBLE
};

// 2. Arreglo de traducción actualizado
const std::string NOMBRES_TOKENS[] = {
    "ID", "ENTERO", "DECIMAL", "CADENA", "MAS", "INCREMENTO", "MENOS", "DECREMENTO",
    "MULTIPLICACION", "DIVISION", "ASIGNACION", "IGUALDAD", "DESIGUALDAD",
    "SIMBOLO", "FUNCION_PROPIA", "ERROR", "FIN", "COMENT_LINEA", "COMENT_BLOQUE",
    "KW_ENTERO", "KW_CLASE", "KW_INTERRUPTOR", "KW_CASO", "KW_CARACTER", "KW_CADENA",
    "KW_SI", "KW_SINO", "KW_DEMAS", "KW_MIENTRAS", "KW_HACER", "KW_QUEBRAR",
    "KW_VERDADERO", "KW_FALSO", "KW_BOOL", "KW_DOBLE"
};

struct Token {
    TipoToken tipo;
    std::string lexema;
};