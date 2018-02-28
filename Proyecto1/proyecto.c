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
sem_t *buffer_padre_parser_lleno, *buffer_padre_parser_vacio;
sem_t *buffer_parser_palindromos_lleno, *buffer_parser_palindromos_vacio;
// Pipe
int pipe_padre_parser[2];
int pipe_parser_palindromos[2];

// Funcion que calcula la profundidad de un directorio
int calcular_profundidad(char str[]){
	int profundidad = 0; 
	char c;
	int i = 0;
	c = str[i];
	while (c != '\0'){
		if (c == '/'){ // Si encontramos un "/" subimos una profundidad
			profundidad++;
		}
		i++;
		c = str[i];
	}
	return profundidad;
}

// Funcion para parsear un path y eliminar / y acentos
char *parsear(char str[]){
	char parseado[strlen(str)];
	int i=0;
	int k = 0;
	while(str[i] != '\0'){	
		if (str[i]>0 && str[i]!='.' && str[i]!='/'){
			parseado[k] = tolower(str[i]);
			k++;
		}
		else if (str[i] == -61){
			parseado[k++] = str[i++];			
			switch(str[i]){
				case (-127): // Á
					parseado[k] = -95; // á
					k++;
					break;
				case (-119): // É
					parseado[k] = -87; // é
					k++;
					break;
				case (-115): // Ï
					parseado[k] = -83; // í
					k++;
					break;					
				case (-109): // Ó
					parseado[k] = -77; // ó
					k++;
					break;
				case (-102): // Ú
					parseado[k] = -70; // ú
					k++;
					break;					
				case (-111): // Ñ
					parseado[k] = -79; // ñ
					k++;
					break;
				case (-100): // Ü
					parseado[k] = -68; // ü
					k++;
					break;
				default:
					parseado[k] = str[i];
					k++;
					break;
			}
		}
		i++;
	}
	char *final = calloc(k+1,sizeof(char));
	int y = 0;
	while(y < k){
		final[y]=parseado[y];
		y++;
	}
	final[k] = '\0';
	return final;
}

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
    int specialChar;
	printf("Palindromos encontrados");

	// Si se encuentra con un caracter dentro del conjunto
	// {á, é, í, ó, ú, ñ, ü}, este ocupara 2 espacios en vez de solo
	// 1. Nos aseguraremos que las posiciones tomadas sean siempre las
	// de la segunda posicion
    for (i=0; i<=tamano-1;i++){ // Empezamos tomando como centro la segunda letra y avanzamos hasta el final
		// Para buscar los palindromos pares
		if(str[i] < 0){//Si el caracter i es especial 
			i++;       //Se apunta a su segunda posicion
		}
		specialChar = 0; //Valor identificador de un primer caracter especial
		len = 0; //Longitud del palindromo (+2 caracter especial)
		l = i-1;
		h = i;
		if(str[i] < 0){//Si el caracter i es especial 
			l--;       //Se asegura no apuntar con l a su primera posicion
		}

		while (l >= 0 && h < tamano){
			if(str[h] > 0){//Si h no es especial
				if (compare_acentos(str, l, h)){ //Comparamos igualdad
					len += 2; 
					if (specialChar) printf(", %.*s", len, str+l);
					specialChar = 1; //Ajuste al largo
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

			else {//Si h es especial
				if (compare_acentos(str, l, h)){
					len += 4;
					if (specialChar) printf(", %.*s", len, str+l-1);
					specialChar = 1;
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
					printf(", %.*s", len, str+l);
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
					printf(", %.*s", len, str+l-1);
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
	printf("\n");
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
	buffer_padre_parser_lleno = mmap(NULL, sizeof(*buffer_padre_parser_lleno), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	buffer_padre_parser_vacio = mmap(NULL, sizeof(*buffer_padre_parser_vacio), 
      PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);	
	sem_init(buffer_padre_parser_vacio, 1, 1);
	sem_init(buffer_padre_parser_lleno, 1, 0);
	
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
		kill(proceso_palindromos,1);
		kill(proceso_parser,1);
	}
	// Proceso Palindromos
	else if (proceso_palindromos == 0 && proceso_parser == -2){
		while(1){
			sem_wait(buffer_parser_palindromos_lleno); // Esperamos que el buffer este lleno			
			read(pipe_parser_palindromos[0], buffer_parser_palindromos, 10000); // Leemos el pipe
			palindromo(buffer_parser_palindromos);	// Pasamos el path encontrado
			//char *path;			
			//strcpy(path,buffer_parser_palindromos); // Parseamos el path para elimimar "/", "." y mayusculas 
			memset(buffer_parser_palindromos,0,10000);  // Vaciamos el buffer para evitar conflictos
			sem_post(buffer_parser_palindromos_vacio);	// Enviamos señal que el buffer esta vacio
			//palindromo(path);	// Pasamos el path encontrado
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

/*
			sem_wait(buffer_padre_parser_lleno); // Esperamos que el buffer este lleno
			read(pipe_padre_parser[0], buffer_padre_parser, 10000); // Leemos el pipe
			char *path;
			strcpy(path,buffer_padre_parser);
										printf("Entro %s \n", buffer_padre_parser);
			memset(buffer_padre_parser,0,10000);  // Vaciamos el buffer para evitar conflictos
			sem_post(buffer_padre_parser_vacio);	// Enviamos señal que el buffer esta vacio

			char *path_parseado = parsear(path); // Parseamos el path para elimimar "/", "." y mayusculas 
			if (path_parseado == 0){ // No se buscaran palindromos en ese path
				continue;
			}			
			sem_wait(buffer_parser_palindromos_vacio); // Esperamos que el buffer este vacio			
			write(pipe_parser_palindromos[1], path_parseado, strlen(path_parseado)); // Leemos el pipe
			sem_post(buffer_parser_palindromos_lleno);	// Enviamos señal que el buffer esta vacio
*/
		}
	}
}



