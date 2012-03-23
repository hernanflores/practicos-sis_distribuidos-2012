#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_DIGITOS 99
/*
	FUNCIONES AUXILIARES
*/
void limpiar_cadena(char *cadena)
{
  char *p;
  p = strchr(cadena, '\n');
  if(p){
  	*p = '\0';
  }
}

/*
	Imprime los nodos intermedio (el camino) para llegar al nodo destino
	desde del nodo origen.
*/
void routear(char *src, char *dst){

	int dim = strlen(dst);

	int i = 0;
    while(i < dim){
        if(src[i] != dst[i]){
            if(src[i] == '0'){
            	src[i] = '1';
            }
            else{
            	src[i] = '0';
            }
            
            printf("Nodo intermedio: %s \n", src);
        }
        i++;
    }
}

/*
	Lee los nodos origen y destino desde el teclado.
*/
int main(int argc, char *argv[]) {
	char c_origen[MAX_DIGITOS];
	char c_destino[MAX_DIGITOS];

	printf("Nodo origen: ");
	fgets(c_origen, MAX_DIGITOS, stdin);
	limpiar_cadena(c_origen);

	printf("Nodo destino: ");
	fgets(c_destino, MAX_DIGITOS, stdin);
	limpiar_cadena(c_destino);

	if(strlen(c_destino) != strlen(c_origen)){
		printf("%s\n", "ERROR: Nodos de distinta dimension");
        return (EXIT_FAILURE);
    }

	printf("%s\n", "Routeando...");
	routear(c_origen, c_destino);
	printf("%s\n", "COMPLETADO.");
	return 0;
}