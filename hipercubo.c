#include <stdio.h>
#include <stdlib.h>

#define MAX_DIGITOS 9
/*
	FUNCIONES AUXILIARES
*/
void limpiar_cadena(char *cadena)
{
  char *p;
  p = strchr (cadena, '\n');
  if(p){
  	*p = '\0';
  }
}

/*
	Imprime los nodos intermedio (el camino) para llegar al nodo destino
	desde del nodo origen.
*/
void routear(char *src, char *dst, char *dim){

	//dependiendo de la dimension, tengo que paddear el string
	printf("|%10s-|", "Hello");

	//llegue?
	while(src != dst){
		//xoreo
		//cambio bit menos significativo
		//imprimo
		printf "Nodo: "%s"n", nodo_actual);	
	}
	
}

/*
	Lee el nro de dimensiones y los nodos origen y destino desde el teclado.
*/
int main(int argc, char *argv[]) {
	char c_dimension[MAX_DIGITOS];
	char c_origen[MAX_DIGITOS];
	char c_destino[MAX_DIGITOS];

	printf("Dimensiones del hipercubo: ");
	fgets(c_dimension, MAX_DIGITOS, stdin);
	limpiar_cadena(c_dimension);

	printf("Nodo origen: ");
	fgets(c_origen, MAX_DIGITOS, stdin);
	limpiar_cadena(c_origen);

	printf("Nodo destino: ");
	fgets(c_destino, MAX_DIGITOS, stdin);
	limpiar_cadena(c_destino);

	routear(c_origen, c_destino, c_dimension);

	return 0;
}