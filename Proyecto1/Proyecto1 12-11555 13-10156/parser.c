# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <ctype.h>


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
	char parseado[strlen(str)];	// 
	int i=0; // Contador para el string que estamos parseando
	int k = 0; // Contador para el string que estamos creando
	while(str[i] != '\0'){ 	// Leemos hasta el final del string	
		if (str[i]>0 && str[i]!='.' && str[i]!='/'){ // Un caracter normal
			parseado[k] = tolower(str[i]); // Lo ponemos en minuscula
			k++;	// Proximo caracter
		}
		else if (str[i] == -61){ // Un caracter especial, 2 espacios ocupados
			parseado[k++] = str[i++]; // El	primer numero siempre es el mismo
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
					parseado[k] = str[i]; // Cualquier otro string
					k++;
					break;
			}
		}
		i++;
	}
	char *final = calloc(k+1,sizeof(char)); // Creamos una nueva memoria
	int y = 0;
	while(y < k){
		final[y]=parseado[y];
		y++;
	}
	final[k] = '\0';
	return final;
}

int main(){
	return 0;
}