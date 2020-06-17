#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning(disable:4996)
# define MAX_LINE 200 //maximal charts per line in the input of new line
# define WEIGHT_DEFAULT 10
# define FLOW_NAME 50

typedef struct {
	long pktID;
	long Time;
	char Sadd[16]; //255.255.255.255
	unsigned short Sport;
	char Dadd[16]; //255.255.255.255
	unsigned short Dport;
	int Length;//shuld be 64 - 16384 Bytes - need to changed
	int Weight;
	struct Stream *nxtstrm; //poineting to the next stream in the current flow
	char flwname[FLOW_NAME]; //describe the flow name of the packet
} Stream;

typedef struct {
	char flwname[FLOW_NAME]; //describe the flow name of the packet
	struct Stream *flwstrm; //poineting to head of streams in current flow
	int Weight; //Weight of the specific flow
	struct Flow *nxtflw; //poineting to next flow 
	struct Flow *prvflw; //poineting to previeus flow
	int num_of_pckts;
}Flow;

Stream *Create_new_stream(char *line)//creating a new stream
{
	Stream *strm = (Stream*)malloc(sizeof(Stream));

	if (strm == NULL) {//check if malloc failed
		printf("Error: memory allocation failed\n");
		exit(1);
	}
	else {
		int i, j = 0, ctr1 = 0;
		char subString[8][FLOW_NAME];//sub lists of the splited line

		strm->Weight = WEIGHT_DEFAULT;//need to changed to the size we will get from the input name
		for (i = 0; i < (strlen(line)); i++)//running the loop till '\n' and not till '\0' of each single line
		{
			if (line[i] == ' ' || line[i] == '\0' || line[i] == '\n')// if space or NULL found, assign NULL into subString[ctr1]
			{
				subString[ctr1][j] = '\0';//end of the new sub word
				ctr1++;  //for next word
				j = 0;    //for next word, init index to 0
			}
			else {
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
		strm->nxtstrm = NULL;
		{//creating flwname by Concatenate subStrings 2-5 (Sadd, Sport, Dadd, Dport)
			strcat(subString[2], ">");
			strcat(subString[2], subString[3]);
			strcat(subString[2], ">");
			strcat(subString[2], subString[4]);
			strcat(subString[2], ">");
			strcat(subString[2], subString[5]);
			strcpy(strm->flwname, subString[2]);//the flwname in string
		}
		if (ctr1 == 8)// if Weight is not the DEFAULT
			strm->Weight = atoi(subString[7]);// optional - need to set condition

		//A sanity check printing OUTPUT to CMD
		//printf("pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d, flwname=%s|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight, strm->flwname);
		return strm;
	}
}

Flow* Create_new_flow(Stream *strm, Flow* prvflw)//creating a new flow
{
	Flow *flw = (Flow*)malloc(sizeof(Flow));
	if (flw == NULL) {//check if malloc failed
		printf("Error: memory allocation failed\n");
		exit(1);
	}
	else {
		strcpy(flw->flwname, strm->flwname);
		flw->Weight = strm->Weight;
		flw->nxtflw = NULL;
		flw->prvflw = prvflw;
		flw->flwstrm = strm;
		flw->num_of_pckts = 1;
	}
	return flw;
}

Flow *Stream_into_flow(Stream *strm, Flow *flw_head)//
{
	Stream *strm_ptr;
	Flow *flw_ptr = flw_head, *prev_flow;
	int comp;
	if (flw_head == NULL) {
		return flw_head = Create_new_flow(strm, NULL);
	}
	do { 	//there is at least one flow and it has at least one stream in it
		comp = strcmp(flw_ptr->flwname, strm->flwname);
		if (comp == 0) {//if the new strm has the same flow it will combine to the previous strm in the same flow and the flow packets will increase+1
			strm_ptr = flw_ptr->flwstrm;
			while (strm_ptr->nxtstrm != NULL) {//will step forword to the last current strm in the flow
				strm_ptr = strm_ptr->nxtstrm;
			}
			strm_ptr->nxtstrm = strm;
			flw_ptr->num_of_pckts++;
			strm->Weight = flw_ptr->Weight;
			return flw_head;
		}
		prev_flow = flw_ptr;
		flw_ptr = flw_ptr->nxtflw;
	} while (flw_ptr != NULL);
	//finish to run all existing flows but no one match to strm->flwname so will create a new flow and 
	prev_flow->nxtflw = Create_new_flow(strm, prev_flow);
	return flw_head;
}

void Printing_flows(Flow *flw)
{
	Stream *strm_ptr;

	while (flw != NULL)
	{
		printf("name=%s, Whigt=%d:\n", flw->flwname, flw->Weight);
		strm_ptr = flw->flwstrm;
		while (strm_ptr != NULL) {//will step forword to the last current strm in the flow
			printf("ID=%d\n", strm_ptr->pktID);
			strm_ptr = strm_ptr->nxtstrm;
		}
		flw = flw->nxtflw;
	}
}

Flow *Remove_flow_from_flows_list(Flow *flw_head, Flow *crt_flw)
{
	Flow *tmp_flw;
	if (crt_flw == flw_head) {//if the head isthe empty folw
		if (flw_head->nxtflw == NULL) {//if head is empty and single in the list - return NULL
			free(flw_head);//or free(crt_flw)?
			free(crt_flw);
			return NULL;
		}
		else if (flw_head->nxtflw != NULL) {//if head is empty and not single in the list - return nextflw is head
			//crt_flw = flw_head;// Q:which pointe do I free here?
			flw_head = flw_head->nxtflw;
			return flw_head;
		}
	}
	else {//if the empty flow isnt the head
		crt_flw->prvflw = crt_flw->nxtflw;
		tmp_flw = crt_flw->nxtflw;
		free(crt_flw);
		return tmp_flw;
	}
}

int WRR_algo(Flow *crt_flw)
{
	Flow *flw_ptr = NULL;
	Stream *strm_ptr;
	int Weight_ctr = 0, len_ctr = 0;

	while (crt_flw != NULL || crt_flw->num_of_pckts != 0 || crt_flw->flwstrm != NULL || Weight_ctr != 0)
	{
		Weight_ctr = crt_flw->Weight;
		strm_ptr = crt_flw->flwstrm;
		len_ctr += strm_ptr->Length;
		printf("pkID=%d, length=%d\n", strm_ptr->pktID, strm_ptr->Length);//in this line we print/write to file the ID of releasing pckts

		crt_flw->num_of_pckts--;
		Weight_ctr--;
		crt_flw->flwstrm = strm_ptr->nxtstrm;
		free(strm_ptr);
	}
	return len_ctr;
}

Flow *Return_current_flow(Flow *flw_head, Flow *crt_flw)
{
	printf("HERE!\n");
	if (flw_head == NULL) {//no flows at all
		return NULL;
	}
	if (flw_head != NULL && flw_head->nxtflw == NULL) {//only head flow
		return flw_head;
	}
	else {//more than one flow in list
		if (crt_flw->nxtflw != NULL) {// crt_flw isnt the last one in flows list
			return crt_flw = crt_flw->nxtflw;
		}
		else { //crt_flw is the last one in flows list
			return flw_head;
		}
	}
}

/*void pass1(char *line, Stream *strm)//getting each line and decompose each part to the right argument
{
	int i, j = 0, ctr1 = 0;
	char subString[8][50];//sub lists of the splited line

	for (i = 0; i < (strlen(line)); i++)//running the loop till '\n' and not till '\0' of each single line
	{
		if (line[i] == ' ' || line[i] == '\0' || line[i] == '\n')// if space or NULL found, assign NULL into subString[ctr1]
		{
			subString[ctr1][j] = '\0';//end of the new sub word
			ctr1++;  //for next word
			j = 0;    //for next word, init index to 0
		}
		else {
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
	strm->nxtstrm = NULL;
	if (ctr1 == 8)// if Weight is not the DEFAULT
		strm->Weight = atoi(subString[7]);// optional - need to set condition

	//A sanity check printing OUTPUT to CMD
	printf("pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight);

	//writing to the OUTPUT file (fp2)
	//fprintf(fp2, "pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight);
}

void pass2(FILE *fp2, Stream *strm)//printing each strm to the OUTPUT file
{
	fprintf(fp2, "pktID=%ld, Time=%ld, Sadd=%s, Sport=%u, Dadd=%s, Dport=%u, Length=%d, Weight=%d|\n", strm->pktID, strm->Time, strm->Sadd, strm->Sport, strm->Dadd, strm->Dport, strm->Length, strm->Weight);
}*/

int main()
{
	FILE *fp1, *fp2;
	Flow *flw_head = NULL, *crt_flw = NULL;
	Stream *strm = NULL;
	char line[MAX_LINE];
	long crt_Time = 0, cntdwn = 0;
	long long ToTtime = 0;
	int flag = 0;

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

	while (fgets(line, MAX_LINE, fp1) != NULL) {//reading line by line till EOF
		if (*line == '\n')//if there is empty line or an extre '\n' ENTER
		{
			printf("This is an empty line\n");
			continue;
		}

		strm = Create_new_stream(line);
		if ((crt_Time != strm->Time) && (crt_Time > cntdwn)) {
			if (flag == 0)
				crt_flw = Return_current_flow(flw_head, crt_flw);
			if (crt_flw != NULL) {
				cntdwn = WRR_algo(crt_flw);//printf("WRR\n");
				if (crt_flw->flwstrm == NULL) {//if this flow is EMPTY - remove this flow from flows list
					crt_flw = Remove_flow_from_flows_list(flw_head, crt_flw);
					flag = 1;
				}
			}
		}
		flw_head = Stream_into_flow(strm, flw_head); //sorting and placing each new stream to right flow
		crt_Time = strm->Time;

		//pass2(fp2, strm);
	}
	//also here need WRR algo
	if (1) {
		if (flag == 0)
			crt_flw = Return_current_flow(flw_head, crt_flw);
		if (crt_flw != NULL) {
			cntdwn = WRR_algo(crt_flw);//printf("WRR\n");
			if (crt_flw->flwstrm == NULL) {//if this flow is EMPTY - remove this flow from flows list
				crt_flw = Remove_flow_from_flows_list(flw_head, crt_flw);
				flag = 1;
			}
		}
	}

	//Printing_flows(flw_head);


	fclose(fp1);
	fclose(fp2);

	printf("End Of Program!\n");
	return 0;
}