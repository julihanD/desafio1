/*
 * Programa demostrativo de manipulaciónprocesamiento de imágenes BMP en C++ usando Qt.
 *
 * Descripción:
 * Este programa realiza las siguientes tareas:
 * 1. Carga una imagen BMP desde un archivo (formato RGB sin usar estructuras ni STL).
 * 2. Modifica los valores RGB de los píxeles asignando un degradado artificial basado en su posición.
 * 3. Exporta la imagen modificada a un nuevo archivo BMP.
 * 4. Carga un archivo de texto que contiene una semilla (offset) y los resultados del enmascaramiento
 *    aplicados a una versión transformada de la imagen, en forma de tripletas RGB.
 * 5. Muestra en consola los valores cargados desde el archivo de enmascaramiento.
 * 6. Gestiona la memoria dinámicamente, liberando los recursos utilizados.
 *
 * Entradas:
 * - Archivo de imagen BMP de entrada ("I_O.bmp").
 * - Archivo de salida para guardar la imagen modificada ("I_D.bmp").
 * - Archivo de texto ("M1.txt") que contiene:
 *     • Una línea con la semilla inicial (offset).
 *     • Varias líneas con tripletas RGB resultantes del enmascaramiento.
 *
 * Salidas:
 * - Imagen BMP modificada ("I_D.bmp").
 * - Datos RGB leídos desde el archivo de enmascaramiento impresos por consola.
 *
 * Requiere:
 * - Librerías Qt para manejo de imágenes (QImage, QString).
 * - No utiliza estructuras ni STL. Solo arreglos dinámicos y memoria básica de C++.
 */


#include <fstream>
#include <iostream>
#include <QCoreApplication>
#include <QImage>

using namespace std;

// Prototipos de funciones ya existentes (parte del código base)
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);

// ====================================================================
// Nuevas funciones de operaciones bit a bit
// ====================================================================

/*
 * Función bitwiseXOR:
 * Realiza la operación XOR entre dos bytes.
 */
unsigned char bitwiseXOR(unsigned char a, unsigned char b) {
    return a ^ b;
}

/*
 * Función applyXOR:
 * Aplica la operación XOR a cada byte del arreglo 'data' usando la clave 'key'.
 */
void applyXOR(unsigned char* data, int size, unsigned char key) {
    for (int i = 0; i < size; i++) {
        data[i] = data[i] ^ key;
    }
}

/*
 * Función rotateRight:
 * Rota a la derecha los bits de un byte 'byte' en 'n' posiciones.
 */
unsigned char rotateRight(unsigned char byte, int n) {
    n = n % 8; // Asegurar que n esté en [0,7]
    return (byte >> n) | (byte << (8 - n));
}

/*
 * Función rotateLeft:
 * Rota a la izquierda los bits de un byte 'byte' en 'n' posiciones.
 */
unsigned char rotateLeft(unsigned char byte, int n) {
    n = n % 8;
    return (byte << n) | (byte >> (8 - n));
}

/*
 * Función shiftLeftOp:
 * Desplaza (shift left) un byte 'byte' en 'n' posiciones.
 */
unsigned char shiftLeftOp(unsigned char byte, int n) {
    return byte << n;
}

/*
 * Función shiftRightOp:
 * Desplaza (shift right) un byte 'byte' en 'n' posiciones.
 */
unsigned char shiftRightOp(unsigned char byte, int n) {
    return byte >> n;
}

