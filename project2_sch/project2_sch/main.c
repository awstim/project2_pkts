#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning(disable:4996)
# define MAX_LINE 500 //maximal charts per line in the input of new line
# define WEIGHT_DEFAULT 10

typedef struct
{
	long pktID;
	long Time;
	char Sadd[16]; //255.255.255.255
	unsigned short Sport;
	char Dadd[16]; //255.255.255.255
	unsigned short Dport;
	int Length;//shuld be 64 - 16384 Bytes - need to changed
	int Weight;
} Stream;

void pass1(FILE *fp1, FILE *fp2, Stream *strm)//reading each line and decompose each part to the right argument
{
	int i, j, ctr1 = 0;
	char line[MAX_LINE], subString[8][50];//sub lists of the splited line

	//reading each line to parameters//
	while (fgets(line, MAX_LINE, fp1) != NULL)//reading line by line till EOF
	{
		strm->Weight = WEIGHT_DEFAULT;//need to changed to the size we will get from the input name
		ctr1 = 0;
		j = 0;

		if (*line == '\n')//if there is empty line or an extre '\n' ENTER
		{
			printf("This is an empty line\n");
			continue;
		}
		for (i = 0; i < (strlen(line)); i++)//running the loop till '\n' and not till '\0' of each single line
		{
			if (line[i] == ' ' || line[i] == '\0' || line[i] == '\n')// if space or NULL found, assign NULL into subString[ctr1]
			{
				subString[ctr1][j] = '\0';//end of the new sub word
				ctr1++;  //for next word
				j = 0;    //for next word, init index to 0
			}
			else
			{
				subString[ctr1][j] = line[i];
				j++;
			}
		}
		//passing all the subStrings to strm in the right values and formats
		strm->pktID = atol(subString[0]);
		strm->Time = atol(subString[1]);
		strcpy(strm->Sadd, subString[2]);
		strm->Sport = (unsigned short)strtoul(subString[3], NULL, 10);
		strcpy(strm->Dadd, subString[4]);
		strm->Dport = (unsigned short)strtoul(subString[5], NULL, 10);
		strm->Length = atoi(subString[6]);// should be 64-16384 Bytes only!!
		if (ctr1 == 8)// if Weight is not the DEFAULT
			strm->Weight = atoi(subString[7]);// optional - need to set condition

		//A sanity check printing OUTPUT to CMD
		printf("pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight);

		//writing to the OUTPUT file (fp2)
		fprintf(fp2,"pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight);
	}
}

int main() 
{
	FILE *fp1, *fp2;
	Stream *strm = (Stream*)malloc(sizeof(Stream));
	
	fp1 = fopen("INPUT_test.txt", "r");
	fp2 = fopen("OUTPUT_test.txt", "w");
	if (fp1 == NULL) {// Check if file exists
		printf("Can't open INPUT file\n");
		exit(1);
	}
	if (fp2 == NULL) {// Check if file exists
		printf("Can't open OUTPUT file\n");
		exit(1);
	}

	pass1(fp1, fp2, strm);//passing each input line to parameters in strm(Stream)

	fclose(fp1);
	fclose(fp2);

	printf("End Of Program!\n");
	return 0;
}