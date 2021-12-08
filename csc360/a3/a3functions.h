struct node;
void *is_file_empty(char *p_map);
char *get_os_name(char *p_map);
char *get_label(char *p_map);
int get_total_size(char *p_map);
int get_fat_entry_result(int entry, char *p_map); 
int get_free_size(int total_size, char *p_map);
int get_num_files(char *p_map);
int get_file_size(char *p_map, char *file_name); 
