# define _XOPEN_SOURCE 500 // Para usar nftw
# include <limits.h> // Para usar PATH_MAX
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <getopt.h>
# include <unistd.h>
# include <pthread.h>
# include <ftw.h>
# include <stdint.h>
# include <semaphore.h>
# include "hash.h"
# define TRUE = 1==1
# define FALSE = !TRUE


// Flags
char *carpeta_inicial;
int altura;
char *archivo_indice;
int no_add;
int no_update;
// Semaforo
pthread_mutex_t escribiendo_tabla;
// Tabla de Hash
Indice *tabla_hash;



// Acciones que se tomaran en cada nodo del arbol directorio
int accion_por_nodo(const char *path, const struct stat *inode, int tflags, struct FTW *ftwbuf) {
	if (ftwbuf->level > altura){ // La profundidad maxima a buscar
			return 2; // 2 = Saltar hijos
	} else {
		char full_path [PATH_MAX];
		char *ptr;
		ptr = realpath(path, full_path);
		int incluido = buscar_path(tabla_hash,ptr);
		if (incluido && no_update){ // Si tenemos el flag no update y encontramos una carpeta ya incluida
			return 2; // 2 = Saltar hijos
		}
		if (!incluido && no_add){ // Si tenemos el flag no add y encontra una carpeta no incluida
			return 2; // 2 = Saltar hijos
		}	
	}
	return 0;
}

// Funcion para el hilo indizador
void *indizador(){
	// Recorrido del arbol
	nftw(carpeta_inicial,&accion_por_nodo,1,16); // 16 es el flag para nftw
	escribir_indice(tabla_hash, archivo_indice);
}

// Funcion para el hilo buscador
void *buscador(void *termino_busqueda){
	// Buscador
	Nodo_Hash *nodo = buscar(tabla_hash,(char *) termino_busqueda);
	//printf("Buscando \"%s\"\n", (char *) termino_busqueda);
	if (nodo != NULL){
		//imprimir_nodo(nodo);
		imprimir_nodo_simple(nodo);
	} else {
		printf("No se encontraron coincidencias\n");
	}
}

int main(int argc, char **argv){
	
	// Flags
	carpeta_inicial = ".";
	altura = 20;
	archivo_indice = "Index.txt";
	no_add = 0;
	no_update = 0;

    int c;
    while (1){    
        static struct option long_options[] =
            {   
                {"noupdate", no_argument, 0, 'u'},
                {"noadd", no_argument, 0, 'a'},    
                {"dir", required_argument, 0, 'd'},
                {"max", required_argument, 0, 'm'},
                {"index", required_argument, 0, 'i'},
                {0,0,0,0}
		};

		int option_index = 0;

		c = getopt_long (argc, argv, "uad:m:i:", long_options, &option_index);

		if(c == -1){
			break;
		}
		
		switch (c) {
			case 0:
				if (long_options[option_index].flag != 0)
					break;
				break;
			
			case 'u':
				no_update = 1;
				break;

			case 'a':
				no_add = 1;
				break;

			case 'd':
				carpeta_inicial = optarg;
				break;

			case 'm':
				altura = atoi(optarg);
				break;

			case 'i':
				archivo_indice = optarg;
				break;

			case '?':
				break;

			default:
				abort();
		}
	}
	FILE *indice_entrante;
	if(indice_entrante = fopen(archivo_indice,"r")){ // Si se logra abrir el archivo cargamos la tabla desde ahi
		tabla_hash = leer_indice(archivo_indice);
	} else { // Creamos una nueva tabla vacia
		tabla_hash = crear_indice(30);
		indice_entrante = fopen(archivo_indice,"w");
		fclose(indice_entrante);
	}
	// Iniciamos el mutex para exclusion mutua
    if (pthread_mutex_init(&escribiendo_tabla, NULL) != 0){
        printf("No se pude iniciar el mutex para exclusion mutua.\n");
        return EXIT_FAILURE;
    }
	pthread_t hilo_indizador, hilo_buscador;
	pthread_create(&hilo_indizador, NULL, indizador, NULL);
	pthread_create(&hilo_buscador, NULL, buscador, (void *)argv[argc-1]);
	pthread_join(hilo_indizador, NULL);
	pthread_join(hilo_buscador, NULL);
    pthread_mutex_destroy(&escribiendo_tabla);
}












