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
# include <errno.h>
# include <sys/mman.h>
# include <ctype.h>

// Flags
char *flag_carpeta_inicial;
int flag_profundidad;
int flag_incluir_archivos;
// Semaforos
sem_t *buffer_vacio, *buffer_lleno;	
// Pipe
int pipe_padre_palindromos[2];

// Funcion para parsear un path y eliminar / y acentos


int compare_acentos(char str[], int l, int h){
	if (str[h] > 0){ 
		return (str[l] == str[h]);
	}
	
	else {
		if (str[l-1] == str[h-1]){
			return (str[l] == str[h]);
		}
	}
	return 0;
}

int palindromo(char str[]){
    int l; // Sera la posicion mas izquierda de la palabra
    int h; // Sera la posicion mas a la derecha de la palabra
    int tamano = strlen(str); // Tamaño de la palabra introducida
	int i; // Centro desde se busca identificar si es palindromo
    int len;
    int first = 0;
    int cool;
    for (i=0; i<=tamano-1;i++){ // Empezamos tomando como centro la segunda letra y avanzamos hasta el final
		// Para buscar los palindromos pares
		if(str[i] < 0){
			i++;
		}
		first = 1;
		cool = 0;
		len = 0;
		l = i-1;
		h = i;
		if(str[i] < 0){
			l--;
		}

		while (l >= 0 && h < tamano){
			if(str[h] > 0){
				if (compare_acentos(str, l, h)){
					len += 2;
					if (cool) printf("%.*s\n", len, str+l);
					cool = 1;
				}

				else{
					break;
				}
				h++;

				if(str[h]<0){
					h++;
				}
				l--;
			}

			else {
				if (compare_acentos(str, l, h)){
					len += 4;
					if (cool) printf("%.*s\n", len, str+l-1);
					cool = 1;
				}
				else{
					break;
				}

				h++;
				if(str[h]<0){
					h++;
				}
				l--;
				l--;
			}
		}
		
		len = 1;
		l = i-1;
		h = i+1;
		if(str[i] < 0){
			l--;
		}
		if(str[h] < 0){
			h++;
		}

		while (l >= 0 && h < tamano){
			if(str[h] > 0){
				if (compare_acentos(str, l, h)){
					len += 2;
					printf("%.*s\n", len, str+l);
				}

				else{
					break;
				}
				h++;

				if(str[h]<0){
					h++;
				}
				l--;
			}

			else {
				if (compare_acentos(str, l, h)){
					len += 4;
					printf("%.*s\n", len, str+l-1);
				}
				else{
					break;
				}

				h++;
				if(str[h]<0){
					h++;
				}
				l--;
				l--;
			}
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
		if (flag_incluir_archivos){		// Chequeamos el flag de incluir archivos
			sem_wait(buffer_vacio);		// Esperamos que el buffer este vacio
			char *buffer_nombre = nombre;	// buffer para guardar el path a escribir
			write(pipe_padre_palindromos[1], buffer_nombre ,strlen(buffer_nombre)); // Escribimos en el pipe
			sem_post(buffer_lleno);		// Enviamos señal de que el buffer esta lleno
		}
	}
	if(!(S_ISREG(inode->st_mode))){ // Chequeamos segun el modo del inode si es un directorio
		if(directorio_vacio(nombre)){ // Chequeamos si el directorio esta vacio
			sem_wait(buffer_vacio);	// Esperamos que el buffer este vacio
			char *buffer_nombre = nombre; // buffer para guardar el path a escribir
			write(pipe_padre_palindromos[1], buffer_nombre ,strlen(buffer_nombre)); // Escribimos en el pipe
			sem_post(buffer_lleno);		// Enviamos señal de que el buffer esta lleno
		}
	}
	return 0;
}

int main(int argc, char *argv[]){
	// Inicializamos valores para los flags
	flag_incluir_archivos = 0;
	flag_carpeta_inicial = ".";
	flag_profundidad = -1;	
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
	buffer_lleno = mmap(NULL, sizeof(*buffer_lleno), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	buffer_vacio = mmap(NULL, sizeof(*buffer_vacio), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);	
	sem_init(buffer_vacio, 1, 1);
	sem_init(buffer_lleno, 1, 0);
	// Creamos el pipe y el buffer para contener los paths
	pipe(pipe_padre_palindromos);
	char buffer[10000];	
	// Creamos 1 proceso hijo que recibira por un pipe las paths "limpios" para
	// buscar palindromos
	pid_t proceso_palindromos;
	proceso_palindromos = fork();
	// Proceso Padre
    if (proceso_palindromos != 0){
		ftw(flag_carpeta_inicial, &accion_por_nodo, 1);  // Funcion que recorre el arbol de directorio
		kill(proceso_palindromos,1); // Matamos el hijo al finalizar el padre
	}
	// Proceso Palindromos
	else {
		while(1){
			sem_wait(buffer_lleno); // Esperamos que el buffer este lleno 
			read(pipe_padre_palindromos[0], buffer, 10000); // Leemos el pipe
			palindromo(buffer);	// Pasamos el path encontrado
			sem_post(buffer_vacio);	// Enviamos señal que el buffer esta vacio
		}
	}
}



