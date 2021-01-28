#include "yolov3.h"

HI_S32 SAMPLE_SVP_NNIE_Yolov3_ParamInit(
            SAMPLE_SVP_NNIE_CFG_S* pstCfg,
            SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, 
            SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam) {
    HI_S32 s32Ret = HI_SUCCESS;
    /*init hardware para*/
    s32Ret = SAMPLE_COMM_SVP_NNIE_ParamInit(pstCfg,pstNnieParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_COMM_SVP_NNIE_ParamInit failed!\n",s32Ret);

    /*init software para*/
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(pstCfg,pstNnieParam,
        pstSoftWareParam);
    SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,INIT_FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_SoftwareInit failed!\n",s32Ret);

    return s32Ret;
INIT_FAIL_0:
    s32Ret = SAMPLE_SVP_NNIE_Yolov3_Deinit(pstNnieParam,pstSoftWareParam,NULL);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error(%#x),SAMPLE_SVP_NNIE_Yolov3_Deinit failed!\n",s32Ret);
    return HI_FAILURE;
}

/******************************************************************************
* function : Yolov3 software para init
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareInit(SAMPLE_SVP_NNIE_CFG_S* pstCfg,
    SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam) {
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32ClassNum = 0;
    HI_U32 u32TotalSize = 0;
    HI_U32 u32DstRoiSize = 0;
    HI_U32 u32DstScoreSize = 0;
    HI_U32 u32ClassRoiNumSize = 0;
    HI_U32 u32TmpBufTotalSize = 0;
    HI_U64 u64PhyAddr = 0;
    HI_U8* pu8VirAddr = NULL;

    pstSoftWareParam->u32OriImHeight = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Height;
    pstSoftWareParam->u32OriImWidth = pstNnieParam->astSegData[0].astSrc[0].unShape.stWhc.u32Width;
    pstSoftWareParam->u32BboxNumEachGrid = 3;//一般不动
    pstSoftWareParam->u32ClassNum = SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM - 5;//算法分类数目
    pstSoftWareParam->au32GridNumHeight[0] = 13;//yolo1层，32倍数，416/32=13,608...，由小到大
    pstSoftWareParam->au32GridNumHeight[1] = 26;//yolo2层，16倍数...
    pstSoftWareParam->au32GridNumHeight[2] = 52;//yolo3层，8倍数...
    pstSoftWareParam->au32GridNumWidth[0] = 13;//yolo1层，32倍数
    pstSoftWareParam->au32GridNumWidth[1] = 26;//yolo2层，32倍数
    pstSoftWareParam->au32GridNumWidth[2] = 52;//yolo3层，32倍数
    pstSoftWareParam->u32NmsThresh = (HI_U32)(0.3f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32ConfThresh = (HI_U32)(0.5f*SAMPLE_SVP_NNIE_QUANT_BASE);
    pstSoftWareParam->u32MaxRoiNum = 10;//最大Roi数
    pstSoftWareParam->af32Bias[0][0] = 116;//anchor值，0指代yolo1层，由大到小
    pstSoftWareParam->af32Bias[0][1] = 90;
    pstSoftWareParam->af32Bias[0][2] = 156;
    pstSoftWareParam->af32Bias[0][3] = 198;
    pstSoftWareParam->af32Bias[0][4] = 373;
    pstSoftWareParam->af32Bias[0][5] = 326;
    pstSoftWareParam->af32Bias[1][0] = 30;//1指代yolo2层
    pstSoftWareParam->af32Bias[1][1] = 61;
    pstSoftWareParam->af32Bias[1][2] = 62;
    pstSoftWareParam->af32Bias[1][3] = 45;
    pstSoftWareParam->af32Bias[1][4] = 59;
    pstSoftWareParam->af32Bias[1][5] = 119;
    pstSoftWareParam->af32Bias[2][0] = 10;//2指代yolo3层
    pstSoftWareParam->af32Bias[2][1] = 13;
    pstSoftWareParam->af32Bias[2][2] = 16;
    pstSoftWareParam->af32Bias[2][3] = 30;
    pstSoftWareParam->af32Bias[2][4] = 33;
    pstSoftWareParam->af32Bias[2][5] = 23;

    /*Malloc assist buffer memory*/
    u32ClassNum = pstSoftWareParam->u32ClassNum+1;

    SAMPLE_SVP_CHECK_EXPR_RET(SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM != pstNnieParam->pstModel->astSeg[0].u16DstNum,
        HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel->astSeg[0].u16DstNum(%d) should be %d!\n",
        pstNnieParam->pstModel->astSeg[0].u16DstNum,SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM);
    u32TmpBufTotalSize = SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(pstNnieParam,pstSoftWareParam);
    u32DstRoiSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    u32DstScoreSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    u32ClassRoiNumSize = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    u32TotalSize = u32TotalSize+u32DstRoiSize+u32DstScoreSize+u32ClassRoiNumSize+u32TmpBufTotalSize;
    s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_YOLOV3_INIT",NULL,(HI_U64*)&u64PhyAddr,
        (void**)&pu8VirAddr,u32TotalSize);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,Malloc memory failed!\n");
    memset(pu8VirAddr,0, u32TotalSize);
    SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

   /*set each tmp buffer addr*/
    pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = u64PhyAddr;
    pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = (HI_U64)(pu8VirAddr);

    /*set result blob*/
    pstSoftWareParam->stDstRoi.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstRoi.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize;
    pstSoftWareParam->stDstRoi.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize);
    pstSoftWareParam->stDstRoi.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32)*SAMPLE_SVP_NNIE_COORDI_NUM);
    pstSoftWareParam->stDstRoi.u32Num = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstRoi.unShape.stWhc.u32Width = u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*SAMPLE_SVP_NNIE_COORDI_NUM;

    pstSoftWareParam->stDstScore.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stDstScore.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+u32DstRoiSize;
    pstSoftWareParam->stDstScore.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+u32DstRoiSize);
    pstSoftWareParam->stDstScore.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*
        pstSoftWareParam->u32MaxRoiNum*sizeof(HI_U32));
    pstSoftWareParam->stDstScore.u32Num = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stDstScore.unShape.stWhc.u32Width = u32ClassNum*pstSoftWareParam->u32MaxRoiNum;

    pstSoftWareParam->stClassRoiNum.enType = SVP_BLOB_TYPE_S32;
    pstSoftWareParam->stClassRoiNum.u64PhyAddr = u64PhyAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize;
    pstSoftWareParam->stClassRoiNum.u64VirAddr = (HI_U64)(pu8VirAddr+u32TmpBufTotalSize+
        u32DstRoiSize+u32DstScoreSize);
    pstSoftWareParam->stClassRoiNum.u32Stride = SAMPLE_SVP_NNIE_ALIGN16(u32ClassNum*sizeof(HI_U32));
    pstSoftWareParam->stClassRoiNum.u32Num = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Chn = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Height = 1;
    pstSoftWareParam->stClassRoiNum.unShape.stWhc.u32Width = u32ClassNum;

    return s32Ret;
}

