#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SECTOR_SIZE 512
#define TRUE 1
#define FALSE 0

/*---------- GLOBAL VARIBALES ----------*/
struct node {
	struct node *next;
	int jump_size;
	int file_steps;
};
 typedef struct node node;

node *head = NULL;
node *tail = NULL;
int just_returned = FALSE;


/*---------- LINKED LIST ----------*/
/* Code taken from work in assignment 1 */
/* adds subdirectory location to queue */
void enqueue_subdir(int jump){
	node *cur_node = (node *) malloc(sizeof(node));
	cur_node->next = NULL;
	cur_node->jump_size = jump;
	cur_node->file_steps = 0;
	
	if (head == NULL) {
		head = cur_node;
		tail = cur_node;

	} else {
		tail->next = cur_node;
		tail = tail->next;
	}
} 


/* removes subdirectory location from queue */
void dequeue_subdir(int jump) {
	node *cur = head;
	if (cur->jump_size == jump){
		head = cur->next;
	} else {
		while (cur->next->jump_size != jump) {
			cur = cur->next;
		}
		cur->next = cur->next->next;
		free(cur);
	}
}


/*---------- HELPER FUNCTIONS ----------*/

/* get_os_name finds the name of the OS currently in use
 * by going to the boot sector and finding the information
 * starting at byte 3 (length 8) */
char *get_os_name(char *p_map) {

	int i;
	char *os_name = (char*) malloc(9*sizeof(char));

	for (i = 0; i < 9; i++) {
		os_name[i] = p_map[i+3];
	}
	
	return os_name;
}

/* get_label finds the label of the disk by going to the
 * boot sector and root directory */
char *get_label(char *p_map) {
	
	int i;	
	char *label = (char*) malloc(11*sizeof(char));

	for (i = 0; i < 8; i++) {
		label[i] = p_map[i+43];
	}
	if (label[0] == ' ') {
		p_map += SECTOR_SIZE * 19; 			// go to root directory
		while (p_map[0] != 0x00) {
			if (p_map[11] == 0x08) { 		// if the volume label is in the attribute section
				for (i = 0; i < 8; i++) {
					label[i] = p_map[i];
				}
			}
			p_map += 32; 					// move to data area
		}
	}
	
	return label;
}
/* get_total_size finds the total size of the disk
 * by taking the total sector count and multiplying by 
 * the number of bytes per sector */
int get_total_size(char *p_map) {
	int bytes_per_sector;
	int total_sector_count;
	bytes_per_sector = p_map[11] + (p_map[12] << 8);
	total_sector_count = p_map[19] + (p_map[20] << 8);

	return bytes_per_sector * total_sector_count;
}


/* get_fat_entry_result returns the value of the fat entry
 * it is passed */
int get_fat_entry_result(int entry, char *p_map) {
	int result;
	if (entry % 2 == 0) {
		result = ((p_map[SECTOR_SIZE + 1+3*(entry)/2] & 0x0F) << 8) + ((p_map[SECTOR_SIZE + 3*(entry)/2]) & 0xFF);
	} else {
		result = ((p_map[SECTOR_SIZE + 3*(entry)/2] & 0xF0) >> 4) + ((p_map[SECTOR_SIZE + 1+3*(entry)/2] & 0xFF) << 4);
	}
	return result ;
}


/* get_free_size goes through FAT entries and determines
 * if the entry is zero (fat entry == 0x00), which means 
 * it is empty */
int get_free_size(int total_size, char *p_map) {
	int cur_entry;
	int num_free_sectors = 0;
	int result = 0;
	
	for (cur_entry = 2; cur_entry < (total_size/SECTOR_SIZE-31); cur_entry++) {
		result = get_fat_entry_result(cur_entry,p_map); 
		if (result == 0x00) {
			num_free_sectors++;
		}	
	}
	return (num_free_sectors)* SECTOR_SIZE;		// account for the root directory offset
	
}


/* get_num_files returns the number of files in the disk
 * including all files in the root directory and files in
 * the subdirectories */
int get_num_files(char *p_map) {
	int first_logical_cluster=0,physical_sector=0,current_spot=0, file_count = 0;
	int jump = 0;
	node *cur = (node*) malloc(sizeof(node));
	if (head == NULL) {
		p_map += SECTOR_SIZE * 19;									// move pointer to root directory 
	}

	while (p_map[0] != 0x00) {	
		if (p_map[11] == 0x0F || p_map[26] == 0 || p_map[26] == 1 || p_map[11] == 0x08 || p_map[0] == 0xE5 || p_map[0] == 0x00) {													// if im not a valid file
			p_map += 32;										// move to the next location
			continue;											// loop
		} else {
			if (head != NULL) {								// if there are items in the queue
				for (int i=0; i < head->file_steps; i++){	// find previous location & number of file steps
					cur = cur->next;
				}
				if (just_returned == FALSE) {
					cur->file_steps++;							// im at a new location, tell me how far!
				}
			}
			if (p_map[11] == 0x10) {						// if this is a subdirectory
				p_map += 32;
				first_logical_cluster = (p_map[26] & 0xFF) + ((p_map[27] & 0xFF) << 8);
				physical_sector = 31 + first_logical_cluster - 19;
				current_spot = physical_sector*SECTOR_SIZE + 32;
				p_map += current_spot;
				file_count++;
				p_map -= current_spot;
				
				just_returned = TRUE;

			} else if ((p_map+32)[0] == 0x00) {				// if the next item is null (end of subdir)
				if (head != NULL) {								// if im in a subdirectory
					file_count++;								// increase the file count
					if (cur->jump_size == head->jump_size) {	// if im going back to the head
						dequeue_subdir(jump);						// remove the final item from queue
					} else {									// if im still in a subdirectory
						p_map -= 32 * cur->file_steps;				// go back up to previous location
						dequeue_subdir(jump);						// remove jump from queue
					}
				}
				p_map += 32;
				file_count++;
			} else {
				file_count++;									// I've found a file!
				p_map += 32;									// move to next location
			}
		}
	}
	free(cur);
	return file_count;
}



int get_file_size(char *p_map, char *file_name) {
	int i = 0;
	p_map += 19 * SECTOR_SIZE;
	while (p_map[0] != 0x00) {			
		if (((p_map[11] & 0x02)) == 0 && ((p_map[11] & 0x08) == 0)) {	// if its not hidden or a volume label
			char *cur_file_name = (char *) malloc(8*sizeof(char));
			char *cur_file_extension = (char *) malloc(3*sizeof(char));

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
			strcat(cur_file_name,cur_file_extension);

			if (!strcmp(cur_file_name, file_name)) {
				return (p_map[28] & 0xFF) + ((p_map[29] & 0xFF) << 8) + ((p_map[30] & 0xFF) << 16) + ((p_map[31] & 0xFF) << 24);
			}
		}
		p_map += 32;
	}
	return -1;
}
