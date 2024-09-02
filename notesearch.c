#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "hacking.h"

#define FILENAME "/var/notes"

// Declaración de funciones
int print_notes(int, int, char *);   // Función para imprimir notas.
int find_user_note(int, int);        // Función para buscar una nota de usuario.
int search_note(char *, char *);     // Función para buscar una palabra clave en una nota.
void fatal(char *);                  // Manejo de errores fatales.

int main(int argc, char *argv[]) {
    int userid, printing = 1, fd; // Descriptor de archivo y otras variables
    char searchstring[100];

    // Verificar si se proporcionó un argumento (cadena de búsqueda)
    if (argc > 1)
        strcpy(searchstring, argv[1]);
    else
        searchstring[0] = 0;  // Cadena de búsqueda vacía si no hay argumentos

    userid = getuid();  // Obtener el ID de usuario
    fd = open(FILENAME, O_RDONLY);  // Abrir el archivo en modo de solo lectura
    if (fd == -1)
        fatal("in main() while opening file for reading"); // Manejar error al abrir archivo

    // Imprimir las notas mientras haya más para imprimir
    while (printing)
        printing = print_notes(fd, userid, searchstring);

    printf("-------[ end of note data ]-------\n");
    close(fd);  // Cerrar el archivo
    return 0;
}

// Función para imprimir las notas de un usuario que coincidan con una cadena de búsqueda
int print_notes(int fd, int uid, char *searchstring) {
    int note_length;
    char note_buffer[100];

    note_length = find_user_note(fd, uid);  // Buscar la próxima nota del usuario
    if (note_length == -1)  // Si se llega al final del archivo
        return 0;

    read(fd, note_buffer, note_length);  // Leer los datos de la nota
    note_buffer[note_length] = 0;  // Terminar la cadena

    if (search_note(note_buffer, searchstring))  // Si se encuentra la cadena de búsqueda
        printf(note_buffer);  // Imprimir la nota

    return 1;
}

// Función para encontrar la siguiente nota de un usuario específico
int find_user_note(int fd, int user_uid) {
    int note_uid = -1;
    unsigned char byte;
    int length;

    // Buscar hasta encontrar una nota para el user_uid
    while (note_uid != user_uid) {
        if (read(fd, &note_uid, 4) != 4)  // Leer el UID
            return -1;  // Si no se leen 4 bytes, se llega al final del archivo

        if (read(fd, &byte, 1) != 1)  // Leer el separador de nueva línea
            return -1;

        byte = length = 0;
        while (byte != '\n') {  // Calcular la longitud de la nota
            if (read(fd, &byte, 1) != 1)  // Leer un byte
                return -1;
            length++;
        }
    }

    lseek(fd, length * -1, SEEK_CUR);  // Rewind para leer la nota completa
    printf("[DEBUG] found a %d byte note for user id %d\n", length, note_uid);
    return length;
}

// Función para buscar una palabra clave en una nota
int search_note(char *note, char *keyword) {
    int i, keyword_length, match = 0;
    keyword_length = strlen(keyword);

    if (keyword_length == 0)  // Si no hay palabra clave
        return 1;  // Siempre es "match"

    for (i = 0; i < strlen(note); i++) {  // Iterar sobre los bytes en la nota
        if (note[i] == keyword[match])  // Si el byte coincide con la palabra clave
            match++;  // Prepararse para verificar el siguiente byte
        else {  // Si no coincide
            if (note[i] == keyword[0])  // Si coincide con el primer byte de la palabra clave
                match = 1;  // Empezar a contar el match
            else
                match = 0;  // No hay coincidencia
        }

        if (match == keyword_length)  // Si hay una coincidencia completa
            return 1;  // Retornar como coincidencia
    }

    return 0;  // Retornar como no coincidencia
}

// Manejo de errores fatales (definir la función según sea necesario)
void fatal(char *message) {
    perror(message);
    exit(1);
}
