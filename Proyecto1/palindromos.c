// Sistemas de Operacion I
// Proyecto 1
// Rafael Cisneros 13-11156
// Erick Flejan 12-11555
# include <ftw.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "arbol.h"
# include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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


int main(){
	char *prueba = "anitalavalatinaabba";
	palindromo(prueba);
}
