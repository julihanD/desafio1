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
#include <QImage>
using namespace std;

// Función para cargar una imagen BMP y obtener sus píxeles en formato RGB888.
unsigned char* loadPixels(QString filename, int &width, int &height) {
    QImage image(filename);
    if(image.isNull()){
        cerr << "Error: No se pudo cargar " << filename.toStdString() << endl;
        return nullptr;
    }
    image = image.convertToFormat(QImage::Format_RGB888);
    width = image.width();
    height = image.height();
    int dataSize = width * height * 3;
    unsigned char* data = new unsigned char[dataSize];
    for (int y = 0; y < height; y++) {
        const uchar* line = image.scanLine(y);
        for (int x = 0; x < width * 3; x++) {
            data[y * width * 3 + x] = line[x];
        }
    }
    return data;
}

// Función para exportar un arreglo de píxeles (RGB888) a un archivo BMP.
bool exportImage(unsigned char* data, int width, int height, QString filename) {
    QImage image(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; y++) {
        uchar* line = image.scanLine(y);
        for (int x = 0; x < width * 3; x++) {
            line[x] = data[y * width * 3 + x];
        }
    }
    if(!image.save(filename, "BMP")){
        cerr << "Error al guardar " << filename.toStdString() << endl;
        return false;
    }
    return true;
}

// Función para leer el archivo de enmascaramiento (M1.txt).
// Se espera que la primera línea contenga la semilla y luego cada línea tres valores enteros (R G B)
// por cada píxel.
unsigned int* loadSeedMasking(const char* filename, int &seed, int &n_pixels) {
    ifstream file(filename);
    if(!file.is_open()){
        cerr << "Error abriendo " << filename << endl;
        return nullptr;
    }
    file >> seed;
    int r, g, b;
    n_pixels = 0;
    while (file >> r >> g >> b)
        n_pixels++;
    file.close();

    file.open(filename);
    if(!file.is_open()){
        cerr << "Error reabriendo " << filename << endl;
        return nullptr;
    }
    // Se crea un arreglo para almacenar 3 valores por píxel.
    unsigned int* mask = new unsigned int[n_pixels * 3];
    file >> seed; // Leer y descartar la semilla nuevamente.
    for (int i = 0; i < n_pixels; i++) {
        file >> r >> g >> b;
        mask[i * 3]     = r;
        mask[i * 3 + 1] = g;
        mask[i * 3 + 2] = b;
    }
    file.close();
    return mask;
}

int main() {
    int width = 0, height = 0, totalBytes;

    // ============================
    // Paso 1: P1 = I_O XOR I_M
    // ============================
    unsigned char* ioData = loadPixels("I_O.bmp", width, height);
    unsigned char* imData = loadPixels("I_M.bmp", width, height);
    if (!ioData || !imData) {
        delete [] ioData;
        delete [] imData;
        return -1;
    }
    totalBytes = width * height * 3;
    unsigned char* p1Data = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; i++) {
        p1Data[i] = ioData[i] ^ imData[i];
    }
    exportImage(p1Data, width, height, "P1.bmp");
    delete [] ioData;
    delete [] imData;

    // ============================
    // Paso 2: P2 = rotateRight(P1, 3)
    // ============================
    unsigned char* p1Loaded = loadPixels("P1.bmp", width, height);
    if(!p1Loaded) { delete [] p1Data; return -1; }
    unsigned char* p2Data = new unsigned char[totalBytes];
    // Rotación de 3 bits a la derecha para cada byte.
    for (int i = 0; i < totalBytes; i++) {
        p2Data[i] = (p1Loaded[i] >> 3) | (p1Loaded[i] << (8 - 3));
    }
    exportImage(p2Data, width, height, "P2.bmp");
    delete [] p1Loaded;
    delete [] p1Data;

    // ============================
    // Paso 3: P3 = P2 XOR I_M
    // ============================
    unsigned char* p2Loaded = loadPixels("P2.bmp", width, height);
    unsigned char* imReloaded = loadPixels("I_M.bmp", width, height);
    if(!p2Loaded || !imReloaded) {
        delete [] p2Loaded;
        delete [] imReloaded;
        delete [] p2Data;
        return -1;
    }
    unsigned char* p3Data = new unsigned char[totalBytes];
    for (int i = 0; i < totalBytes; i++) {
        p3Data[i] = p2Loaded[i] ^ imReloaded[i];
    }
    exportImage(p3Data, width, height, "P3.bmp");
    delete [] p2Loaded;
    delete [] imReloaded;
    delete [] p2Data;
    delete [] p3Data;

    // ============================
    // Lectura y Procesamiento de la Máscara (M1.txt)
    // ============================
    int seed = 0, n_pixels = 0;
    unsigned int* mask = loadSeedMasking("M1.txt", seed, n_pixels);
    if (mask) {
        cout << "Semilla: " << seed << endl;
        cout << "Cantidad de píxeles en la máscara: " << n_pixels << endl;
        // Si la máscara es de 10x10 (100 píxeles), se muestra en formato matricial.
        if(n_pixels == 100) {
            cout << "Máscara (10x10):" << endl;
            for (int i = 0; i < 10; i++) {
                for (int j = 0; j < 10; j++) {
                    int idx = (i * 10 + j) * 3;
                    cout << "(" << mask[idx] << "," << mask[idx + 1] << "," << mask[idx + 2] << ") ";
                }
                cout << endl;
            }
        } else {
            cout << "La máscara no es de 10x10." << endl;
        }
        delete [] mask;
    }

    return 0;
}
