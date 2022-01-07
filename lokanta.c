#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

#define TABLE_AMOUNT 10
#define CHAIRS_EACH_TABLE 8
float open_table_price = 99.9;
float order_repeat_price = 19.9;
#define RICE_PRICE 20



typedef struct Philosopher{
	int id;
	int eatCount;
	int eatTime;
	int thinkTime;
	
	pthread_t philo_thread;
	
	
}Philosopher;

typedef struct Table{
	int id;
	int orderCount;
	int openTableCount;
	int empty_chair_amount;
	int pList[CHAIRS_EACH_TABLE];
	int riceAmount;
	pthread_mutex_t lockDining;
}Table; 

sem_t lock_restaurant;
Table* tableList;
Philosopher* philosopherList;
int filozofSayi;
int restaurant_capacity;

pthread_mutex_t lockTable;

void dining(int pthread_id, int table_id){
	
	while(1){
		pthread_mutex_lock(&tableList[table_id].lockDining);
		if(tableList[table_id].riceAmount==0){
			tableList[table_id].orderCount++;
			tableList[table_id].riceAmount+=2000;
		}
		philosopherList[pthread_id].eatCount++;
		printf("%d\n",tableList[table_id].riceAmount);
		tableList[table_id].riceAmount-=100;
		sleep(philosopherList[pthread_id].eatTime/100);
		
		
	
		pthread_mutex_unlock(&tableList[table_id].lockDining);
		sleep(philosopherList[pthread_id].thinkTime/100);
		
		//masadaki bütün insanlar doymuşsa çık
		int i;
		int count = 0;
		for(i=0;i<CHAIRS_EACH_TABLE;i++){
			if(philosopherList[tableList[table_id].pList[i]].eatCount>0){
				count++;
			}
		}
		
		if(count==8){
			break;
		}
		
	
	}
	pthread_mutex_destroy(&tableList[table_id].lockDining);
	
}

void printReceipt(){
	int i;
	for(i=0; i<TABLE_AMOUNT; i++){
		printf("Masa %d\n",i);
		printf("Masa açma: %.2f\n",open_table_price);
		printf("Masa tazeleme: %.2f\n",(tableList[i].orderCount*order_repeat_price));
		printf("Pirinç Fiyat: %d\n",(tableList[i].orderCount*2*RICE_PRICE + RICE_PRICE));
		printf("Toplam: %.2f\n", open_table_price+(tableList[i].orderCount*order_repeat_price)+(tableList[i].orderCount*2*RICE_PRICE + RICE_PRICE));
		printf("-------------\n");
	}
	
}

void pToT(int pthread_id, int table_id){
	int i;
	for(i=0; i<CHAIRS_EACH_TABLE; i++){
		if(tableList[table_id].pList[i]==-1){
			tableList[table_id].pList[i] = pthread_id;
		}
	}
}

void findTable(int pthread_id){
	
	int i;
	for(i=0; i<TABLE_AMOUNT; i++){
		pthread_mutex_lock(&lockTable);
		if(tableList[i].empty_chair_amount!=0){
			tableList[i].empty_chair_amount--;
			pToT(pthread_id,i);
			
			//printf("ben %d masadayım",i);
			pthread_mutex_unlock(&lockTable);
			
			dining(pthread_id,i);
			
		}
		
		
		pthread_mutex_destroy(&lockTable);
	}
	
	
}

void *openRestaurant(void * i){
	
	sem_wait(&lock_restaurant);
	int *pthread_id = i;
	findTable(*pthread_id);
	
	
	
	
	
	
	sem_post(&lock_restaurant);
	return NULL;
}



Table createTable(int id){
	Table table;
	table.id = id;
	table.orderCount = 0;
	table.openTableCount = 1;
	table.empty_chair_amount = CHAIRS_EACH_TABLE;
	table.riceAmount = 2000; //2000gr
	int i;
	for(i=0;i<CHAIRS_EACH_TABLE;i++){
		table.pList[i]=-1;
	}
	pthread_mutex_init(&table.lockDining,NULL);
	return table;
	
}

Philosopher createPhilosopher(int id){
	
	Philosopher philosopher;
	philosopher.id = id;
	philosopher.eatCount = 0;
	philosopher.eatTime = (rand() % 5)+1;
	philosopher.thinkTime = (rand() % 5)+1;
	return philosopher;
}

int main(){
	srand(time(NULL));
	void* status;
	int grupSayi;
	printf("Filozof grup sayısını girin:");
	scanf("%d",&grupSayi);
	
	filozofSayi = grupSayi*CHAIRS_EACH_TABLE;
	restaurant_capacity = TABLE_AMOUNT*CHAIRS_EACH_TABLE;
	sem_init(&lock_restaurant,0,restaurant_capacity);
	pthread_mutex_init(&lockTable,NULL);
	
	tableList = (Table*)calloc(TABLE_AMOUNT,sizeof(Table));
	philosopherList = (Philosopher*)calloc(filozofSayi,sizeof(Philosopher));
	
	int i;
	for(i = 0; i<TABLE_AMOUNT; i++){
		tableList[i] = createTable(i);
		       
	}
	
	
	for(i = 0; i<filozofSayi; i++){
		philosopherList[i] = createPhilosopher(i);
		
		if (pthread_create(&philosopherList[i].philo_thread, NULL, openRestaurant, &i) != 0) {
    			perror("thread olusturulamadi");
   			exit(1);
  		}
  		if (pthread_join(philosopherList[i].philo_thread, &status) != 0) { perror("pthread_join"); exit(1); }     
	}
	
	
	printReceipt();
	

	sem_destroy(&lock_restaurant);
	
	
	
	return 0;
}
