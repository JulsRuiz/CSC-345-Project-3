#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define FRAME_COUNT 128
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define OFFSET_MASK 0xFF
#define PAGE_NUMBER_MASK 0xFF00


typedef struct {
   int pg_number;
   int frame_num;
} TLBEntry;


int main(int argc, char*argv[]){
  if(argc!= 2){ //get txt file
       fprintf(stderr, "Usage: ./main2 addresses.txt\n");
       return 1;
  }
   
  //open the file and backing bin
  FILE *addr_fp = fopen(argv[1], "r");
  FILE *bs_fp = fopen("BACKING_STORE.bin", "rb");
  if(!addr_fp || !bs_fp){
     perror("File error!");
     return 1;
  }


  //create the physical memory and pagetable
  signed char physical_memory[FRAME_COUNT][FRAME_SIZE];
  int pg_table[PAGE_TABLE_SIZE];
  memset(pg_table, -1, sizeof(pg_table));

  //set TLB entry table
  TLBEntry tlb[TLB_SIZE];
  for(int i = 0; i < TLB_SIZE; i++) tlb[i].pg_number = -1; //set all num to -1


  int tlb_ind = 0;
  int pg_to_frame_map[FRAME_COUNT]; //create a page to frame mapping
  memset(pg_to_frame_map, -1, sizeof(pg_to_frame_map)); //sets all the values to -1


  FILE *out1 = fopen("out1.txt", "w");
  FILE *out2 = fopen("out2.txt", "w");
  FILE *out3 = fopen("out3.txt", "w");

  
  int nxt_frame = 0;
  int fifo_ind = 0;
  int pg_faults = 0, tlb_hit_count = 0;
  int total_addrs = 0;
  int log_address;

  //loops through entire address file while getting addresses
  while(fscanf(addr_fp, "%d", &log_address) == 1){
     total_addrs++; //add to total
     int pg_num = (log_address & PAGE_NUMBER_MASK) >> 8; //find its page num
     int offset = log_address & OFFSET_MASK; //calculate the offset
     int frame_num = -1; //set frame for later


     for(int i = 0; i < TLB_SIZE; i++){ //search through TLB for hit
         if (tlb[i].pg_number == pg_num){ // if match, hit
             frame_num = tlb[i].frame_num; 
             tlb_hit_count++;
             break;
         }
     }


     if(frame_num == -1){ //if miss
         frame_num = pg_table[pg_num]; //add number to frame


     if(frame_num == -1){
         pg_faults++; //since miss, there is a page fault
    
     if(nxt_frame < FRAME_COUNT){ //if the number of frames used is less than total number of frames
         frame_num = nxt_frame++; //add to frame number
     }


     else{ //if there is no more frames, evict a number
         int evicted_frame = fifo_ind; //use fifo to evict


     for(int i = 0; i < PAGE_TABLE_SIZE; i++){ //evicts the number in page table
         if(pg_table[i] == evicted_frame){
             pg_table[i] = -1;
             break;
         }
     }


     frame_num = evicted_frame;
     fifo_ind = (fifo_ind + 1) % FRAME_COUNT;
     }


     fseek(bs_fp, pg_num * PAGE_SIZE, SEEK_SET);// find the page in backing bin
     if(fread(physical_memory[frame_num], sizeof(signed char), PAGE_SIZE, bs_fp) != PAGE_SIZE){//then read the page into physical memory
         fprintf(stderr, "Error: Failed to read FROM BACKING_STORE.bin\n");
         exit(1);
     }
     pg_table[pg_num] = frame_num;
     pg_to_frame_map[frame_num] = pg_num;
  }

  //add to TLB
  tlb[tlb_ind].pg_number = pg_num;
  tlb[tlb_ind].frame_num = frame_num;
  tlb_ind = (tlb_ind + 1) % TLB_SIZE;


  }

  //get the address and its value for output
  int physical_address = (frame_num << 8) | offset;
  signed char value = physical_memory[frame_num][offset];


  fprintf(out1, "%d\n", log_address);
  fprintf(out2, "%d\n", physical_address);
  fprintf(out3, "%d\n", value);
  }


  printf("Translated addresses = %d\n", total_addrs);
  printf("Page faults = %d\n", pg_faults);
  printf("Page fault race = %.3f\n", (float)pg_faults / total_addrs);
  printf("TLB hits = %d\n", tlb_hit_count);
  printf("TLB hit rate = %.3f\n", (float)tlb_hit_count / total_addrs);

  
  //close files
  fclose(addr_fp); fclose(bs_fp);
  fclose(out1); fclose(out2); fclose(out3);
  return 0;
}

