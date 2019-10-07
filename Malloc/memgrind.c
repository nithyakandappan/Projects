#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "mymalloc.h"

void A();
void B();
void C();
void D();
void E();
void F();

int main(int argc, char** argv) {
	int i;
	struct timeval tv1;
	struct timeval tv2;
	int j;
	time_t time_taken;

	//Running A
	time_t A_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		A();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		A_run_times[i] = time_taken;
	}
	time_t sumA = 0;
	for (j=0;j<100;j++){
		sumA = sumA + A_run_times[j];
	}
	time_t mean_time_A = sumA/100;
	printf("mean run time for A (in 100 runs) : %ld (microseconds)\n",mean_time_A);

	//Running B
	time_t B_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		B();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		B_run_times[i] = time_taken;
	}
	time_t sumB = 0;
	for (j=0;j<100;j++){
		sumB = sumB + B_run_times[j];
	}
	time_t mean_time_B = sumB/100;
	printf("\nmean run time for B (in 100 runs) : %ld (microseconds)\n",mean_time_B);

	//Running C
	time_t C_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		C();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		C_run_times[i] = time_taken;
	}
	time_t sumC = 0;
	for (j=0;j<100;j++){
		sumC = sumC + C_run_times[j];
	}
	time_t mean_time_C = sumC/100;
	printf("\nmean run time for C (in 100 runs) : %ld (microseconds)\n",mean_time_C);

	//Running D
	time_t D_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		D();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		D_run_times[i] = time_taken;
	}
	time_t sumD = 0;
	for (j=0;j<100;j++){
		sumD = sumD + D_run_times[j];
	}
	time_t mean_time_D = sumD/100;
	printf("\nmean run time for D (in 100 runs) : %ld (microseconds)\n",mean_time_D);

	//Running E
	time_t E_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		E();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		E_run_times[i] = time_taken;
	}
	time_t sumE = 0;
	for (j=0;j<100;j++){
		sumE = sumE + E_run_times[j];
	}
	time_t mean_time_E = sumE/100;
	printf("\nmean run time for E (in 100 runs) : %ld (microseconds)\n",mean_time_E);

	//Running F

	time_t F_run_times[100];
	for (i=0;i<100;i++){
		gettimeofday(&tv1,NULL);
		F();
		gettimeofday(&tv2,NULL);
		// 1 second [.tv_sec] = 1,000,000 microsecond [.tv_usec]
		time_taken = (tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec;
		F_run_times[i] = time_taken;
	}
	time_t sumF = 0;
	for (j=0;j<100;j++){
		sumF = sumF + F_run_times[j];
	}
	time_t mean_time_F = sumF/100;
	printf("\nmean run time for F (in 100 runs) : %ld (microseconds)\n",mean_time_F);

}

void A() {
	int i=0;
	while (i<150) {
		char * p=(char *)malloc(1*sizeof(char));
		free(p);
		i = i+ 1;
	}
}

void B() {
	int i;
	char * c[50];
	int j;
	for (j=0;j<3;j++)
	{
		i = 0;
		while (i<50){
			c[i] =(char *)malloc(1*sizeof(char));
			i++;
		}
		i = 0;
		while (i<50){
			free(c[i]);
			i++;
		}
	}
}

void C() {
	int mallocCounter=0;
	int freeCounter=0;
	char * c[50];
	while (mallocCounter<50) {
		int num = (rand() % (2 - 1 + 1)) + 1;
		if (num==1) { // malloc
			c[mallocCounter]=malloc(1*sizeof(char));
			mallocCounter=mallocCounter+1;
		}
		else {	// free
			if (freeCounter<mallocCounter) {
				free(c[freeCounter]);
				freeCounter=freeCounter+1;
			}
		}
	}
	while(freeCounter<50) {
		free(c[freeCounter]);
		freeCounter=freeCounter+1;
	}
}

void D() {
	int mallocCounter=0;
	int freeCounter=0;
	char * c[50];
	//malloc 50 blocks of data each with random size between 1 and 64 bytes
	//total allocated size will never exceed the total memory capacity (4096)
	//since each metadata takes 3 bytes, 50*(64+3) = 3350 < 4096
	while (mallocCounter<50) {
		int num = (rand() % (2 - 1 + 1)) + 1;
		if (num==1) // malloc
		{
			int size=(rand() % (64 - 1 + 1)) + 1;
			c[mallocCounter]=(char *)malloc(size*sizeof(char));
			mallocCounter=mallocCounter+1;
		}
             	else // free
		{
			if (freeCounter < mallocCounter) {
				free(c[freeCounter]);
				freeCounter=freeCounter+1;
			}
		}
	}
	while(freeCounter<50) {
		free(c[freeCounter]);
		freeCounter=freeCounter+1;
	}
}

void E() {
	int i = 0;
	int j = 0;
	char * c[94];
	//malloc 40-byte blocks 95 times
	while (i<4096 && j < 95){
  		char * p = (char*)malloc(40*sizeof(char));
  		c[j] = p;
  		i = i+3+40;
  		j++;
	}
	int k = 0;
	char * d[36];
	int l = 0;
	//free 2-blocks at a time followed by mallocing 80-bytes, until first 32 blocks are done
	//there should be 16 83-bytes blocks
	while (k<32){
  		free(c[k]);
  		free(c[k+1]);
  		d[l] = (char *)malloc(80);
  		k=k+2;
		l++;
	}
	//free 3-blocks at a time followed by mallocing 120-bytes, until 63 blocks are done
	//there should be 21 120-byte blocks
	while (k<95){
		free(c[k]);
		free(c[k+1]);
		free(c[k+2]);
		d[l] = (char *)malloc(120);
		k= k+3;
		l++;
	}
	//free 16 plus 21 = 37 blocks
	int m;
	for (m=0;m<37;m++){
		free(d[m]);
	}
}
//Prints out 0 error statements
void F() {
	int i;
	char * p = (char *)malloc(7);
	for (i=0;i<100;i++){
  		malloc(7);
	}

	int j;
	for (j=0;j<46;j++){
  		free(p);
  		p=p+20;
	}

	int k;
	char *q = (char *)malloc(6);
	for (k=0;k<45;k++){
  		int num = (rand() % (6 - 4 + 1)) + 4;
  		malloc(num);
	}

	int l;
	for (l=0;l<101;l++){
  		free(q);
  		q=q+10;
	}
}
