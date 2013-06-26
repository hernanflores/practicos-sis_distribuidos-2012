#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//libs
#include "mpi.h"
#include <omp.h>
#include <time.h>

typedef struct bmp_file_header
{
  /* La firma (2 bytes), que indica que se trata de un archivo BMP con dos 
caracteres  */
  uint32_t size;        /* El tamaño total del archivo en bytes (codificado en 4 bytes)  */
  uint16_t resv1;       /* Reservado */
  uint16_t resv2;       /* Reservado */
  uint32_t offset;      /* El desajuste de la imagen (en 4 bytes) hasta los datos de imagen */
} bmp_file_header;

typedef struct bmp_info_header
{
  uint32_t headersize;      /* Tamaño de la cabecera */
  uint32_t width;       /* Ancho */
  uint32_t height;      /* Alto */
  uint16_t planes;          /* Planos de color (Siempre 1) */
  uint16_t bpp;             /* bits por pixel */
  uint32_t compress;      
  uint32_t imgsize;     /* tamaño de los datos de imagen */
  uint32_t bpmx;        /* Resolución X en bits por metro */
  uint32_t bpmy;        /* Resolución Y en bits por metro */
  uint32_t colors;     
  uint32_t imxtcolors; 
} bmp_info_header;

unsigned char *load_image(char *filename, bmp_info_header *info_header);
unsigned char * mix(int chunksize, int height, unsigned char *img, unsigned char *img2);
void save_bmp(char *file_name, bmp_info_header *info, unsigned char *imgdata);

bmp_file_header g_header;
int main(int argc, char *argv[] ){
  clock_t t_ini, t_fin;
  double secs;

  t_ini = clock();

  //Inicializacion MPI
  int nproc; /* Numero de procesos */
  int pid, /* proc actual */
  chunksize, offset,dest, tag1, tag2, width;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);
  MPI_Comm_rank(MPI_COMM_WORLD, &pid);
  //fin inicializacion MPI
  printf ("task %d comenzo...\n", pid);

  bmp_info_header info;
  bmp_info_header info2;
  unsigned char *img;
  unsigned char *img2;
  unsigned char *localimg;
  unsigned char *localimg2;
  tag2 = 1;
  tag1 = 2;

  //Imagenes del mismo tamaño
  if (pid == 0) {
    img   = load_image("A.bmp", &info);
    img2  = load_image("B.bmp", &info2);

    chunksize = (info.imgsize / nproc);

    localimg =(unsigned char*)malloc(chunksize);
    localimg2 =(unsigned char*)malloc(chunksize);

    printf("chunksize: %d\n", chunksize);
  }
  width = info.width;
   MPI_Barrier(MPI_COMM_WORLD);

  //scatter entre los procs.
  MPI_Scatter(
    img, chunksize, MPI_UNSIGNED_CHAR,      /* envio chunk de imagen */
    localimg,  chunksize, MPI_UNSIGNED_CHAR,      /* cada proc recibe un chunk de imagen */
    0, MPI_COMM_WORLD);   /* envia root, a los procs de MPI_COMM_WORLD */

  MPI_Scatter(
      img2, chunksize, MPI_UNSIGNED_CHAR,
      localimg2,  chunksize, MPI_UNSIGNED_CHAR,
      0, MPI_COMM_WORLD);
  
  MPI_Bcast(chunksize, 1,MPI_INT,0, MPI_COMM_WORLD);
  MPI_Bcast(width, 1,MPI_INT,0, MPI_COMM_WORLD);
  
  unsigned char * retimg=(unsigned char*)malloc(chunksize);
  // if(pid > 0){
    printf("Mezclando...\n");
    retimg = mix(chunksize, width, localimg, localimg2);
    /* Esperamos que todos terminen */  
  // }

  unsigned char * nuimg=(unsigned char*)malloc(info.imgsize);
  MPI_Gather(retimg, chunksize, MPI_UNSIGNED_CHAR, nuimg, chunksize, MPI_UNSIGNED_CHAR,  0, MPI_COMM_WORLD);
  
  MPI_Barrier(MPI_COMM_WORLD);

  if (pid == 0) {
    printf("Salvando...\n");
    save_bmp("test-mix.bmp", &info, nuimg);

    free(img);
    free(img2);
    free(nuimg);
  }

  MPI_Finalize();
  t_fin = clock();

  secs = (double)(t_fin - t_ini) / CLOCKS_PER_SEC;
  printf("%.16g milisegundos\n", secs * 1000.0);
  return 0;
}

