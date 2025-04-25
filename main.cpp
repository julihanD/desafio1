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
#include <QCoreApplication>
#include <QImage>
#include <fstream>
#include <iostream>

    using namespace std;
// Carga los píxeles de una imagen BMP
unsigned char* loadPixels(QString path, int &width, int &height);
// Guarda los píxeles en una imagen BMP
bool exportImage(unsigned char* data, int width, int height, QString path);
// Carga la semilla y datos de enmascaramiento desde un archivo de texto
unsigned int* loadSeedMasking(const char* file, int &seed, int &n_pixels);

// Función para revertir el enmascaramiento (lineal):
// Se asume que "seed" es el offset en el buffer donde empieza la región afectada.
void desenmascarar(unsigned char* img, const unsigned char* mask,
                   const unsigned int* S, int seed, int totalBytes) {
    if (!img || !mask || !S || totalBytes <= 0)
        return;
    for (int k = 0; k < totalBytes; ++k) {
        // Se convierte el resultado (S[k] - mask[k]) a unsigned char (rango 0-255)
        img[seed + k] = static_cast<unsigned char>((S[k] - mask[k]) & 0xFF);
    }
}

// Operaciones a nivel de bits
static inline unsigned char bxor(unsigned char a, unsigned char b) {
    return a ^ b;
}

static inline unsigned char brotate_left(unsigned char v, unsigned k) {
    k &= 7; // Asegura que k esté en el rango 0-7 (para rotar dentro de un byte)
    return static_cast<unsigned char>(((v << k) | (v >> (8 - k))) & 0xFF);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    // Cargar imagen principal (P3.bmp)
    int w = 0, h = 0;
    unsigned char* img = loadPixels(QString("P3.bmp"), w, h);
    if (!img) return 1;

    // Cargar imagen de distorsión (I_M.bmp)
    int w2 = 0, h2 = 0;
    unsigned char* imRand = loadPixels(QString("I_M.bmp"), w2, h2);
    if (!imRand) {
        delete [] img;
        return 1;
    }
    if (w != w2 || h != h2) {
        cerr << "Error: Las imagenes deben tener las mismas dimensiones." << endl;
        delete [] img;
        delete [] imRand;
        return 1;
    }

    // Cargar imagen máscara (M.bmp)
    int mi = 0, mj = 0;
    unsigned char* mask = loadPixels(QString("M.bmp"), mi, mj);
    if (!mask) {
        delete [] img;
        delete [] imRand;
        return 1;
    }
    int totalMaskBytes = mi * mj * 3; // Total de bytes de la máscara

    // Cargar datos de enmascaramiento desde archivos de texto
    int seed1 = 0, n1 = 0;
    unsigned int* S1 = loadSeedMasking("M1.txt", seed1, n1);
    if (!S1) {
        delete [] img;
        delete [] imRand;
        delete [] mask;
        return 1;
    }
    int seed2 = 0, n2 = 0;
    unsigned int* S2 = loadSeedMasking("M2.txt", seed2, n2);
    if (!S2) {
        delete [] img;
        delete [] imRand;
        delete [] mask;
        delete [] S1;
        return 1;
    }

    int dataSize = w * h * 3; // Total en bytes de la imagen

    // ========================================================
    // Se aplican las operaciones inversas.
    // El orden y la forma de aplicar cada paso dependerán de
    // cómo se aplicaron originalmente las transformaciones.
    // ========================================================

    // Paso 3 inverso: Aplicar XOR con imRand a toda la imagen
    if (dataSize > 0 && (w2 * h2 * 3 == dataSize)) {
        for (int i = 0; i < dataSize; ++i) {
            img[i] = bxor(img[i], imRand[i]);
        }
        cout << "Paso 3 inverso: XOR aplicado(con im rand)." << endl;
    }

    // Paso 2 inverso: Desenmascarar usando S2 y luego rotar a la izquierda 3 bits
    if ((n2 * 3 >= totalMaskBytes) && ((seed2 + totalMaskBytes) <= dataSize)) {
        desenmascarar(img, mask, S2, seed2, totalMaskBytes);
        for (int i = 0; i < dataSize; ++i) {
            img[i] = brotate_left(img[i], 3);
        }
        cout << "Paso 2 inverso: Desenmascarado con S2 y rotacion aplicada." << endl;
    } else {
        cout << "S2: La  correccion no es valida." << endl;
    }

    // Paso 1 inverso: Desenmascarar usando S1 y luego aplicar XOR con imRand
    if ((n1 * 3 >= totalMaskBytes) && ((seed1 + totalMaskBytes) <= dataSize)) {
        desenmascarar(img, mask, S1, seed1, totalMaskBytes);
        for (int i = 0; i < dataSize; ++i) {
            img[i] = bxor(img[i], imRand[i]);
        }
        cout << "Paso 1 inverso: Desenmascarado con S1 y XOR aplicado." << endl;
    } else {
        cout << "S1: La region de correccion no es valida." << endl;
    }

    // Exportar la imagen resultante ("I_D.bmp")
    if (!exportImage(img, w, h, QString("I_D.bmp"))) {
        cerr << "Error al exportar la imagen I_D.bmp" << endl;
    } else {
        cout << "Imagen I_D.bmp exportada correctamente." << endl;
    }

    // Liberar memoria
    delete [] img;
    delete [] imRand;
    delete [] mask;
    delete [] S1;
    delete [] S2;

    return 0;
}

