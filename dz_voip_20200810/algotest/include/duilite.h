#ifndef __DUILITE_H__
#define __DUILITE_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NO_ANDROID
#undef __ANDROID__
#endif

#if (!(defined DUILITE_CALL) || !(defined DUILITE_IMPORT_OR_EXPORT))
	#if defined _WIN32
		#if defined _WIN64
			#define DUILITE_CALL __stdcall
		#else
			#define DUILITE_CALL
		#endif

		#ifdef DUILITE_IMPLEMENTION
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllexport)
		#else
			#define DUILITE_IMPORT_OR_EXPORT __declspec(dllimport)
		#endif
	#elif defined __ANDROID__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
		#undef  JNIEXPORT
		#define JNIEXPORT __attribute ((visibility("default")))
	#elif defined __APPLE__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#elif defined __unix__
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT __attribute ((visibility("default")))
	#else
		#define DUILITE_CALL
		#define DUILITE_IMPORT_OR_EXPORT
	#endif
#endif

enum duilite_log_level {
	DUILITE_LOG_LEVEL_VERBOSE = 1,
	DUILITE_LOG_LEVEL_DEBUG,
	DUILITE_LOG_LEVEL_INFO,
	DUILITE_LOG_LEVEL_WARNING,
	DUILITE_LOG_LEVEL_ERROR,
};

/* define duilite_callback begin*/
enum duilite_msg_type {
	DUILITE_MSG_TYPE_JSON = 0,
	DUILITE_MSG_TYPE_BINARY,
	DUILITE_MSG_TYPE_TLV,
};

enum duilite_callback_type {
	DUILITE_CALLBACK_FESPA_WAKEUP = 0,
	DUILITE_CALLBACK_FESPA_DOA,
	DUILITE_CALLBACK_FESPA_BEAMFORMING,
	DUILITE_CALLBACK_FESPA_VPRINTCUT,
	DUILITE_CALLBACK_FESPL_WAKEUP,
	DUILITE_CALLBACK_FESPL_DOA,
	DUILITE_CALLBACK_FESPL_BEAMFORMING,
	DUILITE_CALLBACK_FESPL_VPRINTCUT,
	DUILITE_CALLBACK_MR_BEAMFORMING,
	DUILITE_CALLBACK_MR_POST,
	DUILITE_CALLBACK_MR_DOA,
	DUILITE_CALLBACK_FESPCAR_WAKEUP,
	DUILITE_CALLBACK_FESPCAR_BEAMFORMING,
	DUILITE_CALLBACK_FESPCAR_DOA,
	DUILITE_CALLBACK_FESPCAR_VPRINTCUT,
	DUILITE_CALLBACK_FESPD_WAKEUP,
	DUILITE_CALLBACK_FESPD_DOA,
	DUILITE_CALLBACK_FESPD_BEAMFORMING,
	DUILITE_CALLBACK_FESPD_VPRINTCUT,
	DUILITE_CALLBACK_WAKEUP_VPRINTCUT,
	DUILITE_CALLBACK_AECAGC_AECPOP,
	DUILITE_CALLBACK_AECAGC_VOIP,
	DUILITE_CALLBACK_FASP_DATA_CHS1,
	DUILITE_CALLBACK_FASP_DATA_CHS2,
	DUILITE_CALLBACK_ECHO_VOIP,
	DUILITE_CALLBACK_SEVC_OUTPUT,
};

typedef int (*duilite_callback)(void *userdata, int type, char *msg, int len);
/* define duilite_callback end*/


/* define duilite_model_callback begin */
enum duilite_model_msg_type {
	DUILITE_MODEL_MSG_TYPE_VPSELECT = 0,
	DUILITE_MODEL_MSG_TYPE_VPUPDATE,
	DUILITE_MODEL_MSG_TYPE_VPINSERT,
	DUILITE_MODEL_MSG_TYPE_VPDELETE,
};

enum duilite_model_callback_type {
	DUILITE_MODEL_CALLBACK_VPRINT = 0,
};

typedef int (*duilite_model_callback)(void *userdata, int type, char *id, char **data, int *data_size, int *data_nmemb);
/* define duilite_model_callback end */


DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_library_load(char *cfg);
DUILITE_IMPORT_OR_EXPORT void DUILITE_CALL duilite_library_release();

