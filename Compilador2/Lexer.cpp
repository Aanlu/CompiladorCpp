#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Constructor: Ingesta de código fuente y carga dinámica de la matriz
Lexer::Lexer(std::string fuente, std::string rutaMatriz) : codigo(fuente), pos(0) {
    cargarMatriz(rutaMatriz);
}

// Lógica de inyección de dependencias de datos
void Lexer::cargarMatriz(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        throw std::runtime_error("Error critico: No se pudo abrir el archivo de transiciones: " + rutaArchivo);
    }

    std::string linea;
    while (std::getline(archivo, linea)) {
        std::vector<int> fila;
        std::stringstream ss(linea);
        std::string valor;

        // Parseo de valores separados por coma
        while (std::getline(ss, valor, ',')) {
            fila.push_back(std::stoi(valor));
        }
        matriz.push_back(fila);
    }
    archivo.close();
}

int Lexer::obtenerColumna(char c) {
    if (isalpha(c) || c == '_') return 0;
    if (isdigit(c)) return 1;
    if (c == '.') return 2;
    if (c == '"') return 3;
    if (c == '+') return 4;
    if (c == '-') return 5;
    if (c == '/') return 6;
    if (c == '*') return 7;
    if (c == '=') return 8;
    if (c == '!') return 9;

    std::string simples = "[](){}&|^~><";
    if (simples.find(c) != std::string::npos) return 10;

    if (isspace(c)) return 11;
    return 12;
}

TipoToken Lexer::obtenerTipoPorEstado(int estado) {
    switch (estado) {
    case 1: return ID; case 2: return ENTERO; case 4: return DECIMAL;
    case 6: return CADENA; case 7: return MAS; case 8: return INCREMENTO;
    case 9: return MENOS; case 10: return DECREMENTO; case 11: return DIVISION;
    case 14: return ASIGNACION; case 15: return IGUALDAD; case 17: return DESIGUALDAD;
    case 18: return SIMBOLO; case 19: return MULTIPLICACION;
    default: return ERROR_LEX;
    }
}

Token Lexer::siguienteToken() {
    if (pos >= codigo.length()) return { FIN, "EOF" };

    int estadoActual = 0;
    int ultimoEstadoAceptado = -1;
    size_t posGuardada = pos;
    std::string lexema = "";

    // Ciclo principal de lectura del autómata
    while (pos < codigo.length()) {
        char c = codigo[pos];
        int col = obtenerColumna(c);
        int sigEstado = matriz[estadoActual][col];

        // Manejo de intercepción: Comentario de línea
        if (sigEstado == 12) {
            lexema += c;
            pos++;
            while (pos < codigo.length() && codigo[pos] != '\n') {
                if (codigo[pos] != '\r') lexema += codigo[pos];
                pos++;
            }
            return { COMENTARIO_LINEA, lexema };
        }

        // Manejo de intercepción: Comentario de bloque
        if (sigEstado == 13) {
            lexema += c;
            pos++;
            bool cerrado = false;
            while (pos < codigo.length()) {
                lexema += codigo[pos];
                if (codigo[pos] == '*' && pos + 1 < codigo.length() && codigo[pos + 1] == '/') {
                    lexema += '/';
                    pos += 2;
                    cerrado = true;
                    break;
                }
                pos++;
            }
            if (!cerrado) return { ERROR_LEX, "Comentario no cerrado: " + lexema };
            return { COMENTARIO_BLOQUE, lexema };
        }

        // Condición de salida
        if (sigEstado == -1) break;

        // Si el autómata está en inicio y lee un espacio/salto de línea, 
        // el estado sigue siendo 0. Avanzamos, pero NO concatenamos.
        if (estadoActual == 0 && sigEstado == 0) {
            pos++;
            continue;
        }

        // Consumo del carácter y actualización del estado
        lexema += c;
        estadoActual = sigEstado;
        pos++;

        // Validación de coincidencia más larga (Longest Match)
        if (estadoActual == 1 || estadoActual == 2 || estadoActual == 4 ||
            estadoActual == 6 || estadoActual == 7 || estadoActual == 8 ||
            estadoActual == 9 || estadoActual == 10 || estadoActual == 11 ||
            estadoActual == 14 || estadoActual == 15 || estadoActual == 17 ||
            estadoActual == 18 || estadoActual == 19) {
            ultimoEstadoAceptado = estadoActual;
            posGuardada = pos;
        }
    }

    // Manejo de error léxico
    if (ultimoEstadoAceptado == -1) {
        lexema = codigo[pos];
        pos++;
        return { ERROR_LEX, lexema };
    }

    // Backtracking
    pos = posGuardada;
    lexema = lexema.substr(0, pos - (pos - lexema.length()));

    TipoToken tipoFinal = obtenerTipoPorEstado(ultimoEstadoAceptado);

    // Resolución de Identificadores contra el diccionario
    if (tipoFinal == ID) {
        if (diccionario.count(lexema)) {
            tipoFinal = diccionario[lexema];
        }
    }
    return { tipoFinal, lexema };
}