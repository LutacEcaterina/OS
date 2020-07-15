#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"

void thread_function(void* arg) { 
	// possible thread’ actions ...
// inform the tester about thread start
	info(BEGIN, 2, 5);
// other possible thread’ actions ...
// inform the tester about thread termination 
	info(END, 2, 5);
}

void create

int main(){
    init();

//******************************* P1
    info(BEGIN, 1, 0);
	printf("START P1\n");
	for(int i=2;i<)
	//createa new process
	if (fork() == 0) {
//******************************* P2
		info(BEGIN, 2, 0);
		printf("START P2\n");
		//new thread creation
		//pthread_create();

		if (fork() == 0) {
//******************************* P5
			info(BEGIN, 5, 0);
			printf("START P5\n");
			//new thread creation
			//pthread_create();

			info(END, 5, 0);
			printf("	END P5\n");
		}
		if (fork() == 0) {
//******************************* P7
			info(BEGIN, 7, 0);
			printf("START P7\n");
			//new thread creation
			//pthread_create();

			info(END, 7, 0);
			printf("	END P7\n");
		}

		info(END, 2, 0);
		printf("	END P2\n");
		if (fork() == 0) {
//******************************* P3
			info(BEGIN, 3, 0);
			printf("START P3\n");
			//new thread creation
			//pthread_create();

			if (fork() == 0) {
//******************************* P4
				info(BEGIN, 4, 0);
				printf("START P4\n");
				//new thread creation
				//pthread_create();

				info(END, 4, 0);
				printf("	END P4\n");
			}
			if (fork() == 0) {
//******************************* P6
				info(BEGIN, 6, 0);
				printf("START P6\n");
				//new thread creation
				//pthread_create();
				
				if (fork() == 0) {
//******************************* P8
					info(BEGIN, 8, 0);
					printf("START P8\n");
					//new thread creation
					//pthread_create();

					info(END, 8, 0);
					printf("	END P8\n");
				}

				info(END, 6, 0);
				printf("	END P6\n");
			}

			info(END, 3, 0);
			printf("	END P3\n");
		}
	}
    
	//new thread creation
	pthread_create();

    info(END, 1, 0);
	printf("	END P8\n");
    return 0;
}
