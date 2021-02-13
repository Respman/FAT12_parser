#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

struct boot {
	uint8_t jmp_instr[3];
	uint64_t OEM;
	uint16_t amount_in_sector;
	uint8_t amount_in_cluster;
	uint16_t amount_reserv_sectors;
	uint8_t amount_of_FAT_tables;
	uint16_t amount_of_files_in_root;
	uint16_t total_amount_sect_in_partition;
	uint8_t drive_type;
	uint16_t amount_of_sectors_in_FAT;
	uint16_t amount_of_sectors_in_track;
	uint16_t amount_of_headers;
	uint32_t amount_of_hide_sectors;
	uint32_t total_amount_of_sectors_in_partition;
	uint8_t int13h;
	uint8_t reserved;
	uint8_t ex_boot;
	uint32_t lod_disk;
	uint8_t text[11];
	uint8_t file_sys_type[8];
	uint8_t nop[472];
}__attribute__((packed));

struct catalog_element {
	uint8_t short_name[11];
	uint8_t attribute;
	uint8_t reserved;
	uint8_t creation_time_milli;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t last_appeal;
	uint16_t older_word_of_first_cluster_number;
	uint16_t last_instruction_time;
	uint16_t last_instruction_date;
	uint16_t younger_word_of_first_cluster_number;
	uint32_t size_of_file;
}__attribute__((packed));

struct long_name {
	uint8_t number;
	uint8_t first_name_part[10];
	uint8_t attribute;
	uint8_t flags;
	uint8_t CRC8;
	uint8_t second_name_part[12];
	uint16_t first_cluster_number;
	uint8_t third_name_part[4];

}__attribute__((packed));


int main (int argc, char** argv){
	int fd;
	struct stat st;
	char *ptr;
	struct boot *boot;
	struct catalog_element *catalog_element;
	struct long_name *long_name;
	int i, j;
	long int root;
	char* FAT;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &st);
	ptr = (char*)mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, fd, 0);
	boot = (struct boot*)(ptr);
	if (argc == 2){
		root = boot->amount_in_sector*boot->amount_of_sectors_in_FAT*boot->amount_of_FAT_tables;
		FAT = (ptr + 512);
		for (i = 0; i < boot->amount_of_files_in_root; i++){
			catalog_element = (struct catalog_element*)(ptr + 512 + root + i*32);
			if (catalog_element->attribute != 0){
				if (catalog_element->attribute == 0x0f){
					long_name = (struct long_name*)(catalog_element);
					printf("-------------\n");
					printf("long name: ");
					for (j = 0; j < 10; j++)
						if ((long_name->first_name_part[j] >=20)&&(long_name->first_name_part[j] <=127)) 
							printf("%c", long_name->first_name_part[j]);
					for (j = 0; j < 12; j++)
						if ((long_name->second_name_part[j] >=20)&&(long_name->second_name_part[j] <=127)) 
							printf("%c", long_name->second_name_part[j]);

					for (j = 0; j < 4; j++)
					if ((long_name->third_name_part[j] >=20)&&(long_name->third_name_part[j] <=127)) 
						printf("%c", long_name->third_name_part[j]);
					printf("\n");

				}
				else if (catalog_element->attribute == 0x10){
					printf("catalog: ");
					for (j = 0; j < 8; j++)
						if (catalog_element->short_name[j] != ' ') printf("%c", catalog_element->short_name[j]);
					printf(".");
					for (j = 8; j < 11; j++)
						if (catalog_element->short_name[j] != ' ') printf("%c", catalog_element->short_name[j]);
					printf("\n");
				}
				else{
					printf("short name: ");
					for (j = 0; j < 8; j++)
						if (catalog_element->short_name[j] != ' ') printf("%c", catalog_element->short_name[j]);
					printf(".");
					for (j = 8; j < 11; j++)
						if (catalog_element->short_name[j] != ' ') printf("%c", catalog_element->short_name[j]);
					printf("\nsize: %d bytes\n", catalog_element->size_of_file);
				}
			}
		}
	}
	else if (argc == 3){
		
	}
	else printf("wrong amount of parametrs\n");

	munmap(ptr, st.st_size);
	close(fd);

	return(0);
}
