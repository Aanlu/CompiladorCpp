#include "Lexer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>

Lexer::Lexer(std::string fuente,
    std::string rutaMatriz,
    std::string rutaNombres,
    std::string rutaEstados,
    std::string rutaChars,
    std::string rutaKeywords)
    : codigo(fuente), pos(0), lineaActual(1), colActual(1),
    colAlpha(-1), colDigito(-1), colEspacio(-1), colSaltoLinea(-1), colOtros(-1)
{
    cargarNombresTokens(rutaNombres);
    cargarMatriz(rutaMatriz);
    cargarEstadosTokens(rutaEstados);
    cargarCharColumnas(rutaChars);
    cargarKeywords(rutaKeywords);
}

void Lexer::cargarNombresTokens(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) throw std::runtime_error("No se pudo abrir: " + ruta);
    std::string linea;
    bool primera = true;
    while (std::getline(archivo, linea)) {
        if (primera) { primera = false; continue; }
        std::stringstream ss(linea);
        std::string nombre, sValor;
        std::getline(ss, nombre, ',');
        std::getline(ss, sValor, ',');
        nombreATipo[nombre] = static_cast<TipoToken>(std::stoi(sValor));
    }
}

void Lexer::cargarMatriz(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) throw std::runtime_error("No se pudo abrir: " + ruta);
    std::string linea;
    bool primera = true;
    while (std::getline(archivo, linea)) {
        if (primera) { primera = false; continue; }
        std::vector<int> fila;
        std::stringstream ss(linea);
        std::string valor;
        bool primeraCol = true;
        while (std::getline(ss, valor, ',')) {
            if (primeraCol) { primeraCol = false; continue; }
            fila.push_back(std::stoi(valor));
        }
        matriz.push_back(fila);
    }
}

void Lexer::cargarEstadosTokens(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) throw std::runtime_error("No se pudo abrir: " + ruta);
    std::string linea;
    bool primera = true;
    while (std::getline(archivo, linea)) {
        if (primera) { primera = false; continue; }
        std::stringstream ss(linea);
        std::string sEstado, nombre;
        std::getline(ss, sEstado, ',');
        std::getline(ss, nombre, ',');

        if (nombreATipo.count(nombre)) estadoATipo[std::stoi(sEstado)] = nombreATipo[nombre];
        else throw std::runtime_error("Token desconocido en estados_tokens.csv: " + nombre);
    }
}

void Lexer::cargarCharColumnas(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) throw std::runtime_error("No se pudo abrir: " + ruta);
    std::string linea;
    bool primera = true;
    while (std::getline(archivo, linea)) {
        if (primera) { primera = false; continue; }
        std::stringstream ss(linea);
        std::string sChar, sCol;
        std::getline(ss, sChar, ',');
        std::getline(ss, sCol, ',');
        if (!sChar.empty()) {
            // Guardamos las etiquetas dinámicas leyendo el CSV
            if (sChar == "[ALPHA]") colAlpha = std::stoi(sCol);
            else if (sChar == "[DIGITO]") colDigito = std::stoi(sCol);
            else if (sChar == "[ESPACIO]") colEspacio = std::stoi(sCol);
            else if (sChar == "[SALTO_LINEA]") colSaltoLinea = std::stoi(sCol);
            else if (sChar == "[OTROS]") colOtros = std::stoi(sCol);           
            else charAColumna[sChar] = std::stoi(sCol);
        }
    }
}

void Lexer::cargarKeywords(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) throw std::runtime_error("No se pudo abrir: " + ruta);
    std::string linea;
    bool primera = true;
    while (std::getline(archivo, linea)) {
        if (primera) { primera = false; continue; }
        std::stringstream ss(linea);
        std::string lexema, nombre;
        std::getline(ss, lexema, ',');
        std::getline(ss, nombre, ',');

        if (nombreATipo.count(nombre)) diccionario[lexema] = nombreATipo[nombre];
        else throw std::runtime_error("Token desconocido en keywords.csv: " + nombre);
    }
}

int Lexer::obtenerColumna(char c) {
    if (isalpha(c) || c == '_') return colAlpha;
    if (isdigit(c))             return colDigito;

    // ESTO TIENE QUE IR PRIMERO
    if (c == '\n' || c == '\r') return colSaltoLinea;

    // ESTO VA DESPUÉS
    if (isspace(c))             return colEspacio;

    std::string s(1, c);
    auto it = charAColumna.find(s);
    if (it != charAColumna.end()) return it->second;

    // Si no es nada de lo anterior, devolvemos la columna configurada como OTROS
    return colOtros;
}