//void mix(bmp_info_header *info, bmp_info_header *info2, unsigned char *img, unsigned char *img2)
unsigned char * mix(int chunksize, int width, unsigned char *img, unsigned char *img2){
  unsigned int r,g,b;
  unsigned int r2,g2,b2;
  uint32_t j,i;
  printf("chunksize %d\n", chunksize);
  printf("width %d\n", width);
  /* recorremos la imagen */
  for (i=0; i<chunksize; i++){
  //  #pragma omp parallel shared(width,img,img2) private(r,g,b,r2,g2,b2)
    //{
      //#pragma omp parallel for
      for (j=0; j<width; j++){
        b = img[3*(j+i*width)];
        b2 = img2[3*(j+i*width)];

        g=img[3*(j+i*width)+1];
        g2=img2[3*(j+i*width)+1];

        r = img[3*(j+i*width)+2];
        r2= img2[3*(j+i*width)+2];
        
        b = (b/2+b2/2);
        g = (g/2+g2/2);
        r = (r/2+r2/2);
        //printf("B:%u,G:%u,R:%u\n", b,g,r);
        unsigned int average = (b+g+r)/3;

        img[3*(j+i*width)] = average;
        img[3*(j+i*width)+1] = average;
        img[3*(j+i*width)+2] = average;
      }
    //}
  }
  return img;
}

unsigned char *load_image(char *filename, bmp_info_header *info_header){

  FILE *f;
  bmp_file_header header;     /* cabecera */
  unsigned char *imgdata;   /* datos de imagen */
  uint16_t type;        /* 2 bytes identificativos */

  f=fopen (filename, "r");
  if (!f)
    return NULL;        /* Si no podemos leer, no hay imagen*/

  /* Leemos los dos primeros bytes */
  fread(&type, sizeof(uint16_t), 1, f);
  if (type !=0x4D42)        /* Comprobamos el formato */
    {
      fclose(f);
      printf("No era BMP!?");
      return NULL;
    }

  /* Leemos la cabecera de fichero completa */
  fread(&header, sizeof(bmp_file_header), 1, f);
  g_header = header;

  /* Leemos la cabecera de información completa */
  fread(info_header, sizeof(bmp_info_header), 1, f);

  /* Reservamos memoria para la imagen, ¿cuánta? 
     Tanto como indique imgsize */
  imgdata=(unsigned char*)malloc(info_header->imgsize);

  /* Nos situamos en el sitio donde empiezan los datos de imagen,
   nos lo indica el offset de la cabecera de fichero*/
  fseek(f, header.offset, SEEK_SET);

  /* Leemos los datos de imagen, tantos bytes como imgsize */
  fread(imgdata, info_header->imgsize,1, f);

  /* Cerramos */
  fclose(f);
  /* Devolvemos la imagen */
  return imgdata;
}

void save_bmp(char *file_name, bmp_info_header *info, unsigned char *imgdata){
  bmp_file_header header;
  FILE *f;
  uint16_t type;
  
  printf("Comenzamos a escribir en file %s\n", file_name);
  f=fopen(file_name, "w+");

  header.size= info->imgsize + sizeof(bmp_file_header) + sizeof(bmp_info_header);
  header.resv1=0;
  header.resv2=1;
  /* El offset será el tamaño de las dos cabeceras + 2 (información de fichero)*/
  header.offset=sizeof(bmp_file_header)+sizeof(bmp_info_header)+2;
  printf("Tamaño offset nuevo file: %u\n", header.offset);

  printf("Escribimos la identificación del archivo\n");
  type=0x4D42;
  fwrite(&type, sizeof(uint16_t),1,f);

  printf("Escribimos la cabecera de fichero\n");
  fwrite(&g_header, sizeof(bmp_file_header),1,f);

  printf("Escribimos la información básica de la imagen\n");
  fwrite(info, sizeof(bmp_info_header),1,f);

  printf("Escribimos la imagen\n");
  fwrite(imgdata, info->imgsize, 1, f);
  
  printf("Cerramos file\n");
  fclose(f);
}
