#include "hi_comm_vb.h"
#include "hi_nnie.h"
#include "mpi_sys.h"
#include "mpi_nnie.h"
#include "parameters.h"
#include "common.h"

HI_VOID SAMPLE_COMM_SVP_CheckSysInit(HI_VOID) {
	if(s_bSampleSvpInit == HI_FALSE) {
        if (SAMPLE_COMM_SVP_SysInit()) {
            printf("SVP MPI init failed! code[%x]\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
            exit(-1);
        }
        s_bSampleSvpInit = HI_TRUE;
    }
    printf("SVP MPI init OK! code[%x]\n", SAMPLE_SVP_ERR_LEVEL_DEBUG);
}

/*
*System init
*/
HI_S32 SAMPLE_COMM_SVP_SysInit(HI_VOID) {
    HI_S32 s32Ret = HI_FAILURE;
    VB_CONFIG_S struVbConf;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    memset(&struVbConf,0,sizeof(VB_CONFIG_S));

    struVbConf.u32MaxPoolCnt             = 2;
    struVbConf.astCommPool[1].u64BlkSize = 768*576*2;
    struVbConf.astCommPool[1].u32BlkCnt  = 1;

    s32Ret = HI_MPI_VB_SetConfig((const VB_CONFIG_S *)&struVbConf);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_SetConf failed!\n", s32Ret);

    s32Ret = HI_MPI_VB_Init();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Init failed!\n", s32Ret);

    s32Ret = HI_MPI_SYS_Init();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Init failed!\n", s32Ret);

    return s32Ret;
}

HI_S32 SAMPLE_COMM_SVP_MallocMem(HI_CHAR *pszMmb, HI_CHAR *pszZone, HI_U64 *pu64PhyAddr, HI_VOID **ppvVirAddr, HI_U32 u32Size) {
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_SYS_MmzAlloc(pu64PhyAddr, ppvVirAddr, pszMmb, pszZone, u32Size);
	return s32Ret;
}

HI_S32 SAMPLE_COMM_SVP_NNIE_LoadModel(HI_CHAR * pszModelFile, SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel) {
	HI_S32 s32Ret = HI_INVALID_VALUE;
	HI_U64 u64PhyAddr = 0;
	HI_U8* pu8VirAddr = NULL;
	HI_SL slFileSize = 0;
	/*Get model file size*/
	FILE* fp=fopen(pszModelFile,"rb");
    if (fp == NULL) {
        printf("Error, open model file failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
    }
	// SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error, open model file failed!\n");
    s32Ret = fseek(fp,0L,SEEK_END);
    if (s32Ret == -1) {
        printf("Error, fseek failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto FAIL_0;
    }
	// SAMPLE_SVP_CHECK_EXPR_GOTO(-1 == s32Ret,FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error, fseek failed!\n");
    slFileSize = ftell(fp);
    if (slFileSize <= 0) {
        printf("Error, ftell failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto FAIL_0;
    }
	// SAMPLE_SVP_CHECK_EXPR_GOTO(slFileSize <= 0,FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error, ftell failed!\n");
	s32Ret = fseek(fp, 0L, SEEK_SET);
    if (slFileSize <= 0) {
        printf("Error, fseek failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto FAIL_0;
    }
	// SAMPLE_SVP_CHECK_EXPR_GOTO(-1 == s32Ret,FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error, fseek failed!\n");
//||0909
	/*malloc model file mem*/
	s32Ret = SAMPLE_COMM_SVP_MallocMem("SAMPLE_NNIE_MODEL",NULL,(HI_U64*)&u64PhyAddr,(void**)&pu8VirAddr,slFileSize);
    if (s32Ret != HI_SUCCESS) {
        printf("Error, Malloc memory failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto FAIL_0;
    }
	// SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL_0,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x),Malloc memory failed!\n",s32Ret);

	pstNnieModel->stModelBuf.u32Size = (HI_U32)slFileSize;
	pstNnieModel->stModelBuf.u64PhyAddr = u64PhyAddr;
	pstNnieModel->stModelBuf.u64VirAddr = (HI_U64)(HI_UL)pu8VirAddr;

    s32Ret = fread(pu8VirAddr,slFileSize,1,fp);
	SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL_1,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,read model file failed!\n");

	/*load model*/
	s32Ret = HI_MPI_SVP_NNIE_LoadModel(&pstNnieModel->stModelBuf,&pstNnieModel->stModel);
    if (s32Ret != HI_SUCCESS) {
        printf("Error, HI_MPI_SVP_NNIE_LoadModel failed! code:%x\n", SAMPLE_SVP_ERR_LEVEL_ERROR);
        goto FAIL_1;
    }
	// SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL_1,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,HI_MPI_SVP_NNIE_LoadModel failed!\n");

	fclose(fp);
    return HI_SUCCESS;
FAIL_1:
    SAMPLE_SVP_MMZ_FREE(pstNnieModel->stModelBuf.u64PhyAddr,pstNnieModel->stModelBuf.u64VirAddr);
    pstNnieModel->stModelBuf.u32Size  = 0;
FAIL_0:
    if (NULL != fp) {
        fclose(fp);
    }

	return HI_FAILURE;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_ParamInit
*   Description  : Fill info of NNIE Forward parameters
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg    NNIE configure parameter
* 		            SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam	 NNIE parameters
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-03-14
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam) {
	HI_U32 i = 0, j = 0;
	HI_U32 u32TotalSize = 0;
	HI_U32 u32TotalTaskBufSize = 0;
	HI_U32 u32TmpBufSize = 0;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32Offset = 0;
	HI_U64 u64PhyAddr = 0;
	HI_U8 *pu8VirAddr = NULL;
	SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[SVP_NNIE_MAX_NET_SEG_NUM] = {0};

	/*fill forward info*/
	s32Ret = SAMPLE_SVP_NNIE_FillForwardInfo(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_FillForwardCtrl failed!\n");

	/*Get taskInfo and Blob mem size*/
	s32Ret = SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(pstNnieCfg,pstNnieParam,&u32TotalTaskBufSize,
		&u32TmpBufSize,astBlobSize,&u32TotalSize);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize failed!\n");

	/*Malloc mem*/
	s32Ret = SAMPLE_COMM_SVP_MallocCached("SAMPLE_NNIE_TASK",NULL,(HI_U64*)&u64PhyAddr,(void**)&pu8VirAddr,u32TotalSize);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error,Malloc memory failed!\n");
	memset(pu8VirAddr, 0, u32TotalSize);
	SAMPLE_COMM_SVP_FlushCache(u64PhyAddr,(void*)pu8VirAddr,u32TotalSize);

	/*fill taskinfo mem addr*/
	pstNnieParam->stTaskBuf.u32Size = u32TotalTaskBufSize;
	pstNnieParam->stTaskBuf.u64PhyAddr = u64PhyAddr;
	pstNnieParam->stTaskBuf.u64VirAddr = (HI_U64)(HI_UL)pu8VirAddr;

	/*fill Tmp mem addr*/
	pstNnieParam->stTmpBuf.u32Size = u32TmpBufSize;
	pstNnieParam->stTmpBuf.u64PhyAddr = u64PhyAddr+u32TotalTaskBufSize;
	pstNnieParam->stTmpBuf.u64VirAddr = (HI_U64)(HI_UL)pu8VirAddr+u32TotalTaskBufSize;

	/*fill forward ctrl addr*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++) {
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType) {
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType) {
			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];
	}

	/*fill each blob's mem addr*/
	u64PhyAddr =  u64PhyAddr+u32TotalTaskBufSize+u32TmpBufSize;
	pu8VirAddr = pu8VirAddr+u32TotalTaskBufSize+u32TmpBufSize;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++) {
		/*first seg has src blobs, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(0 == i) {
			for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++) {
				if(j!=0) {
					u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
					pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
				}
				pstNnieParam->astSegData[i].astSrc[j].u64PhyAddr = u64PhyAddr;
				pstNnieParam->astSegData[i].astSrc[j].u64VirAddr = (HI_U64)(HI_UL)pu8VirAddr;
			}
			u64PhyAddr += astBlobSize[i].au32SrcSize[j-1];
			pu8VirAddr += astBlobSize[i].au32SrcSize[j-1];
		}

		/*fill the mem addrs of each seg's output blobs*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++) {
			if(j!=0) {
				u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
				pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
			}
			pstNnieParam->astSegData[i].astDst[j].u64PhyAddr = u64PhyAddr;
			pstNnieParam->astSegData[i].astDst[j].u64VirAddr = (HI_U64)(HI_UL)pu8VirAddr;
		}
		u64PhyAddr += astBlobSize[i].au32DstSize[j-1];
		pu8VirAddr += astBlobSize[i].au32DstSize[j-1];
	}
	return s32Ret;
}

HI_S32 SAMPLE_COMM_SVP_NNIE_ParamInit(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam) {
	HI_S32 s32Ret = HI_SUCCESS;

    /*check*/
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieCfg || NULL == pstNnieParam),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieCfg and pstNnieParam can't be NULL!\n");
    SAMPLE_SVP_CHECK_EXPR_RET((NULL == pstNnieParam->pstModel),HI_ERR_SVP_NNIE_ILLEGAL_PARAM,
        SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,pstNnieParam->pstModel can't be NULL!\n");

	/*NNIE parameter initialization */
	s32Ret = SAMPLE_SVP_NNIE_ParamInit(pstNnieCfg,pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_SVP_NNIE_ParamInit failed!\n");

	return s32Ret;
FAIL:
	s32Ret = SAMPLE_COMM_SVP_NNIE_ParamDeinit(pstNnieParam);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, SAMPLE_COMM_SVP_NNIE_ParamDeinit failed!\n");
	return HI_FAILURE;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_FillForwardInfo
*   Description  : fill NNIE forward ctrl information
*   Input        : SAMPLE_SVP_NNIE_CFG_S   *pstNnieCfg       NNIE configure info
* 	               SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
*
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_FillForwardInfo(SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam) {
	HI_U32 i = 0, j = 0;
	HI_U32 u32Offset = 0;
	HI_U32 u32Num = 0;

	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++) {
        /*fill forwardCtrl info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType) {
			pstNnieParam->astForwardWithBboxCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardWithBboxCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardWithBboxCtrl[i].u32ProposalNum = 1;
			pstNnieParam->astForwardWithBboxCtrl[i].u32NetSegId = i;
			pstNnieParam->astForwardWithBboxCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardWithBboxCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		else if(SVP_NNIE_NET_TYPE_CNN == pstNnieParam->pstModel->astSeg[i].enNetType ||
            SVP_NNIE_NET_TYPE_RECURRENT== pstNnieParam->pstModel->astSeg[i].enNetType) {
			pstNnieParam->astForwardCtrl[i].enNnieId = pstNnieCfg->aenNnieCoreId[i];
			pstNnieParam->astForwardCtrl[i].u32SrcNum = pstNnieParam->pstModel->astSeg[i].u16SrcNum;
			pstNnieParam->astForwardCtrl[i].u32DstNum = pstNnieParam->pstModel->astSeg[i].u16DstNum;
			pstNnieParam->astForwardCtrl[i].u32NetSegId = i;
			pstNnieParam->astForwardCtrl[i].stTmpBuf = pstNnieParam->stTmpBuf;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64PhyAddr= pstNnieParam->stTaskBuf.u64PhyAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u64VirAddr= pstNnieParam->stTaskBuf.u64VirAddr+u32Offset;
			pstNnieParam->astForwardCtrl[i].stTskBuf.u32Size= pstNnieParam->au32TaskBufSize[i];
		}
		u32Offset += pstNnieParam->au32TaskBufSize[i];

        /*fill src blob info*/
		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16SrcNum; j++) {
            /*Recurrent blob*/
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType) {
                pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u32Dim = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
                pstNnieParam->astSegData[i].astSrc[j].unShape.stSeq.u64VirAddrStep = pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM];
            }
            else {
    		    pstNnieParam->astSegData[i].astSrc[j].enType = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].enType;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Chn;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Height;
    	        pstNnieParam->astSegData[i].astSrc[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astSrcNode[j].unShape.stWhc.u32Width;
    	        pstNnieParam->astSegData[i].astSrc[j].u32Num = pstNnieCfg->u32MaxInputNum;
            }
	    }

        /*fill dst blob info*/
		if(SVP_NNIE_NET_TYPE_ROI == pstNnieParam->pstModel->astSeg[i].enNetType) {
			u32Num = pstNnieCfg->u32MaxRoiNum*pstNnieCfg->u32MaxInputNum;
		}
		else {
			u32Num = pstNnieCfg->u32MaxInputNum;
		}

		for(j = 0; j < pstNnieParam->pstModel->astSeg[i].u16DstNum; j++) {
            if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType) {
    			pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    			pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u32Dim =
                    pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.u32Dim;
                pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
                pstNnieParam->astSegData[i].astDst[j].unShape.stSeq.u64VirAddrStep =
                    pstNnieCfg->au64StepVirAddr[i*SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM+1];
            }
            else {
    		    pstNnieParam->astSegData[i].astDst[j].enType = pstNnieParam->pstModel->astSeg[i].astDstNode[j].enType;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Chn = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Chn;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Height = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Height;
    		    pstNnieParam->astSegData[i].astDst[j].unShape.stWhc.u32Width = pstNnieParam->pstModel->astSeg[i].astDstNode[j].unShape.stWhc.u32Width;
    		    pstNnieParam->astSegData[i].astDst[j].u32Num = u32Num;
            }
		}
	}
	return HI_SUCCESS;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize
*   Description  : Get taskinfo and blob memory size
*   Input        : SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam     NNIE parameter
* 	                HI_U32                  *pu32TaskInfoSize Task info size
*                  HI_U32                  *pu32TmpBufSize    Tmp buffer size
*                  SAMPLE_SVP_NNIE_BLOB_SIZE_S  astBlobSize[] each seg input and output blob mem size
*                  HI_U32                  *pu32TotalSize     Total mem size
*
*
*   Output       :
*   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_GetTaskAndBlobBufSize(
                SAMPLE_SVP_NNIE_CFG_S *pstNnieCfg, 
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                HI_U32*pu32TotalTaskBufSize, 
                HI_U32*pu32TmpBufSize,
                SAMPLE_SVP_NNIE_BLOB_SIZE_S astBlobSize[],
                HI_U32*pu32TotalSize) {
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 i = 0, j = 0;
    HI_U32 u32TotalStep = 0;

	/*Get each seg's task buf size*/
	s32Ret = HI_MPI_SVP_NNIE_GetTskBufSize(pstNnieCfg->u32MaxInputNum, pstNnieCfg->u32MaxRoiNum,
		pstNnieParam->pstModel, pstNnieParam->au32TaskBufSize,pstNnieParam->pstModel->u32NetSegNum);
	SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error,HI_MPI_SVP_NNIE_GetTaskSize failed!\n");

    /*Get total task buf size*/
	*pu32TotalTaskBufSize = 0;
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++) {
		*pu32TotalTaskBufSize += pstNnieParam->au32TaskBufSize[i];
	}

	/*Get tmp buf size*/
	*pu32TmpBufSize = pstNnieParam->pstModel->u32TmpBufSize;
	*pu32TotalSize += *pu32TotalTaskBufSize + *pu32TmpBufSize;

	/*calculate Blob mem size*/
	for(i = 0; i < pstNnieParam->pstModel->u32NetSegNum; i++) {
        if(SVP_NNIE_NET_TYPE_RECURRENT == pstNnieParam->pstModel->astSeg[i].enNetType) {
            for(j = 0; j < pstNnieParam->astSegData[i].astSrc[0].u32Num; j++) {
                u32TotalStep += *((HI_S32*)(HI_UL)pstNnieParam->astSegData[i].astSrc[0].unShape.stSeq.u64VirAddrStep+j);
            }
        }
		/*the first seg's Src Blob mem size, other seg's src blobs from the output blobs of
		those segs before it or from software output results*/
		if(i == 0) {
			SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astSrcNode[0]),
				pstNnieParam->pstModel->astSeg[i].u16SrcNum,u32TotalStep,&(pstNnieParam->astSegData[i].astSrc[0]),
				SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32SrcSize[0]));
		}

		/*Get each seg's Dst Blob mem size*/
		SAMPLE_SVP_NNIE_GetBlobMemSize(&(pstNnieParam->pstModel->astSeg[i].astDstNode[0]),
			pstNnieParam->pstModel->astSeg[i].u16DstNum,u32TotalStep,&(pstNnieParam->astSegData[i].astDst[0]),
			SAMPLE_SVP_NNIE_ALIGN_16, pu32TotalSize, &(astBlobSize[i].au32DstSize[0]));
	}
	return s32Ret;
}

/*****************************************************************************
*   Prototype    : SAMPLE_SVP_NNIE_GetBlobMemSize
*   Description  : Get blob mem size
*   Input        : SVP_NNIE_NODE_S astNnieNode[]   NNIE Node
*                  HI_U32          u32NodeNum      Node num
*                  HI_U32          astBlob[]       blob struct
*                  HI_U32          u32Align        stride align type
*                  HI_U32          *pu32TotalSize  Total size
*                  HI_U32          au32BlobSize[]  blob size
*
*
*
*
*   Output       :
*   Return Value : VOID
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
void SAMPLE_SVP_NNIE_GetBlobMemSize(
                SVP_NNIE_NODE_S astNnieNode[], 
                HI_U32 u32NodeNum,
                HI_U32 u32TotalStep,
                SVP_BLOB_S astBlob[], 
                HI_U32 u32Align, 
                HI_U32* pu32TotalSize,
                HI_U32 au32BlobSize[]) {
	HI_U32 i = 0;
	HI_U32 u32Size = 0;
	HI_U32 u32Stride = 0;

	for(i = 0; i < u32NodeNum; i++) {
		if(SVP_BLOB_TYPE_S32== astNnieNode[i].enType||SVP_BLOB_TYPE_VEC_S32== astNnieNode[i].enType||
            SVP_BLOB_TYPE_SEQ_S32== astNnieNode[i].enType) {
			u32Size = sizeof(HI_U32);
		}
		else {
			u32Size = sizeof(HI_U8);
		}
        if(SVP_BLOB_TYPE_SEQ_S32 == astNnieNode[i].enType) {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align) {
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
    		else {
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.u32Dim*u32Size);
    		}
            au32BlobSize[i] = u32TotalStep*u32Stride;
        }
        else {
            if(SAMPLE_SVP_NNIE_ALIGN_16 == u32Align) {
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN16(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		else {
    			u32Stride = SAMPLE_SVP_NNIE_ALIGN32(astNnieNode[i].unShape.stWhc.u32Width*u32Size);
    		}
    		au32BlobSize[i] = astBlob[i].u32Num*u32Stride*astNnieNode[i].unShape.stWhc.u32Height*
    			astNnieNode[i].unShape.stWhc.u32Chn;
        }
		*pu32TotalSize += au32BlobSize[i];
	    astBlob[i].u32Stride = u32Stride;
	}
}

/*
*Malloc memory with cached
*/
HI_S32 SAMPLE_COMM_SVP_MallocCached(HI_CHAR *pszMmb, HI_CHAR *pszZone,HI_U64 *pu64PhyAddr, HI_VOID **ppvVirAddr, HI_U32 u32Size) {
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_SYS_MmzAlloc_Cached(pu64PhyAddr, ppvVirAddr, pszMmb, pszZone, u32Size);

	return s32Ret;
}

/*
*Fulsh cached
*/
HI_S32 SAMPLE_COMM_SVP_FlushCache(HI_U64 u64PhyAddr, HI_VOID *pvVirAddr, HI_U32 u32Size) {
	HI_S32 s32Ret = HI_SUCCESS;
	s32Ret = HI_MPI_SYS_MmzFlushCache(u64PhyAddr, pvVirAddr,u32Size);
	return s32Ret;
}

/*****************************************************************************
*   Prototype    : SAMPLE_COMM_SVP_NNIE_ParamDeinit
*   Description  : Deinit NNIE parameters
*   Input        : SAMPLE_SVP_NNIE_PARAM_S        *pstNnieParam     NNIE Parameter
*                  SAMPLE_SVP_NNIE_SOFTWARE_MEM_S *pstSoftWareMem   software mem
*   Output       :
*   Return Value :  HI_S32,HI_SUCCESS:Success,Other:failure
*   Spec         :
*   Calls        :
*   Called By    :
*   History:
*
*       1.  Date         : 2017-11-20
*           Author       :
*           Modification : Create
*
*****************************************************************************/
HI_S32 SAMPLE_COMM_SVP_NNIE_ParamDeinit(SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam) {
	SAMPLE_SVP_CHECK_EXPR_RET(NULL == pstNnieParam,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR,
		"Error, pstNnieParam can't be NULL!\n");

	if(0!=pstNnieParam->stTaskBuf.u64PhyAddr && 0!=pstNnieParam->stTaskBuf.u64VirAddr) {
		SAMPLE_SVP_MMZ_FREE(pstNnieParam->stTaskBuf.u64PhyAddr,pstNnieParam->stTaskBuf.u64VirAddr);
		pstNnieParam->stTaskBuf.u64PhyAddr = 0;
		pstNnieParam->stTaskBuf.u64VirAddr = 0;
	}

    if(0!=pstNnieParam->stStepBuf.u64PhyAddr && 0!=pstNnieParam->stStepBuf.u64VirAddr) {
		SAMPLE_SVP_MMZ_FREE(pstNnieParam->stStepBuf.u64PhyAddr,pstNnieParam->stStepBuf.u64VirAddr);
		pstNnieParam->stStepBuf.u64PhyAddr = 0;
		pstNnieParam->stStepBuf.u64VirAddr = 0;
	}
	return HI_SUCCESS;
}

/*****************************************************************************
 *   Prototype    : SAMPLE_COMM_SVP_NNIE_UnloadModel
 *   Description  : unload NNIE model
 *   Input        : SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel      NNIE Model
 *
 *
 *
 *   Output       :
 *   Return Value : HI_S32,HI_SUCCESS:Success,Other:failure
 *   Spec         :
 *   Calls        :
 *   Called By    :
 *   History:
 *
 *       1.  Date         : 2017-11-20
 *           Author       :
 *           Modification : Create
 *
 *****************************************************************************/
HI_S32 SAMPLE_COMM_SVP_NNIE_UnloadModel(SAMPLE_SVP_NNIE_MODEL_S *pstNnieModel) {
	if(0!=pstNnieModel->stModelBuf.u64PhyAddr && 0!=pstNnieModel->stModelBuf.u64VirAddr) {
		SAMPLE_SVP_MMZ_FREE(pstNnieModel->stModelBuf.u64PhyAddr,pstNnieModel->stModelBuf.u64VirAddr);
		pstNnieModel->stModelBuf.u64PhyAddr = 0;
		pstNnieModel->stModelBuf.u64VirAddr = 0;
	}
    return HI_SUCCESS;
}

/******************************************************************************
* function : Fill Src Data
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_FillSrcData(
                SAMPLE_SVP_NNIE_CFG_S* pstNnieCfg,
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam, 
                SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx) {
    FILE* fp = NULL;
    HI_U32 i =0, j = 0, n = 0;
    HI_U32 u32Height = 0, u32Width = 0, u32Chn = 0, u32Stride = 0, u32Dim = 0;
    HI_U32 u32VarSize = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U8*pu8PicAddr = NULL;
    HI_U32*pu32StepAddr = NULL;
    HI_U32 u32SegIdx = pstInputDataIdx->u32SegIdx;
    HI_U32 u32NodeIdx = pstInputDataIdx->u32NodeIdx;
    HI_U32 u32TotalStepNum = 0;

    /*open file*/
    if (NULL != pstNnieCfg->pszPic) {
        fp = fopen(pstNnieCfg->pszPic,"rb");
        SAMPLE_SVP_CHECK_EXPR_RET(NULL == fp,HI_INVALID_VALUE,SAMPLE_SVP_ERR_LEVEL_ERROR, "Error, open file failed!\n");
    }

    /*get data size*/
    if(SVP_BLOB_TYPE_U8 <= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType &&
        SVP_BLOB_TYPE_YVU422SP >= pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType) {
        u32VarSize = sizeof(HI_U8);
    }
    else {
        u32VarSize = sizeof(HI_U32);
    }

    /*fill src data*/
    if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType) {
        u32Dim = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u32Dim;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu32StepAddr = (HI_U32*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stSeq.u64VirAddrStep);
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++) {
            for(i = 0;i < *(pu32StepAddr+n); i++) {
                s32Ret = fread(pu8PicAddr,u32Dim*u32VarSize,1,fp);
                SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                pu8PicAddr += u32Stride;
            }
            u32TotalStepNum += *(pu32StepAddr+n);
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr, u32TotalStepNum*u32Stride);
    }
    else {
        u32Height = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Height;
        u32Width = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Width;
        u32Chn = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].unShape.stWhc.u32Chn;
        u32Stride = pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Stride;
        pu8PicAddr = (HI_U8*)(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr);
        if(SVP_BLOB_TYPE_YVU420SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType) {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++) {
                for(i = 0; i < u32Chn*u32Height/2; i++) {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else if(SVP_BLOB_TYPE_YVU422SP== pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].enType) {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++) {
                for(i = 0; i < u32Height*2; i++) {
                    s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                    SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                    pu8PicAddr += u32Stride;
                }
            }
        }
        else {
            for(n = 0; n < pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num; n++) {
                for(i = 0;i < u32Chn; i++) {
                    for(j = 0; j < u32Height; j++) {
                        s32Ret = fread(pu8PicAddr,u32Width*u32VarSize,1,fp);
                        SAMPLE_SVP_CHECK_EXPR_GOTO(1 != s32Ret,FAIL,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,Read image file failed!\n");
                        pu8PicAddr += u32Stride;
                    }
                }
            }
        }
        SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64PhyAddr,
            (HI_VOID *) pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u64VirAddr,
            pstNnieParam->astSegData[u32SegIdx].astSrc[u32NodeIdx].u32Num*u32Chn*u32Height*u32Stride);
    }

    fclose(fp);
    return HI_SUCCESS;
