/*
 * Student Name: Briana Johnson
 * Student Number: V00929120
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <wait.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0
#define NQUEUE 1
#define MAX_FILENAME 30
#define IDLE 0




/*--------------- STRUCTS ---------------*/
struct customer_info{ 
    int user_id;
	int class_type;
	int service_time;
	int arrival_time;
	int clerk;					// clerk serving customer
	float wait_start;			// start of service
	float wait_end;				// end of service
};
typedef struct customer_info customer_info;

struct clerk_info {
	int id;
	int busy;					// 0 = not busy, 1 = busy
};
typedef struct clerk_info clerk_info;

struct node {
	struct node *next;
	struct customer_info *customer;
};
typedef struct node node;





/*--------------- GLOBAL VARIABLES ---------------*/
node *head = NULL;				// linked list of all customers
node *tail = NULL;				// linked list of all customers
node *busi_head = NULL;
node *busi_tail = NULL;
node *econ_head = NULL;
node *econ_tail = NULL;

customer_info *queue[2];		// list of current customers in use
clerk_info clerks[4];			// list of all clerks

struct timeval init_time; 		// record simulation start time
// No need to use mutex_lock when reading this var since the val would not be changed by thread once initial time was set.
struct timeval busiTime;		// record business-class wait time
struct timeval econTime;		// record economy-class wait time

double overall_waiting_time; 	// add up wait time for all customers, every customer adds their own wait time, mutex_lock is necessary.
double busiCount;				// record number of business-class customers
double econCount;				// record number of economy-class customers

int queue_length[NQUEUE];		// variable stores the real-time queue length information; mutex_lock needed
int queue_status[NQUEUE]; 		// variable to record the status of a queue, the value could be idle (not using by any clerk) or the clerk id (1 ~ 4), indicating that the corresponding clerk is now signaling this queue.
int winner_selected[NQUEUE]; 	// record if the first customer in a queue has been successfully selected & left the queue

void * thread_1(void *); 		// adding thread
void * thread_2(void *); 		// notification thread
pthread_mutex_t busi_lock; 		// business mutex declaration
pthread_mutex_t econ_lock;		// economy mutex declaration
pthread_mutex_t queue_lock;
pthread_mutex_t init_time_lock;
pthread_mutex_t select_lock;	// selecting clerk mutex declaration
pthread_mutex_t queue_broad_lock;

pthread_cond_t econ_convar = PTHREAD_COND_INITIALIZER; // economy convar initialization
pthread_cond_t busi_convar = PTHREAD_COND_INITIALIZER; // business convar initialization
pthread_cond_t econ_wait = PTHREAD_COND_INITIALIZER; // economy convar initialization
pthread_cond_t busi_wait = PTHREAD_COND_INITIALIZER; // business convar initialization
pthread_cond_t no_cust = PTHREAD_COND_INITIALIZER;

/* individual clerk declarations */
pthread_mutex_t clerk1_lock;								
pthread_cond_t clerk1_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk2_lock;							
pthread_cond_t clerk2_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk3_lock;								
pthread_cond_t clerk3_convar = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t clerk4_lock;								
pthread_cond_t clerk4_convar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t clerk5_lock;							
pthread_cond_t clerk5_convar = PTHREAD_COND_INITIALIZER;	




/*--------------- FUNCTION DECLARATIONS ---------------*/
void *clerk_entry(void * clerkNum);
void * customer_entry(void * cus_info);




/*--------------- LINKED LIST ---------------*/

/* adds node to linked list */
void addNode(customer_info *new_cust, node *cur){
	cur->next = NULL;
	cur->customer = new_cust;
	
	if (head == NULL) {
		head = cur;
		tail = cur;

	} else if (tail->customer->arrival_time <= new_cust->arrival_time) {
		tail->next = cur;
		tail = tail->next;

	} else {
		node *iter_node = head;

		if (iter_node->next == NULL) {
			iter_node->next = cur;
			tail = cur;
		} else if (head->customer->arrival_time > new_cust->arrival_time) {
			cur->next = head;
			head = cur;
		} else {
		
			while (iter_node->next != NULL) {
				if (iter_node->next->customer->arrival_time > new_cust->arrival_time) {
					cur->next = iter_node->next;
					iter_node->next = cur;
					break;
				}
				iter_node = iter_node->next;
			}
		}
	}
} 



/* removes node from linked list */
void removeNode(int user_id) {
	node *cur = head;
	if (cur->customer->user_id == user_id){
		head = cur->next;
	} else {
		while (cur->next->customer->user_id != user_id) {
			cur = cur->next;
		}
		cur->next = cur->next->next;
		free(cur);
	}
}




/*--------------- QUEUE ---------------*/