// ====================================================================
// Función main
// ====================================================================
int main()
{
    // ---------------------------------------------------------------
    // PARTE 1: Procesamiento de la imagen base (I_O.bmp)
    // ---------------------------------------------------------------
    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";

    // Variables para almacenar las dimensiones de la imagen
    int width = 0, height = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);
    if (pixelData == nullptr) {
        cout << "Error: No se pudo cargar la imagen base. Terminando el programa." << endl;
        return 1;
    }

    int dataSize = width * height * 3;  // 3 canales: R, G, B

    // Se aplican operaciones bit a bit al buffer de píxeles:
    // 1. Aplicar XOR con una clave fija (ejemplo: 0xAA)
    unsigned char xorKey = 0xAA;
    applyXOR(pixelData, dataSize, xorKey);

    // 2. Rotar cada byte 3 bits a la derecha
    for (int i = 0; i < dataSize; i++) {
        pixelData[i] = rotateRight(pixelData[i], 3);
    }

    // 3. Rotar cada byte 3 bits a la izquierda (ejemplo para revertir)
    for (int i = 0; i < dataSize; i++) {
        pixelData[i] = rotateLeft(pixelData[i], 3);
    }

    // 4. Desplazar cada byte 1 bit a la izquierda (como ejemplo de operación)
    for (int i = 0; i < dataSize; i++) {
        pixelData[i] = shiftLeftOp(pixelData[i], 1);
    }

    // Exporta la imagen modificada a un nuevo archivo BMP
    bool exportI = exportImage(pixelData, width, height, archivoSalida);
    cout << "Exportación de imagen modificada: " << exportI << endl;

    // Libera la memoria usada para el buffer de píxeles
    delete[] pixelData;
    pixelData = nullptr;

    // ---------------------------------------------------------------
    // PARTE 2: Procesamiento de las máscaras de enmascaramiento
    // Se leen dos archivos de máscara (M1.txt y M2.txt) y se aplican
    // ejemplos de operaciones inversas sobre los valores RGB.
    // ---------------------------------------------------------------

    // Procesar la primera máscara (M1.txt)
    int seed1 = 0, n_pixels1 = 0;
    unsigned int *maskingData1 = loadSeedMasking("M1.txt", seed1, n_pixels1);
    if (maskingData1 != nullptr) {
        cout << "\nProcesando la primera máscara (M1.txt) con semilla: " << seed1 << endl;
        for (int i = 0; i < n_pixels1 * 3; i += 3) {
            // Cada píxel está representado por 3 valores (R, G, B)
            unsigned char R = static_cast<unsigned char>(maskingData1[i]);
            unsigned char G = static_cast<unsigned char>(maskingData1[i + 1]);
            unsigned char B = static_cast<unsigned char>(maskingData1[i + 2]);

            // Ejemplo de operaciones inversas:
            // Se aplica XOR con la semilla para intentar revertir una transformación XOR.
            R = bitwiseXOR(R, static_cast<unsigned char>(seed1));
            // Se rota a la izquierda 3 bits suponiendo que originalmente se rotó a la derecha.
            G = rotateLeft(G, 3);
            // Se desplaza a la derecha 1 bit (inverso al desplazamiento a la izquierda).
            B = shiftRightOp(B, 1);

            cout << "Píxel M1[" << (i / 3) << "] revertido: ("
                 << (int)R << ", " << (int)G << ", " << (int)B << ")" << endl;
        }
        delete[] maskingData1;
        maskingData1 = nullptr;
    }

    // Procesar la segunda máscara (M2.txt)
    int seed2 = 0, n_pixels2 = 0;
    unsigned int *maskingData2 = loadSeedMasking("M2.txt", seed2, n_pixels2);
    if (maskingData2 != nullptr) {
        cout << "\nProcesando la segunda máscara (M2.txt) con semilla: " << seed2 << endl;
        for (int i = 0; i < n_pixels2 * 3; i += 3) {
            unsigned char R = static_cast<unsigned char>(maskingData2[i]);
            unsigned char G = static_cast<unsigned char>(maskingData2[i + 1]);
            unsigned char B = static_cast<unsigned char>(maskingData2[i + 2]);

            // Operaciones inversas ejemplo:
            // Se aplica XOR con la semilla.
            R = bitwiseXOR(R, static_cast<unsigned char>(seed2));
            // Se rota a la derecha 3 bits para revertir una rotación previa.
            G = rotateRight(G, 3);
            // Se desplaza a la izquierda 1 bit.
            B = shiftLeftOp(B, 1);

            cout << "Píxel M2[" << (i / 3) << "] revertido: ("
                 << (int)R << ", " << (int)G << ", " << (int)B << ")" << endl;
        }
        delete[] maskingData2;
        maskingData2 = nullptr;
    }

    return 0;
}

// ====================================================================
// Funciones del código base (modificadas para eliminar memcpy)
// ====================================================================

unsigned char* loadPixels(QString input, int &width, int &height) {
    // Carga la imagen BMP usando QImage y la convierte a formato RGB888.
    QImage imagen(input);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP." << endl;
        return nullptr;
    }
    imagen = imagen.convertToFormat(QImage::Format_RGB888);

    width = imagen.width();
    height = imagen.height();
    int dataSize = width * height * 3;
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copia los datos de la imagen línea por línea, byte a byte, sin usar memcpy.
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);
        unsigned char* dstLine = pixelData + y * width * 3;
        for (int i = 0; i < width * 3; ++i) {
            dstLine[i] = srcLine[i];
        }
    }
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida) {
    QImage outputImage(width, height, QImage::Format_RGB888);
    // Copia el buffer de píxeles al objeto QImage, línea por línea, sin usar memcpy.
    for (int y = 0; y < height; ++y) {
        unsigned char* srcLine = pixelData + y * width * 3;
        uchar* dstLine = outputImage.scanLine(y);
        for (int i = 0; i < width * 3; ++i) {
            dstLine[i] = srcLine[i];
        }
    }
    if (!outputImage.save(archivoSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP modificada." << endl;
        return false;
    } else {
        cout << "Imagen BMP modificada guardada como " << archivoSalida.toStdString() << endl;
        return true;
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels) {
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo " << nombreArchivo << endl;
        return nullptr;
    }

    archivo >> seed;

    int r, g, b;
    // Contamos la cantidad de píxeles (después de la semilla, cada píxel = 3 valores)
    while (archivo >> r >> g >> b) {
        n_pixels++;
    }
    archivo.close();

    // Reabrir el archivo para leer los datos
    archivo.open(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo " << nombreArchivo << endl;
        return nullptr;
    }

    unsigned int* RGB = new unsigned int[n_pixels * 3];
    // Leer nuevamente la semilla (se descarta para los datos)
    archivo >> seed;
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i]     = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }
    archivo.close();

    cout << "\nArchivo " << nombreArchivo << ": Semilla = " << seed
         << ", Cantidad de píxeles leídos = " << n_pixels << endl;
    return RGB;
}
