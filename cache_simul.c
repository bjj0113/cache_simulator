#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct cacheset {
	int s;
	int b;
	int a;
	char f[50];
}cacheset;

typedef struct trace {
	char address[9];
	char rw;
	int data;
}trace;

typedef struct cache {
	int tag;
	int valid;
	int dirty;
	int time;
	int* data;
	int traceaddress;
	int traceblockoffset;
}cache;

typedef struct memory {
	int Maddress; // 주소값을 10진수로 바꿔 넣을 것임
	int Mdata;
}memory;

int* Cvt16to2(char a);
int* Cvtaddress(int trcadd[], trace trc);
int log_2(int a);
int pow_2(int a);
int findaddvalue(int a, int b, int trcadd[]);
int gettrcfileLine(char* name);
int Cvt2to10(int trcadd[]);
void getmemorydata(int adr, int blf, cache* ca, int caindex, cacheset cs, memory* M, int Msize);
void getcachedata(int adr, int blf, cache* ca, int caindex, cacheset cs, memory* M, int Msize);

int main(int argc, char* argv[]) {
	cacheset cs;

	//block1개의 크기는 4byte
	for (int i = 0; i < argc; i++) {
		char* argvcut = strtok(argv[i], "=");
		if (strcmp(argvcut, "-s") == 0)
		{
			argvcut = strtok(NULL, "=");
			cs.s = atoi(argvcut);
		}
		if (strcmp(argvcut, "-b") == 0)
		{
			argvcut = strtok(NULL, "=");
			cs.b = atoi(argvcut);
		}
		if (strcmp(argvcut, "-a") == 0)
		{
			argvcut = strtok(NULL, "=");
			cs.a = atoi(argvcut);
		}
		if (strcmp(argvcut, "-f") == 0)
		{
			argvcut = strtok(NULL, "=");
			strcpy(cs.f, argvcut);
		}
		argvcut = strtok(NULL, "=");

	}

	memory* M;
	cache* ca;

	int indexn = cs.s / (cs.b * cs.a); //총 캐시인덱스 개수
	int missn = 0;
	int hitn = 0;
	int cycle = 0;
	int trcLine = gettrcfileLine(cs.f);
	M = (memory*)calloc(trcLine, sizeof(memory));// 메모리구조체를 trc파일의 길이만큼 공간할당
	ca = (cache*)calloc((indexn * cs.a), sizeof(cache));// 캐시 크기 만큼의 공간할당
	for (int i = 0; i < indexn * cs.a; i++) {
		ca[i].data = (int*)calloc((cs.b / 4), sizeof(int));
	}

	//printf("%d\n", trcLine);

	FILE* f;
	trace trc;
	int m = 0; // memoryindex

	//trace file open
	f = fopen(cs.f, "r");
	if (f == NULL)
	{
		printf("trace file open error!!!");
		return 0;
	}
	int Msize = 0;

	while (!feof(f)) {
		//trace의 값 한줄 받아오기.
		fscanf(f, "%s %c ", trc.address, &trc.rw);
		if (trc.rw == 'W')
		{
			fscanf(f, "%d ", &trc.data);
		}
		/*
				printf("%s %c ", trc.address, trc.rw);

				if (trc.rw == 'W')
				{
					printf("%d ", trc.data);
				}

				*/
		int trcadd[32]; // trc에서 받아서 바꾼 2진수 주소
		Cvtaddress(trcadd, trc);

		/*
		for (int i = 0; i < 32; i++)
			printf("%d", trcadd[i]);
			*/

			//메모리 자료구조에 주소값 넣어주기 윗줄부터 하나씩
		int adr = (Cvt2to10(trcadd));
		if (trc.rw == 'W') {
			M[m].Maddress = adr;
			//	printf("\n메모리에 저장된주소:%d\n", M[m].Maddress);
			M[m].Mdata = 0;
			m++;
			Msize++;
		}

		int byteoffbitn, blockoffbitn, indexbitn;// , tagbitn; // 주소에 들어갈 비트의 개수

		byteoffbitn = 2; //1word 단위로 읽으니까
		blockoffbitn = log_2(cs.b / 4); // 예시에서 1
		indexbitn = log_2(indexn);  // 예시에서 2
	//	tagbitn = 32 - byteoffbitn - blockoffbitn - indexbitn; // 예시에서 27

		//		printf("\n%d %d %d %d", byteoffbitn, blockoffbitn, indexbitn, tagbitn);

		//int adbyteoff, adblockoff, adindex, adtag;
		int adblockoff, adindex, adtag;
		// 주소를 비트 수만큼 잘라서 값을 구함
		//adbyteoff = findaddvalue(0, byteoffbitn, trcadd);
		adblockoff = findaddvalue(byteoffbitn, blockoffbitn + byteoffbitn, trcadd);
		adindex = findaddvalue(blockoffbitn + byteoffbitn, blockoffbitn + byteoffbitn + indexbitn, trcadd);
		adtag = findaddvalue(blockoffbitn + byteoffbitn + indexbitn, 32, trcadd);

		//	printf("\n%d %d %d %d\n\n", adbyteoff, adblockoff, adindex, adtag);

			//trc주소보고 캐시에 저장될 위치 찾아가기

		int repeat;
		int caindex = adindex * cs.a; // 찾는index set의 첫번째 cacheline
		int oldtime = ca[caindex].time;
		int oldindex = caindex;
		if (trc.rw == 'R') {


			for (repeat = 0; repeat < cs.a; repeat++) // oldtime캐시 검사
			{
				if (oldtime < ca[caindex].time) {
					oldtime = ca[caindex].time;
					oldindex = caindex;
				}
			}
			for (repeat = 0; repeat < cs.a; repeat++) { // vaild tag 검사
				if (ca[caindex].valid == 1)
				{
					if (ca[caindex].tag == adtag)
					{
						hitn++;
						cycle++;
						ca[caindex].time = 0;
						ca[caindex].traceaddress = adr;
						ca[caindex].traceblockoffset = adblockoff;
						break;
					}
				}
				caindex++;

			}
			caindex = adindex * cs.a;

			if (repeat == cs.a) {// v, tag 일치없음

				// set에 빈자리 있나 검사
				for (repeat = 0; repeat < cs.a; repeat++) {
					if (ca[caindex].valid == 0) // 빈자리에 메모리에서 값 가져와 넣기
					{
						missn++;
						cycle = cycle + 201;
						ca[caindex].valid = 1;
						ca[caindex].tag = adtag;
						ca[caindex].dirty = 0;
						ca[caindex].time = 0;
						ca[caindex].traceaddress = adr;
						ca[caindex].traceblockoffset = adblockoff;
						getmemorydata(adr, adblockoff, ca, caindex, cs, M, Msize);
						break;
					}
					else
						caindex++;
				}
				caindex = adindex * cs.a;

				if (repeat == cs.a)
				{

					if (ca[oldindex].dirty == 1)
					{
						missn++;
						cycle = cycle + 201;
						getcachedata(ca[oldindex].traceaddress, ca[oldindex].traceblockoffset, ca, oldindex, cs, M, Msize);
						ca[oldindex].valid = 1;
						ca[oldindex].tag = adtag;
						ca[oldindex].dirty = 0;
						ca[oldindex].time = 0;
						ca[oldindex].traceaddress = adr;
						ca[oldindex].traceblockoffset = adblockoff;
						getmemorydata(adr, adblockoff, ca, oldindex, cs, M, Msize);
					}
					else //dirty가 0인경우
					{
						missn++;
						cycle = cycle + 201;
						ca[oldindex].valid = 1;
						ca[oldindex].tag = adtag;
						ca[oldindex].dirty = 0;
						ca[oldindex].time = 0;
						ca[oldindex].traceaddress = adr;
						ca[oldindex].traceblockoffset = adblockoff;
						getmemorydata(adr, adblockoff, ca, oldindex, cs, M, Msize);

					}
				}
			}

		}
		else // trc.rw == 'W'
		{

			for (repeat = 0; repeat < cs.a; repeat++) // oldtime캐시 검사
			{
				if (oldtime < ca[caindex].time) {
					oldtime = ca[caindex].time;
					oldindex = caindex;
				}
			}
			for (repeat = 0; repeat < cs.a; repeat++) // vaild tag 검사
			{

				if (ca[caindex].valid == 1)
				{
					if (ca[caindex].tag == adtag)
					{
						hitn++;
						cycle++;
						ca[caindex].valid = 1;
						ca[caindex].tag = adtag;
						ca[caindex].dirty = 1;
						ca[caindex].time = 0;
						ca[caindex].traceaddress = adr;
						ca[caindex].traceblockoffset = adblockoff;
						ca[caindex].data[adblockoff] = trc.data;
						break;
					}

				}
				caindex++;
			}
			caindex = adindex * cs.a;

			if (repeat == cs.a) {// v, tag 일치없음

				// set에 빈자리 있나 검사
				for (repeat = 0; repeat < cs.a; repeat++) {
					if (ca[caindex].valid == 0) // 빈자리에 Write 해주기
					{
						missn++;
						cycle = cycle + 201;
						ca[caindex].valid = 1;
						ca[caindex].tag = adtag;
						ca[caindex].dirty = 1;
						ca[caindex].time = 0;
						ca[caindex].traceaddress = adr;
						ca[caindex].traceblockoffset = adblockoff;
						getmemorydata(ca[caindex].traceaddress, ca[caindex].traceblockoffset, ca, caindex, cs, M, Msize);
						ca[caindex].data[adblockoff] = trc.data;
						break;
					}
					caindex++;
				}
				caindex = adindex * cs.a;

				if (repeat == cs.a)
				{

					if (ca[oldindex].dirty == 1)
					{
						missn++;
						cycle = cycle + 201;
						getcachedata(ca[oldindex].traceaddress, ca[oldindex].traceblockoffset, ca, oldindex, cs, M, Msize);
						ca[oldindex].valid = 1;
						ca[oldindex].tag = adtag;
						ca[oldindex].dirty = 1;
						ca[oldindex].time = 0;
						ca[oldindex].traceaddress = adr;
						ca[oldindex].traceblockoffset = adblockoff;
						getmemorydata(adr, adblockoff, ca, oldindex, cs, M, Msize);
						ca[oldindex].data[adblockoff] = trc.data;
					}
					else { // dirty == 0
						missn++;
						cycle = cycle + 201;
						ca[oldindex].valid = 1;
						ca[oldindex].tag = adtag;
						ca[oldindex].dirty = 1;
						ca[oldindex].time = 0;
						ca[oldindex].traceaddress = adr;
						ca[oldindex].traceblockoffset = adblockoff;
						getmemorydata(adr, adblockoff, ca, oldindex, cs, M, Msize);
						ca[oldindex].data[adblockoff] = trc.data;
					}
				}
			}

		}
		/*
				printf("%d", adr);
				for (int i = 0; i < Msize; i++)
				{
					printf("\n====================%d:%d %d \n",i, M[i].Maddress, M[i].Mdata);
				}
				*/
		for (int i = 0; i < indexn * cs.a; i++)
		{
			ca[i].time++;
		}
	}

	int k = 0;
	for (int i = 0; i < indexn; i++) {

		printf("%d: ", i);
		for (int y = 0; y < cs.b / 4; y++) {
			printf("%.8X ", ca[k].data[y]);
		}
		printf("v:%d d:%d\n", ca[k].valid, ca[k].dirty);
		k++;
		for (int j = 1; j < cs.a; j++) {
			printf("  ");
			for (int y = 0; y < cs.b / 4; y++) {
				printf(" %.8X", ca[k].data[y]);
			}
			printf(" v:%d d:%d\n", ca[k].valid, ca[k].dirty);
			k++;
		}
	}
	int dirtycnt = 0;
	for (int i = 0; i < indexn * cs.a; i++)
	{
		if (ca[i].dirty == 1)
			dirtycnt++;
	}

	printf("\ntotal number of hits : %d\n", hitn);
	printf("total number of misses: %d\n", missn);
	printf("miss rate: %.1lf%%\n", ((double)missn / (double)(hitn + missn)) * 100);
	printf("total number of dirty block: %d\n", dirtycnt);
	printf("average memory access cycle: %.1lf\n", ((double)cycle / (double)(hitn + missn)));
	return 0;
}

