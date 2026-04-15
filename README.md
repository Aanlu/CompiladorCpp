# Analizador Léxico: Compilador Fase 1 - Motor Dinámico

Este repositorio contiene la implementación de la primera fase de nuestro compilador. A diferencia de un lexer estático, este sistema utiliza un **Motor de Autómata Finito Determinista (AFD)** que se configura dinámicamente mediante tablas externas (CSV).


### 1. Motor Léxico: Análisis del Funcionamiento
El motor (ubicado en `Lexer.cpp` y `Lexer.h`) opera bajo tres principios críticos que garantizan la integridad de los datos:

* **Algoritmo Maximal Munch (Bocado Máximo):** El motor es ambicioso por diseño. Si encuentra un `+`, no se detiene; mira el siguiente carácter. Si es otro `+`, entrega un `INCREMENTO`. Esto evita ambigüedades lógicas en operadores compuestos.
* **Gestión de Retroceso (Lookahead):** Cuando el autómata llega a un estado de error o de no aceptación, el motor tiene la capacidad de hacer *backtracking* al último estado "feliz" (de aceptación) guardado. Esto asegura que si una cadena falla a la mitad (ej. `12.a`), el motor pueda rescatar el `12` como entero y marcar el error justo donde empezó el fallo.
* **Rastreo Espacial Invariante:** Cada `Token` generado lleva consigo `linea` y `columna`. El Parser debe usar estos datos para emitir errores sintácticos profesionales. No basta con decir "error de sintaxis", el Parser deberá decir: "Error en Línea X: se esperaba ';' después de 'ID'".

### 2. Configuración Dinámica - El "Cerebro" en CSV
El comportamiento del compilador se define aquí. Si el Parser necesita un nuevo símbolo, el cambio debe hacerse en estas tablas, no en el código C++.

* **`matriz_transiciones.csv`**: Es la lógica de estados. 
    * **Estados Trampa (q4, q38, q40)**: Son fundamentales para la robustez. El estado `q40`, por ejemplo, es el que impide que una cadena sin cerrar se "coma" todo el código fuente. Si el motor entra en `q40`, cortará el token al detectar un salto de línea, protegiendo las instrucciones siguientes para que el Parser pueda procesarlas.
* **`char_columnas.csv`**: Define el alfabeto del lenguaje.
    * **Análisis de Separación**: Hemos separado estrictamente `[ESPACIO]` de `[SALTO_LINEA]`. Esto es vital para que el Lexer pueda detectar errores de "Newline in constant" (saltos de línea en cadenas o caracteres) antes de que lleguen al Parser.
* **`estados_tokens.csv`**: Es el mapa que traduce números de estado a categorías gramaticales. El Parser debe consultar el `enum TipoToken` en `Tokens.h`, el cual está sincronizado con este archivo.
* **`keywords.csv`**: Actúa como un filtro post-procesamiento. El motor primero identifica todo como un `ID` (Identificador) y luego hace un "lookup" en esta tabla. Si el lexema existe aquí (ej. `mientras`, `si`, `Imprimir`), el token se reclasifica automáticamente antes de ser entregado al Parser.

### 3. Interfaz de Comunicación: `Tokens.h`
Este archivo es el "contrato" entre ambas fases.
* **`TipoToken`**: Enumera todas las categorías posibles. El Parser basará su estructura de control (usualmente un `switch` o un árbol de decisiones) en estos tipos.
* **`struct Token`**: Es la unidad mínima de información. El Parser recibirá una `std::vector<Token>` limpia, sin espacios en blanco y sin comentarios, permitiéndole enfocarse exclusivamente en la jerarquía gramatical.

---

## Instrucciones de Uso

1.  **Carga de Tablas**: Asegúrate de que las rutas a los archivos CSV en el constructor del `Lexer` sean correctas en tu entorno local.
2.  **Modificación de la Gramática**: Para agregar nuevos operadores o palabras reservadas:
    * Añadir el lexema en `keywords.csv`.
    * Añadir el tipo en `nombres_tokens.csv` y en el `enum` de `Tokens.h`.
    * Actualizar la `matriz_transiciones.csv` si el símbolo requiere un nuevo camino de estados.
