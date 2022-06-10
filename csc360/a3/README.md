## Overview
For this assignment, I have implemented utilities to perform operations on a simple file system (SFS), FAT12, used by MS-DOS.

## How to use this SFS:
* Step 1: execute `make` in terminal to compile all
* Step 2: 
	* to run `diskinfo`, execute `./diskinfo <some_disk_image>`
	* to run `disklist`, execute `./disklist <some_disk_image>`
	* to run `diskget`, execute `./diskget <some_disk_image> <some_file>`
	* to run `diskput`, execute `./diskput <some_disk_image> <some_local_file>`

## What do these functions do?
* `diskinfo`: output disk information which includes the OS name, label of the disk,
	total size of the disk, free size of the disk, number of files on the disk,
	number of FAT copies, and the sectors per fat
* `disklist`: get a list of the files/subdirectories with their date/time and file size
* `diskget`: copies a file from root directory of the file system to the current directory in Linux
* `diskput`: copies a file from the current Linux directory into a specified directory 
	(i.e. root directory or subdirectory) of the file system.

## Note
I wrote all code except for:
* the first 15 lines in the `main` functions from `diskinfo.c` and `disklist.c`
* the first 20 lines in the `main` funcitons in `diskget.c` and `diskput.c`
