#ifndef __YOLOV3_H__
#define __YOLOV3_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
// ==================================================================

#include "hi_type.h"
#include "parameters.h"
#include "common.h"
#include "yolov3_struct.h"

#define SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM   85  // yolov3 inference result num of each bbox: = CLASSNUM + 5
#define SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM    3             // yolov3 report blob num
#define SAMPLE_SVP_NNIE_YOLOV3_EACH_GRID_BIAS_NUM 6             // yolov3 bias num of each grid

// ============= Accessable function below ===========================

HI_S32 SAMPLE_SVP_NNIE_Yolov3_ParamInit(
                SAMPLE_SVP_NNIE_CFG_S* pstCfg,
                SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, 
                SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam);

HI_S32 SAMPLE_SVP_NNIE_Yolov3_GetResult(
                SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, 
                SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam);

HI_U32 SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(
                SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, 
                SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam);

HI_S32 SAMPLE_SVP_NNIE_Yolov3_Deinit(
                SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam,
                SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,
                SAMPLE_SVP_NNIE_MODEL_S* pstNnieModel);

// ============= Unaccessable function below ==========================

static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam);

static HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(
            SAMPLE_SVP_NNIE_CFG_S* pstCfg,
            SAMPLE_SVP_NNIE_PARAM_S* pstNnieParam, 
            SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam);

static HI_S32 SVP_NNIE_Yolov3_GetResult(
            HI_S32** pps32InputData,
            HI_U32 au32GridNumWidth[],
            HI_U32 au32GridNumHeight[],
            HI_U32 au32Stride[],
            HI_U32 u32EachGridBbox,
            HI_U32 u32ClassNum,
            HI_U32 u32SrcWidth,
            HI_U32 u32SrcHeight,
            HI_U32 u32MaxRoiNum,
            HI_U32 u32NmsThresh,
            HI_U32 u32ConfThresh,
            HI_FLOAT af32Bias[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM][SAMPLE_SVP_NNIE_YOLOV3_EACH_GRID_BIAS_NUM],
            HI_S32* ps32TmpBuf,
            HI_S32* ps32DstScore, 
            HI_S32* ps32DstRoi, 
            HI_S32* ps32ClassRoiNum);

static HI_S32 SVP_NNIE_Yolo3_NonRecursiveArgQuickSort(
            HI_S32* ps32Array,
            HI_S32 s32Low, 
            HI_S32 s32High, 
            HI_U32 u32ArraySize,
            HI_U32 u32ScoreIdx,
            SAMPLE_SVP_NNIE_STACK_S* pstStack);

static HI_S32 SVP_NNIE_Yolov3_NonMaxSuppression(
            SAMPLE_SVP_NNIE_YOLOV3_BBOX_S* pstBbox,
            HI_U32 u32BboxNum,
            HI_U32 u32NmsThresh,
            HI_U32 u32MaxRoiNum);

static HI_DOUBLE SVP_NNIE_Yolov3_Iou(
            SAMPLE_SVP_NNIE_YOLOV3_BBOX_S* pstBbox1,
            SAMPLE_SVP_NNIE_YOLOV3_BBOX_S* pstBbox2);

static HI_FLOAT SVP_NNIE_Yolov3_GetMaxVal(
            HI_FLOAT* pf32Val,
            HI_U32 u32Num,
            HI_U32* pu32MaxValueIndex);    
    
static void SVP_NNIE_Yolov3_Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2, HI_U32 u32ArraySize);

// ===================================================================
#ifdef __cplusplus
}
#endif // __cplusplus
#endif