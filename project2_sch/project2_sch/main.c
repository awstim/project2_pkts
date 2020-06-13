#include <stdio.h>

typedef struct
{
	int PktID;
	int Time;
	char* Sadd;
	int Sport;
	char* Dadd;
	int Dport;
	int length;
	int weight;

} Stream;

void init_Stream(Stream *stream) {


	stream->Sadd = (char*)malloc(15 * sizeof(char)); //255.255.255.255
	stream->Dadd = (char*)malloc(15 * sizeof(char));

}
int read_file(FILE *fp, Stream *stream) {
	fscanf(fp, "%d %d %s %d %d %d %d %d", stream->PktID, stream->Time, stream->Sadd,
		stream->Sport, stream->Dadd, stream->Dport,
		stream->length, stream->weight);
	printf("\n%d %d %s %d %d %d %d %d", stream->PktID, stream->Time, stream->Sadd,
		stream->Sport, stream->Dadd, stream->Dport,
		stream->length, stream->weight);

		

}

int main() {

	FILE *fp;
	Stream *stream;
	int file = 0;
	stream = (Stream*)malloc(sizeof(Stream));
	init_Stream(stream);
	fp = fopen("1_DRR_q2_in.txt", "r");
	file = read_file(fp, stream);
	fclose(fp);
	getchar();


	return 0;
}