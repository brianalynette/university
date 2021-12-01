/* 
 * Student Name: Briana Johnson
 * Student Number: V00929120
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <ctype.h>

#define MAX_FILENAME 10
#define MAX_OUTPUT_LEN 128




/*--------------- STRUCTS ---------------*/

struct node {
	struct node *next;
	pid_t pid;
	char comm[MAX_FILENAME];
};
typedef struct node node;




/*--------------- GLOBAL VARIABLES & FUNCTION DECLARATIONS ---------------*/

node *head = NULL;
node *tail = NULL;
int validProcess(pid_t pid);




/*--------------- LINKED LIST ---------------*/

/* adds process to linked list */
void addProcess(pid_t new_pid, node *cur_node){
	cur_node->next = NULL;
	cur_node->pid = new_pid;
	
	if (head == NULL) {
		head = cur_node;
		tail = cur_node;

	} else {
		tail->next = cur_node;
		tail = tail->next;
	}
} 



/* removes process from linked list */
void removeProcess(pid_t pid) {
	node *cur = head;
	if (cur->pid == pid){
		head = cur->next;
	} else {
		while (cur->next->pid != pid) {
			cur = cur->next;
		}
		cur->next = cur->next->next;
		free(cur);
	}
}




/*--------------- COMMANDS ---------------*/

