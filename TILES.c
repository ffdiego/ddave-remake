// extract the VGA tileset from the binary
// each tile will be output as a bitmap in the format tilexxx.bmp

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <SDL.h>

int main(int argc, char* argv[]){
  const uint32_t vga_data_addr = 0x120f0;
  const uint32_t vga_pal_addr = 0x26b0a;

  //open EXE file and go to VGA pixel data
  FILE *fin;
  fin = fopen("DAVE.EXE", "rb");
  fseek(fin, vga_data_addr, SEEK_SET);

  //undo RLE and read all pixel data
  //read file length - first 4 bytes LE
  uint32_t final_length;
  final_length = 0;

  final_length |= fgetc(fin);
  final_length |= fgetc(fin) << 8;
  final_length |= fgetc(fin) << 16;
  final_length |= fgetc(fin) << 24;

  // Read each byte and un-encode
  uint32_t current_length;
  unsigned char out_data[150000];
  uint8_t byte_buffer;

  current_length = 0;
  memset(&out_data, 0, sizeof(out_data));

  while(current_length < final_length){
    byte_buffer = fgetc(fin);
    if (byte_buffer & 0x80){
      byte_buffer &= 0x7F;
      byte_buffer++;
      while(byte_buffer) {
        out_data[current_length++] = fgetc(fin);
        byte_buffer--;
      }
    }
    else {
      byte_buffer +=3;
      char next = fgetc(fin);
      while(byte_buffer){
        out_data[current_length++] = next;
        byte_buffer--;
      }
    }
  }

  // read in vga palette, 256 colors of 3 bytes (RGB)
  fseek(fin,vga_pal_addr, SEEK_SET);  
  uint8_t palette[768];
  uint32_t i,j;
  for (i=0; i<256; i++) {
    for(j=0;j<3;j++){
      palette[i*3+j] = fgetc(fin);
      palette[i*3+j] <<= 2;
    }
  }

  fclose(fin);

  // get the total tile count from the first byte
  uint32_t tile_count;
  tile_count = 0;
  tile_count |= out_data[3] << 24;
  tile_count |= out_data[2] << 16;
  tile_count |= out_data[1] << 8;
  tile_count |= out_data[0];

  // each in offset index for each tile 
  uint32_t tile_index[500];
  for(i=0;i<tile_count; i++){
    tile_index[i] |= out_data[i*4+4];
    tile_index[i] |= out_data[i*4+5] << 8;
    tile_index[i] |= out_data[i*4+6] << 16;
    tile_index[i] |= out_data[i*4+7] << 24;
  }
  // the last tile ends at EOF
  tile_index[i] = final_length;

  uint16_t tile_width;
  uint16_t tile_heigth;
  uint32_t current_byte;
  uint32_t current_tile_byte;
  uint8_t current_tile;

  for (current_tile = 0; current_tile<tile_count;current_tile++) {
    current_tile_byte = 0;
    current_byte = tile_index[current_tile];

    // assume 16x16
    tile_width = 16;
    tile_heigth = 16;

    // skip unusual byte
    if(current_byte > 65280) current_byte++;

    // read first 4 bytes for possible custom dimensions
    if(out_data[current_byte+1] == 0 && out_data[current_byte+3] == 0){
      if(out_data[current_byte] > 0 && out_data[current_byte] < 0xBF && out_data[current_byte+2] >0 && out_data[current_byte+2] < 0x64){
        tile_width = out_data[current_byte];
        tile_heigth = out_data[current_byte+2];
        current_byte+=4;
      }
    }

    // create a SDL surface 
    SDL_Surface *surface;
    uint8_t *dst_byte;
    surface = SDL_CreateRGBSurface(0, tile_width,tile_heigth, 32, 0,0,0,0);
    dst_byte = (uint8_t*) surface->pixels;

    uint8_t src_byte;
    uint8_t red_p, green_p, blue_p;
    for (;current_byte<tile_index[current_tile+1];current_byte++){
      src_byte = out_data[current_byte];
      red_p = palette[src_byte*3];
      green_p = palette[src_byte*3+1];
      blue_p = palette[src_byte*3+2];
      
      dst_byte[current_tile_byte*4] = blue_p;
      dst_byte[current_tile_byte*4+1] = green_p;
      dst_byte[current_tile_byte*4+2] = red_p;
      dst_byte[current_tile_byte*4+3] = 0xff;

      current_tile_byte++;
    }

    // create output filename
    char file_num[4];
    char fout[12];
    fout[0]='\0';
    strcat(fout, "tile");
    sprintf(&file_num[0], "%u", current_tile);
    strcat(fout, file_num);
    strcat(fout, ".bmp");

    printf("Saving %s as bitmap (%d x %d) \n", fout, tile_width,tile_heigth);

    // save and free 
    SDL_SaveBMP(surface, fout);
    SDL_FreeSurface(surface);
  }
  
  return 0;
}