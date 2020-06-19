#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#pragma warning(disable:4996)
# define MAX_LINE 200 //maximal charts per line in the input of new line
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
	int Weight_ctr; //cntdown of Weight till finishing sending all pckts
	struct Flow *nxtflw; //poineting to next flow 
	int num_of_pckts;
}Flow;

Stream *Create_new_stream(char *line, int wgt)//creating a new stream
{
	Stream *strm = (Stream*)malloc(sizeof(Stream));

	if (strm == NULL) {//check if malloc failed
		printf("Error: memory allocation failed\n");
		exit(1);
	}
	else {
		int i, j = 0, ctr1 = 0;
		char subString[8][FLOW_NAME];//sub lists of the splited line

		strm->Weight = wgt;//need to changed to the size we will get from the input name
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

Flow* Create_new_flow(Stream *strm)//creating a new flow
{
	Flow *flw = (Flow*)malloc(sizeof(Flow));
	if (flw == NULL) {//check if malloc failed
		printf("Error: memory allocation failed\n");
		exit(1);
	}
	else {
		strcpy(flw->flwname, strm->flwname);
		flw->Weight = strm->Weight;
		flw->Weight_ctr = strm->Weight;
		flw->nxtflw = NULL;
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
		return flw_head = Create_new_flow(strm);
	}
	do { 	//there is at least one flow and it has at least one stream in it
		comp = strcmp(flw_ptr->flwname, strm->flwname);
		if (comp == 0) {//if the new strm has the same flow it will combine to the previous strm in the same flow and the flow packets will increase+1
			if (flw_ptr->flwstrm == NULL) {//if the flow is empty
				flw_ptr->flwstrm = strm;
				flw_ptr->num_of_pckts++;
				strm->Weight = flw_ptr->Weight;
				return flw_head;
			}
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
	prev_flow->nxtflw = Create_new_flow(strm);
	return flw_head;
}

void Printing_flows(Flow *flw)//**EXTRA** bakara1 - to delete
{
	Stream *strm_ptr;

	while (flw != NULL)
	{
		printf("name=%s, Weight=%d, credit=%d:\n", flw->flwname, flw->Weight, flw->Weight_ctr);
		strm_ptr = flw->flwstrm;
		while (strm_ptr != NULL) {//will step forword to the last current strm in the flow
			printf("ID=%d L = %d\n", strm_ptr->pktID, strm_ptr->Length);
			strm_ptr = strm_ptr->nxtstrm;
		}
		flw = flw->nxtflw;
	}
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
}

void Print_flow(Flow *flw)//**EXTRA** bakara2 -to delete  
{
	Stream *strm_ptr;
	printf("name=%s, Whigt=%d:\n", flw->flwname, flw->Weight);
	strm_ptr = flw->flwstrm;
	while (strm_ptr != NULL) {//will step forword to the last current strm in the flow
		printf("ID=%d\n", strm_ptr->pktID);
		strm_ptr = strm_ptr->nxtstrm;
	}
}

Flow *Step_current_flow(Flow *flw_head, Flow *crt_flw)
{
	if (crt_flw->nxtflw != NULL) {// crt_flw isnt the last one in flows list
		return crt_flw = crt_flw->nxtflw;
	}
	else { //crt_flw is the last one in flows list
		return flw_head;
	}
}

Flow *WRR_algo(Flow *flw_head, Flow *crt_flw, long *ptr_ToTtime, FILE *fp2)
{
	Stream *strm_ptr;

	if (crt_flw == NULL || crt_flw->num_of_pckts == 0 || crt_flw->flwstrm == NULL) {//if crt_flw is empty or null
		crt_flw = Step_current_flow(flw_head, crt_flw);
		return crt_flw;
	}

	while (crt_flw != NULL && crt_flw->num_of_pckts != 0 && crt_flw->flwstrm != NULL && crt_flw->Weight_ctr > 0)//**MOST BUGS ARE HERE!!**
	{
		strm_ptr = crt_flw->flwstrm;
		fprintf(fp2, "%ld: %d\n", *ptr_ToTtime, strm_ptr->pktID);//in this line we write to file the ID of releasing pckts
		printf("%ld: %d\n", *ptr_ToTtime, strm_ptr->pktID);//in this line we print to cmd the ID of releasing pckts
		*ptr_ToTtime += strm_ptr->Length;

		crt_flw->num_of_pckts--;//how many pckts left in flow
		crt_flw->Weight_ctr--;// how many pckst still allowd to send
		crt_flw->flwstrm = strm_ptr->nxtstrm;
		free(strm_ptr);
	}
	if (crt_flw->Weight_ctr == 0) {
		crt_flw->Weight_ctr = crt_flw->Weight;
		crt_flw = Step_current_flow(flw_head, crt_flw);
	}

	return crt_flw;
}

Flow *DRR_ALGO(Flow *flw_head, Flow *crt_flw, long *ptr_ToTtime, FILE *fp2)
{
	Stream *strm_ptr;

	if (crt_flw == NULL || crt_flw->num_of_pckts == 0 || crt_flw->flwstrm == NULL) {//if crt_flw is empty or null
		crt_flw = Step_current_flow(flw_head, crt_flw);
		return crt_flw;
	}

	strm_ptr = crt_flw->flwstrm;
	if (crt_flw->Weight_ctr < strm_ptr->Length) {//if the flow is not empty but not have enought credit to send crnt packt
		crt_flw->Weight_ctr += crt_flw->Weight;
		return crt_flw = Step_current_flow(flw_head, crt_flw);
	}

	//if current flow isnt empty - check if has enought credit to send packets
	while (crt_flw != NULL && crt_flw->num_of_pckts > 0 && crt_flw->flwstrm != NULL && crt_flw->Weight_ctr >= strm_ptr->Length)//**MOST BUGS ARE HERE!!**
	{
		fprintf(fp2, "%ld: %d\n", *ptr_ToTtime, strm_ptr->pktID);//in this line we write to file the ID of releasing pckts
		printf("%ld: %d\n", *ptr_ToTtime, strm_ptr->pktID);//**NEED TO BE DELETED IN THE END** in this line we print to cmd the ID of releasing pckts
		*ptr_ToTtime += strm_ptr->Length;

		crt_flw->num_of_pckts--;//how many pckts left in flow
		crt_flw->Weight_ctr -= strm_ptr->Length;// reduce the credit from flow
		crt_flw->flwstrm = strm_ptr->nxtstrm;//head of streams step one step forwords no next stream in current flow

		free(strm_ptr);
		strm_ptr = crt_flw->flwstrm;
	}
	crt_flw->Weight_ctr = crt_flw->Weight;//restarting the credit till next time
	return crt_flw = Step_current_flow(flw_head, crt_flw);
}

int If_All_Flows_Are_Empty(Flow *flw)//getting ptr of head flws list and check if all flws are empty
{
	while (flw != NULL)
	{
		if (flw->flwstrm == NULL)
			flw = flw->nxtflw;
		else
			return 0;
	}
	return  1;//if all flows are empty
}

int main()
{
	FILE *fp1, *fp2;
	Flow *flw_head = NULL, *crt_flw = NULL;
	Stream *strm = NULL;
	char line[MAX_LINE], algo[] = "DRR", file_name = "EXAMPLE";
	long crt_Time = 0, cnt = 0, ToTtime = 0, *ptr_ToTtime;
	int flag1 = 0, wgt = 10;

	//printf("Doesn't Working\n");

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

	ptr_ToTtime = &ToTtime;
	while (fgets(line, MAX_LINE, fp1) != NULL) {//reading line by line till EOF
		if (*line == '\n')//if there is empty line or an extre '\n' ENTER
		{
			printf("This is an empty line\n");
			continue;
		}

		strm = Create_new_stream(line, wgt);
		if ((crt_Time != strm->Time) && (ToTtime <= crt_Time && crt_flw != NULL)) {
			crt_flw = DRR_ALGO(flw_head, crt_flw, ptr_ToTtime, fp2);
			while (!strcmp(algo, "DRR") && ToTtime <= crt_Time && !If_All_Flows_Are_Empty(flw_head)) {
				crt_flw = DRR_ALGO(flw_head, crt_flw, ptr_ToTtime, fp2);
			}
		}
		flw_head = Stream_into_flow(strm, flw_head); //sorting and placing each new stream to right flow
		crt_Time = strm->Time;
		cnt += strm->Length;//counting the total time it will take to send all pckts
		if (flag1 == 0) {
			crt_flw = flw_head;
			ToTtime = crt_Time;
			flag1 = 1;
		}
	}
	//Printing_flows(flw_head);
	while (crt_flw != NULL && ToTtime < cnt) {//also finishing reading from file need WRR algo
		crt_flw = DRR_ALGO(flw_head, crt_flw, ptr_ToTtime, fp2);
	}
	fclose(fp1);
	fclose(fp2);
	printf("End Of Program!\n");//**NEED TO BE DELETED IN THE END**
	return 0;
}