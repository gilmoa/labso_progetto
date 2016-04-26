#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

//USO: ./a.out x input.txt

int countlines(char *filename);
int *fileToArray(char *filename, int nr);

bool presente = false;

void main(int argc, char *argv[])
{
	int zero = 0;
  	int nr = countlines(argv[2]);
	int *numberArray = fileToArray(argv[2], nr);

	int n = atoi(argv[1]);
	splitsearchtarocco(numberArray, zero, nr-1, n);


	if(presente == false)
	{
		printf("non trovato: 0\n");
	}
}


int countlines(char *filename)
{                                    
	FILE *fp = fopen(filename,"r");
	int ch=0;
	int lines=0;

	while(!feof(fp))
	{
		ch = fgetc(fp);
  		if(ch == '\n')
  		{
    		lines++;
  		}
	}
  	fclose(fp);
	return lines;
}

int *fileToArray(char *filename, int nr)
{
	FILE *fp = fopen(filename, "r");

    //read file into array
    int numberArray[nr];
    int i;

    for (i = 0; i < nr; i++)
    {
        fscanf(fp, "%d", &numberArray[i]);
    }

    for (i = 0; i < nr; i++)
    {
        printf("Number is: %d\n", numberArray[i]);
    }

	return numberArray;
}

void splitsearch(int *numberArray, int start, int end, int n)
{

	if(start==end)
	{		
		if(n==numberArray[start])
		{
			presente = true;
			printf("Sto guardando start: %i end: %i\n", start, end);
			
			printf("RIGA TROVATO: %i\n", start);
		}
	}	
	else{
		
		int mid = (start+end)/2;

		if(fork()==0)
		{
			printf("Sono il figlio e start: %i mid: %i end: %i\n", start, mid, end);
			splitsearch(numberArray, start, mid, n);
		}
		else{
			printf("Sono il padre e start: %i mid: %i end: %i\n", start, mid+1, end);
			splitsearch(numberArray, mid+1, end, n);
		}
	}
}

void splitsearchtarocco(int *numberArray, int start, int end, int n)
{

	if(start==end)
	{		
		if(n==numberArray[start])
		{
			presente = true;
			printf("Sto guardando start: %i end: %i\n", start, end);
			
			printf("RIGA TROVATO: %i\n", start);
		}
	}	
	else{
		
		int mid = (start+end)/2;
			splitsearchtarocco(numberArray, start, mid, n);
			splitsearchtarocco(numberArray, mid+1, end, n);
	}
}








