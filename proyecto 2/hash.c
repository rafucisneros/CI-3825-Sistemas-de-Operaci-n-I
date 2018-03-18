# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <getopt.h>
//# include <unistd.h>
//# include <pthread.h>
//# include <ftw.h>
# define TRUE = 1==1
# define FALSE = !TRUE

// Flags
char *carpeta_inicial;
int altura;
char *archivo_indice;
int no_add;
int no_update;

// Estructura que contendra los diferentes archivos que se encuentran
// al buscar una palabra
typedef struct Nodo_Path {
    char *path;
    struct Nodo_Path *siguiente;
    } Nodo_Path;
    
// Estructura que contendra las llaves y la lista de paths que coinciden con la llave
typedef struct Nodo_Hash {
    char *llave;    // Palabra que representa
    Nodo_Path *archivos;    // Lista de archivos que contienen esa palabra
    struct Nodo_Hash *siguiente;    // Siguiente llave que entro en ese indice de la tabla Hash
} Nodo_Hash;

// Lista que guardara los archivos que contienen una palabra
typedef struct Lista_Llaves{
    struct Nodo_Hash *primer_elemento; // Nodo que contiene el primer item de la lista
    int numero_elementos;         // Cantidad de items en la lista
} Lista_Llaves;

typedef struct Indice{
    Lista_Llaves ** contenido;
    int tamano;
} Indice;


// Funcion que imprime los paths correspondientes a una llave
void imprimir_nodo(Nodo_Hash *nodo){
    printf("\tArchivos de la llave: %s\n", nodo->llave);
    Nodo_Path *paths = nodo->archivos;
    while(paths != NULL){
        printf("\t%s\n",paths->path);
        paths = paths->siguiente;
    }
}

// Funcion que imprime todos las llaves correspondiente a un indice de
// una tabla de hash
void imprimir_lista(Lista_Llaves *lista){
    Nodo_Hash *nodo = lista->primer_elemento;
    while(nodo != NULL){
        imprimir_nodo(nodo);
        nodo = nodo->siguiente;
    }
}

// Funcion que imprime todos los elementos de una tabla de hash
void imprimir_tabla(Indice *tabla){
    int i;
    for(i = 0; i < tabla->tamano; i++){
        printf("Elementos en el indice: %d. %d llaves.\n", i, tabla->contenido[i]->numero_elementos);
        imprimir_lista(tabla->contenido[i]);
        printf("\n");
    }
}

Indice *crear_indice(int tamano){
    Lista_Llaves ** contenido = (Lista_Llaves **) malloc(tamano * sizeof(Lista_Llaves *)); 
    Indice *tabla_de_hash = (Indice*) malloc(sizeof(struct Indice));
    for(int i=0; i<tamano;i++){
        contenido[i] = (Lista_Llaves *) malloc(sizeof(Lista_Llaves));
        contenido[i]->numero_elementos = 0;
        contenido[i]->primer_elemento = NULL;
    }
    tabla_de_hash->contenido = contenido;
    tabla_de_hash->tamano = tamano;
    return tabla_de_hash;
}

// Funcion de Hash para mapear las llaves
int hash(char *llave, int tamano_arreglo){
    size_t tamano = strlen(llave);
    int indice = 0;
    int i = 0;
    for(i=0; i<tamano; i++){
        indice = indice + llave[i];
    }
    return indice % tamano_arreglo;
}

// Funcion que busca en una tabla de hash una llave. Retorna un apuntador
// al nodo que coincide con la llave o NULL si no coincide ningun elemento
Nodo_Hash *buscar(Indice *tabla, char *llave){
    Nodo_Hash *nodo = (tabla->contenido[hash(llave, tabla->tamano)])->primer_elemento;
    if (nodo != NULL){ // Si la lista tiene nodos
        while(nodo != NULL){ // Mientras existan mas nodos
            if (!strcmp(nodo->llave, llave)){
                return nodo; // Si coinciden las llaves retornamos el nodo
            } else {
                nodo = nodo->siguiente; // Buscamos la siguiente llave del nodo
            } 
        }
    }
    return nodo;
}

