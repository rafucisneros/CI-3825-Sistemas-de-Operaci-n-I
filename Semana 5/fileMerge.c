# include <stdio.h>
# include <stdlib.h>
# include <string.h>

int main(){
	pid_t procesoNombres, procesoApellidos;
	FILE *archivoNombres, *archivoApellidos, *archivoTemporal, *archivoSalida;
	archivoNombres = fopen("nombres.txt", "r");
	archivoApellidos = fopen("apellidos.txt","r");
	archivoTemporal = fopen("temp.txt","r+");//tmpfile();
	archivoSalida = fopen("salida.txt","w");
	if (archivoNombres==NULL || archivoApellidos==NULL ||
	archivoTemporal==NULL) {
		printf("Hubo un problema cargando los archivos\n");
		exit(-1);
	}

	char linea[100], *nombres, *apellido; // Para leer el archivo y quitar el \n

	procesoNombres = fork();
	if (procesoNombres < 0){
		printf("Error en el Fork.\n");
		exit(-1);		
	}
	
	if (procesoNombres !=0 ){ // Proceso Padre
		printf("Proceso Padre 1\n");
		wait(NULL);
		procesoApellidos = fork();
		if (procesoApellidos != 0){
			wait(NULL);
			printf("Proceso Padre 2\n");
			rewind(archivoTemporal);
			fgets(linea,100, archivoTemporal);
			nombres = strtok(linea," ");
			nombres[strlen(nombres)-1] = '\0';
			apellido = strtok(NULL,"\n");
			fprintf(archivoSalida,"%s %s",nombres, apellido);
		} else if (procesoApellidos == 0){
			printf("Proceso Apellidos\n");
			fgets(linea,100,archivoApellidos);
			fprintf(archivoTemporal, "%s", linea);
			exit(0);
		}
	} else if (procesoNombres == 0){ // Estoy en el hijo
		printf("Proceso Nombres\n");
		fgets(linea,100,archivoNombres);
		nombres = strtok(linea,"\n");	
		fprintf(archivoTemporal, "%s ", nombres);
		exit(0);		
	}
	return 0;
}