int* Cvt16to2(char a) {
	int b[4];
	int* c;
	c = (int*)malloc(4 * sizeof(int));
	int n = (int)a - '0';
	int j = 3;

	if (n > 9) n = n - 7; //A를10으로

	for (int i = 0; i < 4; i++) {
		b[i] = (n % 2);
		n = n / 2;
	}

	for (int i = 0; i < 4; i++) {
		c[j] = b[i];
		j--;
	}

	return c;
}

int Cvt2to10(int trcadd[])
{
	int ten = 0;
	int j = 0;
	for (int i = 31; i >= 0; i--) {
		ten = ten + (trcadd[i] * pow_2(j));
		j++;

	}
	return ten;
}
int* Cvtaddress(int trcadd[], trace trc) {
	//trace.address 2진수 32개로 변경
	int n = 0;

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			trcadd[n] = Cvt16to2(trc.address[i])[j];
			n++;
		}
	}

	return trcadd;
}

int log_2(int a) {
	int x = 0;

	while (a > 1) {
		a = a / 2;
		x++;
	}

	return x;
}

int pow_2(int a) {
	int x = 1;

	for (int i = 0; i < a; i++) {
		x = x * 2;
	}

	return x;
}

int findaddvalue(int a, int b, int trcadd[]) {
	int x;
	int y = 0;
	for (int i = a; i < b; i++) {

		x = (trcadd[31 - i] * pow_2(i - a));
		y = y + x;
	}

	return y;
}

int gettrcfileLine(char* name)
{
	FILE* fp;
	int line = 0;
	char c;
	fp = fopen(name, "r");
	while ((c = fgetc(fp)) != EOF)
		if (c == '\n') line++;
	fclose(fp);
	return(line);
}

void getmemorydata(int adr, int blf, cache* ca, int caindex, cacheset cs, memory* M, int Msize) {
	int index = adr - (blf * 4); // 이래야 캐시데이터0번부터 데이터를 입력 가능
	int j;
	for (int i = 0; i < (cs.b / 4); i++) {
		for (j = 0; j < Msize; j++) {
			if (index == M[j].Maddress) {
				ca[caindex].data[i] = M[j].Mdata;
				break;
			}
		}
		index += 4;
	}

}

void getcachedata(int adr, int blf, cache* ca, int caindex, cacheset cs, memory* M, int Msize) {
	int index = adr - (blf * 4); // 이래야 캐시데이터0번부터 데이터를 메모리 주소에 입력 가능
	int j;
	int i;
	for (i = 0; i < (cs.b / 4); i++) {
		for (j = 0; j < Msize; j++)
			if (index == M[j].Maddress) {
				M[j].Mdata = ca[caindex].data[i];
				break;
			}
		index += 4;
	}
}