TipoToken Lexer::obtenerTipoPorEstado(int estado) {
    auto it = estadoATipo.find(estado);
    if (it != estadoATipo.end()) return it->second;
    return ERROR_LEX;
}

// Avanza los contadores de linea y columna de manera precisa
void Lexer::avanzarPosicion(size_t inicio, size_t fin) {
    for (size_t i = inicio; i < fin; i++) {
        if (codigo[i] == '\n') {
            lineaActual++;
            colActual = 1;
        }
        else {
            colActual++;
        }
    }
}

Token Lexer::siguienteToken() {
    if (pos >= codigo.length()) return { FIN, "EOF", lineaActual, colActual };

    int    estadoActual = 0;
    int    ultimoEstadoAceptado = -1;
    size_t posInicial = pos;
    size_t posGuardada = pos;

    // Guardamos la linea y columna al inicio del token
    int startLinea = lineaActual;
    int startCol = colActual;

    while (pos < codigo.length()) {
        char c = codigo[pos];
        int  col = obtenerColumna(c);
        int  sigEstado = matriz[estadoActual][col];

        // Validamos dinámicamente si es un comentario
        TipoToken tipoSig = obtenerTipoPorEstado(sigEstado);

        // Ya no necesitamos saber si venimos del estado 16, la matriz ya lo validó
        if (tipoSig == COMENTARIO_LINEA) {
            pos++;
            while (pos < codigo.length() && codigo[pos] != '\n') pos++;
            std::string lexema = codigo.substr(posInicial, pos - posInicial);
            avanzarPosicion(posInicial, pos);
            return { COMENTARIO_LINEA, lexema, startLinea, startCol };
        }

        if (tipoSig == COMENTARIO_BLOQUE) {
            pos++;
            bool cerrado = false;
            while (pos < codigo.length()) {
                if (codigo[pos] == '*' && pos + 1 < codigo.length() && codigo[pos + 1] == '/') {
                    pos += 2; cerrado = true; break;
                }
                pos++;
            }
            std::string lexema = codigo.substr(posInicial, pos - posInicial);
            avanzarPosicion(posInicial, pos);
            if (!cerrado) return { ERROR_LEX, "Comentario no cerrado: " + lexema, startLinea, startCol };
            return { COMENTARIO_BLOQUE, lexema, startLinea, startCol };
        }

        if (sigEstado == -1) break; // Transicion invalida

        if (estadoActual == 0 && sigEstado == 0) {
            // Absorber espacios sin crear tokens
            avanzarPosicion(pos, pos + 1);
            pos++;
            posInicial = pos;
            startLinea = lineaActual;
            startCol = colActual;
            continue;
        }

        estadoActual = sigEstado;
        pos++;

        if (estadoATipo.count(estadoActual)) {
            ultimoEstadoAceptado = estadoActual;
            posGuardada = pos;
        }
    }

    if (pos == posInicial && pos >= codigo.length()) {
        return { FIN, "EOF", startLinea, startCol };
    }

    // Ningun estado de aceptacion alcanzado (MAXIMAL MUNCH APLICADO)
    if (ultimoEstadoAceptado == -1) {
        std::string lexemaCorrupto = codigo.substr(posInicial, pos - posInicial);
        if (lexemaCorrupto.empty()) { // Si se estanco en el primer caracter
            pos = posInicial + 1;
            lexemaCorrupto = std::string(1, codigo[posInicial]);
        }
        avanzarPosicion(posInicial, pos);
        return { ERROR_LEX, lexemaCorrupto, startLinea, startCol };
    }

    // Regresar a estado seguro
    pos = posGuardada;
    std::string lexema = codigo.substr(posInicial, pos - posInicial);

    // Actualizamos contadores oficiales solo sobre lo que procesamos con exito
    avanzarPosicion(posInicial, pos);

    TipoToken tipoFinal = obtenerTipoPorEstado(ultimoEstadoAceptado);

    if (tipoFinal == ID && diccionario.count(lexema))
        tipoFinal = diccionario[lexema];

    return { tipoFinal, lexema, startLinea, startCol };
}

std::vector<Token> Lexer::generarListaTokens() {
    std::vector<Token> lista;
    Token tokenActual;
    do {
        tokenActual = siguienteToken();
        if (tokenActual.tipo != FIN) {
            lista.push_back(tokenActual);
        }
    } while (tokenActual.tipo != FIN);

    return lista;
}