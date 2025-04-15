#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_COUNT 256
#define PAGE_TABLE_SIZE 256
#define OFFSET_MASK 0xFF
#define PAGE_NUMBER_MASK 0xFF00


int main(int argc, char *argv[]){
  if(argc != 2){ //get text file
    fprintf(stderr, "Usage: ./main1 addresses.txt\n");
    return 1;
  }



  //open the file and backing bin
  FILE *add_fp = fopen(argv[1], "r");
  FILE *b_fp = fopen("BACKING_STORE.bin", "rb");


  if(!add_fp || !b_fp){
    perror("File error!");
    return 1;
  }

  //create the physical memory and pagetable
  signed char physical_mem[FRAME_COUNT][FRAME_SIZE];
  int page_table[PAGE_TABLE_SIZE];
  memset(page_table, -1, sizeof(page_table));

  //open the output files
  FILE *outone = fopen("out1.txt", "w");
  FILE *outtwo = fopen("out2.txt", "w");
  FILE *outthree = fopen("out3.txt", "w");

  
  int nxt_frame = 0; 
  int pg_faults = 0; 
  int total_add = 0; 
  int log_address; 

  //loops through the entire addresses file while getting the addresses
  while(fscanf(add_fp, "%d", &log_address)==1){ 
     total_add++;//add to total

     
     int page_num = (log_address & PAGE_NUMBER_MASK) >> 8; //find its page num
     int offset = log_address & OFFSET_MASK; //calculate the offset
     int frame_num = page_table[page_num]; // get frame number from page table


     if (frame_num == -1){
         pg_faults++; //add a page fault

         //find the page in backing file
         fseek(b_fp, page_num * PAGE_SIZE, SEEK_SET);
        
         //then read the page into physical memory
         if(fread(physical_mem[nxt_frame], sizeof(signed char), PAGE_SIZE, b_fp) != PAGE_SIZE){
             fprintf(stderr, "Error reading FROM BACKING_STORE.bin\n");
             exit(1);
         }

         //set the next frame
         page_table[page_num] = nxt_frame;
         frame_num = nxt_frame;
         nxt_frame++;
     }

     //get the address and its value for output
     int physical_address = (frame_num << 8) | offset;
     signed char value = physical_mem[frame_num][offset];


     fprintf(outone, "%d\n", log_address);
     fprintf(outtwo, "%d\n", physical_address);
     fprintf(outthree, "%d\n", value);
  }
  printf("Number of translated addresses = %d\n", total_add);
  printf("Page faults = %d\n", pg_faults);
  printf("Page fault rate = %.3f\n", (float)pg_faults / total_add);

  //close files
  fclose(add_fp); fclose(b_fp);
  fclose(outone); fclose(outtwo); fclose(outthree);
  return 0;
}



