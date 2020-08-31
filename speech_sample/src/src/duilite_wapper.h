#include "aispeech/duilite/duilite.h"

#ifndef __DUILTE_WRAPPER_H__
#define __DUILTE_WRAPPER_H__

#ifdef _MIC_TYPE
	#if _MIC_TYPE==ARRAY_MIC_TYPE
        #define DUILITE_FESP duilite_fespa
        #define DUILITE_FESP_NEW duilite_fespa_new
        #define DUILITE_FESP_REGISTER duilite_fespa_register
        
        #define DUILITE_FESP_START duilite_fespa_start
        #define DUILITE_FESP_FEED duilite_fespa_feed
        #define DUILITE_FESP_STOP duilite_fespa_stop
        #define DUILITE_FESP_DELETE duilite_fespa_delete
        
        
        #define DUILITE_CALLBACK_FESP_WAKEUP DUILITE_CALLBACK_FESPA_WAKEUP
        #define DUILITE_CALLBACK_FESP_DOA DUILITE_CALLBACK_FESPA_DOA
        #define DUILITE_CALLBACK_FESP_BEAMFORMING DUILITE_CALLBACK_FESPA_BEAMFORMING
        #define DUILITE_CALLBACK_FESP_VPRINTCUT DUILITE_CALLBACK_FESPA_VPRINTCUT

        
	#elif _MIC_TYPE==DUAL_MIC_TYPE
        #define DUILITE_FESP duilite_fespd
        #define DUILITE_FESP_NEW duilite_fespd_new
        #define DUILITE_FESP_REGISTER duilite_fespd_register
        #define DUILITE_FESP_START duilite_fespd_start
        #define DUILITE_FESP_FEED duilite_fespd_feed
        #define DUILITE_FESP_STOP duilite_fespd_stop
        #define DUILITE_FESP_DELETE duilite_fespd_delete

        #define DUILITE_CALLBACK_FESP_WAKEUP DUILITE_CALLBACK_FESPD_WAKEUP
        #define DUILITE_CALLBACK_FESP_DOA DUILITE_CALLBACK_FESPD_DOA
        #define DUILITE_CALLBACK_FESP_BEAMFORMING DUILITE_CALLBACK_FESPD_BEAMFORMING
        #define DUILITE_CALLBACK_FESP_VPRINTCUT DUILITE_CALLBACK_FESPD_VPRINTCUT
	#elif _MIC_TYPE==LINEAR_MIC_TYPE
        #define DUILITE_FESP duilite_fespl
        #define DUILITE_FESP_NEW duilite_fespl_new
        #define DUILITE_FESP_REGISTER duilite_fespl_register
        #define DUILITE_FESP_START duilite_fespl_start
        #define DUILITE_FESP_FEED duilite_fespl_feed
        #define DUILITE_FESP_STOP duilite_fespl_stop
        #define DUILITE_FESP_DELETE duilite_fespl_delete

        #define DUILITE_CALLBACK_FESP_WAKEUP DUILITE_CALLBACK_FESPL_WAKEUP
        #define DUILITE_CALLBACK_FESP_DOA DUILITE_CALLBACK_FESPL_DOA
        #define DUILITE_CALLBACK_FESP_BEAMFORMING DUILITE_CALLBACK_FESPL_BEAMFORMING
        #define DUILITE_CALLBACK_FESP_VPRINTCUT DUILITE_CALLBACK_FESPL_VPRINTCUT
	#else
		#error _MIC_TYPE "not support"
	#endif
#else
	#error "_MIC_TYPE not defined"
#endif

#endif //__DUILTE_WRAPPER_H__