HI_U32 SAMPLE_SVP_NNIE_Yolov3_GetResultTmpBuf(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam) {
    HI_U32 u32TotalSize = 0;
    HI_U32 u32AssistStackSize = 0;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32TotalBboxSize = 0;
    HI_U32 u32DstBlobSize = 0;
    HI_U32 u32MaxBlobSize = 0;
    HI_U32 i = 0;

    for(i = 0; i < pstNnieParam->pstModel->astSeg[0].u16DstNum; i++) {
        u32DstBlobSize = pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Width*sizeof(HI_U32)*
            pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Height*
            pstNnieParam->pstModel->astSeg[0].astDstNode[i].unShape.stWhc.u32Chn;
        if(u32MaxBlobSize < u32DstBlobSize) {
            u32MaxBlobSize = u32DstBlobSize;
        }
        u32TotalBboxNum += pstSoftwareParam->au32GridNumWidth[i]*pstSoftwareParam->au32GridNumHeight[i]*
            pstSoftwareParam->u32BboxNumEachGrid;
    }
    u32AssistStackSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_STACK_S);
    u32TotalBboxSize = u32TotalBboxNum*sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S);
    u32TotalSize += (u32MaxBlobSize+u32AssistStackSize+u32TotalBboxSize);

    return u32TotalSize;
}