/* adds to front of queue */
void enqueue(customer_info *cust) {
	node *cur = (node *) malloc(sizeof(node));
	cur->customer = cust;
	
	if (cust->class_type == 1){
		if (busi_head == NULL) {
			busi_head = cur;
			busi_tail = busi_head;
		} else {
			busi_tail->next = cur;
			busi_tail = cur;
		} 
	} else {
		if (econ_head == NULL) {
			econ_head = cur;
			econ_tail = econ_head;
		} else {
			econ_tail->next = cur;
			econ_tail = cur;
		} 
	}
}


/* remove from the front of the queue */
void dequeue(int class){
	if (class == 1) {
		if (busi_head != NULL) {
			if (busi_head->next != NULL) {
				busi_head = busi_head->next;
			} else {
				busi_head = NULL;
			}
			free(busi_head);
		}
	} else {
		if (econ_head != NULL) {
			if (econ_head->next != NULL) {
				econ_head = econ_head->next;
			} else {
				econ_head = NULL;
			}
			free(econ_head);
		}
	}
}




/*--------------- HELPER FUNCTIONS ---------------*/

/* parses input and creates a list of customers based on input file */
int parse_input(char *filename){

	char line[20];
	int i = 0;
	FILE *fp_stream;
	fp_stream = fopen(filename, "r");
	fgets(line,20,fp_stream); 			// remove initial line with the number of customers

	while(fgets(line,20,fp_stream) != NULL) {

		node *cur = (node *) malloc(sizeof(node));
		customer_info *cust = (customer_info *) malloc(sizeof(customer_info));

		cust->user_id = atoi(strtok(line,":"));
		cust->class_type = atoi(strtok(NULL,","));
		cust->arrival_time = atoi(strtok(NULL,","));
		cust->service_time = atoi(strtok(NULL,"\n"));

		addNode(cust,cur);
		i++;
	}
	return i;
}

/* code from helper file on gettimeofday() */
double getCurrentSimulationTime(){
	
	struct timeval cur_time;
	double cur_secs, init_secs;
	
	/* lock mutex */
	if (pthread_mutex_lock(&init_time_lock) != 0) {
		printf("Error: failed to lock mutex\n");
		exit(1);
	}

	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);

	/* unlock mutex */
	if (pthread_mutex_unlock(&init_time_lock) != 0) {
		printf("Error: failed to lock mutex\n");
		exit(1);
	}
	
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	
	return cur_secs - init_secs;
}



/* customer arrives, enters a queue, waits for a clerk to signal the queue, gets served by the clerk */
void * handle_customer (customer_info *my_info, clock_t enter_time) {

	if (queue[my_info->class_type] == NULL) {	// current user being handled
		queue[my_info->class_type] = my_info;
	}
	pthread_mutex_t *cur_lock;
	pthread_cond_t *cur_convar;
	node *cur_head;
	pthread_cond_t *cur_wait;

	
	/* declare "current" variables */
	if (my_info->class_type == 1) {
		cur_lock = &busi_lock;
		cur_convar = &busi_convar;
	} else {
		cur_lock = &econ_lock;
		cur_convar = &econ_convar;
	}

	/* lock mutex */
	if (pthread_mutex_lock(cur_lock) != 0) {
		printf("Error: failed to lock mutex\n");
		exit(1);
	}
	{
		enqueue(my_info);
		pthread_cond_broadcast(&no_cust);
		queue_length[my_info->class_type]++;
		fprintf(stdout, "A customer enters a queue: the queue ID %d, and length of the queue %2d. \n", my_info->class_type,queue_length[my_info->class_type]);
	}

	while (TRUE) {	
		if (my_info->class_type == 1) {
			cur_head = busi_head;
			cur_wait = &busi_wait;
		} else {
			cur_head = econ_head;
			cur_wait = &econ_wait;
		}

		/* wait for another clerk to signal the queue */
		if (pthread_cond_wait(cur_convar,cur_lock) != 0) {
			printf("Error: failed to wait for convar\n");
			exit(1);
		}
		/* if im the head & the first customer has not been selected yet */
		if (cur_head && cur_head->customer->user_id == winner_selected[my_info->class_type]){

			my_info->clerk = queue_status[my_info->class_type];
			dequeue(my_info->class_type);
			queue_length[my_info->class_type]--;
			fprintf(stdout, "A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",getCurrentSimulationTime(),my_info->user_id,queue_status[my_info->class_type]);

			queue_status[my_info->class_type] = IDLE;		// current queue is no longer in use
			pthread_cond_signal(cur_wait);					// i have an item in my queue, wake up a clerk!
			break;
		}
	}

	/* unlock mutex */
	if (pthread_mutex_unlock(cur_lock) != 0) {
		printf("Error: failed to unlock mutex\n");
		exit(1);
	}
	/* sleep for 10 seconds */
	usleep(10);

	usleep(my_info->service_time * 100000);
	fprintf(stdout, "A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n", getCurrentSimulationTime(),my_info->user_id,my_info->clerk);
	if (my_info->clerk == 1) {
		pthread_cond_signal(&clerk1_convar); 
	} else if (my_info->clerk == 2) {
		pthread_cond_signal(&clerk2_convar); 
	} else if (my_info->clerk == 3) {
		pthread_cond_signal(&clerk3_convar); 
	} else if (my_info->clerk == 4) {
		pthread_cond_signal(&clerk4_convar); 
	} else if (my_info->clerk == 5) {
		pthread_cond_signal(&clerk5_convar); 
	}

}



