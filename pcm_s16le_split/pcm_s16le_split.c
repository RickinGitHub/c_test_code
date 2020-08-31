#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int pcm_s16le_merge(const char *input, char *output)
{

}

/*
 * copy input-str per 1 byte to 6 times
 */
#if 0
int pcm_s16le_split(const char *input, int len, char *output)
{
	int  size = 6, i;
	char *sample = (char *)malloc(size);

	memset(sample, 0, size);
	for(i = 0; i < len; i++) {
		memset(sample, input[i],size);
		memcpy(output + 6*i, sample, size);
		memset(sample, 0, size);
	}
	output[6*len] = '\0';
	free(sample);

	return len*6;
}
#endif

/*
 * copy input-str per 2 bytes to 6 times
 */
int pcm_s16le_split(const char *input, int len, char *output)
{
	int  size = 6, i;
	char *sample = (char *)malloc(size);

	memset(sample, 0, size);
	for(i = 0; i < len; i+=2) {
		memcpy(sample, input + i, 1);
		memcpy(sample+1, input+i+1, 1);
		memcpy(output + i*6+0,  sample, 2);
		memcpy(output + i*6+2,  sample, 2);
		memcpy(output + i*6+4,  sample, 2);
		memcpy(output + i*6+6,  sample, 2);
		memcpy(output + i*6+8,  sample, 2);
		memcpy(output + i*6+10, sample, 2);
		memset(sample, 0, size);
	}
	output[6*len] = '\0';
	free(sample);

	return len*6;
}

int main(int argc, char* argv[])
{
	printf("pcm_s16le_split...\n");
	char output[64] = {0};
	int  ret;
	if(argc < 2) {
		printf("Usage:%s str\n",argv[0]);
		return 0;
	}
	memset(output, 0, 64);
	ret = pcm_s16le_split(argv[1], strlen(argv[1]), output);
	printf("output:%s\n",output);

	return 0;
}
