For this assignment, I have implemented utilities to perform operations
on a simple file system (SFS), FAT12, used by MS-DOS.

======================================================================

How to use this SFS:
Step 1: execute "make" in terminal to compile all
Step 2: to run diskinfo, execute `./diskinfo <some_disk_image.ima>`
	to run disklist, execute "./disklist <some_disk_image.ima>"
	to run diskget, execute "./diskget <some_disk_image.ima> <some_file.ext>"
	to run diskput, execute "./diskput <some_disk_image.ima> <some_file_in_local_dir.ext>"

======================================================================

What do these functions do?
- `diskinfo`: output disk information which includes the OS name, label of the disk,
	total size of the disk, free size of the disk, number of files on the disk,
	number of FAT copies, and the sectors per fat
- `disklist`: get a list of the files/subdirectories with their date/time and file size
- `diskget`: copies a file from root directory of the file system to the current directory in Linux
- `diskput`: copies a file from the current Linux directory into a specified directory 
	(i.e. root directory or subdirectory) of the file system.