/* set up for customer threads, after customer arrives, send it to be handled by the clerks*/
void * customer_entry(void * cus_info){

	/*--- info set up ---*/
	struct customer_info * p_myInfo = (struct customer_info *) cus_info;
	clock_t queue_enter_time;

	/*--- wait until customer is supposed to arrive ---*/
	usleep(p_myInfo->arrival_time * 100000);						// wait till customer is supposed to arrive
	fprintf(stdout, "A customer arrives: customer ID %2d. \n", p_myInfo->user_id); // customer arrives

	handle_customer(p_myInfo,queue_enter_time);						// send customer to enter the queue and be handled by clerk
	pthread_exit(NULL);
	return NULL;
} 





/* handle clerks: serve customers,  */
void *clerk_entry(void * clerkNum){

	int *temp = (int*) clerkNum;
	int clerkID = *temp;
	customer_info *current_customer;
	int bus_econ = 0;

	while(TRUE){
		/* lock broadcast mutex */
		if (pthread_mutex_lock(&queue_broad_lock) != 0) {
			printf("Error: failed to lock signal mutex\n");
			exit(1);
		}
		{
			/* lock business mutex */
			if (pthread_mutex_lock(&busi_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			if (queue_status[1] != 0) {
				pthread_cond_wait(&busi_wait,&busi_lock);
			}	
			{
				/* if business queue has a customer waiting */
				if (busi_head != NULL) {	
					winner_selected[1] = busi_head->customer->user_id;
					busi_head->customer->clerk = clerkID;
					queue_status[1] = clerkID;
					pthread_cond_broadcast(&busi_convar); 				// awake first customer from busi queue
					bus_econ = 1;
					
				} else {	
					/* unlock business mutex */
					if (pthread_mutex_unlock(&busi_lock) != 0) {
						printf("error: failed to lock mutex\n");
						exit(1);
					}
					
					/* lock economy mutex */
					if (pthread_mutex_lock(&econ_lock) != 0) {
						printf("Error: failed to lock mutex\n");
						exit(1);
					}
					if (queue_status[0] != 0) {
						pthread_cond_wait(&econ_wait,&econ_lock);
					}
					{
						/* if economy has a customer waiting */
						if (econ_head != NULL) {
							winner_selected[0] = econ_head->customer->user_id;
							econ_head->customer->clerk = clerkID;
							queue_status[0] = clerkID;
							pthread_cond_broadcast(&econ_convar); 		// awake first customer from econ queue
							bus_econ = 0;
							
						/* if neither business nor economy has a customer waiting, wait for someone to arrive */
						} else {
							/* unlock economy mutex */
							if (pthread_mutex_unlock(&econ_lock) != 0) {
								printf("error: failed to lock mutex\n");
								exit(1);
							}
							pthread_cond_wait(&no_cust,&queue_broad_lock);
							/* unlock broadcast mutex */
							if (pthread_mutex_unlock(&queue_broad_lock) != 0) {
								printf("error: failed to lock mutex\n");
								exit(1);
							}

							continue;
						}
					}
				}	
			}
		}
		if (bus_econ == 1) {
			/* unlock business mutex */
			if (pthread_mutex_unlock(&busi_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}
		} else {
			/* unlock economy mutex */	
			if (pthread_mutex_unlock(&econ_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}			
		}
		/* unlock broadcast mutex */
		if (pthread_mutex_unlock(&queue_broad_lock) != 0) {
			printf("error: failed to lock mutex\n");
			exit(1);
		}
		if (clerkID == 1) {
			/* lock clerk1 mutex */
			if (pthread_mutex_lock(&clerk1_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			pthread_cond_wait(&clerk1_convar,&clerk1_lock);
			/* unlock clerk1 mutex */
			if (pthread_mutex_unlock(&clerk1_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}
				
		} else if (clerkID == 2) {
			/* lock clerk2 mutex */
			if (pthread_mutex_lock(&clerk2_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			pthread_cond_wait(&clerk2_convar,&clerk2_lock);
			/* unlock clerk2 mutex */
			if (pthread_mutex_unlock(&clerk2_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}

		} else if (clerkID == 3) {
			/* lock clerk3 mutex */
			if (pthread_mutex_lock(&clerk3_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			pthread_cond_wait(&clerk3_convar,&clerk3_lock);
			/* unlock clerk3 mutex */
			if (pthread_mutex_unlock(&clerk3_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}

		} else if (clerkID == 4) {
			/* lock clerk4 mutex */
			if (pthread_mutex_lock(&clerk4_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			pthread_cond_wait(&clerk4_convar,&clerk4_lock);
			/* unlock clerk4 mutex */
			if (pthread_mutex_unlock(&clerk4_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}

		} else if (clerkID == 5) {
			/* lock clerk5 mutex */
			if (pthread_mutex_lock(&clerk5_lock) != 0) {
				printf("Error: failed to lock mutex\n");
				exit(1);
			}
			pthread_cond_wait(&clerk5_convar,&clerk5_lock);
			/* unlock clerk5 mutex */
			if (pthread_mutex_unlock(&clerk5_lock) != 0) {
				printf("error: failed to lock mutex\n");
				exit(1);
			}
		} 
	}

	pthread_exit(NULL);
	return NULL;
}





/*--------------- MAIN ---------------*/

int main(int argc, char *argv[]) {
	
	/*--- initialize all condition variables and thread locks that will be used ---*/
	char *filename = argv[1],*class;
	int i = 0, NClerks = 5;
	int clerks[5] = {1,2,3,4,5};
	pthread_t clerkID[5];
	double cur_simulation_secs;

	/*--- parse input ---*/
	int NCustomers = parse_input(filename);	
	customer_info *custom_info[NCustomers];
	pthread_t customID[NCustomers];

	/*--- initialize mutex locks ---*/
	if (pthread_mutex_init(&busi_lock,NULL) != 0) {
		printf("Error: failed to initialize business mutex\n");
		exit(1);
	} else 	if (pthread_mutex_init(&econ_lock,NULL) != 0) {
		printf("Error: failed to initialize economy mutex\n");
		exit(1);
	}

	/*--- initialize conditional variables ---*/
	if (pthread_cond_init(&busi_convar,NULL) != 0) {
		printf("Error: failed to initialize business convar\n");
		exit(1);
	} else 	if (pthread_cond_init(&econ_convar,NULL) != 0) {
		printf("Error: failed to initialize economy convar\n");
		exit(1);
	}

	/*--- make threads joinable ---*/
	pthread_attr_t joinable;
	if (pthread_attr_init(&joinable) != 0){
		printf("Error: failed to create joinable attribute\n");
		exit(1);
	}
	if (pthread_attr_setdetachstate(&joinable,PTHREAD_CREATE_JOINABLE) != 0) {
		printf("Error: failed to set a detach state for the joinable attribute\n");
		exit(1);
	}
	
	/*--- create clerk thread ---*/
	for(i = 0; i < NClerks; i++){ 
		if (pthread_create(&clerkID[i], NULL, clerk_entry, (void *)&clerks[i]) != 0) {
			printf("Error: failed to create clerk thread \n");
			exit(1);
		}
	} 
		
	gettimeofday(&init_time,NULL);

	/*--- create customer thread ---*/
	node *cur = head;
	for(i = 0; i < NCustomers; i++){
		if (cur != NULL) {
			custom_info[i] = cur->customer;
			if (pthread_create(&customID[i], NULL, customer_entry, (void *)custom_info[i])) {
				printf("Error: failed to create clerk thread \n");
				exit(1);
			}
			cur = cur->next;	
		} else {
			break;
		}
	}

	/*--- wait for all customer threads to terminate ---*/
	for(i = 0; i < NCustomers; i++){
		if (pthread_join(customID[i],NULL) != 0) {
			printf("error: failed to join customers\n");
			exit(1);
		}
		printf("joining thread %d\n",i);
	}

	cur_simulation_secs = getCurrentSimulationTime();



	/*--- destroy mutex & condition variable (optional) ---*/

	/*--- calculate the average waiting time of all customers ---*/
//	float bwait_time = (float) busiTime / busiCount;
//	float ewait_time = (float) econTime / econCount;
//	printf("The average waiting time for all customers in the system is: %.2f seconds. \n",overall_waiting_time);
//	printf("The average waiting time for all business-class customers is: %.2f seconds. \n",bwait_time);
//	printf("The average waiting time for all economy-class customers is: %.2f seconds. \n",ewait_time);



	return 0;
}


