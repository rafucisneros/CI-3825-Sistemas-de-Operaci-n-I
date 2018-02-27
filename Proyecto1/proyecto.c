// Sistemas de Operacion I
// Proyecto 1
// Rafael Cisneros 13-11156
// Erick Flejan 12-11555
# include <ftw.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <dirent.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
# include <semaphore.h>
# include <fcntl.h>
# include <signal.h>
#include <errno.h>
#include <sys/mman.h>

// Flags
char *flag_carpeta_inicial;
int flag_profundidad;
int flag_incluir_archivos;
// Semaforos
sem_t *semaforo_palindromos_principal, *semaforo_principal_palindromos;	
// Pipe
int pipe_padre_palindromos[2];

// Funcion que busca todos los palindromos contenidos en una palabra
int palindromo(char str[]){
    int l; // Sera la posicion mas izquierda de la palabra
    int h; // Sera la posicion mas a la derecha de la palabra
    int tamano = strlen(str); // Tamaño de la palabra introducida

	int i; // Centro desde se busca identificar si es palindromo
    for (i=1; i<tamano-1;i++){ // Empezamos tomando como centro la segunda letra y avanzamos hasta el final
		// Para buscar los palindromos pares
		l = i-1;
		h = i;		
		while (l >= 0 && h < tamano && str[l] == str[h]){
			if (h-l+1 > 2){ // Tomamos las palabras de tamaño mayor a 2
				printf("%.*s\n", h-l+1, str+l);			
			}
			h++; // Nos alejamos del centro una letra hacia la derecha
			l--; // Nos alejamos del centro una letra hacia la izquierda
		}
		// Para buscar los palindromos impares
		l = i-1;
		h = i+1;
		while (l >= 0 && h < tamano && str[l] == str[h]){
			if (h-l+1 > 2){ // Tomamos las palabras de tamaño mayor a 2
				printf("%.*s\n", h-l+1, str+l);				
			}
			h++; // Nos alejamos del centro una letra hacia la derecha
			l--; // Nos alejamos del centro una letra hacia la izquierda
		}
	}	
}

// Funcion que identifica las carpetas que no contienen nada
int directorio_vacio(const char *dirname) {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(dirname);
  if (dir == NULL) //Not a directory or doesn't exist
    return 1;
  while ((d = readdir(dir)) != NULL) {
    if(++n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) //Directory Empty
    return 1;
  else
    return 0;
}

// Acciones que se tomaran en cada nodo del arbol directorio
int accion_por_nodo(const char *nombre, const struct stat *inode, int algo) {
	// Hacemos algo solo cuando estamos en las hojas del arbol
	
	if(S_ISREG(inode->st_mode)){ // Chequeamos segun el modo del inode si es un archivo
		if (flag_incluir_archivos){		
			sem_wait(semaforo_palindromos_principal);
			char *buffer_nombre = nombre;
			write(pipe_padre_palindromos[1], buffer_nombre ,strlen(buffer_nombre));
			sem_post(semaforo_principal_palindromos);
		}
	}
	if(!(S_ISREG(inode->st_mode))){ // Chequeamos segun el modo del inode si es un directorio
		if(directorio_vacio(nombre)){ // Chequeamos si el directorio esta vacio
			sem_wait(semaforo_palindromos_principal);
			char *buffer_nombre = nombre;
			write(pipe_padre_palindromos[1], buffer_nombre ,strlen(buffer_nombre));
			sem_post(semaforo_principal_palindromos);
		}
	}
	return 0;
}

int main(int argc, char *argv[]){

	// Chequeamos flags
	flag_incluir_archivos = 0;
	flag_carpeta_inicial = ".";
	flag_profundidad = -1;
	char c;
	while ((c = getopt (argc, argv, "d:m:f")) != -1){
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
		}
	}

	// Creamos 1 proceso hijo que recibira por un pipe las paths para
	// buscar palindromos
	pid_t proceso_palindromos;
	// Iniciamos los semaforos para sincronizar los procesos
	semaforo_palindromos_principal = mmap(NULL, sizeof(*semaforo_palindromos_principal), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,
      -1, 0);
	semaforo_principal_palindromos = mmap(NULL, sizeof(*semaforo_principal_palindromos), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,
      -1, 0);
	
	sem_init(semaforo_palindromos_principal, 1, 1);
	sem_init(semaforo_principal_palindromos, 1, 0);
	// Creamos el pipe y el buffer para contener las 
	pipe(pipe_padre_palindromos);
	char buffer[10000];
	
	proceso_palindromos = fork();

	// Proceso Padre
    if (proceso_palindromos != 0){

		/*
		int i;
		for (i=0;i<2;i++){
			char *nombre = "anitalavalatinaarepera";
			sem_wait(semaforo_palindromos_principal);
			char *buffer_nombre = nombre;
			write(pipe_padre_palindromos[1], buffer_nombre ,strlen(buffer_nombre));
			sem_post(semaforo_principal_palindromos);
		}
		*/
		
		ftw(flag_carpeta_inicial, &accion_por_nodo, 1);
		kill(proceso_palindromos,1); // Matamos el hijo al finalizar el padre
		
	}
	// Proceso Palindromos
	else {
		while(1){
			sem_wait(semaforo_principal_palindromos);
			read(pipe_padre_palindromos[0], buffer, 10000);
			palindromo(buffer);
			sem_post(semaforo_palindromos_principal);
		}
	}
}


































