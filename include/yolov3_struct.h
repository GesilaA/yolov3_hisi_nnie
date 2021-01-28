#ifndef __YOLOV3_STRUCT_H__
#define __YOLOV3_STRUCT_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
// =================================================================

#include "common_struct.h"

/*Yolov3 software parameter for postprocess*/ 
typedef struct hiSAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S {
    HI_U32 u32OriImHeight;
    HI_U32 u32OriImWidth;
    HI_U32 u32BboxNumEachGrid;
    HI_U32 u32ClassNum;
    HI_U32 au32GridNumHeight[3];
    HI_U32 au32GridNumWidth[3];
    HI_U32 u32NmsThresh;
    HI_U32 u32ConfThresh;
    HI_U32 u32MaxRoiNum;
    HI_FLOAT af32Bias[3][6];
    SVP_MEM_INFO_S stGetResultTmpBuf;
    SVP_DST_BLOB_S stClassRoiNum;
    SVP_DST_BLOB_S stDstRoi;
    SVP_DST_BLOB_S stDstScore;
} SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S;

typedef struct hiSAMPLE_SVP_NNIE_YOLOV3_BBOX {
    HI_FLOAT f32Xmin;
    HI_FLOAT f32Xmax;
    HI_FLOAT f32Ymin;
    HI_FLOAT f32Ymax;
    HI_S32 s32ClsScore;
    HI_U32 u32ClassIdx;
    HI_U32 u32Mask;
} SAMPLE_SVP_NNIE_YOLOV3_BBOX_S;

// =================================================================
#ifdef __cplusplus
}
#endif // __cplusplus
#endif