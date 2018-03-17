# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# define TRUE = 1==1
# define FALSE = !TRUE

// Estructura que contendra los diferentes archivos que se encuentran
// al buscar una palabra
typedef struct Nodo_Path {
	char *path;
	struct Nodo_Path *siguiente;
	} Nodo_Path;
	
// Estructura que contendra las llaves y la lista de paths que coinciden con la llave
typedef struct Nodo_Hash {
	char *llave;	// Palabra que representa
	Nodo_Path *archivos;	// Lista de archivos que contienen esa palabra
	struct Nodo_Hash *siguiente;	// Siguiente llave que entro en ese indice de la tabla Hash
} Nodo_Hash;

// Lista que guardara los archivos que contienen una palabra
typedef struct Lista_Llaves{
	struct Nodo_Hash *primer_elemento; // Nodo que contiene el primer item de la lista
	int numero_elementos;		  // Cantidad de items en la lista
} Lista_Llaves;

// Funcion de Hash para mapear las llaves
int Hash(char *llave, int tamano_arreglo){
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
Nodo_Hash *Buscar(Lista_Llaves **tabla, int tamano_arreglo, char *llave){
	//int indice = Hash(llave, tamano_arreglo);
	Nodo_Hash *nodo = tabla[Hash(llave, tamano_arreglo)]->primer_elemento;
	if (nodo != NULL){ // Si la lista tiene nodos
		while(nodo != NULL){ // Mientras existan mas nodos
			if (!strcmp(nodo->llave, llave)){
				return nodo; // Si coinciden las llaves retornamos el nodo
			} else {
				nodo = nodo->siguiente;	// Buscamos la siguiente llave del nodo
			} 
		}
	}
	return nodo;
}

// Funcion que imprime los paths correspondientes a una llave
void Imprimir_Nodo(Nodo_Hash *nodo){
	printf("Archivos para la llave: %s\n", nodo->llave);
	Nodo_Path *paths = nodo->archivos;
	while(paths != NULL){
		printf("%s\n",paths->path);
		paths = paths->siguiente;
	}
}

// Funcion que imprime todos las llaves correspondiente a un indice de
// una tabla de hash
void Imprimir_Lista(Lista_Llaves *lista){
	Nodo_Hash *nodo = lista->primer_elemento;
	while(nodo != NULL){
		Imprimir_Nodo(nodo);
		nodo = nodo->siguiente;
	}
}

// Funcion que imprime todos los elementos de una tabla de hash
void Imprimir_Tabla(Lista_Llaves **tabla, int tamano){
	int i;
	for(i = 0; i < tamano; i++){
		printf("Elementos en el indice: %d\n", i);
		Imprimir_Lista(tabla[i]);
	}
}

// Funcion que inserta un Nodo con paths en una tabla de hash
void Insertar_Coleccion_Paths(Lista_Llaves **tabla, int tamano_tabla, Nodo_Hash* coleccion){
	int indice = Hash(coleccion->llave, tamano_tabla);
	Lista_Llaves *lista_actual = (Lista_Llaves *) tabla[indice];	
	if(!lista_actual->numero_elementos){
		lista_actual->primer_elemento = coleccion;
	} else {
		Nodo_Hash *nodo = lista_actual->primer_elemento;
		for(int i = 1; i < lista_actual->numero_elementos; i++) {
			nodo = nodo->siguiente;
		}
		nodo->siguiente = coleccion;
	}
	lista_actual->numero_elementos++;
}

// Funcion que hace rehash sobre una tabla de hash
Lista_Llaves* rehash(Lista_Llaves **tabla_temp, int *tamano){
	int tamano_viejo = *tamano;
	*tamano = *tamano * 2 + 1;	
	Lista_Llaves *tabla_hash[*tamano];
	int i, j;
	for(i=0; i<*tamano;i++){
		tabla_hash[i] = (Lista_Llaves *) malloc(sizeof(Lista_Llaves));
		tabla_hash[i]->numero_elementos = 0;
		tabla_hash[i]->primer_elemento = NULL;
	}
	for(i=0; i<tamano_viejo; i++){
		Lista_Llaves *lista_actual = (Lista_Llaves *) tabla_temp[i];
		if(lista_actual->numero_elementos){
			continue;
		} else {
			Nodo_Hash *nodo_a_insertar = lista_actual->primer_elemento;
			while(nodo_a_insertar != NULL){
				Insertar_Coleccion_Paths(tabla_hash,*tamano,nodo_a_insertar);
				nodo_a_insertar = nodo_a_insertar->siguiente;
			}
		}
	}
	free(tabla_temp);
	return(&tabla_hash);
}

// Funcion que inserta un path en todos las llaves con las cuales podria
// encontrarse al hacer una busqueda
void Insertar_Llave_Hash(Lista_Llaves **tabla, int tamano_tabla, char *path, int inicio){
	printf("PATH %s\n", path);
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
			printf("Substring %s\n", substring);
			// Insertar en la tabla de hash
			int indice = Hash(substring, tamano_tabla);
			Nodo_Hash *nodo = Buscar(tabla, tamano_tabla, substring);
			if (nodo == NULL){ // Nueva insercion en la tabla
				if (tabla[indice]->numero_elementos > 10){ // Demasiadas colisiones
					//*tabla = rehash(tabla,&tamano_tabla);
					//Insertar_Llave_Hash(tabla,tamano_tabla,path,inicio); // REVISAR LOS ELEMENTOS QUE YA HABIAN SIDO INTRODUCIDOS
				} else {
					Nodo_Hash *nueva_llave = malloc(sizeof(Nodo_Hash));
					Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
					nuevo_path->path = path;
					nuevo_path->siguiente = NULL;
					nueva_llave->llave = substring;
					nueva_llave->archivos = nuevo_path;
					nueva_llave->siguiente = tabla[indice]->primer_elemento;
					tabla[indice]->primer_elemento = nueva_llave;
					tabla[indice]->numero_elementos++;			
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
}


int main(){
	int tamano_tabla = 30;
	Lista_Llaves *tabla_hash[tamano_tabla]; 
	int i;
	for(i=0; i<30;i++){
		tabla_hash[i] = (Lista_Llaves *) malloc(sizeof(Lista_Llaves));
		tabla_hash[i]->numero_elementos = 0;
		tabla_hash[i]->primer_elemento = NULL;
	}
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Casa de Papel.pdf",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Clase Proba.txt",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Ricitos de oro.pdf",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Arbol Binario.c",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Arbol Ternario.java",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Proyecto.c",0);					
	printf("\n");
	Insertar_Llave_Hash(tabla_hash, tamano_tabla, "home/rafael/tarea/Proyecto 2.c",0);					
	printf("\n");
	Imprimir_Tabla(tabla_hash, tamano_tabla);
	printf("\n");
}