enum duilite_opt_type {
	DUILITE_OPT_VERSION_STR_GET = 0,
	DUILITE_OPT_AUTH_CHECK,
	DUILITE_OPT_AUTH_TRY,
	DUILITE_OPT_UPLOAD_MODE_GET	
};
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_library_opt(int opt, char *data, int size);


struct duilite_vad;
DUILITE_IMPORT_OR_EXPORT struct duilite_vad * DUILITE_CALL duilite_vad_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_start(struct duilite_vad *vad, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_feed(struct duilite_vad *vad, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_stop(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_cancel(struct duilite_vad *vad);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vad_delete(struct duilite_vad *vad);

struct duilite_speexenc;
DUILITE_IMPORT_OR_EXPORT struct duilite_speexenc * DUILITE_CALL duilite_speexenc_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_start(struct duilite_speexenc *speexenc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_feed(struct duilite_speexenc *speexenc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_stop(struct duilite_speexenc *speexenc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_speexenc_delete(struct duilite_speexenc *speexenc);

struct duilite_echo;
DUILITE_IMPORT_OR_EXPORT struct duilite_echo * DUILITE_CALL duilite_echo_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_register(struct duilite_echo *echo, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_start(struct duilite_echo *echo, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_feed(struct duilite_echo *echo, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_stop(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_cancel(struct duilite_echo *echo);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_echo_delete(struct duilite_echo *echo);

struct duilite_wakeup;
DUILITE_IMPORT_OR_EXPORT struct duilite_wakeup * DUILITE_CALL duilite_wakeup_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_register(struct duilite_wakeup *wakeup, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_start(struct duilite_wakeup *wakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_feed(struct duilite_wakeup *wakeup, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_stop(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_cancel(struct duilite_wakeup *wakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_set(struct duilite_wakeup *wakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_wakeup_delete(struct duilite_wakeup *wakeup);

struct duilite_cntts;
DUILITE_IMPORT_OR_EXPORT struct duilite_cntts * DUILITE_CALL duilite_cntts_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_start(struct duilite_cntts *cntts, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_feed(struct duilite_cntts *cntts, char *data);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_set(struct duilite_cntts *cntts, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_cntts_delete(struct duilite_cntts *cntts);

struct duilite_gram;
DUILITE_IMPORT_OR_EXPORT struct duilite_gram * DUILITE_CALL duilite_gram_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_start(struct duilite_gram *gram, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gram_delete(struct duilite_gram *gram);

struct duilite_asr;
DUILITE_IMPORT_OR_EXPORT struct duilite_asr * DUILITE_CALL duilite_asr_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_start(struct duilite_asr *asr, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_feed(struct duilite_asr *asr, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_stop(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_cancel(struct duilite_asr *asr);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asr_delete(struct duilite_asr *asr);

struct duilite_fespa;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespa * DUILITE_CALL duilite_fespa_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_register(struct duilite_fespa *fespa, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_start(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_feed(struct duilite_fespa *fespa, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_stop(struct duilite_fespa *fespa);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_set(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_get(struct duilite_fespa *fespa, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespa_delete(struct duilite_fespa *fespa);

struct duilite_fespl;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespl * DUILITE_CALL duilite_fespl_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_register(struct duilite_fespl *fespl, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_start(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_feed(struct duilite_fespl *fespl, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_stop(struct duilite_fespl *fespl);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_set(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_get(struct duilite_fespl *fespl, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespl_delete(struct duilite_fespl *fespl);

struct duilite_fdm;
DUILITE_IMPORT_OR_EXPORT struct duilite_fdm * DUILITE_CALL duilite_fdm_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_start(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_set(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_get(struct duilite_fdm *fdm, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_feed(struct duilite_fdm *fdm, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_stop(struct duilite_fdm *fdm);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fdm_delete(struct duilite_fdm *fdm);

struct duilite_vprint;
DUILITE_IMPORT_OR_EXPORT struct duilite_vprint * DUILITE_CALL duilite_vprint_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT struct duilite_vprint * DUILITE_CALL duilite_vprint_new2(char *cfg, duilite_callback callback, duilite_model_callback model_callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_start(struct duilite_vprint *vprint, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_feed(struct duilite_vprint *vprint, char *data, int len, int type);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_stop(struct duilite_vprint *vprint);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_vprint_delete(struct duilite_vprint *vprint);

struct duilite_qbye;
DUILITE_IMPORT_OR_EXPORT struct duilite_qbye * DUILITE_CALL duilite_qbye_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_start(struct duilite_qbye *qbye, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_feed(struct duilite_qbye *qbye, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_stop(struct duilite_qbye *qbye);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_qbye_delete(struct duilite_qbye *qbye);

struct duilite_gender;
DUILITE_IMPORT_OR_EXPORT struct duilite_gender * DUILITE_CALL duilite_gender_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_start(struct duilite_gender *gender, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_feed(struct duilite_gender *gender, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_stop(struct duilite_gender *gender);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_gender_delete(struct duilite_gender *gender);

struct duilite_nwakeup;
DUILITE_IMPORT_OR_EXPORT struct duilite_nwakeup * DUILITE_CALL duilite_nwakeup_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_start(struct duilite_nwakeup *nwakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_feed(struct duilite_nwakeup *nwakeup, int index, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_stop(struct duilite_nwakeup *nwakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_cancel(struct duilite_nwakeup *nwakeup);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_set(struct duilite_nwakeup *nwakeup, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_nwakeup_delete(struct duilite_nwakeup *nwakeup);

struct duilite_dmasp;
DUILITE_IMPORT_OR_EXPORT struct duilite_dmasp * DUILITE_CALL duilite_dmasp_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_start(struct duilite_dmasp *dmasp, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_feed(struct duilite_dmasp *dmasp, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_stop(struct duilite_dmasp *dmasp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_cancel(struct duilite_dmasp *dmasp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_set(struct duilite_dmasp *dmasp, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_dmasp_delete(struct duilite_dmasp *dmasp);

struct duilite_asrpp;
DUILITE_IMPORT_OR_EXPORT struct duilite_asrpp * DUILITE_CALL duilite_asrpp_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_start(struct duilite_asrpp *asrpp, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_feed(struct duilite_asrpp *asrpp, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_stop(struct duilite_asrpp *asrpp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_cancel(struct duilite_asrpp *asrpp);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_asrpp_delete(struct duilite_asrpp *asrpp);

struct duilite_agc;
DUILITE_IMPORT_OR_EXPORT struct duilite_agc * DUILITE_CALL duilite_agc_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_start(struct duilite_agc *agc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_feed(struct duilite_agc *agc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_stop(struct duilite_agc *agc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_cancel(struct duilite_agc *agc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_agc_delete(struct duilite_agc *agc);

struct duilite_audioCut;
DUILITE_IMPORT_OR_EXPORT struct duilite_audioCut * DUILITE_CALL duilite_audioCut_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_start(struct duilite_audioCut *audioCut, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_feed(struct duilite_audioCut *audioCut, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_stop(struct duilite_audioCut *audioCut);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_cancel(struct duilite_audioCut *audioCut);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_audioCut_delete(struct duilite_audioCut *audioCut);

struct duilite_fespd;
DUILITE_IMPORT_OR_EXPORT struct duilite_fespd * DUILITE_CALL duilite_fespd_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_register(struct duilite_fespd *fespd, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_start(struct duilite_fespd *fespd, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_feed(struct duilite_fespd *fespd, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_stop(struct duilite_fespd *fespd);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_set(struct duilite_fespd *fespd, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_get(struct duilite_fespd *fespd, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_fespd_delete(struct duilite_fespd *fespd);

struct duilite_mds;
DUILITE_IMPORT_OR_EXPORT struct duilite_mds * DUILITE_CALL duilite_mds_new(char *cfg, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_mds_start(struct duilite_mds *mds, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_mds_feed(struct duilite_mds *mds, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_mds_stop(struct duilite_mds *mds);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_mds_cancel(struct duilite_mds *mds);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_mds_delete(struct duilite_mds *mds);

struct duilite_sevc;
DUILITE_IMPORT_OR_EXPORT struct duilite_sevc * DUILITE_CALL duilite_sevc_new(char *cfg);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_sevc_register(struct duilite_sevc *sevc, int callback_type, duilite_callback callback, void *userdata);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_sevc_start(struct duilite_sevc *sevc, char *param);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_sevc_feed(struct duilite_sevc *sevc, char *data, int len);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_sevc_stop(struct duilite_sevc *sevc);
DUILITE_IMPORT_OR_EXPORT int DUILITE_CALL duilite_sevc_delete(struct duilite_sevc *sevc);

#ifdef __cplusplus
}
#endif

#endif