// Funcion que inserta un Nodo con paths en una tabla de hash
void insertar_coleccion_paths(Indice* tabla, char *llave, Nodo_Path *archivos){
    int indice = hash(llave, tabla->tamano);
    Lista_Llaves *lista_actual = (Lista_Llaves *) tabla->contenido[indice]; 

    Nodo_Hash *nueva_coleccion = (Nodo_Hash *) malloc(sizeof(Nodo_Hash));
    nueva_coleccion->archivos = archivos;
    nueva_coleccion->llave = llave;
    nueva_coleccion->siguiente = NULL;

    if(!lista_actual->numero_elementos){
        lista_actual->primer_elemento = nueva_coleccion;
    } else {
        Nodo_Hash *nodo = lista_actual->primer_elemento;
        for(int i = 1; i < lista_actual->numero_elementos; i++) {
            nodo = nodo->siguiente;
        }
        nodo->siguiente = nueva_coleccion;
    }
    lista_actual->numero_elementos++;
}

Indice* rehash(Indice* tabla_antigua){
    int tamano_viejo = tabla_antigua->tamano;
    int tamano = tamano_viejo * 2 + 1;
    Indice *nueva_tabla = crear_indice(tamano);
    for(int i; i<tamano_viejo; i++){
        Lista_Llaves *lista_actual = tabla_antigua->contenido[i];
        if(lista_actual->numero_elementos == 0){
        } else {
            Nodo_Hash *nodo_a_reinsertar = lista_actual->primer_elemento;
            while(nodo_a_reinsertar != NULL){
                insertar_coleccion_paths(nueva_tabla, nodo_a_reinsertar->llave, nodo_a_reinsertar->archivos);
                nodo_a_reinsertar = nodo_a_reinsertar->siguiente;
            }
        }
    }
    free(tabla_antigua);
    return nueva_tabla;
}

// Funcion que inserta un path en todos las llaves con las cuales podria
// encontrarse al hacer una busqueda
Indice* insertar_llave_hash(Indice *tabla, char *path, int inicio){
    //printf("PATH %s\n", path);
    char c;
    int i = 0;
    int tamano = 0; // La cantidad de letras del substring
    c = path[i];

    // Saltamos los directorios
    int ultimo_directorio = 0;
    while(c != '\0'){
        i++;
        if (c == '/'){
            ultimo_directorio = i;
        }
        c = path[i];
    }
    inicio = ultimo_directorio;
    i = ultimo_directorio;
    c = path[i];
    while (c != '\0'){
        // Separadores de palabras
        while(c != ' ' && c != '/' && c != '.' && c != '\0'){
            tamano++;
            i++;
            c = path[i];
        }
        char *substring = (char *) calloc(tamano+1,sizeof(char)); // +1 para el \0
        if (tamano != 0){
            strncpy(substring,path+inicio,tamano);
            substring[tamano] == '\0';
            //printf("Substring %s\n", substring);
            // Insertar en la tabla de hash
            int indice = hash(substring, tabla->tamano);
            Nodo_Hash *nodo = buscar(tabla, substring);
            if (nodo == NULL){ // Nueva insercion en la tabla
                Nodo_Hash *nueva_llave = malloc(sizeof(Nodo_Hash));
                Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
                nuevo_path->path = path;
                nuevo_path->siguiente = NULL;
                nueva_llave->llave = substring;
                nueva_llave->archivos = nuevo_path;
                nueva_llave->siguiente = tabla->contenido[indice]->primer_elemento;
                tabla->contenido[indice]->primer_elemento = nueva_llave;
                tabla->contenido[indice]->numero_elementos++;

                if (tabla->contenido[indice]->numero_elementos > 3){ // Demasiadas colisiones
                    tabla = rehash(tabla);
                }
            } else { // llave ya incluida
                Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
                nuevo_path->path = path;
                nuevo_path->siguiente = nodo->archivos;
                nodo->archivos = nuevo_path; 
            }
            if (c != '\0'){
                // Siguiente palabra
                inicio = inicio + tamano + 1;
                tamano = 0; 
                i++;
                c = path[i];
            }
        } else {
            free(substring);
        }
    }

    return tabla;
}