/* runs a program in the background */
void bg(char **argv,node *cur){
	
	pid_t pid;
	pid = fork();
	int status;	

	/* Child Process */
	if(pid == 0){
		if(execvp(argv[0], argv) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	/* Parent Process */
	else if(pid > 0) {
		addProcess(pid,cur);
		waitpid(pid,&status,WNOHANG);
	}
	/* Failed to create new process */
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
}



/* prints a list of all background processes currently running with their PID */
void bglist(){
	node *cur = head;
	int count = 0;

	if(cur == NULL) {
		printf("Total background jobs: %d\n",count);
		return;
	} else if (cur->next == NULL) {
		printf("%d:\t %s\n",cur->pid,cur->comm);
		count++;
	} else {
		while (cur->next != NULL) {
			printf("%d:\t %s\n",cur->pid,cur->comm);
			count++;
			cur = cur->next;
		}
		printf("%d:\t %s\n",cur->pid,cur->comm);
		count++;
	}
	printf("Total background jobs: %d\n",count);
}



/* sends a signal to process (kill, stop, and continue) */
void bgsig (pid_t pid_cur, char ctype[]){
	if (validProcess(pid_cur) == -1) {
		printf("Error: Process %d does not exist\n",pid_cur);
		return;
	}

	if (strcmp(ctype,"bgkill") == 0){
		removeProcess(pid_cur);
		kill(pid_cur,SIGTERM);
		printf("Process %d successfully terminated\n",pid_cur);
	
	} else if (strcmp(ctype,"bgstop") == 0) {
		kill(pid_cur,SIGSTOP);
	
	} else if (strcmp(ctype,"bgcont") == 0) {
		kill(pid_cur,SIGCONT);

	} else {
		printf("Error: invalid command\n");
		return;
	}
}



/* prints a list of stats about the selected process including
 * comm,state,utime,stime,rss,voluntary_ctxt_switches,nonvoluntary_ctxt_switches */
void pstat(int pid_cur){
	if (validProcess(pid_cur) == -1) {
		printf("Error: Process %d does not exist\n",pid_cur);
		return;
	}
	/* stat */
	char filepathStat[18];
	char filepathStatus[20];
	char *comm = NULL;
	char *state;
	long utime;
	long stime;
	int rss;
	/* status */
	char *vctxts;
	char *nvctxts;

	sprintf(filepathStat, "/proc/%d/stat", pid_cur);
	FILE *fpStat = fopen(filepathStat,"r");

	sprintf(filepathStatus,"/proc/%d/status", pid_cur);
	FILE *fpStatus = fopen(filepathStatus,"r");
	
	if (fpStat) {
		char *linestat = NULL;
		size_t length = 0; 
		getline(&linestat,&length,fpStat);
		char *infostat = strtok(linestat," ");
		int count = 1;
		while (infostat != NULL) {
			if (count == 2) {
				comm = infostat;
				printf("comm:\t%s\n",comm);

			} else if (count == 3) {
				state = infostat;
				printf("state:\t%s\n",state);

			} else if (count == 14) {
				utime = atoi(infostat);
				utime = utime/sysconf(_SC_CLK_TCK);
				printf("utime:\t%ld\n",utime);

			} else if (count == 15) {
				stime = atoi(infostat);
				stime = stime/sysconf(_SC_CLK_TCK);
				printf("stime:\t%ld\n",stime);

			} else if (count == 24) {
				rss = atoi(infostat);
				printf("rss:\t%d\n",rss);

			}
			infostat = strtok(NULL," ");
			count++;
		}

		char *linestatus = NULL;
		length = 0;
		getline(&linestatus,&length,fpStatus);	
		count = 1;
		while (getline(&linestatus,&length,fpStatus) != -1) {
			infostat = strtok(linestatus,"\n");
			if (count == 53){
				infostat = strtok(linestatus,"\t");
				vctxts = strtok(NULL,"\n");
				printf("voluntary_ctxt_switches:\t%s\n",vctxts);

			} else 	if (count == 54) {
				infostat = strtok(linestatus,"\t");
				nvctxts = strtok(NULL,"\0");
				printf("nonvoluntary_ctxt_switches:\t%s\n",nvctxts);
			}
			count++;
		}
	} 
	fclose(fpStat);
	fclose(fpStatus);
}



/* check if currently running process is a zombie process, remove if so */
void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);
		if(head == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			removeProcess(retVal);
			printf("Zombie process successfully terminated\n");
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
	return ;
}




/*--------------- HELPERS ---------------*/

/* splits input into tokens and adds to an argument array */
int checkInput(char **inputArgs){
	char *input = readline("PMan: > ");

	if (strcmp(input,"") == 0){
		return 0;
	} else {
		char *token = strtok(input," ");
		inputArgs[0] = token;
		int i = 1;
		while (token != NULL) {
			token = strtok(NULL," ");
			inputArgs[i] = token;
			i++;
		}
		return 1;
	}
}



/* parses input and runs commands based on input */
void parseInput(char **inputArgs){
	node *cur = (node *) malloc(sizeof(node));
	char *cmd_type = inputArgs[0];
	char *fileEx = inputArgs[1];

	/*----- bg -----*/
	if (strcmp(cmd_type,"bg") == 0){
		strcpy(cur->comm,fileEx);
		int i=1;
		int i1 = 2;
		char *argv[999999];
		argv[0] = fileEx;
		while (inputArgs[i1] != NULL) {
			argv[i] = inputArgs[i1];
			i++;
			i1++;
		}
		argv[i] = NULL;
		bg(argv,cur);
	} 
	
	/*----- bglist -----*/
	else if(strcmp(cmd_type,"bglist") == 0){
		bglist();
	}
	
	/*----- bgkill OR bgstop OR bgcont -----*/
	else if((strcmp(cmd_type,"bgkill") == 0)||(strcmp(cmd_type,"bgstop") == 0)||(strcmp(cmd_type,"bgcont") == 0)) {
		if (inputArgs[1]){
			if (isdigit(inputArgs[1][0])) {
				cur->pid = (pid_t) atoi(inputArgs[1]);
				bgsig(cur->pid, cmd_type);
			}
		} else {
			printf("Error: please enter a valid PID\n");
		}
	}

	/*----- pstat -----*/
	else if(strcmp(cmd_type,"pstat") == 0){
		if (inputArgs[1]){
			if (isdigit(inputArgs[1][0])) {
				cur->pid = (pid_t) atoi(inputArgs[1]);
				pstat(cur->pid);
			}
		} else {
			printf("Error: please enter a valid PID\n");
		}

	}

	/*----- invalid command -----*/ 
	else {
		printf("PMan: > %s:\t command not found\n",cmd_type);
	}
}



/* check if process is valid */
int validProcess(pid_t pid) {
	node *cur = head;

	if (cur == NULL) {
		return -1;
	} else if (cur->pid == pid) {
		return 1;
	}

	while (cur->next != NULL){
		if (cur->next->pid == pid) {
			return 1;
		}
	} 
	return -1;
}




/*--------------- MAIN ---------------*/

int main(){

	while(1){	
		char *input[128];
		if (checkInput(input)) {
			parseInput(input);
		}
		check_zombieProcess();
	}
}