FAIL:

    fclose(fp);
    return HI_FAILURE;
}

/******************************************************************************
* function : NNIE Forward
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Forward(
                SAMPLE_SVP_NNIE_PARAM_S *pstNnieParam,
                SAMPLE_SVP_NNIE_INPUT_DATA_INDEX_S* pstInputDataIdx,
                SAMPLE_SVP_NNIE_PROCESS_SEG_INDEX_S* pstProcSegIdx,
                HI_BOOL bInstant) {
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 i = 0, j = 0;
    HI_BOOL bFinish = HI_FALSE;
    SVP_NNIE_HANDLE hSvpNnieHandle = 0;
    HI_U32 u32TotalStepNum = 0;

    SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64PhyAddr,
        (HI_VOID *) pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u64VirAddr,
        pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].stTskBuf.u32Size);

    /*set input blob according to node name*/
    if(pstInputDataIdx->u32SegIdx != pstProcSegIdx->u32SegIdx) {
        for(i = 0; i < pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].u16SrcNum; i++) {
            for(j = 0; j < pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum; j++) {
                if(0 == strncmp(pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].astDstNode[j].szName,
                    pstNnieParam->pstModel->astSeg[pstProcSegIdx->u32SegIdx].astSrcNode[i].szName,
                    SVP_NNIE_NODE_NAME_LEN)) {
                    pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc[i] =
                        pstNnieParam->astSegData[pstInputDataIdx->u32SegIdx].astDst[j];
                    break;
                }
            }
            SAMPLE_SVP_CHECK_EXPR_RET((j == pstNnieParam->pstModel->astSeg[pstInputDataIdx->u32SegIdx].u16DstNum),
                HI_FAILURE,SAMPLE_SVP_ERR_LEVEL_ERROR,"Error,can't find %d-th seg's %d-th src blob!\n",
                pstProcSegIdx->u32SegIdx,i);
        }
    }

    /*NNIE_Forward*/
    s32Ret = HI_MPI_SVP_NNIE_Forward(&hSvpNnieHandle,
        pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astSrc,
        pstNnieParam->pstModel, pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst,
        &pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx], bInstant);
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret,s32Ret,SAMPLE_SVP_ERR_LEVEL_ERROR,
        "Error,HI_MPI_SVP_NNIE_Forward failed!\n");

    if(bInstant) {
        /*Wait NNIE finish*/
        while(HI_ERR_SVP_NNIE_QUERY_TIMEOUT == (s32Ret = HI_MPI_SVP_NNIE_Query(pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].enNnieId,
            hSvpNnieHandle, &bFinish, HI_TRUE))) {
            usleep(100);
            SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_INFO,
                "HI_MPI_SVP_NNIE_Query Query timeout!\n");
        }
    }

    bFinish = HI_FALSE;
    for(i = 0; i < pstNnieParam->astForwardCtrl[pstProcSegIdx->u32SegIdx].u32DstNum; i++) {
        if(SVP_BLOB_TYPE_SEQ_S32 == pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].enType) {
            for(j = 0; j < pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num; j++) {
                u32TotalStepNum += *((HI_U32*)(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stSeq.u64VirAddrStep)+j);
            }
            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                u32TotalStepNum*pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);

        }
        else {

            SAMPLE_COMM_SVP_FlushCache(pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64PhyAddr,
                (HI_VOID *) pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u64VirAddr,
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Num*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Chn*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].unShape.stWhc.u32Height*
                pstNnieParam->astSegData[pstProcSegIdx->u32SegIdx].astDst[i].u32Stride);
        }
    }

    return s32Ret;
}