/******************************************************************************
* function : Yolov3 Deinit
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_Deinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam,SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel) {
    HI_S32 s32Ret = HI_SUCCESS;
    /*hardware deinit*/
    if(pstNnieParam!=NULL) {
        s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
    }
    /*software deinit*/
    if(pstSoftWareParam!=NULL) {
        s32Ret = SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(pstSoftWareParam);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit failed!\n");
    }
    /*model deinit*/
    if(pstNnieModel!=NULL) {
        s32Ret = SAMPLE_COMM_SVP_NNIE_UnloadModel(pstNnieModel);
        SAMPLE_SVP_CHECK_EXPR_TRACE(HI_SUCCESS != s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
            "Error,SAMPLE_COMM_SVP_NNIE_UnloadModel failed!\n");
    }
    return s32Ret;
}

/******************************************************************************
* function : Yolov3 software deinit
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_SoftwareDeinit(SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftWareParam) {
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_SVP_CHECK_EXPR_RET(NULL== pstSoftWareParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error, pstSoftWareParam can't be NULL!\n");
    if(0!=pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr && 0!=pstSoftWareParam->stGetResultTmpBuf.u64VirAddr) {
        SAMPLE_SVP_MMZ_FREE(pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr,
            pstSoftWareParam->stGetResultTmpBuf.u64VirAddr);
        pstSoftWareParam->stGetResultTmpBuf.u64PhyAddr = 0;
        pstSoftWareParam->stGetResultTmpBuf.u64VirAddr = 0;
        pstSoftWareParam->stDstRoi.u64PhyAddr = 0;
        pstSoftWareParam->stDstRoi.u64VirAddr = 0;
        pstSoftWareParam->stDstScore.u64PhyAddr = 0;
        pstSoftWareParam->stDstScore.u64VirAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64PhyAddr = 0;
        pstSoftWareParam->stClassRoiNum.u64VirAddr = 0;
    }
    return s32Ret;
}

/*****************************************************************************
* Prototype :   SAMPLE_SVP_NNIE_Yolov3_GetResult
* Description : this function is used to Get Yolov3 result
* Input :      SAMPLE_SVP_NNIE_PARAM_S*               pstNnieParam     [IN]  the pointer to YOLOV3 NNIE parameter
*              SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S*   pstSoftwareParam [IN]  the pointer to YOLOV3 software parameter
*
*
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Yolov3_GetResult(SAMPLE_SVP_NNIE_PARAM_S*pstNnieParam,
    SAMPLE_SVP_NNIE_YOLOV3_SOFTWARE_PARAM_S* pstSoftwareParam) {
    HI_U32 i = 0;
    HI_S32 *aps32InputBlob[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM] = {0};
    HI_U32 au32Stride[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM] = {0};

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++) {
        aps32InputBlob[i] = (HI_S32*)pstNnieParam->astSegData[0].astDst[i].u64VirAddr;
        au32Stride[i] = pstNnieParam->astSegData[0].astDst[i].u32Stride;
    }
	return SVP_NNIE_Yolov3_GetResult(
		aps32InputBlob, 
		pstSoftwareParam->au32GridNumWidth,
		pstSoftwareParam->au32GridNumHeight, 
		au32Stride, 
		pstSoftwareParam->u32BboxNumEachGrid,
		pstSoftwareParam->u32ClassNum, 
		pstSoftwareParam->u32OriImWidth,
		pstSoftwareParam->u32OriImWidth, 
		pstSoftwareParam->u32MaxRoiNum, 
		pstSoftwareParam->u32NmsThresh,
		pstSoftwareParam->u32ConfThresh, 
		pstSoftwareParam->af32Bias,
		(HI_S32*)pstSoftwareParam->stGetResultTmpBuf.u64VirAddr,
		(HI_S32*)pstSoftwareParam->stDstScore.u64VirAddr,
		(HI_S32*)pstSoftwareParam->stDstRoi.u64VirAddr,
		(HI_S32*)pstSoftwareParam->stClassRoiNum.u64VirAddr);
}

/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov3_GetResult
*   Description  : Yolov3 GetResult function
* Input :      HI_S32    **pps32InputData     [IN]  pointer to the input data
*              HI_U32    au32GridNumWidth[]   [IN]  Grid num in width direction
*              HI_U32    au32GridNumHeight[]  [IN]  Grid num in height direction
*              HI_U32    au32Stride[]         [IN]  stride of input data
*              HI_U32    u32EachGridBbox      [IN]  Bbox num of each gird
*              HI_U32    u32ClassNum          [IN]  class num
*              HI_U32    u32SrcWidth          [IN]  input image width
*              HI_U32    u32SrcHeight         [IN]  input image height
*              HI_U32    u32MaxRoiNum         [IN]  Max output roi num
*              HI_U32    u32NmsThresh         [IN]  NMS thresh
*              HI_U32    u32ConfThresh        [IN]  conf thresh
*              HI_U32    af32Bias[][]         [IN]  bias
*              HI_U32*   pu32TmpBuf           [IN]  assist buffer
*              HI_S32    *ps32DstScores       [OUT] dst score
*              HI_S32    *ps32DstRoi          [OUT] dst roi
*              HI_S32    *ps32ClassRoiNum     [OUT] class roi num
*
*   Output       :
*   Return Value : HI_FLOAT: max score value.
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Yolov3_GetResult(HI_S32 **pps32InputData,HI_U32 au32GridNumWidth[],
        HI_U32 au32GridNumHeight[],HI_U32 au32Stride[],HI_U32 u32EachGridBbox,HI_U32 u32ClassNum,HI_U32 u32SrcWidth,
        HI_U32 u32SrcHeight,HI_U32 u32MaxRoiNum,HI_U32 u32NmsThresh,HI_U32 u32ConfThresh,
        HI_FLOAT af32Bias[SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM][SAMPLE_SVP_NNIE_YOLOV3_EACH_GRID_BIAS_NUM],
        HI_S32* ps32TmpBuf,HI_S32 *ps32DstScore, HI_S32 *ps32DstRoi, HI_S32 *ps32ClassRoiNum) {
    HI_S32 *ps32InputBlob = NULL;
    HI_FLOAT *pf32Permute = NULL;
    SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox = NULL;
    HI_S32 *ps32AssistBuf = NULL;
    HI_U32 u32TotalBboxNum = 0;
    HI_U32 u32ChnOffset = 0;
    HI_U32 u32HeightOffset = 0;
    HI_U32 u32BboxNum = 0;
    HI_U32 u32GridXIdx;
    HI_U32 u32GridYIdx;
    HI_U32 u32Offset;
    HI_FLOAT f32StartX;
    HI_FLOAT f32StartY;
    HI_FLOAT f32Width;
    HI_FLOAT f32Height;
    HI_FLOAT f32ObjScore;
    HI_U32 u32MaxValueIndex = 0;
    HI_FLOAT f32MaxScore;
    HI_S32 s32ClassScore;
    HI_U32 u32ClassRoiNum;
    HI_U32 i = 0, j = 0, k = 0, c = 0, h = 0, w = 0;
    HI_U32 u32BlobSize = 0;
    HI_U32 u32MaxBlobSize = 0;

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++) {
        u32BlobSize = au32GridNumWidth[i]*au32GridNumHeight[i]*sizeof(HI_U32)*
            SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM*u32EachGridBbox;
        if(u32MaxBlobSize < u32BlobSize) {
            u32MaxBlobSize = u32BlobSize;
        }
    }

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++) {
        u32TotalBboxNum += au32GridNumWidth[i]*au32GridNumHeight[i]*u32EachGridBbox;
    }

    //get each tmpbuf addr
    pf32Permute = (HI_FLOAT*)ps32TmpBuf;
    pstBbox = (SAMPLE_SVP_NNIE_YOLOV3_BBOX_S*)(pf32Permute+u32MaxBlobSize/sizeof(HI_S32));
    ps32AssistBuf = (HI_S32*)(pstBbox+u32TotalBboxNum);

    for(i = 0; i < SAMPLE_SVP_NNIE_YOLOV3_REPORT_BLOB_NUM; i++) {
        //permute
        u32Offset = 0;
        ps32InputBlob = pps32InputData[i];
        u32ChnOffset = au32GridNumHeight[i]*au32Stride[i]/sizeof(HI_S32);
        u32HeightOffset = au32Stride[i]/sizeof(HI_S32);
        for (h = 0; h < au32GridNumHeight[i]; h++) {
            for (w = 0; w < au32GridNumWidth[i]; w++) {
                for (c = 0; c < SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM*u32EachGridBbox; c++) {
                    pf32Permute[u32Offset++] = (HI_FLOAT)(ps32InputBlob[c*u32ChnOffset+h*u32HeightOffset+w]) / SAMPLE_SVP_NNIE_QUANT_BASE;
                }
            }
        }

        //decode bbox and calculate score
        for(j = 0; j < au32GridNumWidth[i]*au32GridNumHeight[i]; j++) {
            u32GridXIdx = j % au32GridNumWidth[i];
            u32GridYIdx = j / au32GridNumWidth[i];
            for (k = 0; k < u32EachGridBbox; k++) {
                u32MaxValueIndex = 0;
                u32Offset = (j * u32EachGridBbox + k) * SAMPLE_SVP_NNIE_YOLOV3_EACH_BBOX_INFER_RESULT_NUM;
                //decode bbox
                f32StartX = ((HI_FLOAT)u32GridXIdx + SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 0])) / au32GridNumWidth[i];
                f32StartY = ((HI_FLOAT)u32GridYIdx + SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 1])) / au32GridNumHeight[i];
                f32Width = (HI_FLOAT)(exp(pf32Permute[u32Offset + 2]) * af32Bias[i][2*k]) / u32SrcWidth;
                f32Height = (HI_FLOAT)(exp(pf32Permute[u32Offset + 3]) * af32Bias[i][2*k + 1]) / u32SrcHeight;

                //calculate score
                f32ObjScore = SAMPLE_SVP_NNIE_SIGMOID(pf32Permute[u32Offset + 4]);
                (void)SVP_NNIE_SoftMax(&pf32Permute[u32Offset + 5], u32ClassNum);
                f32MaxScore = SVP_NNIE_Yolov3_GetMaxVal(&pf32Permute[u32Offset + 5], u32ClassNum, &u32MaxValueIndex);
                s32ClassScore = (HI_S32)(f32MaxScore * f32ObjScore*SAMPLE_SVP_NNIE_QUANT_BASE);

                //filter low score roi
                if (s32ClassScore > u32ConfThresh) {
                    pstBbox[u32BboxNum].f32Xmin= (HI_FLOAT)(f32StartX - f32Width * 0.5f);
                    pstBbox[u32BboxNum].f32Ymin= (HI_FLOAT)(f32StartY - f32Height * 0.5f);
                    pstBbox[u32BboxNum].f32Xmax= (HI_FLOAT)(f32StartX + f32Width * 0.5f);
                    pstBbox[u32BboxNum].f32Ymax= (HI_FLOAT)(f32StartY + f32Height * 0.5f);
                    pstBbox[u32BboxNum].s32ClsScore = s32ClassScore;
                    pstBbox[u32BboxNum].u32Mask= 0;
                    pstBbox[u32BboxNum].u32ClassIdx = (HI_S32)(u32MaxValueIndex+1);
                    u32BboxNum++;
                }
            }
        }
    }

    //quick sort
    (void)SVP_NNIE_Yolo3_NonRecursiveArgQuickSort((HI_S32*)pstBbox, 0, u32BboxNum - 1,
        sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S)/sizeof(HI_U32),4,(SAMPLE_SVP_NNIE_STACK_S*)ps32AssistBuf);

    //Yolov3 Nms operation
    (void)SVP_NNIE_Yolov3_NonMaxSuppression(pstBbox, u32BboxNum, u32NmsThresh, sizeof(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S)/sizeof(HI_U32));

    //Get result
    for (i = 1; i < u32ClassNum; i++) {
        u32ClassRoiNum = 0;
        for(j = 0; j < u32BboxNum; j++) {
            if ((0 == pstBbox[j].u32Mask) && (i == pstBbox[j].u32ClassIdx) && (u32ClassRoiNum < u32MaxRoiNum)) {
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Xmin*u32SrcWidth), 0);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MAX((HI_S32)(pstBbox[j].f32Ymin*u32SrcHeight), 0);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Xmax*u32SrcWidth), u32SrcWidth);
                *(ps32DstRoi++) = SAMPLE_SVP_NNIE_MIN((HI_S32)(pstBbox[j].f32Ymax*u32SrcHeight), u32SrcHeight);
                *(ps32DstScore++) = pstBbox[j].s32ClsScore;
                u32ClassRoiNum++;
            }
        }
        *(ps32ClassRoiNum+i) = u32ClassRoiNum;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_Yolov3_NonRecursiveArgQuickSort
* Description : this function is used to do quick sort
* Input :     HI_S32*  ps32Array          [IN] the array need to be sorted
*             HI_S32   s32Low             [IN] the start position of quick sort
*             HI_S32   s32High            [IN] the end position of quick sort
*             HI_U32   u32ArraySize       [IN] the element size of input array
*             HI_U32   u32ScoreIdx        [IN] the score index in array element
*             SAMPLE_SVP_NNIE_STACK_S *pstStack [IN] the buffer used to store start positions and end positions
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Yolo3_NonRecursiveArgQuickSort(HI_S32* ps32Array,
        HI_S32 s32Low, HI_S32 s32High, HI_U32 u32ArraySize,HI_U32 u32ScoreIdx,
        SAMPLE_SVP_NNIE_STACK_S *pstStack) {
    HI_S32 i = s32Low;
    HI_S32 j = s32High;
    HI_S32 s32Top = 0;
    HI_S32 s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];
    pstStack[s32Top].s32Min = s32Low;
    pstStack[s32Top].s32Max = s32High;

    while(s32Top > -1) {
        s32Low = pstStack[s32Top].s32Min;
        s32High = pstStack[s32Top].s32Max;
        i = s32Low;
        j = s32High;
        s32Top--;

        s32KeyConfidence = ps32Array[u32ArraySize * s32Low + u32ScoreIdx];

        while(i < j) {
            while((i < j) && (s32KeyConfidence > ps32Array[j * u32ArraySize + u32ScoreIdx])) {
                j--;
            }
            if(i < j) {
                SVP_NNIE_Yolov3_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                i++;
            }

            while((i < j) && (s32KeyConfidence < ps32Array[i*u32ArraySize + u32ScoreIdx])) {
                i++;
            }
            if(i < j) {
                SVP_NNIE_Yolov3_Argswap(&ps32Array[i*u32ArraySize], &ps32Array[j*u32ArraySize],u32ArraySize);
                j--;
            }
        }

        if(s32Low < i-1) {
            s32Top++;
            pstStack[s32Top].s32Min = s32Low;
            pstStack[s32Top].s32Max = i-1;
        }

        if(s32High > i+1) {
            s32Top++;
            pstStack[s32Top].s32Min = i+1;
            pstStack[s32Top].s32Max = s32High;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov3_NonMaxSuppression
*   Description  : Yolov3 NonMaxSuppression function
* Input :     SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox [IN]  input bbox
*              HI_U32    u32BoxNum       [IN]  Bbox num
*              HI_U32    u32ClassNum     [IN]  Class num
*              HI_U32    u32NmsThresh    [IN]  NMS thresh
*              HI_U32    u32BboxNum      [IN]  bbox num
*              HI_U32    u32MaxRoiNum    [IN]  max roi num
*
*   Output       :
*   Return Value : HI_SUCCESS: Success;Error codes: Failure.
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SVP_NNIE_Yolov3_NonMaxSuppression( SAMPLE_SVP_NNIE_YOLOV3_BBOX_S* pstBbox,
    HI_U32 u32BboxNum, HI_U32 u32NmsThresh,HI_U32 u32MaxRoiNum) {
    HI_U32 i,j;
    HI_U32 u32Num = 0;
    HI_DOUBLE f64Iou = 0.0;

    for (i = 0; i < u32BboxNum && u32Num < u32MaxRoiNum; i++) {
        if(pstBbox[i].u32Mask == 0 ) {
            u32Num++;
            for(j= i+1;j< u32BboxNum; j++) {
                if( pstBbox[j].u32Mask == 0 ) {
                    f64Iou = SVP_NNIE_Yolov3_Iou(&pstBbox[i],&pstBbox[j]);
                    if(f64Iou >= (HI_DOUBLE)u32NmsThresh/SAMPLE_SVP_NNIE_QUANT_BASE) {
                        pstBbox[j].u32Mask = 1;
                    }
                }
            }
        }
    }

    return HI_SUCCESS;
}

/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov3_Iou
*   Description  : Yolov3 IOU
* Input :     SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox1 [IN]  first bbox
*              SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox2 [IN]  second bbox
*              HI_U32    u32ClassNum     [IN]  Class num
*              HI_U32    u32GridNum      [IN]  grid num
*              HI_U32    u32BboxNum      [IN]  bbox num
*              HI_U32    u32ConfThresh   [IN]  confidence thresh
*              HI_U32    u32NmsThresh    [IN]  Nms thresh
*              HI_U32    u32OriImgWidth  [IN]  input image width
*              HI_U32    u32OriImgHeight [IN]  input image height
*              HI_U32*   pu32MemPool     [IN]  assist buffer
*              HI_S32    *ps32DstScores  [OUT]  dst score of ROI
*              HI_S32    *ps32DstRoi     [OUT]  dst Roi
*              HI_S32    *ps32ClassRoiNum[OUT]  dst roi num of each class
*
*   Output       :
* Return Value : HI_DOUBLE: IOU result
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_DOUBLE SVP_NNIE_Yolov3_Iou(SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox1,
    SAMPLE_SVP_NNIE_YOLOV3_BBOX_S *pstBbox2) {
    HI_FLOAT InterWidth = 0.0;
    HI_FLOAT InterHeight = 0.0;
    HI_DOUBLE f64InterArea = 0.0;
    HI_DOUBLE f64Box1Area = 0.0;
    HI_DOUBLE f64Box2Area = 0.0;
    HI_DOUBLE f64UnionArea = 0.0;

    InterWidth =  SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Xmax, pstBbox2->f32Xmax) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Xmin,pstBbox2->f32Xmin);
    InterHeight = SAMPLE_SVP_NNIE_MIN(pstBbox1->f32Ymax, pstBbox2->f32Ymax) - SAMPLE_SVP_NNIE_MAX(pstBbox1->f32Ymin,pstBbox2->f32Ymin);

    if(InterWidth <= 0 || InterHeight <= 0) return 0;

    f64InterArea = InterWidth * InterHeight;
    f64Box1Area = (pstBbox1->f32Xmax - pstBbox1->f32Xmin)* (pstBbox1->f32Ymax - pstBbox1->f32Ymin);
    f64Box2Area = (pstBbox2->f32Xmax - pstBbox2->f32Xmin)* (pstBbox2->f32Ymax - pstBbox2->f32Ymin);
    f64UnionArea = f64Box1Area + f64Box2Area - f64InterArea;

    return f64InterArea/f64UnionArea;
}

/*****************************************************************************
*   Prototype    : SVP_NNIE_Yolov3_GetMaxVal
*   Description  : Yolov3 get max score value
* Input :     HI_FLOAT *pf32Val           [IN]  input score
*              HI_U32    u32Num            [IN]  score num
*              HI_U32 *  pu32MaxValueIndex [OUT] the class index of max score
*
*   Output       :
*   Return Value : HI_FLOAT: max score.
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_FLOAT SVP_NNIE_Yolov3_GetMaxVal(HI_FLOAT *pf32Val,HI_U32 u32Num,
    HI_U32 * pu32MaxValueIndex) {
    HI_U32 i = 0;
    HI_FLOAT f32MaxTmp = 0;

    f32MaxTmp = pf32Val[0];
    *pu32MaxValueIndex = 0;
    for(i = 1;i < u32Num;i++) {
        if(pf32Val[i] > f32MaxTmp) {
            f32MaxTmp = pf32Val[i];
            *pu32MaxValueIndex = i;
        }
    }

    return f32MaxTmp;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_Yolov3_Argswap
* Description : this function is used to exchange data
* Input :     HI_S32*  ps32Src1           [IN] first input array
*             HI_S32*  ps32Src2           [IN] second input array
*             HI_U32  u32ArraySize        [IN] array size
*
*
* Output :
* Return Value : HI_SUCCESS: Success;Error codes: Failure.
* Spec :
* Calls :
* Called By :
* History:
*
* 1. Date : 2017-11-10
* Author :
* Modification : Create
*
*****************************************************************************/
void SVP_NNIE_Yolov3_Argswap(HI_S32* ps32Src1, HI_S32* ps32Src2,
    HI_U32 u32ArraySize) {
    HI_U32 i = 0;
    HI_S32 s32Tmp = 0;
    for( i = 0; i < u32ArraySize; i++ ) {
        s32Tmp = ps32Src1[i];
        ps32Src1[i] = ps32Src2[i];
        ps32Src2[i] = s32Tmp;
    }
}