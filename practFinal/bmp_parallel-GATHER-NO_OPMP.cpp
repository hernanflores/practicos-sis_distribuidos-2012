#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*  libs  */
#include "mpi.h"
#include <omp.h>

#define BYTE  unsigned char   // 8-bits
#define WORD  unsigned short  // 16-bits
#define DWORD unsigned int    // 32-bits

static const int SEND_IMGA = 10;
static const int SEND_IMGB = 30;
static const int REC_IMGA = 50;
static const int REC_IMGB = 70;
static const int MASTER = 0;

typedef struct BMPHeader{
  WORD   identifier;             // Magic number: BM
  DWORD  size;                   // tamaño del archivo
  DWORD  reserved;               //
  DWORD  bitoffset;              // offset para comenzar a leer la data 
  DWORD  headerSize;             // tamaño de la cabecera
  DWORD  width;                  // ancho
  DWORD  height;                 // alto
  WORD   planes;                 // numero de planos del bitmap
  WORD   bitsPerPixel;           // bit por pixel
  DWORD  compression;            // especificacion de la compresion
  DWORD  imageSize;              // tamaño de la seccion de datos
  DWORD  hresolution;            // resolucion horizontal medida en pixeles por metro
  DWORD  vresolution;            // resolucion vertical medida en pixeles por metro
  DWORD  numberOfColours;        // numero de colores ( 2^bitsPerPixel )
  DWORD  importantColours;       // numero de colores importantes( si = 0: todos los colores son importantes )

}BMPHeader;

void printHeader(BMPHeader header){
  printf("  ===========================================================\n");
  printf("    Image information:\n");
  printf("  ===========================================================\n");

  printf("    + Identifier: \t\t| Ox%X\n", header.identifier);
  printf("    + File size: \t\t| %d bytes\n", header.size);
  printf("    + Data offset: \t\t| %d bytes\n", header.bitoffset);
  printf("    + Header size: \t\t| %d bytes\n", header.headerSize);
  printf("    + Width: \t\t\t| %dpx\n", header.width);
  printf("    + Height: \t\t\t| %dpx\n", header.height);
  printf("    + Planes: \t\t\t| %d\n", header.planes);
  printf("    + Bits per pixel: \t\t| %d-bits\n", header.bitsPerPixel);
  printf("    + Compression: \t\t| %d\n", header.compression);
  printf("    + Image size: \t\t| %d bytes\n", header.imageSize);
  printf("    + Horizontal resolution: \t| %d\n", header.hresolution);
  printf("    + Vertical resolution: \t| %d\n", header.vresolution);
  printf("    + Number of colours: \t| %d\n", header.numberOfColours);
  printf("    + Important colours: \t| %d\n", header.importantColours);
  printf("  -----------------------------------------------------------\n");
}


BYTE *loadImg(const char *name, BMPHeader *headerA){

  BYTE *pImageData = NULL;
  BMPHeader header;
  // Apertura del archivo
  FILE *pFile = fopen(name, "rb");  

  // Si se pudo cargar el archivo
  if(pFile){
    // Cargo la cabecera
    fread(&header.identifier, sizeof(WORD),  1, pFile);
    fread(&header.size, sizeof(DWORD), 1, pFile);
    fread(&header.reserved, sizeof(DWORD), 1, pFile);
    fread(&header.bitoffset, sizeof(DWORD), 1, pFile);   
    fread(&header.headerSize, sizeof(DWORD), 1, pFile);   
    fread(&header.width, sizeof(DWORD), 1, pFile);   
    fread(&header.height, sizeof(DWORD), 1, pFile);   
    fread(&header.planes, sizeof(WORD),  1, pFile);   
    fread(&header.bitsPerPixel, sizeof(WORD), 1, pFile);
    fread(&header.compression, sizeof(DWORD), 1, pFile);   
    fread(&header.imageSize, sizeof(DWORD), 1, pFile);   
    fread(&header.hresolution, sizeof(DWORD), 1, pFile);   
    fread(&header.vresolution, sizeof(DWORD), 1, pFile);   
    fread(&header.numberOfColours, sizeof(DWORD), 1, pFile);   
    fread(&header.importantColours, sizeof(DWORD), 1, pFile);

    // Si el archivo no es de 24bits, termino la ejecución
    if(header.bitsPerPixel != 24){
      printf("FILE: %s bpp: %u\n", name, header.bitsPerPixel);
      fclose(pFile);
      printf("FILE: %s archivo no es de 24bits\n", name);
      return NULL;
    }

    // Cargo los datos de la imagen
    pImageData = new BYTE[header.width * header.height * 3];
    fseek(pFile, header.bitoffset, SEEK_SET);
    fread(pImageData, 1, header.width * header.height * 3, pFile);

    *headerA = header;
    // Cierro el archivo
    fclose(pFile);

    printf("FILE: %s cargada\n", name);
    printHeader(header);
  }
  else{
    printf("FILE: %s no se pudo cargar\n", name);
  }

  return pImageData;
}

