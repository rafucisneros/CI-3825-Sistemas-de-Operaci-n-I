# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <getopt.h>
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
void Imprimir_Nodo(Nodo_Hash *nodo){
    printf("\tArchivos de la llave: %s\n", nodo->llave);
    Nodo_Path *paths = nodo->archivos;
    while(paths != NULL){
        printf("\t%s\n",paths->path);
        paths = paths->siguiente;
        printf("\n");
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
void Imprimir_Tabla(Indice *tabla){
    int i;
    for(i = 0; i < tabla->tamano; i++){
        printf("Elementos en el indice: %d. %d llaves.\n", i, tabla->contenido[i]->numero_elementos);
        Imprimir_Lista(tabla->contenido[i]);
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
void insertar_coleccion_paths(Indice* tabla, Nodo_Hash* coleccion){
    int indice = hash(coleccion->llave, tabla->tamano);
    Lista_Llaves *lista_actual = (Lista_Llaves *) tabla->contenido[indice];    
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

Indice* rehash(Indice* tabla_antigua){
    int tamano_viejo = tabla_antigua->tamano;
    int tamano = tamano_viejo * 2 + 1;
    Indice *nueva_tabla = crear_indice(tamano);
    for(int i; i<tamano_viejo; i++){
        Lista_Llaves *lista_actual = tabla_antigua->contenido[i];
        if(!lista_actual->numero_elementos){
            continue;
        } else {
            Nodo_Hash *nodo_a_reinsertar = lista_actual->primer_elemento;
            while(nodo_a_reinsertar != NULL){
                insertar_coleccion_paths(nueva_tabla, nodo_a_reinsertar);
                nodo_a_reinsertar = nodo_a_reinsertar->siguiente;
            }
        }
    }
    free(tabla_antigua);
    return nueva_tabla;
}

// Funcion que inserta un path en todos las llaves con las cuales podria
// encontrarse al hacer una busqueda
Indice* Insertar_Llave_Hash(Indice *tabla, char *path, int inicio){
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
            int indice = hash(substring, tabla->tamano);
            Nodo_Hash *nodo = buscar(tabla, substring);
            if (nodo == NULL){ // Nueva insercion en la tabla
                if (tabla->contenido[indice]->numero_elementos > 2){ // Demasiadas colisiones
                    printf("Necesitamos hacer Rehash\n");
                    tabla = rehash(tabla);
                    tabla = Insertar_Llave_Hash(tabla,path,inicio); // REVISAR LOS ELEMENTOS QUE YA HABIAN SIDO INTRODUCIDOS
                } else {
                    Nodo_Hash *nueva_llave = malloc(sizeof(Nodo_Hash));
                    Nodo_Path *nuevo_path = malloc(sizeof(Nodo_Path));
                    nuevo_path->path = path;
                    nuevo_path->siguiente = NULL;
                    nueva_llave->llave = substring;
                    nueva_llave->archivos = nuevo_path;
                    nueva_llave->siguiente = tabla->contenido[indice]->primer_elemento;
                    tabla->contenido[indice]->primer_elemento = nueva_llave;
                    tabla->contenido[indice]->numero_elementos++;          
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

int main(int argc, char **argv){
    Indice *tabla_hash = crear_indice(10);
    // 16 llaves distintas
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Casa de Papel.pdf",0);                 
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Clase Proba.txt",0);                   
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Ricitos de oro.pdf",0);                    
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Arbol Binario.c",0);                   
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Arbol Ternario.java",0);                   
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Proyecto.c",0);                    
    printf("\n");
    tabla_hash = Insertar_Llave_Hash(tabla_hash, "home/rafael/tarea/Proyecto 2.c",0);                  
    printf("\n");
    Imprimir_Tabla(tabla_hash);
    printf("\n");

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
                    printf("option %s", long_options[option_index].name);
                    if (optarg)
                        printf(" with arg %s", optarg);
                    printf("\n");
                    break;
                
                case 'u':
                    puts ("option -u\n");
                    break;
    
                case 'a':
                    puts ("option -a\n");
                    break;
    
                case 'd':
                    printf ("option -d with value %s\n", optarg);
                    break;
    
                case 'm':
                    printf ("option -m with value %s\n", optarg);
                    break;
    
                case 'i':
                    printf ("option -i with value %s\n", optarg);
                    break;
    
                case '?':
                    break;
    
                default:
                    abort();
            }
        }
}












