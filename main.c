#include <stdio.h>
#include "hi_type.h"

#include "common_struct.h"
#include "yolov3_struct.h"
#include "common.h"
#include "yolov3.h"

static SAMPLE_SVP_NNIE_MODEL_S s_stYolov3Model = {0};
static SAMPLE_SVP_NNIE_PARAM_S s_stYolov3NnieParam = {0};
static SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S s_stYolov3SoftwareParam = {0};

int main(int argc, char** argv) {
    HI_S32 s32Ret = HI_SUCCESS;
    printf("OK\n");
    HI_CHAR* pcBGRFile = "../resources/dog_bike_car_416x416.bgr";
    HI_CHAR* pcModelFile = "../resources/inst_yolov3_inst_coco80.wk";
    HI_U32 u32BatchNum = 1;
    HI_FLOAT f32ConfThres = 0.6f;
    // HI_FLOAT f32NMSThres = 0.4f;

    SAMPLE_SVP_NNIE_CFG_S stNnieCfg = {0};
    stNnieCfg.pszPic = pcBGRFile;
    stNnieCfg.u32MaxInputNum = u32BatchNum;
    stNnieCfg.u32MaxRoiNum = 0;
    stNnieCfg.aenNnieCoreId[0] = SVP_NNIE_ID_0;//set NNIE core

    // input and output information
    SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S stInputDataIdx = {0};
    SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S stProcSegIdx = {0};

    // Check system init
    SAMPLE_COMM_SVP_CheckSysInit();
    
    // SVP load model
    printf("YoloV3 loading model\n");
    s32Ret = SAMPLE_COMM_SVP_NNIE_LoadModel(pcModelFile, &s_stYolov3Model);
    if (s32Ret != HI_SUCCESS) {
        printf("Error, Load_Model failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto YOLOV3_FAIL_0;
    }

    // Yolov3 Parameter initialization
    printf("YoloV3 parameter initializing\n");
    s_stYolov3NnieParam.pstModel = &s_stYolov3Model.stModel;
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_ParamInit(&stNnieCfg,&s_stYolov3NnieParam,&s_stYolov3SoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_Yolov3_ParamInit failed!\n");

    // Fill input data
    printf("YOLOV3 Started!\n");
    stInputDataIdx.u32SegIdx = 0;
    stInputDataIdx.u32NodeIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_FillSrcData(&stNnieCfg,&s_stYolov3NnieParam,&stInputDataIdx);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_FillSrcData failed!\n");

    // NNIE forward
    stProcSegIdx.u32SegIdx = 0;
    s32Ret = SAMPLE_SVP_NNIE_Forward(&s_stYolov3NnieParam,&stInputDataIdx,&stProcSegIdx,HI_TRUE);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_Forward failed!\n");

    // YoloV3 get result
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_GetResult(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,YOLOV3_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_Yolov3_GetResult failed!\n");

    // Print result
    printf("YoloV3 RESULT: \n");
    (void)SAMPLE_SVP_NNIE_Detection_PrintResult(&s_stYolov3SoftwareParam.stDstScore,
        &s_stYolov3SoftwareParam.stDstRoi, &s_stYolov3SoftwareParam.stClassRoiNum, f32ConfThres);

    
YOLOV3_FAIL_0:
    // Deinit
    SAMPLE_SVP_NNIE_Yolov3_Deinit(&s_stYolov3NnieParam,&s_stYolov3SoftwareParam,&s_stYolov3Model);
    SAMPLE_COMM_SVP_CheckSysExit();
    return s32Ret;
}

