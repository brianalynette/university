#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "a3functions.h"

#define TRUE 1
#define FALSE 0
#define SECTOR_SIZE 512


/*---------- HELPER FUNCTIONS ----------*/

/* copies file from disk image to local directory */
void *copy_file(char *p_map, char *new_pmap, char *file_name, int file_size) {
	/*--- variables ---*/
	int next_logical_sector = 0, physical_sector = 0, num_iter = 0, current_spot = 0;
	int first_logical_cluster, i, remaining_bytes;
	p_map += 19 * SECTOR_SIZE;				// move to root directory
	
	while (p_map[0] != 0x00) {			
		if (((p_map[11] & 0x02)) == 0 && ((p_map[11] & 0x08) == 0)) {	// if its not hidden or a volume label
			char *cur_file_name = (char *) malloc(8*sizeof(char));
			char *cur_file_extension = (char *) malloc(3*sizeof(char));

	/*--- Step 1: find the corresponding filename ---*/
			for (i=0; i<8; i++) {
				if (p_map[i] == ' ') {
					break;
				}
				cur_file_name[i] = p_map[i];
			}
			strcat(cur_file_name, ".");	
			for (i=0; i<3; i++){
				cur_file_extension[i] = p_map[i+8];
			}
			strcat(cur_file_name, cur_file_extension);	
			if (!strcmp(cur_file_name, file_name)) {

	/*--- Step 2: using the first logical cluster, find the physical sector	(33 + first_logical_cluster - 2) 
			  	  and copy 512 bytes ---*/
				remaining_bytes = file_size;
				first_logical_cluster = (p_map[26] & 0xFF) + ((p_map[27] & 0xFF) << 8);

	/*--- Step 3: go to physical sector in fat table combined with the high 4 bits at the offset. Combine these 
				  to get the next logical sector. The corresponding physical sector is at (33 + (next_logical_sector) - 2				   ) so read in another 512 bytes from that place. ---*/			
				physical_sector = 31 + first_logical_cluster - 19;
				current_spot = physical_sector*SECTOR_SIZE - num_iter*32;
				p_map += current_spot;

				for (i = 0; i < SECTOR_SIZE; i++) {
					if (remaining_bytes == 0) {
						break;
					}
					new_pmap[file_size - remaining_bytes] = p_map[i];
					remaining_bytes--;
				}
		
	/*--- Step 4: repeat step 3 until the entry in the FAT table has a value 0xFFF (eof) ---*/
				while (get_fat_entry_result(next_logical_sector,p_map) != 0xFFF) {
					if (p_map[0] == 0 || p_map[0] == 0xFFF) {
						break;
					}
					/*--- copy info from current sector ---*/
					for (i = 0; i < SECTOR_SIZE; i++) {
						if (remaining_bytes == 0) {
							return NULL;
						}
						new_pmap[file_size - remaining_bytes] = p_map[i + physical_sector];
						remaining_bytes--;
					}
					/*--- next logical sector ---*/
					next_logical_sector = (0x200 + 1 + 3*(first_logical_cluster)/2) + ((0x200 + 3 * (first_logical_cluster) / 2) >> 4);	
					physical_sector = 33 + (next_logical_sector) - 2;		
					p_map += 512;
				}
			}
		}
		p_map += 32;
		num_iter ++;
	}
	return NULL;
}

/*---------- MAIN ----------*/
int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Error: too few arguments. Please use ./diskget [disk image] [filename]\n");
		exit(1);
	}

	int fd, new_fd, i, fname_len, file_size, test_stretch;
	struct stat sb;							// sb = status buffer
	char *new_file = argv[2];
	fname_len = strlen(new_file);
	char *upper_case_file = (char *) malloc(fname_len*sizeof(char));


	fd = open(argv[1], O_RDWR); 			// open disk image file for reading and writing only
	fstat(fd, &sb);							// get file status

	/*--- create map of image ---*/
	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	/*--- Change string from lowercase to upper case ---*/
	for (i = 0; i<fname_len; i++) {
		upper_case_file[i] = toupper(new_file[i]);
	}

	file_size = get_file_size(p,upper_case_file);
	if (file_size < 0) {
		printf("Error: file not found. Please enter a valid file name.\n");
		exit(1);	
	}
	new_fd = open(argv[2], O_RDWR | O_CREAT, 0666);
	if (new_fd < 0) { 
		printf("Error: unable to open new file\n");
		close(new_fd);
		exit(1);
	}
	/*--- stretch file ---*/
	test_stretch = lseek(new_fd, file_size-1, SEEK_SET);
	if (test_stretch < 0) {
		close(fd);
		close(new_fd);
		printf("Error: unable to seek eof\n");
		exit(1);
	}
	test_stretch = write(new_fd, "", 1);
	if (test_stretch != 1) {
		close(fd);
		close(new_fd);
		printf("Error: unable to write last byte\n");
		exit(1);

	}
	/*--- map memory for new file ---*/
	char * new_p = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED, new_fd, 0);
	if (new_p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	/*--- Collect Info ---*/
	copy_file(p,new_p,argv[2],file_size);	

	munmap(p, sb.st_size); 					// the modifed the memory data would be mapped to the disk image
	munmap(new_p, file_size);
	close(fd);
	close(new_fd);
	return 0;
}