/******************************************************************************
* function : print detection result
******************************************************************************/
HI_S32 SAMPLE_SVP_NNIE_Detection_PrintResult(
                SVP_BLOB_S *pstDstScore,
                SVP_BLOB_S *pstDstRoi, 
                SVP_BLOB_S *pstClassRoiNum, 
                HI_FLOAT f32PrintResultThresh) {
    HI_U32 i = 0, j = 0;
    HI_U32 u32RoiNumBias = 0;
    HI_U32 u32ScoreBias = 0;
    HI_U32 u32BboxBias = 0;
    HI_FLOAT f32Score = 0.0f;
    HI_S32* ps32Score = (HI_S32*)pstDstScore->u64VirAddr;
    HI_S32* ps32Roi = (HI_S32*)pstDstRoi->u64VirAddr;
    HI_S32* ps32ClassRoiNum = (HI_S32*)pstClassRoiNum->u64VirAddr;
    HI_U32 u32ClassNum = pstClassRoiNum->unShape.stWhc.u32Width;
    HI_S32 s32XMin = 0,s32YMin= 0,s32XMax = 0,s32YMax = 0;

    u32RoiNumBias += ps32ClassRoiNum[0];
    for (i = 1; i < u32ClassNum; i++) {
        u32ScoreBias = u32RoiNumBias;
        u32BboxBias = u32RoiNumBias * SAMPLE_SVP_NNIE_COORDI_NUM;
        /*if the confidence score greater than result threshold, the result will be printed*/
        if((HI_FLOAT)ps32Score[u32ScoreBias] / SAMPLE_SVP_NNIE_QUANT_BASE >=
            f32PrintResultThresh && ps32ClassRoiNum[i]!=0) {
            SAMPLE_SVP_TRACE_INFO("==== The %dth class box info====\n", i);
        }
        for (j = 0; j < (HI_U32)ps32ClassRoiNum[i]; j++) {
            f32Score = (HI_FLOAT)ps32Score[u32ScoreBias + j] / SAMPLE_SVP_NNIE_QUANT_BASE;
            if (f32Score < f32PrintResultThresh) {
                break;
            }
            s32XMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM];
            s32YMin = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 1];
            s32XMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 2];
            s32YMax = ps32Roi[u32BboxBias + j*SAMPLE_SVP_NNIE_COORDI_NUM + 3];
            SAMPLE_SVP_TRACE_INFO("%d %d %d %d %f\n", s32XMin, s32YMin, s32XMax, s32YMax, f32Score);
        }
        u32RoiNumBias += ps32ClassRoiNum[i];
    }
    return HI_SUCCESS;
}

