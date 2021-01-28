#ifndef __COMMON_STRUCT_H__
#define __COMMON_STRUCT_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
// =================================================================

#include "hi_nnie.h"
#include "hi_type.h"
#include "parameters.h"

// input information
typedef struct hiSAMPLE_SVP_NNIE_CFG_S {
    HI_CHAR *pszPic;
    HI_U32 u32MaxInputNum;
    HI_U32 u32MaxRoiNum;
    HI_U64 au64StepVirAddr[SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM*SVP_NNIE_MAX_NET_SEG_NUM]; //virtual addr of LSTM's or RNN's step buffer
	SVP_NNIE_ID_E aenNnieCoreId[SVP_NNIE_MAX_NET_SEG_NUM];
} SAMPLE_SVP_NNIE_CFG_S;

// NNIE input or output data index
typedef struct hiSAMPLE_SVP_NNIE_DATA_INDEX_S {
	HI_U32 u32SegIdx;
	HI_U32 u32NodeIdx;
} SAMPLE_SVP_NNIE_DATA_INDEX_S;
// indicate the input data from which seg's input or report node 
typedef SAMPLE_SVP_NNIE_DATA_INDEX_S SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S;
// indicate which seg will be executed
typedef SAMPLE_SVP_NNIE_DATA_INDEX_S SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S;

/*each seg input and output memory*/
typedef struct hiSAMPLE_SVP_NNIE_SEG_DATA_S {
	SVP_SRC_BLOB_S      astSrc[SVP_NNIE_MAX_INPUT_NUM];
	SVP_DST_BLOB_S      astDst[SVP_NNIE_MAX_OUTPUT_NUM];
} SAMPLE_SVP_NNIE_SEG_DATA_S;

// Model manage structure
typedef struct hiSAMPLE_SVP_NNIE_MODEL_S {
    SVP_NNIE_MODEL_S    stModel;    // parsed model structure by HI_MPI_SVP_NNIE_LoadModel
    SVP_MEM_INFO_S      stModelBuf; // store Model file
} SAMPLE_SVP_NNIE_MODEL_S;

// NNIE Execution parameters
typedef struct hiSAMPLE_SVP_NNIE_PARAM_S {
    SVP_NNIE_MODEL_S*   pstModel;       // Model structure parameters
    HI_U32              u32TmpBufSize;  // Temp buffer for inference, base on pstModel的u32TmpBufSize
    HI_U32              au32TaskBufSize[SVP_NNIE_MAX_NET_SEG_NUM];  // Temp buffer, base on HI_MPI_SVP_NNIE_GetTskBufSize
    SVP_MEM_INFO_S      stTaskBuf;      // Temp buffer information
	SVP_MEM_INFO_S      stTmpBuf;       // Temp buffer information
    SVP_MEM_INFO_S      stStepBuf;      // store Lstm step info
    SAMPLE_SVP_NNIE_SEG_DATA_S          astSegData[SVP_NNIE_MAX_NET_SEG_NUM];               // each seg's input and output blob
    SVP_NNIE_FORWARD_CTRL_S             astForwardCtrl[SVP_NNIE_MAX_NET_SEG_NUM];           // CNN/DNN/RNN inference control parameter
	SVP_NNIE_FORWARD_WITHBBOX_CTRL_S    astForwardWithBboxCtrl[SVP_NNIE_MAX_NET_SEG_NUM]; 
} SAMPLE_SVP_NNIE_PARAM_S;

/*each seg input and output data memory size*/
typedef struct hiSAMPLE_SVP_NNIE_BLOB_SIZE_S {
	HI_U32      au32SrcSize[SVP_NNIE_MAX_INPUT_NUM];
	HI_U32      au32DstSize[SVP_NNIE_MAX_OUTPUT_NUM];
} SAMPLE_SVP_NNIE_BLOB_SIZE_S;

/*stack for sort*/
typedef struct hiSAMPLE_SVP_NNIE_STACK {
    HI_S32 s32Min;
    HI_S32 s32Max;
} SAMPLE_SVP_NNIE_STACK_S;

// =================================================================
#ifdef __cplusplus
}
#endif // __cplusplus
#endif