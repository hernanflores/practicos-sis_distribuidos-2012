#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
/*  libs  */
#include "mpi.h"

#define BYTE  unsigned char   // 8-bits
#define WORD  unsigned short  // 16-bits
#define DWORD unsigned int    // 32-bits

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


BYTE *loadImg(const char *name, BMPHeader *header){
  if(!name) return;

  BYTE *pImageData = NULL;

  // Apertura del archivo
  FILE *pFile = fopen(name, "rb");  

  // Si se pudo cargar el archivo
  if(pFile){
    // Cargo la cabecera
    fread(header->identifier,       1, sizeof(WORD),  pFile);   
    fread(header->size,             1, sizeof(DWORD), pFile);   
    fread(header->reserved,         1, sizeof(DWORD), pFile);
    fread(header->bitoffset,        1, sizeof(DWORD), pFile);   
    fread(header->headerSize,       1, sizeof(DWORD), pFile);   
    fread(header->width,            1, sizeof(DWORD), pFile);   
    fread(header->height,           1, sizeof(DWORD), pFile);   
    fread(header->planes,           1, sizeof(WORD),  pFile);   
    fread(header->bitsPerPixel,     1, sizeof(WORD),  pFile);   
    fread(header->compression,      1, sizeof(DWORD), pFile);   
    fread(header->imageSize,        1, sizeof(DWORD), pFile);   
    fread(header->hresolution,      1, sizeof(DWORD), pFile);   
    fread(header->vresolution,      1, sizeof(DWORD), pFile);   
    fread(header->numberOfColours,  1, sizeof(DWORD), pFile);   
    fread(header->importantColours, 1, sizeof(DWORD), pFile); 

    // Si el archivo no es de 24bits, termino la ejecución
    if(header.bitsPerPixel != 24){
      fclose(pFile);
      printf("archivo no es de 24bits\n");
      return;
    }

    // Cargo los datos de la imagen
    pImageData = new BYTE[header.width * header.height * 3];
    fseek(pFile, header.bitoffset, SEEK_SET);
    fread(pImageData, 1, header.width * header.height * 3, pFile);

    isImageLoaded = true;   
    
    // Cierro el archivo
    fclose(pFile);
  }

  return pImageData;
}

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

void saveImg(const char *name, BMPHeader header, BYTE *pImageData){
  if(!name) return;

  FILE *pFile = fopen(name, "wb");

  fwrite(&header.identifier,       sizeof(header.identifier), 1, pFile);
  fwrite(&header.size,             sizeof(header.size), 1, pFile);
  fwrite(&header.reserved,         sizeof(header.reserved), 1, pFile);
  fwrite(&header.bitoffset,        sizeof(header.bitoffset), 1, pFile);
  fwrite(&header.headerSize,       sizeof(header.headerSize), 1, pFile);
  fwrite(&header.width,            sizeof(header.width), 1, pFile);
  fwrite(&header.height,           sizeof(header.height), 1, pFile);
  fwrite(&header.planes,           sizeof(header.planes), 1, pFile);
  fwrite(&header.bitsPerPixel,     sizeof(header.bitsPerPixel), 1, pFile);
  fwrite(&header.compression,      sizeof(header.compression), 1, pFile);
  fwrite(&header.imageSize,        sizeof(header.imageSize), 1, pFile);
  fwrite(&header.hresolution,      sizeof(header.hresolution), 1, pFile);
  fwrite(&header.vresolution,      sizeof(header.vresolution), 1, pFile);
  fwrite(&header.numberOfColours,  sizeof(header.numberOfColours), 1, pFile);
  fwrite(&header.importantColours, sizeof(header.importantColours), 1, pFile);
  
  fseek(pFile, header.bitoffset, SEEK_SET);
  fwrite(pImageData, 1, header.width * header.height * 3, pFile);

  fclose(pFile);    
}

int main(int argc, char const *argv[]){
  BYTE imgA;
  BMPHeader headerA;

  imgA = loadImg("A.bmp", &header);
  saveImg("postA.bmp", header,imgA);
  return 0;
}