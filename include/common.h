#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
// =================================================================

#include "hi_type.h"
#include "common_struct.h"

static HI_BOOL s_bSampleSvpInit = HI_FALSE;

// ============= Accessable function below ===========================
HI_VOID SAMPLE_COMM_SVP_CheckSysInit(HI_VOID);

HI_S32 SAMPLE_COMM_SVP_NNIE_LoadModel(HI_CHAR * pszModelFile, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel);

HI_S32 SAMPLE_COMM_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);

HI_S32 SAMPLE_COMM_SVP_MallocMem(HI_CHAR *pszMmb, HI_CHAR *pszZone, HI_U64 *pu64PhyAddr, HI_VOID **ppvVirAddr, HI_U32 u32Size);

HI_S32 SAMPLE_COMM_SVP_MallocCached(HI_CHAR *pszMmb, HI_CHAR *pszZone,HI_U64 *pu64PhyAddr, HI_VOID **ppvVirAddr, HI_U32 u32Size);

HI_S32 SAMPLE_COMM_SVP_FlushCache(HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size);

HI_S32 SAMPLE_COMM_SVP_NNIE_ParamDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);

HI_S32 SAMPLE_COMM_SVP_NNIE_UnloadModel(SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel);

HI_S32 SAMPLE_SVP_NNIE_FillSrcData(
                SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg, 
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, 
                SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx);

HI_S32 SAMPLE_SVP_NNIE_Forward(
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx, 
                SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,
                HI_BOOL bInstant);

HI_S32 SAMPLE_SVP_NNIE_Detection_PrintResult(
                SVP_BLOB_S *pstDstScore,
                SVP_BLOB_S *pstDstRoi, 
                SVP_BLOB_S *pstClassRoiNum, 
                HI_FLOAT f32PrintResultThresh);

HI_VOID SAMPLE_COMM_SVP_CheckSysExit(HI_VOID);

HI_S32 SVP_NNIE_SoftMax(HI_FLOAT* pf32Src, HI_U32 u32Num);

// ============= Unaccessable function below ==========================

static HI_S32 SAMPLE_COMM_SVP_SysInit(HI_VOID);

static HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);

static HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam);

static HI_S32 SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(
                SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg,
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                HI_U32*pu32TotalTaskBufSize, 
                HI_U32*pu32TmpBufSize,
                SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[],
                HI_U32*pu32TotalSize);

static void SAMPLE_SVP_NNIE_GetBlobMemSize(
                SVP_NNIE_NODE_S astNnieNode[], 
                HI_U32 u32NodeNum,
                HI_U32 u32TotalStep,
                SVP_BLOB_S astBlob[], 
                HI_U32 u32Align, 
                HI_U32* pu32TotalSize,
                HI_U32 au32BlobSize[]);

static HI_S32 SAMPLE_COMM_SVP_SysExit(HI_VOID);

static HI_FLOAT SVP_NNIE_QuickExp(HI_S32 s32Value);
// =================================================================
#ifdef __cplusplus
}
#endif // __cplusplus
#endif