// la6
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h> 
#include <sys/wait.h>

    // for functions semget() to get semaphore, and semctl() set initial value of semget() 
#define KEY1 1100
#define KEY2 1111

#define CHILD      			0  			/* Return value of child proc from fork call */
#define TRUE       			0  
#define FALSE      			1

FILE *fp1, *fp2, *fp3, *fp4;			/* File Pointers */

int count = 0; 
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short  *array;
};

int sem1, sem2;

int SEM_ON(int sem_id, int sem_val);
int SEM_OFF(int sem_id);
int P(int sem_id);
int V(int sem_id);

int main()
{
	int pid;						// Process ID after fork call
	int i;							// Loop index
	int N;							// Number of times dad does update
	int N_Att;						// Number of time sons allowed to do update
	int status;						// Exit status of child process
	int bal1, bal2;					// Balance read by processes
	int flag, flag1;				// End of loop variables
	int dad = 0, son1 =0, son2=0; 

    // set semaphores
    sem1 = semget(KEY1, 1, IPC_CREAT | 0666); 
    if(sem1 < 0)
    {
        perror("ERROR: semget sem1\n"); 
        exit(EXIT_FAILURE); 
    }
    sem2 = semget(KEY2, 1, IPC_CREAT | 0666); 
    if(sem2 < 0)
    {
        perror("ERROR: segmet sem2\n");
        exit(EXIT_FAILURE); 
    }
   
	SEM_ON(sem1,1); //originally 1,0,1
    SEM_ON(sem2,0); 
 
	//Initialize the file balance to be $100                                // ** fp1 opens "balance" and writes the initial amount of the accnt 100
	fp1 = fopen("balance","w");                                             //** prints this to the file at this point there should be 100 
	bal1 = 100;
	fprintf(fp1, "%d\n", bal1);
	fclose(fp1);
	
	//Initialize the number of attempts to be 20                           
	fp4 = fopen("attempt", "w");
	N_Att = 20;//originally 2
	fprintf(fp4, "%d\n", N_Att);
	fclose(fp4);
	
	//Create child processes that will do the updates                            
	if ((pid = fork()) == -1) 
	{
		//fork failed!
		perror("fork");
		exit(1);
	}
	
	if (pid == CHILD){
	//First Child Process. Dear old dad tries to do some updates.                  *********** DAD process P1
		N=5;  // originally 5
		for(i=1;i<=N; i++)
		{   
           
           P(sem1);
			printf("Dear old dad is trying to do update.\n");             
			fp1 = fopen("balance", "r+");
            dad++; 
			printf("DAD waited %d times\n",dad);
            //P(sem1); //------------------------------------------------------------------wait sem2 semwait 
			fscanf(fp1, "%d", &bal2);
			printf("Dear old dad reads balance = %d \n", bal2);
			
			//Dad has to think (0-14 sec) if his son is really worth it
			sleep(rand()%15);
			fseek(fp1,0L,0);
			bal2 += 60;
			printf("Dear old dad writes new balance = %d \n", bal2);        
			fprintf(fp1, "%d \n", bal2);
            V(sem2); //-----------------------------------------------------------------> signal sem1   semsignal 
            V(sem1);  //this works 
			fclose(fp1);
			printf("Dear old dad is done doing update. \n");
			sleep(rand()%5);	/* Go have coffee for 0-4 sec. */
			
	
		}   
	}                                                                               //********** end of DAD process P1
	else
	{
		//Parent Process. Fork off another child process.
		if ((pid = fork()) == -1)
		{
			//Fork failed!
			perror("fork");
			exit(1);
		}
		if (pid == CHILD)
		{
			printf("First Son's Pid: %d\n",getpid());                                               
			//Second child process. First poor son tries to do updates.  
			flag = FALSE;
			while(flag == FALSE) 
			{
                son1++; 
				printf("SON1 waited %d times\n",son1);
                P(sem2);//-----------------------------sem
				fp3 = fopen("attempt" , "r+");                      //** son1 is checking the balance 
				fscanf(fp3, "%d", &N_Att);
				if(N_Att == 0)
				{
					fclose(fp3);
					flag = TRUE;
                     
				}
				else
				{

					printf("Poor SON_1 wants to withdraw money.\n");
                   
					fp2 = fopen("balance", "r+");                               // **son1 reads the balance 
					
                    fscanf(fp2,"%d", &bal2);
					printf("Poor SON_1 reads balance. Available Balance: %d \n", bal2);
					if (bal2 == 0)
					{
						fclose(fp2);
						fclose(fp3);
                    
					}
					else
					{   
                        
						sleep(rand()%5);
						fseek(fp2,0L, 0);
						bal2 -=20;
						printf("Poor SON_1 write new balance: %d \n", bal2);    // ** SON1 CS new balance
                    
						fprintf(fp2,"%d\n", bal2);
						fclose(fp2);
                        
						printf("poor SON_1 done doing update.\n");          
						fseek(fp3,0L, 0);
						N_Att -=1;
						fprintf(fp3, "%d\n", N_Att);
                        V(sem1);
                        V(sem2); // -------------------------------------------------------- signal sem2
						fclose(fp3); 
                       
					}
				}
               
			}

		}
		else
		{
		//Parent Process. Fork off one more child process.
			if ((pid = fork()) == -1) 
			{
				//fork failed!
				perror("fork");
				exit(1);
			}
			if (pid == CHILD)
			{
				printf("Second Son's Pid: %d\n",getpid());                              
				//Third child process. Second poor son tries to do updates.
                //wait??
                 
				flag1 = FALSE;
				while(flag1 == FALSE) 
				{
                    son2++; 
					printf("SON2 waited %d times\n",son2);
                    P(sem2);// wait--------------------------sem
					fp3 = fopen("attempt" , "r+");                              // **  
                    
					fscanf(fp3, "%d", &N_Att);
					if(N_Att == 0)
					{
						fclose(fp3);
						flag1 = TRUE;
                        
					}
					else
					{
                        
						printf("Poor SON_2 wants to withdraw money.\n");
                        
						fp2 = fopen("balance", "r+");                               // ** SON2 reads balance 
						fscanf(fp2,"%d", &bal2);
						printf("Poor SON_2 reads balance. Available Balance: %d \n", bal2);
						if (bal2 == 0)
						{
							fclose(fp2);
							fclose(fp3);
                           
						}
						else
						{   
                            
							sleep(rand()%5);
							fseek(fp2,0L, 0);
							bal2 -=20;
                            
							printf("Poor SON_2 write new balance: %d \n", bal2);        // ** SON2 CS new balance 
							fprintf(fp2,"%d\n", bal2);
							fclose(fp2);
                            
							printf("poor SON_2 done doing update.\n");                  
							fseek(fp3,0L, 0);
							N_Att -=1;
							fprintf(fp3, "%d\n", N_Att);
                            V(sem1); // -------------------------------------------------------- signal sem2 
                            V(sem2); 
							fclose(fp3);       
                                                                   // ** SON2 CS ends 
						}
					}  
				
                    
				}
			}
			else
			{
				//Now parent process waits for the child processes to finish
				pid = wait(&status);
				printf("Process(pid = %d) exited with the status %d. \n", pid, status);
			
				pid = wait(&status);
				printf("Process(pid = %d) exited with the status %d. \n", pid, status);
			
				pid = wait(&status);
				printf("Process(pid = %d) exited with the status %d. \n", pid, status);
                SEM_OFF(sem1); //-------------------------- turn off semaphores 
                SEM_OFF(sem2); 

                
			}
			exit(0);
		}
		exit(0);
	}
	exit(0);    
}



int SEM_ON(int sem_id, int sem_val)
{
    union semun sem_union; 
    sem_union.val = sem_val; 
    return semctl(sem_id, 0, SETVAL, sem_union); 
}

// semmaphore delete 
int SEM_OFF(int sem_id)
{
    return semctl(sem_id, 0, IPC_RMID); 
}

// wait
int P(int sem_id)
{
    struct sembuf sem_buf; 

    sem_buf.sem_num = 0; 
    sem_buf.sem_op = -1; 
    sem_buf.sem_flg = SEM_UNDO; 
    return semop(sem_id, &sem_buf, 1); 
}

// increment semaphore

int V(int sem_id)
{
    struct sembuf sem_buf; 

    sem_buf.sem_num = 0; 
    sem_buf.sem_op = 1; 
    sem_buf.sem_flg = SEM_UNDO; 
    return semop(sem_id, &sem_buf, 1); 
}