void saveImg(const char *name, BMPHeader header, BYTE *pImageData){
  FILE *pFile = fopen(name, "wb");

  fwrite(&header.identifier, sizeof(WORD),  1, pFile);
  fwrite(&header.size, sizeof(DWORD), 1, pFile);
  fwrite(&header.reserved, sizeof(DWORD), 1, pFile);
  fwrite(&header.bitoffset, sizeof(DWORD), 1, pFile);   
  fwrite(&header.headerSize, sizeof(DWORD), 1, pFile);   
  fwrite(&header.width, sizeof(DWORD), 1, pFile);   
  fwrite(&header.height, sizeof(DWORD), 1, pFile);   
  fwrite(&header.planes, sizeof(WORD),  1, pFile);   
  fwrite(&header.bitsPerPixel, sizeof(WORD), 1, pFile);
  fwrite(&header.compression, sizeof(DWORD), 1, pFile);   
  fwrite(&header.imageSize, sizeof(DWORD), 1, pFile);   
  fwrite(&header.hresolution, sizeof(DWORD), 1, pFile);   
  fwrite(&header.vresolution, sizeof(DWORD), 1, pFile);   
  fwrite(&header.numberOfColours, sizeof(DWORD), 1, pFile);   
  fwrite(&header.importantColours, sizeof(DWORD), 1, pFile);
  
 // fseek(pFile, header.bitoffset, SEEK_SET);
  //fwrite(pImageData, header.width * header.height * 3, 1, pFile);
  fwrite(pImageData, header.width * header.height * 3, 1, pFile);

  fclose(pFile);    
  printf("FILE: %s guardada\n", name);
  printHeader(header);

}

BYTE *process(BYTE *img1, BYTE *img2, int width, int chunksize){
  //tamaño de imagen = width * chunksize
  int i,j;
  //almacenan valores de pixel
  BYTE  r1,g1,b1,  // img1
        r2,g2,b2, //  img2
        rf,gf,bf; //  resultado
  //guarda el promedio para el grisado
  BYTE average;

  int chunkbytes = chunksize * width * 3;
   //i itera por y, j itera por x
    for(i = 0; i<chunksize; i++){
      for(j=0; j<width; j++){
        //obtengo valores de color de
        //pixel de 1
        b1 = img1[(i * width * 3) + (j * 3)    ];
        g1 = img1[(i * width * 3) + (j * 3) + 1];
        r1 = img1[(i * width * 3) + (j * 3) + 2];
        //pixel de 2
        b2 = img2[(i * width * 3) + (j * 3)    ];
        g2 = img2[(i * width * 3) + (j * 3) + 1];
        r2 = img2[(i * width * 3) + (j * 3) + 2];
        //mezclo
        bf = (b1/2+b2/2);
        gf = (g1/2+g2/2);
        rf = (r1/2+r2/2);
        //griso
        average = (bf+gf+rf)/3;
        //resultado
        img1[(i * width * 3) + (j * 3)    ] = average;
        img1[(i * width * 3) + (j * 3) + 1] = average;
        img1[(i * width * 3) + (j * 3) + 2] = average;
      }
    }
  //return result;
  return img1;
}

int main(int argc, char *argv[]){
  BYTE *imgA = NULL, *imgB = NULL, *localimgA = NULL, *localimgB = NULL, *resImg = NULL;
  BMPHeader headerA, headerB;
  int procs, rank, chunksize, width, height, offset,chunkbytes;

  /*  init mpi      */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);  //cuantos procs?
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);   //cual es mi rango?
  /*  fin init mpi  */

  if(rank == MASTER){
    imgA = loadImg("A.bmp", &headerA);
    imgB = loadImg("B.bmp", &headerB);

    width = headerA.width; //ambas imagenes deben tener mismo tamaño
    height = headerA.height;
    chunksize = headerA.height / procs;
  }

  printf("COMM: broadcast de vars compartidas\n");
  /*  envio tamaños a todos los procs */
  MPI_Bcast(&chunksize, //dir inicial de buffer a enviar
      1,  //elementos en buffer
      MPI_INT,  //tipo de datos buffer
      MASTER, //root
      MPI_COMM_WORLD  //communicator
  );
  MPI_Bcast(&width, //dir inicial de buffer a enviar
      1,  //elementos en buffer
      MPI_INT,  //tipo de datos buffer
      MASTER, //root
      MPI_COMM_WORLD  //communicator
  );
  MPI_Bcast(&height, //dir inicial de buffer a enviar
      1,  //elementos en buffer
      MPI_INT,  //tipo de datos buffer
      MASTER, //root
      MPI_COMM_WORLD  //communicator
  );

  // scatter de trozos de imagenes a procesar
  chunkbytes = chunksize * width * 3;
  printf("DEBUG: chunksize – %d\n", chunksize );
  printf("DEBUG: chunkbytes – %d\n", chunkbytes );

  localimgA = new BYTE[chunkbytes];
  MPI_Scatter(imgA,
    chunkbytes,
    MPI_UNSIGNED_CHAR,
    localimgA,
    chunkbytes,
    MPI_UNSIGNED_CHAR,
    MASTER,
    MPI_COMM_WORLD);
  printf("[PROC – %d]COMM: recibido chunk de A\n", rank);


  localimgB = new BYTE[chunkbytes];
  MPI_Scatter(imgB,
    chunkbytes,
    MPI_UNSIGNED_CHAR,
    localimgB,
    chunkbytes,
    MPI_UNSIGNED_CHAR,
    MASTER,
    MPI_COMM_WORLD);
  printf("[PROC – %d]COMM: recibido chunk de B\n", rank);

  localimgA = process(localimgA, localimgB, width, chunksize);

  resImg = new BYTE[headerA.height * headerA.width * 3];

  MPI_Gather(localimgA,
    chunkbytes, 
    MPI_UNSIGNED_CHAR, 
    resImg, 
    chunkbytes, 
    MPI_UNSIGNED_CHAR, 
    MASTER, 
    MPI_COMM_WORLD);
  printf("COMM: gather de imagenes\n");

  if(rank == MASTER){
    saveImg("postA.bmp", headerB, resImg);
  }

  if(imgA != NULL){
    delete[] imgA;
  }
  if(imgB != NULL){
    delete[] imgB;
  }
  if(localimgA != NULL){
    delete[] localimgA;
  }
  if(localimgB != NULL){
    delete[] localimgB;
  }
  if(resImg != NULL){
    delete[] resImg;
  }

  MPI_Finalize();

  return 0;
}