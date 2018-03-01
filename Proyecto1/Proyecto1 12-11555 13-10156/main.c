// Sistemas de Operacion I
// Proyecto 1
// Rafael Cisneros 13-11156
// Erick Flejan 12-11555
# include <ftw.h>
# include <stdlib.h>
# include <stdio.h>
# include <dirent.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <semaphore.h>
# include <signal.h>
# include <errno.h>
# include <sys/mman.h>
# include "parser.h"
# include "palindromo.h"

// Flags
char *flag_carpeta_inicial;
int flag_profundidad;
int flag_incluir_archivos;
// Semaforos
sem_t *buffer_padre_parser_lleno, *buffer_padre_parser_vacio;
sem_t *buffer_parser_palindromos_lleno, *buffer_parser_palindromos_vacio;
// Pipe
int pipe_padre_parser[2];
int pipe_parser_palindromos[2];

// Funcion que identifica las carpetas que no contienen nada
int directorio_vacio(const char *dirname) {
	int n = 0;		// Numero de cosas en la carpeta
	struct dirent *d; // Structura con un numero de inode y el nombre
	DIR *dir = opendir(dirname); // Abrir el directorio
	while ((d = readdir(dir)) != NULL) {
		n++;		// Un directorio mas
	}
	closedir(dir); // Cerramos el directorio
	if (n <= 2){ 	//La carpeta esta vacia, todos los directorios tienen . y ..
		return 1;
	}
	else { // n > 0, tenemos al menos 1 elemento, la carpeta no es vacia
		return 0;
	}
}

// Acciones que se tomaran en cada nodo del arbol directorio
int accion_por_nodo(const char *nombre, const struct stat *inode, int algo) {
	// Hacemos algo solo cuando estamos en las hojas del arbol o excedimos la profundidad
	int profundidad = calcular_profundidad((char *)nombre);
	 if (profundidad > flag_profundidad){
		 return 0;
 }
	
	if(S_ISREG(inode->st_mode)){ // Chequeamos segun el modo del inode si es un archivo
		if (flag_incluir_archivos){		// Chequeamos el flag de incluir archivos
			sem_wait(buffer_padre_parser_vacio);		// Esperamos que el buffer este vacio
			char *buffer_nombre = (char*) nombre;	// buffer para guardar el path a escribir
			write(pipe_padre_parser[1], buffer_nombre ,strlen(buffer_nombre)); // Escribimos en el pipe
			sem_post(buffer_padre_parser_lleno);		// Enviamos señal de que el buffer esta lleno
		}
	}
	if(!(S_ISREG(inode->st_mode))){ // Chequeamos segun el modo del inode si es un directorio
		if((directorio_vacio(nombre)) || profundidad == flag_profundidad){ // Chequeamos si el directorio esta vacio o tiene la profundidad tope
			sem_wait(buffer_padre_parser_vacio);	// Esperamos que el buffer este vacio
			char *buffer_nombre = (char*) nombre; // buffer para guardar el path a escribir
			write(pipe_padre_parser[1], buffer_nombre ,strlen(buffer_nombre)); // Escribimos en el pipe
			sem_post(buffer_padre_parser_lleno);		// Enviamos señal de que el buffer esta lleno
		}
	}
	return 0;
}

// Funcion principal
int main(int argc, char *argv[]){
	// Inicializamos valores para los flags
	flag_incluir_archivos = 0;
	flag_carpeta_inicial = ".";
	flag_profundidad = 20;	
	char c; // Buffer para leer los flags
	while ((c = getopt (argc, argv, "d:m:f")) != -1){ // Leemos todos los flags enviados
		switch (c){
			case ('d'):
				flag_carpeta_inicial = optarg;
				break;
			case ('m'):
				flag_profundidad = atoi(optarg);
				break;
			case ('f'):
				flag_incluir_archivos = 1;
				break;				
			default:
				abort ();
		}	}

	// Iniciamos los semaforos en memoria compartida para sincronizar los procesos
	// Proceso Padre - Proceso Parser
	buffer_padre_parser_lleno = mmap(NULL, sizeof(*buffer_padre_parser_lleno), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	buffer_padre_parser_vacio = mmap(NULL, sizeof(*buffer_padre_parser_vacio), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);	
	sem_init(buffer_padre_parser_vacio, 1, 1);
	sem_init(buffer_padre_parser_lleno, 1, 0);
	// Proceso Parser - Proceso Palindromos
	buffer_parser_palindromos_lleno = mmap(NULL, sizeof(*buffer_parser_palindromos_lleno), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	buffer_parser_palindromos_vacio = mmap(NULL, sizeof(*buffer_parser_palindromos_vacio), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);	
	sem_init(buffer_parser_palindromos_vacio, 1, 1);
	sem_init(buffer_parser_palindromos_lleno, 1, 0);
	// Creamos el pipe y el buffer para contener los paths
	pipe(pipe_padre_parser);
	char buffer_padre_parser[10000];	
	pipe(pipe_parser_palindromos);
	char buffer_parser_palindromos[10000];	
	// Creamos 1 proceso hijo que recibira por un pipe las paths "limpios" para
	// buscar palindromos
	pid_t proceso_palindromos;
	proceso_palindromos = fork();
	// Creamos un segundo hijo que tendra valor -2 en el primer hijo para distinguir
	pid_t proceso_parser = -2;
    if (proceso_palindromos != 0){
		proceso_parser = fork();
	}
	
	// Proceso Padre
    if (proceso_palindromos != 0 && proceso_parser != 0){
		ftw(flag_carpeta_inicial, &accion_por_nodo, 1);  // Funcion que recorre el arbol de directorio		
		// Matamos los hijos al terminar el proceso padre
		kill(proceso_palindromos,1);
		kill(proceso_parser,1);
	}
	// Proceso Palindromos
	else if (proceso_palindromos == 0 && proceso_parser == -2){
		while(1){
			sem_wait(buffer_parser_palindromos_lleno); // Esperamos que el buffer este lleno			
			read(pipe_parser_palindromos[0], buffer_parser_palindromos, 10000); // Leemos el pipe
			palindromo(buffer_parser_palindromos);	// Pasamos el path encontrado
			
			memset(buffer_parser_palindromos,0,10000);  // Vaciamos el buffer para evitar conflictos
			sem_post(buffer_parser_palindromos_vacio);	// Enviamos señal que el buffer esta vacio
		}
	} // Proceso parser
	else {
		while(1){
			sem_wait(buffer_padre_parser_lleno); // Esperamos que el buffer este lleno
			read(pipe_padre_parser[0], buffer_padre_parser, 10000); // Leemos el pipe
			char *path_parseado = parsear(buffer_padre_parser); // Parseamos el path para elimimar "/", "." y mayusculas 
			if (path_parseado == 0){ // No se buscaran palindromos en ese path
				memset(buffer_padre_parser,0,10000);  // Vaciamos el buffer para evitar conflictos
				sem_post(buffer_padre_parser_vacio);	// Enviamos señal que el buffer esta vacio
				continue;
			}
			memset(buffer_padre_parser,0,10000);  // Vaciamos el buffer para evitar conflictos
			sem_post(buffer_padre_parser_vacio);	// Enviamos señal que el buffer esta vacio

			sem_wait(buffer_parser_palindromos_vacio); // Esperamos que el buffer este vacio			
			write(pipe_parser_palindromos[1], path_parseado, strlen(path_parseado)); // Leemos el pipe
			sem_post(buffer_parser_palindromos_lleno);	// Enviamos señal que el buffer esta vacio
		}
	}
}