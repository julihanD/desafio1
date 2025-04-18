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

// Declaración de funciones originales
unsigned char* loadPixels(QString input, int &width, int &height);
bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida);
unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels);

int main()
{
    // ----------------- Bloque ORIGINAL -----------------
    // Definición de rutas de archivo de entrada (imagen original) y salida (imagen modificada)
    QString archivoEntrada = "I_O.bmp";
    QString archivoSalida = "I_D.bmp";

    // Variables para almacenar las dimensiones de la imagen
    int height = 0;
    int width = 0;

    // Carga la imagen BMP en memoria dinámica y obtiene ancho y alto
    unsigned char *pixelData = loadPixels(archivoEntrada, width, height);

    // Simula una modificación de la imagen asignando valores RGB incrementales
    // (Esto es solo un ejemplo de manipulación artificial)
    for (int i = 0; i < width * height * 3; i += 3) {
        pixelData[i]     = i;     // Canal rojo
        pixelData[i + 1] = i;     // Canal verde
        pixelData[i + 2] = i;     // Canal azul
    }

    // Exporta la imagen modificada a un nuevo archivo BMP (I_D.bmp)
    bool exportI = exportImage(pixelData, width, height, archivoSalida);
    cout << exportI << endl;

    // Libera la memoria usada para los píxeles
    delete[] pixelData;
    pixelData = nullptr;
    // -------------------------------------------------------

    // ----------------- Bloque NUEVO: Segundo paso -----------------
    // Se carga la imagen resultante del primer paso (I_D.bmp) y se rota cada byte 3 bits a la derecha.
    unsigned char *p1Data = loadPixels(archivoSalida, width, height);
    if (p1Data != nullptr) {
        int totalBytes = width * height * 3;
        unsigned char *rotatedData = new unsigned char[totalBytes];

        for (int i = 0; i < totalBytes; i++) {
            // Rotación a la derecha de 3 bits
            rotatedData[i] = (p1Data[i] >> 3) | (p1Data[i] << (8 - 3));
        }

        bool exportRotated = exportImage(rotatedData, width, height, "P2.bmp");
        cout << exportRotated << endl;

        delete[] rotatedData;
        delete[] p1Data;
    }
    // -----------------------------------------------------------------

    // ----------------- Bloque NUEVO: Tercer paso -----------------
    // Se carga la imagen del segundo paso (P2.bmp)
    unsigned char *p2Data = loadPixels("P2.bmp", width, height);
    // Se carga nuevamente la imagen IM (I_M.bmp)
    unsigned char *imData = loadPixels("I_M.bmp", width, height);

    if (p2Data != nullptr && imData != nullptr) {
        int totalBytes = width * height * 3;
        // Reservar un arreglo dinámico para la imagen resultante (P3.bmp)
        unsigned char *p3Data = new unsigned char[totalBytes];

        // Aplicar XOR byte a byte: p3Data = p2Data XOR imData
        for (int i = 0; i < totalBytes; i++) {
            p3Data[i] = p2Data[i] ^ imData[i];
        }

        bool exportP3 = exportImage(p3Data, width, height, "P3.bmp");
        cout << exportP3 << endl;
        delete[] p3Data;
    }

    // Liberar memorias de p2Data e imData (si se cargaron)
    if (p2Data)
        delete[] p2Data;
    if (imData)
        delete[] imData;
    // -----------------------------------------------------------------

    // ----------------- Bloque ORIGINAL: Lectura de enmascaramiento -----------------
    int seed = 0;
    int n_pixels = 0;
    unsigned int *maskingData = loadSeedMasking("M1.txt", seed, n_pixels);

    for (int i = 0; i < n_pixels * 3; i += 3) {
        cout << "Pixel " << i / 3 << ": ("
             << maskingData[i] << ", "
             << maskingData[i + 1] << ", "
             << maskingData[i + 2] << ")" << endl;
    }

    if (maskingData != nullptr) {
        delete[] maskingData;
        maskingData = nullptr;
    }

    return 0; // Fin del programa
}

unsigned char* loadPixels(QString input, int &width, int &height)
{
    // Cargar la imagen BMP usando QImage
    QImage imagen(input);
    if (imagen.isNull()) {
        cout << "Error: No se pudo cargar la imagen BMP: " << input.toStdString() << endl;
        return nullptr;
    }
    // Convertir la imagen al formato RGB888 (3 bytes por píxel, sin transparencia)
    imagen = imagen.convertToFormat(QImage::Format_RGB888);
    width = imagen.width();
    height = imagen.height();
    int dataSize = width * height * 3;
    unsigned char* pixelData = new unsigned char[dataSize];

    // Copiar los píxeles línea por línea (sin usar memcpy)
    for (int y = 0; y < height; ++y) {
        const uchar* srcLine = imagen.scanLine(y);
        for (int x = 0; x < width * 3; ++x) {
            pixelData[y * width * 3 + x] = srcLine[x];
        }
    }
    return pixelData;
}

bool exportImage(unsigned char* pixelData, int width, int height, QString archivoSalida)
{
    // Crear un QImage en formato RGB888 usando el arreglo pixelData
    QImage outputImage(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y) {
        uchar* dstLine = outputImage.scanLine(y);
        for (int x = 0; x < width * 3; ++x) {
            dstLine[x] = pixelData[y * width * 3 + x];
        }
    }
    if (!outputImage.save(archivoSalida, "BMP")) {
        cout << "Error: No se pudo guardar la imagen BMP: " << archivoSalida.toStdString() << endl;
        return false;
    } else {
        cout << "Imagen BMP guardada como " << archivoSalida.toStdString() << endl;
        return true;
    }
}

unsigned int* loadSeedMasking(const char* nombreArchivo, int &seed, int &n_pixels)
{
    // Abrir el archivo que contiene la semilla y los valores RGB
    ifstream archivo(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "No se pudo abrir el archivo." << endl;
        return nullptr;
    }

    // Leer la semilla de la primera línea
    archivo >> seed;
    int r, g, b;
    n_pixels = 0;
    while (archivo >> r >> g >> b) {
        n_pixels++;  // Cada grupo de tres valores corresponde a un píxel enmascarado
    }
    archivo.close();
    archivo.open(nombreArchivo);
    if (!archivo.is_open()) {
        cout << "Error al reabrir el archivo." << endl;
        return nullptr;
    }
    unsigned int* RGB = new unsigned int[n_pixels * 3];
    archivo >> seed;  // Se vuelve a leer la semilla (se descarta)
    for (int i = 0; i < n_pixels * 3; i += 3) {
        archivo >> r >> g >> b;
        RGB[i]     = r;
        RGB[i + 1] = g;
        RGB[i + 2] = b;
    }
    archivo.close();

    cout << "Semilla: " << seed << endl;
    cout << "Cantidad de píxeles leídos: " << n_pixels << endl;
    return RGB;
}