void escribir_paths(FILE *f, Nodo_Path *path){
    while(path){
        fprintf(f, "%s\n", path->path);
        path = path->siguiente;
    }
}

void escribir_llave(FILE *f, Nodo_Hash *nodo){
    while(nodo){
        fprintf(f, "%s\n", nodo->llave);
        escribir_paths(f, nodo->archivos);
        fprintf(f, "/\n");
        nodo = nodo->siguiente;
    }
}

void escribir_indice(Indice *tabla, char *nombreIndice){
    FILE *f = fopen(nombreIndice, "w");
    if (f == NULL){
        return;
    }
    fprintf(f, "%d\n", tabla->tamano);
    fprintf(f, "/\n");
    for(int i=0;i < tabla->tamano; i++){
        if(tabla->contenido[i]->numero_elementos){
            escribir_llave(f, tabla->contenido[i]->primer_elemento);
        }
    }
    fclose(f);
}

void insertar_rapido(Indice* tabla, char* llavedir, char* pathdir){
    char* llave = (char *) calloc(strlen(llavedir),sizeof(char));
    strcpy(llave, llavedir);
    char* path = (char *) calloc(strlen(pathdir),sizeof(char));
    strcpy(path, pathdir);

    int indice = hash(llave, tabla->tamano);
    Nodo_Hash *nodo = buscar(tabla, llave);
    if(nodo == NULL){
        Nodo_Hash *nueva_llave = malloc(sizeof(Nodo_Hash));
        Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
        nuevo_path->path = path;
        nuevo_path->siguiente = NULL;
        nueva_llave->llave = llave;
        nueva_llave->archivos = nuevo_path;
        nueva_llave->siguiente = tabla->contenido[indice]->primer_elemento;
        tabla->contenido[indice]->primer_elemento = nueva_llave;
        tabla->contenido[indice]->numero_elementos++;
    } else {
        Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
        nuevo_path->path = path;
        nuevo_path->siguiente = nodo->archivos;
        nodo->archivos = nuevo_path; 
    }
}

Indice* leer_indice(char *nombreIndice){
    FILE *f = fopen(nombreIndice, "r");
    char buffer[2048];
    int tamano = 30;

    int scan_result = fscanf(f, "%s", buffer);
    Indice* tabla_hash = crear_indice(atoi(buffer));

    char keybuffer[2048];

    // char * line = NULL;
    // size_t len = 0;
    // ssize_t read;

    int keyswitch = 0;
    int sendup = 0;
    while(fscanf(f, "%s", buffer) != EOF){
        if(!strcmp(buffer, "/")){
            keyswitch = (-keyswitch) + 1;
        } else {
            if(keyswitch){
                strcpy(keybuffer, buffer);
                while (fgets(buffer, sizeof(buffer), f) != NULL){
                    buffer[strlen(buffer) - 1] = '\0'; // eat the newline fgets() stores
                    if(!strcmp(buffer, "/")) break;
                    if(sendup)
                    insertar_rapido(tabla_hash, keybuffer, buffer);
                    else sendup = 1;
                }
                sendup = 0;
            }
        }
    }
    return tabla_hash;
}

