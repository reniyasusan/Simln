#include <stdio.h>
#include <math.h>
#include "lcgrand.c"
#include <stdlib.h>

#define BUSY 1
#define IDLE 0
#define Q_limit 100

int delay_required, server_status, num_in_q, num_delayed, next_event_type ;
float total_delay, total_q, total_busy, time_last_event, time_since_last_event, clock, arrival_time[Q_limit],mean_service,mean_interarrival, next_event[2];
FILE *infile, *outfile ;

void initialize();
void timing();
void arrive();
void departure();
void update_stats();
void report();
float expon(float mean);

void main(){
	infile = fopen("inmm1.txt","r");
	outfile = fopen("opmm1.txt","w");
	fscanf(infile,"%f %f %d ", &mean_interarrival,&mean_service,&delay_required);
	fprintf(outfile," mean interarrival time is %f \n mean service time is %f \n number of delay required is %d \n",mean_interarrival,mean_service,delay_required);

	initialize();

	while(num_delayed<delay_required){

		timing();
		update_stats();
		switch (next_event_type){
			case 0 : 
			arrive();
			break;
			case 1:
			departure();
			break;
			case 2:
			arrive();
			departure();
			break;

		}
	}
	report();
	fclose(infile);
	fclose(outfile);
	return ;
}

void initialize(){
	clock=0.0;
	server_status= IDLE;
	num_in_q=0;
	time_last_event=0.0;
	next_event[0]= clock+expon(mean_interarrival);
	next_event[1]= 1.0e+30;
	num_delayed=0;
	total_delay=0.0;
	total_q=0.0;
	total_busy=0.0;
	return;

}

void timing(){
	if(next_event[0]<next_event[1])
		next_event_type=0;
	else if (next_event[1]<next_event[0])
		next_event_type=1;
	else 
		next_event_type=2;
	if(next_event_type==0){
		clock= next_event[0];
	}
	else clock= next_event[1];
	return;
}

void arrive(){
	next_event[0]=clock+expon(mean_interarrival);
	if(server_status==BUSY){
		num_in_q++;
		if(num_in_q>Q_limit){
			printf("Queue limit exceeded at time %f \n",clock );
			exit(1);
		}
		else {
			arrival_time[num_in_q-1]=clock;
		}
	}
	else {
		num_delayed++;
		server_status=BUSY;
		next_event[1]=clock+expon(mean_service);
	}
	return;
}

void departure(){
	int delay;
	if(num_in_q==0){
		next_event[1]= 1.0e+30 ;
		server_status=IDLE;
	}
	else {
		num_in_q--;
		delay = clock - arrival_time[0];
		total_delay+= delay;
		num_delayed++;
		next_event[1]= clock+ expon(mean_service);
		for(int i=0;i<num_in_q;i++){
			arrival_time[i]=arrival_time[++i];
		}
	}
	return;
}

void update_stats(){
	time_since_last_event=clock- time_last_event;
	time_last_event=clock;
	total_q+=num_in_q*time_since_last_event;
	total_busy+=server_status*time_since_last_event;
	return;
}

void report(){
	float delay_estimate, avg_cust_in_q, busy_time;
	delay_estimate = total_delay/num_delayed;
	avg_cust_in_q = total_q/num_delayed;
	busy_time = total_busy/num_delayed;
	fprintf(outfile," average delay in Queue = %f \n average number of customers in Queue = %f \n proportion of time the server is busy = %f \n",delay_estimate,avg_cust_in_q,busy_time);
	fprintf(outfile,"simulation end time = %f",clock);
	return;
}

float expon( float mean){
	float rand = abs(log(lcgrand(1))*mean);
	return rand;
}
