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


/*---------- MAIN ----------*/
int main(int argc, char *argv[])
{
	int fd;
	struct stat sb;
	char *os_name, *label;
	int total_size, free_size, num_files, num_fat_copies, sectors_per_fat;


	fd = open(argv[1], O_RDWR); 			// open disk image file for reading and writing only
	fstat(fd, &sb);				// get file status


	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
		
	/*--- Collect Info ---*/
	os_name = get_os_name(p);
	label = get_label(p);
	total_size = get_total_size(p);
	free_size = get_free_size(total_size, p);
	num_files = get_num_files(p);
	num_fat_copies = p[16];
	sectors_per_fat = p[22] + (p[23] << 2);

	/*--- Output Info ---*/
	printf("\nOS Name: %s\n",os_name);
	free(os_name);
	printf("Label of the disk: %s\n",label);
	free(label);
	printf("Total size of the disk: %d\n",total_size);
	printf("Free size of the disk: %d\n",free_size);
	printf("\n=================\n");
	printf("The number of files in the disk: %d\n",num_files);
	printf("\n=================\n");
	printf("Number of FAT copies: %d\n",num_fat_copies);
	printf("Sectors per FAT: %d\n\n",sectors_per_fat);

	munmap(p, sb.st_size); 					// the modifed the memory data would be mapped to the disk image
	close(fd);
	return 0;
}
