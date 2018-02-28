// Sistemas de Operacion I
// Proyecto 1
// Rafael Cisneros 13-11156
// Erick Flejan 12-11555
# include <ftw.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
# include<ctype.h>

int comparePal(char str[], int l, int h){
	if (str[l] == str[h]) 
		return 1;
    else 
    	{if (str[l] != str[h] && ((str[l] == str[h+1] && str[l-1] == str[h]) || 
    		                      (str[l] == str[h-1] && str[l+1] == str[h])))
            return 2;}
    return 0;
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


int main(){
	char *prueba = "baabaá";
	palindromo(prueba);
	char *prueba2 = "ábáábaá";
	palindromo(prueba2);
	char *prueba3 = "anitalavalatinaá";
	palindromo(prueba3);
	char *prueba4 = "áánítáláválátínáá";
	palindromo(prueba4);

}

// a'ba'a'ba'
// 0123456789
//   i
//  lh
// len = 0

// len = 0;
// 		l = i-1;
// 		h = i;
// 		if(str[i] < 0){
// 			h++;
// 			if(i != 1){
// 				l++;
// 			}
			
// 			if(str[h] < 0){
// 				h++;
// 			}
// 		}