// Funcion que revisa si un archivo ya ha sido insertado en una tabla de hash
// Retorna 1 si el archivo ya estaba. Si el archivo no estaba, lo inserta y
// retorna 0
int buscar_path(Indice *tabla, char *path){

    //printf("PATH %s\n", path);
    char c;
    int i = 0;
    int tamano = 0; // La cantidad de letras del substring
    c = path[i];

    // Saltamos los directorios
    int ultimo_directorio = 0;
    while(c != '\0'){
        i++;
        if (c == '/'){
            ultimo_directorio = i;
        }
        c = path[i];
    }
    int inicio = ultimo_directorio;
    i = ultimo_directorio;
    c = path[i];
    while (c != '\0'){
        // Separadores de palabras
        while(c != ' ' && c != '/' && c != '.' && c != '\0'){
            tamano++;
            i++;
            c = path[i];
        }
        char *substring = (char *) calloc(tamano+1,sizeof(char)); // +1 para el \0
        if (tamano != 0){
            strncpy(substring,path+inicio,tamano);
            substring[tamano] == '\0';
			Nodo_Hash *nodo = buscar(tabla, substring);
			if (nodo == NULL){ // No hay un nodo con esa llave, por tanto el archivo no ha sido introducido
				insertar_llave_hash(tabla, path,0);
				break;
			} else { // Se encontro un nodo con esa llave
				// Revisamos si algun archivo es el que buscamos
				Nodo_Path *paths = nodo->archivos;
				while (paths != NULL){	// Mientras existan mas paths
					if (strcmp(paths->path,path)){ 	// Si son distintos
						paths = paths->siguiente;	
					} else { // Si son igualesa, terminamos
						return 1;
					}
				}
				insertar_llave_hash(tabla,path,0); // El archivo no es encontrado, se agrega
				break; 	
			}
		}
	}
	return 0;
}

int main(int argc, char **argv){
    Indice *tabla_hash = crear_indice(10);
    
    // 16 llaves distintas
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Casa de Papel.pdf",0);  
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Clase Proba.txt",0);  
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Ricitos de oro.pdf",0); 
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Arbol Binario.c",0);  
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Arbol Ternario.java",0);  
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Proyecto.c",0);      
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Proyecto 2.c",0);  
    //imprimir_tabla(tabla_hash);

    escribir_indice(tabla_hash, "IndiceHash.txt");
    Indice *nueva_tabla = leer_indice("IndiceHash.txt");
    //imprimir_tabla(nueva_tabla);
    

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
				//printf("option %s", long_options[option_index].name);
				//if (optarg)
					//printf(" with arg %s", optarg);
				//printf("\n");
				break;
			
			case 'u':
				no_update = 1;
				//puts ("option -u\n");
				break;

			case 'a':
				no_add = 1;
				//puts ("option -a\n");
				break;

			case 'd':
				carpeta_inicial = optarg;
				//printf ("option -d with value %s\n", optarg);
				break;

			case 'm':
				altura = atoi(optarg);
				//printf ("option -m with value %s\n", optarg);
				break;

			case 'i':
				archivo_indice = optarg;
				//printf ("option -i with value %s\n", optarg);
				break;

			case '?':
				break;

			default:
				abort();
		}
	}
	// Test Flags
/*
	printf("Altura: %d\n", altura);
	printf("carpeta_inicial: %s\n", carpeta_inicial);
	printf("no_add: %d\n", no_add);
	printf("no_update: %d\n", no_update);
	printf("archivo_indice: %s\n", archivo_indice);
*/

	//pthread_t hilo;
	//pthread_create(hilo,NULL,funcion_hilo,carpeta_inicial);
	
	// Buscador
/*
    // 16 llaves distintas
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Casa de Papel.pdf",0); 
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Clase Proba.txt",0); 
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Ricitos de oro.pdf",0); 
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Arbol Binario.c",0);   
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Arbol Ternario.java",0); 
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Proyecto.c",0);       
    tabla_hash = insertar_llave_hash(tabla_hash, "home/rafael/tarea/Proyecto 2.c",0);   
    imprimir_tabla(tabla_hash);
	int p = buscar_path(tabla_hash,"home/rafael/tarea/Proyecto 3.c");
	printf("ENCONTRADO: %d\n\n\n", p);
	imprimir_tabla(tabla_hash);
	Nodo_Hash *nodo = buscar(tabla_hash,argv[1]);
	printf("Buscando \"%s\"\n", argv[1]);
	if (nodo != NULL){
		imprimir_nodo(nodo);
	} else {
		printf("No se encontraron coincidencias");
	}
*/
}












