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
#include <time.h>
#include "a3functions.h"

#define TRUE 1
#define FALSE 0
#define SECTOR_SIZE 512

/*---------- HELPER FUNCTIONS ----------*/

void *update_dir(char *p_map, char *file_name, int file_size, int first_logical_cluster) {

	int i=0, extension = 0;
	char file_name_parse = 'z';
	
	/*--- add filename & attribute to root dir ---*/
	for (i=0; i<8; i++) {
		file_name_parse = file_name[i];
		if (file_name_parse == '.') {
			extension = i;
			break;
		}
		p_map[i] = file_name_parse;
	}
	extension = i;
	p_map[i+1] = ' ';
	for (i=0; i<3; i++) {
		p_map[i+8] = file_name[i+extension+1];
	}
	p_map[11] = 0x00;

	/*--- Set creation time ---*/
	time_t cur_time = time(NULL);
	struct tm *time_now = localtime(&cur_time);
	//int second = time_now->tm_sec;				// might not need
	int minute = time_now->tm_min;
	int hour = time_now->tm_hour;
	
	/*--- Set creation date ---*/
	int day = time_now->tm_mday;
	int month = time_now->tm_mon + 1;			// tm_mon is auto in range (0 to 11)
	int year = time_now->tm_year + 1900;		// tm_year is the num years since 1900

	/*--- Set date/time to be empty before adding values ---*/
	p_map[14] = 0;
	p_map[15] = 0;
	p_map[16] = 0;
	p_map[17] = 0;

	/*--- Add values ---*/
	p_map[17] = (p_map[17] | ((year - 1980) << 1)) | (p_map[17] | ((month - ((p_map[16] & 0xE0) >> 5)) >> 3));
	p_map[16] = (p_map[16] | ((month - ((p_map[17] & 0x01) << 3)) << 5)) | (p_map[16] | (day & 0x1F));
	p_map[15] = (p_map[15] | ((hour << 3) & 0xF8)) | (p_map[15] | ((minute = ((p_map[14] & 0xE0) >> 5)) >> 3));
	p_map[14] = p_map[14] | (minute - ((p_map[15] & 0x07) << 5));

	/*--- Set first logical cluster ---*/
	p_map[26] = (first_logical_cluster - (p_map[27] << 8)) & 0xFF;
	p_map[27] = (first_logical_cluster - p_map[26]) >> 8;

	/*--- Set file size ---*/
	p_map[28] = file_size & 0xFF;
	p_map[29] = (file_size & 0xFF00) >> 8;
	p_map[30] = (file_size & 0xFF0000) >> 16;
	p_map[31] = (file_size & 0xFF000000) >> 24;
	return NULL;
}

int find_free_fat_space(char *p_map) {
	int entry = 2;
	p_map += SECTOR_SIZE;
	while (get_fat_entry_result(entry,p_map) != 0x000) {
		entry++;
	}
	return entry;
}

void *add_new_fat_entry(char *p_map, int fat_entry, int fat_value){
	if ((fat_entry % 2) == 0) {
		p_map[(3*fat_entry)/2 + SECTOR_SIZE] = fat_value & 0xFF;
		p_map[(3*fat_entry)/2+1 + SECTOR_SIZE] = (fat_value >> 8) & 0x0F;
	} else {
		p_map[(3*fat_entry)/2 + SECTOR_SIZE] = (fat_value << 4) & 0xF0;
		p_map[(3*fat_entry)/2+1 + SECTOR_SIZE] = (fat_value >> 4) & 0xFF;
	}
	return NULL;
}

void *copy_file(char *p_map, char *new_pmap, char *file_name, int file_size) {

	/*--- variables ---*/
	int next_fat = 0, physical_sector = 0;
	int i, remaining_bytes;
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
				return NULL;
			}
		}
		p_map += 32;
	}
	
	remaining_bytes = file_size;
	int current_fat = find_free_fat_space(p_map);
	update_dir(p_map,file_name,file_size,current_fat);
	
	while (remaining_bytes > 0) {

		physical_sector = SECTOR_SIZE * (31 + current_fat);

		for (i = 0; i < SECTOR_SIZE; i++) {
			if (remaining_bytes == 0) {
				add_new_fat_entry(p_map, current_fat, 0xFFF);
				return NULL;
			}
			p_map[i + physical_sector] = new_pmap[file_size - remaining_bytes];
			remaining_bytes--;
		}
		add_new_fat_entry(p_map,current_fat,0x69);
		next_fat = find_free_fat_space(p_map);
		add_new_fat_entry(p_map,current_fat,next_fat);
		current_fat = next_fat;
	}


	return NULL;
}




/*---------- MAIN ----------*/
int main(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Error: too few arguments. Please use ./diskput [disk image] [path (optional)]/[filename]\n");
		exit(1);
	}

	int fd, new_fd, i, fname_len, file_size;
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


	char *path_item_1 = strtok(new_file,"/");
	char *path_item_2 = strtok(NULL,"\0");

	if (path_item_2 == NULL) {
		new_file = path_item_1;
	} else {
		new_file = path_item_2;
	}

	if (path_item_2 == NULL) {
		new_fd = open(path_item_1, O_RDWR);
		if (new_fd < 0) { 
			printf("Error: file not found. Please enter a valid file name.\n");
			close(new_fd);
			exit(1);
		}
	} else {
		new_fd = open(path_item_2, O_RDWR);
		if (new_fd < 0) { 
			printf("Error: file not found. Please enter a valid file name.\n");
			close(new_fd);
			exit(1);
		}
	}

	/*--- map memory for new file ---*/
	struct stat new_sb;
	fstat(new_fd, &new_sb);
	file_size = new_sb.st_size;
	char * new_p = mmap(NULL, file_size, PROT_READ, MAP_SHARED, new_fd, 0);
	if (new_p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
	

	/*--- Collect Info ---*/
	int total_disk_space = get_total_size(p);
	int free_disk_space = get_free_size(total_disk_space,p);
	if (total_disk_space - free_disk_space > 0) { 
		copy_file(p,new_p,new_file,file_size);	
	} else {
		printf("Error: not enough free space in the disk image.\n");
		exit(1);
	}

	munmap(p, sb.st_size); 					// the modifed the memory data would be mapped to the disk image
	munmap(new_p, file_size);
	close(fd);
	close(new_fd);
	return 0;
}