// -----------------------------------------------------------------------------
// Función loadPixels: Carga una imagen BMP, la convierte a formato RGB888 y copia
// sus datos en un buffer (sin incluir padding).
unsigned char* loadPixels(QString path, int &width, int &height) {
    QImage image(path);
    if (image.isNull()) {
        cerr << "Error al cargar: " << path.toStdString() << endl;
        return nullptr;
    }
    image = image.convertToFormat(QImage::Format_RGB888);
    width = image.width();
    height = image.height();
    int size = width * height * 3;
    unsigned char* buf = new unsigned char[size];
    // Inicialización a cero usando un bucle (ya que no usamos <cstring>)
    for (int i = 0; i < size; ++i) {
        buf[i] = 0;
    }
    for (int y = 0; y < height; ++y) {
        const uchar* line = image.scanLine(y);
        int offset = y * width * 3;
        for (int i = 0; i < width * 3; ++i) {
            buf[offset + i] = line[i];
        }
    }
    return buf;
}

// -----------------------------------------------------------------------------
// Función exportImage: Guarda un buffer de datos RGB en un archivo BMP.
bool exportImage(unsigned char* data, int width, int height, QString path) {
    QImage out(width, height, QImage::Format_RGB888);
    for (int y = 0; y < height; ++y) {
        uchar* line = out.scanLine(y);
        int offset = y * width * 3;
        for (int i = 0; i < width * 3; ++i) {
            line[i] = data[offset + i];
        }
    }
    return out.save(path, "BMP");
}

// -----------------------------------------------------------------------------
// Función loadSeedMasking: Lee la semilla y los tripletes RGB desde un archivo de texto.
// La semilla se encuentra en la primera línea; luego se leen enteros para cada triplete RGB.
unsigned int* loadSeedMasking(const char* file, int &seed, int &n_pixels) {
    ifstream f(file);
    if (!f) {
        cerr << "Error al abrir " << file << endl;
        return nullptr;
    }
    f >> seed;
    n_pixels = 0;
    int r = 0, g = 0, b = 0;
    while (f >> r >> g >> b)
        ++n_pixels;
    f.clear();
    f.seekg(0);
    f >> seed; // Releer la semilla para reposicionar el puntero
    unsigned int* S = new unsigned int[n_pixels * 3];
    // Inicializar S en 0 mediante bucle
    for (int i = 0; i < n_pixels * 3; ++i)
        S[i] = 0;
    for (int i = 0; i < n_pixels * 3; i += 3) {
        f >> S[i] >> S[i + 1] >> S[i + 2];
    }
    f.close();
    return S;
}

