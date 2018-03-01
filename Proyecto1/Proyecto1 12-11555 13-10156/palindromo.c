# include <stdlib.h>
# include <stdio.h>
# include <string.h>

/*// Funcion que
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
}*/

// Funcion que busca todos los palindromos contenidos en un string
int palindromo(char str[]){
    int l; // Sera la posicion mas izquierda de la palabra
    int h; // Sera la posicion mas a la derecha de la palabra
    int tamano = strlen(str); // Tamaño de la palabra introducida
	int i; // Centro desde se busca identificar si es palindromo
    int len;
    int specialChar;
    int encontrado = 0;
    printf("\nBuscando palindromos en la cadena: %s\n",str);
	printf("  Palindromos encontrados: ");

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
		len = 0; //Longitud del palindromo (+2 cuando es caracter especial)
		l = i-1; // Caracter anterior al centro
		h = i;   // Caracter central
		if(str[i] < 0){//Si el caracter i es especial 
			l--;       //Se asegura no apuntar con l a su primera posicion
		}

		while (l >= 0 && h < tamano){
			if(str[h] > 0){//Si h no es especial
				if (str[l] == str[h]){ //Comparamos igualdad
					len += 2; 
					if (specialChar) printf("%.*s", len, str+l);
					specialChar = 1; //Ajuste al largo
					encontrado = 1;
				}

				else{
					break; //Si no son iguales, continuamos moviendo el centro
				}
				h++;
				if(str[h]<0){
					h++;//Si el nuevo caracter es especial, nos ubicamos en su segunda posicion
				}
				l--;
			}

			else {//Si h es especial
				if (str[l] == str[h]){
					len += 4; //Agregamos doble tamano a la longitud del nuevo string
					if (specialChar) printf("%.*s", len, str+l-1);
					specialChar = 1;//Ajuste al rango
					encontrado = 1;
				}
				else{
					break;
				}
				h++;
				if(str[h]<0){
					h++;//Si el nuevo caracter es especial, nos ubicamos en la segunda posicion
				}
				l--;
				l--;
			}
			printf(". ");
		}
		//Para palindromos impares
		len = 1; // Longitud ya cuenta el centro (no importa que tenga acento)
		l = i-1; // Caracter anterior al centro
		h = i+1; // Caracter siguiente al centro
		if(str[i] < 0){ //Si el centro es especial
			l--;
		}
		if(str[h] < 0){ //Si el siguiente es especial
			h++;
		}

		while (l >= 0 && h < tamano){
			if(str[h] > 0){ //H no especial
				if (str[l] == str[h]){
					len += 2;
					printf("%.*s", len, str+l);
					encontrado = 1;
				}

				else{
					break;
				}
				h++;

				if(str[h]<0){//Si la nueva h es especial
					h++;
				}
				l--;
			}

			else {//h especial
				if (str[l] == str[h]){
					len += 4;
					printf("%.*s", len, str+l-1);
					encontrado = 1;
				}
				else{
					break;
				}

				h++;
				if(str[h]<0){//si la nueva h es especial
					h++;
				}
				l--;
				l--;
			}
			printf(". ");
		}
	}
	if(!encontrado){
		 printf("Ninguno");
	}

	printf("\n");
}

int main(){
	return 0;
}