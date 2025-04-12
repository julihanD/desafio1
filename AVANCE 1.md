AVANCE 1

En este avance quiero plasmar lo que he entendido del código, siento es importante leer e interpretar desde el inicio ya que el desafío lleva su complejidad. De esta forma plasmo mis ideas para poder trazar un seguimiento al desarrollo del código:

**Carga de una imagen BMP**:

- Utiliza la función loadPixels () para cargar una imagen BMP desde un archivo y extrae los datos de píxeles en formato **RGB** (sin transparencia).
- Convierte la imagen al formato **RGB888**, que representa cada píxel con tres canales (Rojo, Verde, Azul) de 8 bits.

**Modificación de la imagen**:

- En el main(), se simula una modificación artificial de los valores RGB de la imagen: asigna valores incrementales a los canales Rojo, Verde y Azul de cada píxel para demostrar una manipulación básica.

**Exportación de la imagen modificada**:

- Usa la función exportImage() para crear una nueva imagen BMP a partir de los datos de píxeles modificados y guardarla en un archivo especificado.
- Esto implica copiar los datos de los píxeles al objeto QImage y guardar el archivo con el formato BMP.

**Lectura de datos de enmascaramiento**:

- La función loadSeedMasking() carga datos de un archivo de texto que contiene:
  - Una **semilla** (valor entero) en la primera línea.
  - Una lista de valores **RGB** correspondientes al resultado de un proceso de enmascaramiento.
- Cuenta la cantidad de píxeles en el archivo y reserva memoria dinámica para almacenarlos.
- Retorna un arreglo con los valores RGB junto con la semilla y el número de píxeles.


Aun me falta por entender bien ciertas cosas del programa, específicamente la gestión de memoria dinámica. Me genera intriga cómo libera correctamente la memoria asignada para los datos de píxeles e información de enmascaramiento, evitando pérdidas de memoria



