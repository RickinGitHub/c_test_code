
#include <stdio.h>
#include <string.h>
#ifdef __unix__
#include <unistd.h>
#endif
#include "aispeech/duilite/duilite_wapper.h"
#include "Logger/log.h"

#include "cjson/cJSON.h"
#include "wavfile/wavfile.h"


#define FESP_PEROID_SIZE_MS 32		// feed preiod 32ms
#define SAMPLE_RATE 16000
#define SAMPLE_BYTE 2

#define _ONLY_TEST_AUTH

static int wakeup_callback(void *user_data, int type, char *msg, int len)
{
	log_i("%.*s", len, msg);
    return 0;
}

static int doa_callback(void *user_data, int type, char *msg, int len)
{
	log_i("%.*s", len, msg);
    return 0;
}

static int beamforming_callback(void *user_data, int type, char *msg, int len)
{
	FILE* output_bf_audio = (FILE*)user_data;
	if (type == DUILITE_MSG_TYPE_JSON) {
		log_i("%.*s", len, msg);
	} else if (type == DUILITE_MSG_TYPE_BINARY) {
		if (output_bf_audio) {
			fwrite(msg, 1, len, output_bf_audio);
		}
	}
    return 0;
}

int main(int argc, char **argv)
{
	// config file
	FILE *fp = NULL;			
	long config_file_size = 0;
	char* config_buff = NULL;
	cJSON *jsroot = NULL;
	cJSON *jsauth = NULL;
	cJSON *jsaim = NULL;
	cJSON *jsfesp = NULL;
	cJSON *jsfesp_res = NULL;
	char *auth_str = NULL;
	char *fesp_res_str = NULL;
	int ret = 0;

	// fesp engine handle
	struct DUILITE_FESP *fesp = NULL;

	// input raw audio
	FILE* input_raw_audio = NULL;
	struct wavfile_header wavhead;
	long input_audio_size = 0;
	// output beamforming audio
	FILE* output_bf_audio = NULL;

	int peroid_len = 0;
	char* peroid_buffer = NULL;


	if (argc != 3) {
		printf("Usage: %s config input.wav\n", argv[0]);
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (!fp) {
		log_e("failed to open %s", argv[1]);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	config_file_size = ftell(fp);
	if (config_file_size > 10240) {
		log_e("%s is to large, size: %d", argv[1], config_file_size);
		return 0;
	}

	config_buff = malloc(config_file_size);
	if (config_buff == NULL) {
		log_e("no enough memory");
		goto _end;
	}

	fseek(fp, 0, SEEK_SET);
	ret = fread(config_buff, 1, config_file_size, fp);
	fclose(fp);

	log_v("config_file_size: %d, config_buff: %s\n", config_file_size, config_buff);

	jsroot = cJSON_Parse(config_buff);
	if (jsroot == NULL) {
		log_e("config file err");
		goto _end;
	}

	jsauth = cJSON_GetObjectItem(jsroot, "auth");
	jsaim = cJSON_GetObjectItem(jsroot, "aim");
	if (jsauth == NULL || jsaim == NULL) {
		log_e("auth/aim not found");
		goto _end;
	}
	jsfesp = cJSON_GetObjectItem(jsaim, "fesp");
	if (jsfesp != NULL) {
		jsfesp_res = cJSON_GetObjectItem(jsfesp, "res");
	}

	if (jsfesp_res == NULL) {
		log_e("fesp res not found");
		goto _end;
	}

	auth_str = cJSON_PrintUnformatted(jsauth);
	fesp_res_str = cJSON_PrintUnformatted(jsfesp_res);

	/*****************************input/output audio****************************/
	output_bf_audio = fopen("./bf.pcm", "wb");
	if (!output_bf_audio) {
		log_e("ERROR: open %s error!", "./bf.pcm");
		goto _end;
	}

	log_i("input raw audio file=%s", argv[2]);
	input_raw_audio = fopen(argv[2], "rb");
	if (!input_raw_audio) {
		log_e("ERROR: open %s error!", argv[2]);
		goto _end;
	}
	fseek(input_raw_audio, 0, SEEK_END);
	input_audio_size = ftell(input_raw_audio);
	if (input_audio_size <= sizeof(wavhead)) {
		log_e("input auduio %s error!", argv[2]);
		goto _end;
	}
	input_audio_size -= sizeof(wavhead);
	
	fseek(input_raw_audio, 0, SEEK_SET);
	ret = fread(&wavhead, sizeof(wavhead), 1, input_raw_audio);

	if (wavhead.sample_rate != SAMPLE_RATE
		 || wavhead.bits_per_sample != SAMPLE_BYTE*8) {
		log_e("input auduio %s format not match", argv[2]);
		goto _end;
	}

	peroid_len = FESP_PEROID_SIZE_MS*SAMPLE_RATE/1000*SAMPLE_BYTE*wavhead.num_channels;
	log_i("peroid_len: %d", peroid_len);
	peroid_buffer = malloc(peroid_len);
	if (peroid_buffer == NULL) {
		log_e("no enough memory");
		goto _end;
	}

	/****************************jsauth****************************************/
	log_i("duilite_library_load, config: %s", auth_str);
	ret = duilite_library_load(auth_str);
	if (ret) {
		log_e("duilite_library_load failed ret=%d", ret);
		goto _end;
	}

	log_i("jsauth success");

	/*****************************init fesp**********************************/
	log_i("DUILITE_FESP_NEW, config: %s", fesp_res_str);
	fesp = DUILITE_FESP_NEW(fesp_res_str);
	if (!fesp) {
		log_e("ERROR: duilite_fespx_new failed");
		goto _end;
	}

	log_i("fesp engine create success");

	/*****************************register cb**********************************/
	DUILITE_FESP_REGISTER(fesp, DUILITE_CALLBACK_FESP_WAKEUP, wakeup_callback, NULL);
	DUILITE_FESP_REGISTER(fesp, DUILITE_CALLBACK_FESP_DOA, doa_callback, NULL);
	DUILITE_FESP_REGISTER(fesp, DUILITE_CALLBACK_FESP_BEAMFORMING, beamforming_callback, output_bf_audio);

	/*****************************start engine*********************************/
	ret = DUILITE_FESP_START(fesp, NULL);
	if (ret != 0){
		log_e("ERROR: duilite_fespx_start failed");
		goto _end;
	}
	log_i("fesp engine start success");

	int total_read_len=0;
	while (1) {
		int len = fread(peroid_buffer, 1, peroid_len, input_raw_audio);
		if (0 == len) {
			break;
		}
		log_v("DUILITE_FESP_FEED: len: %d", len);
		DUILITE_FESP_FEED(fesp, peroid_buffer, len);
		total_read_len+=len;

		static int percent = 0;
		if (percent != total_read_len*100/input_audio_size/5*5) {
			percent = total_read_len*100/input_audio_size/5*5;
			printf("feed file= %d%%\n", percent);
		}

		#ifdef DELAY_SIMULATE
			usleep(FESP_PEROID_SIZE_MS*1000);
		#endif
	}	

	DUILITE_FESP_STOP(fesp);
_end:
	if (fesp) {
		DUILITE_FESP_DELETE(fesp);
		fesp = NULL;
	}

	duilite_library_release();
	if (peroid_buffer != NULL) {
		free(peroid_buffer);
		peroid_buffer = NULL;
	}

	if (config_buff != NULL) {
		free(config_buff);
		config_buff = NULL;
	}
	
	if (auth_str != NULL) {
		free(auth_str);
	}

	if (fesp_res_str != NULL) {
		free(fesp_res_str);
	}

	if (output_bf_audio)
		fclose(output_bf_audio);
	if (input_raw_audio)
		fclose(input_raw_audio);

	return 0;
}
