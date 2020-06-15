#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning(disable:4996)
# define MAX_LINE 500 //maximal charts per line in the input of new line
# define WEIGHT_DEFAULT 10


typedef struct {
	int ID;
	int TOA;
	int TOS;
	int Length;
	int Weight;

}Packet;

typedef struct {
	char Sadd[16]; //255.255.255.255
	unsigned short Sport;
	char Dadd[16]; //255.255.255.255
	unsigned short Dport;
	//struct Flow *next_flow;
}Flow;

typedef struct
{
	long pktID;//stas:should be transfered inside packet
	long Time;//stas:should be transfered inside packet
	Flow *flw;
	int Length;//shuld be 64 - 16384 Bytes - need to changed //stas: and transfered inside packet
	int Weight;
	Packet *head_packet;
	Packet *tail_packet;
	Packet *current_packet;
	Packet *next_packet;
	int packets_sent;
	int packets_waiting;
	int total_packets_in_stream;
	//struct Stream *next_stream;

} Stream;

typedef struct {
	Stream *strm;
	Stream *Head;
	Stream *tail;
	Stream *next_stream;
	Stream *current_stream;
	int streams_in_queue;
	int streams_total;
	int packets_in_queue;
	int packets_total;
}Stream_Queue;


Stream *init_packet(Stream *strm) {
	
	Packet *pckt = (Packet*)malloc(sizeof(Packet));
	strm->head_packet = pckt;
	strm->tail_packet = pckt;
	strm->next_packet = NULL;
	return strm;
}
Stream *add_packet(Stream *strm, Packet *pckt) {
	strm->next_packet = pckt;
	strm->tail_packet = pckt;
	return strm;
}

Stream_Queue *Create_queue() {

	Stream_Queue *Sque = (Stream_Queue*)malloc(sizeof(Stream_Queue));
	Stream *strm = (Stream*)malloc(sizeof(Stream));
	strm->flw = NULL;
	Sque->next_stream = NULL;
	//strm->next_stream = NULL;
	Sque->strm = strm;
	Sque->Head = strm;
	Sque->tail = strm;
	return Sque;
}
Stream *Check_if_flow_exist(Stream_Queue *Sque, Stream *strm) {

	Stream *pointer = Sque->Head;
	if (pointer == NULL) return NULL;
	while (pointer != NULL) {
		if ((strcmp(strm->flw->Dadd, pointer->flw->Dadd) == 0) &&
			(strm->flw->Dport == pointer->flw->Dport) &&
			(strcmp(strm->flw->Sadd, pointer->flw->Sadd) == 0) &&
			(strm->flw->Sport == pointer->flw->Sport))
			return pointer;
		else
			pointer = Sque->next_stream;
	}
	return NULL; //return NULL if didnt find a flow
}

Stream_Queue *Add_Stream_to_queue(Stream_Queue *Sque, Stream *strm) {

	Stream *pointer;
	pointer = Check_if_flow_exist(Sque, strm); //return Stream object
	if (pointer == NULL) {
		if (Sque->Head == NULL){ //first stream in queue
			Sque->Head = strm;
			Sque->tail = strm;

	}
		else {
			//Sque->next_stream = strm; //add to the end
			Sque->tail = strm; // set new end
			

		}
}
	else { //means we have this flow already, we just add a packet
			
		pointer->tail_packet = strm->head_packet;// adds the new packet to the end

	}
	return Sque;
}







Flow *init_flow() {
	Flow *flw = (Flow*)malloc(sizeof(Flow));
	return flw;
}
Stream *init_Stream() {
	Stream *strm = (Stream*)malloc(sizeof(Stream));
	//strm->next_stream = NULL;
	return strm;
}



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
		/**/
		//passing all the subStrings to strm in the right values and formats
		strm->pktID = atol(subString[0]);
		strm->Time = atol(subString[1]);
		strcpy(strm->flw->Sadd, subString[2]);
		strm->flw->Sport = (unsigned short)strtoul(subString[3], NULL, 10);
		strcpy(strm->flw->Dadd, subString[4]);
		strm->flw->Dport = (unsigned short)strtoul(subString[5], NULL, 10);
		strm->Length = atoi(subString[6]);// should be 64-16384 Bytes only!!
		if (ctr1 == 8)// if Weight is not the DEFAULT
			strm->Weight = atoi(subString[7]);// optional - need to set condition

		//A sanity check printing OUTPUT to CMD
		printf("pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->flw->Sadd, strm->flw->Sport, strm->flw->Dadd, strm->flw->Dport, strm->Length, strm->Weight);

		//writing to the OUTPUT file (fp2)
		fprintf(fp2, "pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->flw->Sadd, strm->flw->Sport, strm->flw->Dadd, strm->flw->Dport, strm->Length, strm->Weight);
	}
}

int main()
{
	FILE *fp1, *fp2;
	//Stream *strm = (Stream*)malloc(sizeof(Stream));
	Stream *strm = init_Stream();
	Flow *flw = init_flow();
	strm->flw = flw;
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