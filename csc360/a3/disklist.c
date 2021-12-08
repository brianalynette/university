#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "a3functions.h"


#define SECTOR_SIZE 512


/* outputs file name with date/time and file size
 * lines 20-31 were provided through sample code */
void print_file(char *file_name,char* p_map,char type_of_file) {
			
	int date;
	int hours, minutes, day, month, year;
	
	date = *(unsigned short *)(p_map + 16);
	
	year = ((date & 0xfe00) >> 9) + 1980;
	month = (date & 0x1e0) >> 5;
	day = (date & 0x1f);
	
	hours = (p_map[15] & 0xf8) >> 3;
	minutes = ((p_map[14] & 0xe0) >> 5) + ((p_map[15] & 0x7) <<3);

	int file_size = (p_map[28] & 0xFF) + ((p_map[29] & 0xFF) << 8) + ((p_map[30] & 0xFF) << 16) + ((p_map[31] & 0xFF) << 24);

	printf("%c %10d %-20s %04d-%02d-%02d %02d:%02d\n",type_of_file,file_size,file_name,year,month,day,hours,minutes);
}


/* prints a list of all directories in the disk image */
void *print_dir_list(char *p_map) {
	int first_logical_cluster=0,physical_sector=0,current_spot=0, i=0;	
	char type_of_file;
	int root = 1;
	p_map += 19*SECTOR_SIZE;

	/*--- Print root directory ---*/
	if (root == 1) {
		printf("ROOT\n==================================================\n");
		root--;
	}

	while (p_map[0] != 0x00) {
		char *file_name = (char*) malloc(8*sizeof(char));
		char *file_extension = (char*) malloc(3*sizeof(char));
		/*--- Get name of file/directory ---*/
		for (i = 0; i < 8; i++) {
			if (p_map[i] == ' ') {
				break;
			}
			file_name[i] = p_map[i];
		}
		/*--- Get name of extension---*/
		for (i = 0; i < 3; i++) {
			file_extension[i] = p_map[i+8];
		}
	
		if (file_extension[0] != ' ') {
			strcat(file_name,".");
			strcat(file_name,file_extension);
		}

		/*--- Get type of file ---*/
		if (p_map[11] == 0x10) {
			while(p_map[11] == 0x10) {
				print_file(file_name,p_map,'D');	
				printf("%s\n==================================================\n",file_name);
				first_logical_cluster = (p_map[26] & 0xFF) + ((p_map[27] & 0xFF) << 8);
				physical_sector = 31 + first_logical_cluster - 19;
				current_spot = physical_sector*SECTOR_SIZE + 32;
				p_map += current_spot;

				for (i = 0; i < 8; i++) {
					if (p_map[i] == ' ') {
						break;
					}
					file_name[i] = p_map[i];
				}
				/*--- Get name of extension---*/
				for (i = 0; i < 3; i++) {
					file_extension[i] = p_map[i+8];
				}
			
				if (file_extension[0] != ' ') {
					strcat(file_name,".");
					strcat(file_name,file_extension);
				}

				print_file(file_name,p_map,'F');
				p_map -= current_spot;
				p_map += 32;	
				type_of_file = 'D';
			}
			continue;
		} else {
			type_of_file = 'F';
		} 
		if (type_of_file == 'D') {
			printf("\n%s\n==================================================\n",file_name);
		} 
		/*--- Print file information ---*/
		if (p_map[26] == 0 || p_map[26] == 1) {
			p_map += 32;
			continue;
		}
		print_file(file_name,p_map,type_of_file);	
		p_map += 32;
	}
	return NULL;
}


/*---------- MAIN ----------*/
int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR); 			// open disk image file for reading and writing only
	fstat(fd, &sb);				// get file status


	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
	
	printf("\n");
	print_dir_list(p);
	printf("\n");

	munmap(p, sb.st_size); 					// the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}