HI_VOID SAMPLE_COMM_SVP_CheckSysExit(HI_VOID) {
    if ( s_bSampleSvpInit) {
        SAMPLE_COMM_SVP_SysExit();
        s_bSampleSvpInit = HI_FALSE;
    }

	SAMPLE_SVP_TRACE(SAMPLE_SVP_ERR_LEVEL_DEBUG,"Svp mpi exit ok!\n");
}

/*
*System exit
*/
HI_S32 SAMPLE_COMM_SVP_SysExit(HI_VOID) {
	HI_S32 s32Ret = HI_FAILURE;

	s32Ret = HI_MPI_SYS_Exit();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_SYS_Exit failed!\n", s32Ret);

	s32Ret = HI_MPI_VB_Exit();
    SAMPLE_SVP_CHECK_EXPR_RET(HI_SUCCESS != s32Ret, s32Ret, SAMPLE_SVP_ERR_LEVEL_ERROR, "Error(%#x):HI_MPI_VB_Exit failed!\n", s32Ret);

	return HI_SUCCESS;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_SoftMax
* Description : this function is used to do softmax
* Input :     HI_FLOAT*         pf32Src           [IN]   the pointer to source data
*             HI_U32             u32Num           [IN]   the num of source data
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
// HI_S32 SVP_NNIE_SoftMax(HI_FLOAT* pf32Src, HI_U32 u32Num);
HI_S32 SVP_NNIE_SoftMax(HI_FLOAT* pf32Src, HI_U32 u32Num) {
    HI_FLOAT f32Max = 0;
    HI_FLOAT f32Sum = 0;
    HI_U32 i = 0;

    for(i = 0; i < u32Num; ++i) {
        if(f32Max < pf32Src[i]) {
            f32Max = pf32Src[i];
        }
    }

    for(i = 0; i < u32Num; ++i) {
        pf32Src[i] = (HI_FLOAT)SVP_NNIE_QuickExp((HI_S32)((pf32Src[i] - f32Max)*SAMPLE_SVP_NNIE_QUANT_BASE));
        f32Sum += pf32Src[i];
    }

    for(i = 0; i < u32Num; ++i) {
        pf32Src[i] /= f32Sum;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* Prototype :   SVP_NNIE_QuickExp
* Description : this function is used to quickly get exp result
* Input :     HI_S32    s32Value           [IN]   input value
*
*
*
*
* Output :
* Return Value : HI_FLOAT: output value.
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
HI_FLOAT SVP_NNIE_QuickExp( HI_S32 s32Value ) {
    if( s32Value & 0x80000000 ) {
        s32Value = ~s32Value + 0x00000001;
        return s_af32ExpCoef[5][s32Value & 0x0000000F] * s_af32ExpCoef[6][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[7][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[8][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[9][(s32Value>>16) & 0x0000000F ];
    }
    else {
        return s_af32ExpCoef[0][s32Value & 0x0000000F] * s_af32ExpCoef[1][(s32Value>>4) & 0x0000000F] * s_af32ExpCoef[2][(s32Value>>8) & 0x0000000F] * s_af32ExpCoef[3][(s32Value>>12) & 0x0000000F] * s_af32ExpCoef[4][(s32Value>>16) & 0x0000000F ];
    }
}