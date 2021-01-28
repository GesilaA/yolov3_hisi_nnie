#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
// =================================================================

#define SAMPLE_SVP_NNIE_EACH_SEG_STEP_ADDR_NUM    2
#define SAMPLE_SVP_NNIE_COORDI_NUM  4      /*coordinate numbers*/
#define SAMPLE_SVP_NNIE_QUANT_BASE 4096    /*the base value*/

/*16Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_16 16
#define SAMPLE_SVP_NNIE_ALIGN16(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_16-1) / SAMPLE_SVP_NNIE_ALIGN_16*SAMPLE_SVP_NNIE_ALIGN_16)
/*32Byte align*/
#define SAMPLE_SVP_NNIE_ALIGN_32 32
#define SAMPLE_SVP_NNIE_ALIGN32(u32Num) ((u32Num + SAMPLE_SVP_NNIE_ALIGN_32-1) / SAMPLE_SVP_NNIE_ALIGN_32*SAMPLE_SVP_NNIE_ALIGN_32)

typedef enum hiSAMPLE_SVP_ERR_LEVEL_E
{
	SAMPLE_SVP_ERR_LEVEL_DEBUG	 = 0x0,    // debug-level								   
	SAMPLE_SVP_ERR_LEVEL_INFO	 = 0x1,    // informational 							   
	SAMPLE_SVP_ERR_LEVEL_NOTICE  = 0x2,    // normal but significant condition			   
	SAMPLE_SVP_ERR_LEVEL_WARNING = 0x3,    // warning conditions						   
	SAMPLE_SVP_ERR_LEVEL_ERROR	 = 0x4,    // error conditions							   
	SAMPLE_SVP_ERR_LEVEL_CRIT	 = 0x5,    // critical conditions						   
	SAMPLE_SVP_ERR_LEVEL_ALERT	 = 0x6,    // action must be taken immediately			   
	SAMPLE_SVP_ERR_LEVEL_FATAL   = 0x7,	   // just for compatibility with previous version 

	SAMPLE_SVP_ERR_LEVEL_BUTT
}SAMPLE_SVP_ERR_LEVEL_E;

static HI_FLOAT s_af32ExpCoef[10][16] = {
    {1.0f, 1.00024f, 1.00049f, 1.00073f, 1.00098f, 1.00122f, 1.00147f, 1.00171f, 1.00196f, 1.0022f, 1.00244f, 1.00269f, 1.00293f, 1.00318f, 1.00342f, 1.00367f},
    {1.0f, 1.00391f, 1.00784f, 1.01179f, 1.01575f, 1.01972f, 1.02371f, 1.02772f, 1.03174f, 1.03578f, 1.03984f, 1.04391f, 1.04799f, 1.05209f, 1.05621f, 1.06034f},
    {1.0f, 1.06449f, 1.13315f, 1.20623f, 1.28403f, 1.36684f, 1.45499f, 1.54883f, 1.64872f, 1.75505f, 1.86825f, 1.98874f, 2.117f, 2.25353f, 2.39888f, 2.55359f},
    {1.0f, 2.71828f, 7.38906f, 20.0855f, 54.5981f, 148.413f, 403.429f, 1096.63f, 2980.96f, 8103.08f, 22026.5f, 59874.1f, 162755.0f, 442413.0f, 1.2026e+006f, 3.26902e+006f},
    {1.0f, 8.88611e+006f, 7.8963e+013f, 7.01674e+020f, 6.23515e+027f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f, 5.54062e+034f},
    {1.0f, 0.999756f, 0.999512f, 0.999268f, 0.999024f, 0.99878f, 0.998536f, 0.998292f, 0.998049f, 0.997805f, 0.997562f, 0.997318f, 0.997075f, 0.996831f, 0.996588f, 0.996345f},
    {1.0f, 0.996101f, 0.992218f, 0.98835f, 0.984496f, 0.980658f, 0.976835f, 0.973027f, 0.969233f, 0.965455f, 0.961691f, 0.957941f, 0.954207f, 0.950487f, 0.946781f, 0.94309f},
    {1.0f, 0.939413f, 0.882497f, 0.829029f, 0.778801f, 0.731616f, 0.687289f, 0.645649f, 0.606531f, 0.569783f, 0.535261f, 0.502832f, 0.472367f, 0.443747f, 0.416862f, 0.391606f},
    {1.0f, 0.367879f, 0.135335f, 0.0497871f, 0.0183156f, 0.00673795f, 0.00247875f, 0.000911882f, 0.000335463f, 0.00012341f, 4.53999e-005f, 1.67017e-005f, 6.14421e-006f, 2.26033e-006f, 8.31529e-007f, 3.05902e-007f},
    {1.0f, 1.12535e-007f, 1.26642e-014f, 1.42516e-021f, 1.60381e-028f, 1.80485e-035f, 2.03048e-042f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}
};

// ====================== Define function below =============================

#define SAMPLE_SVP_NNIE_MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define SAMPLE_SVP_NNIE_MIN(a,b)    (((a) < (b)) ? (a) : (b))
#define SAMPLE_SVP_NNIE_SIGMOID(x)   (HI_FLOAT)(1.0f/(1+exp(-x)))

// ====================== Define method below ============================

#define SAMPLE_SVP_PRINTF(LevelStr,Msg, ...) do { fprintf(stderr,"[Level]:%s,[Func]:%s [Line]:%d [Info]:"Msg,LevelStr, __FUNCTION__, __LINE__,## __VA_ARGS__); } while (0)
#define SAMPLE_SVP_PRINTF_RED(LevelStr,Msg, ...) do { fprintf(stderr,"\033[0;31m [Level]:%s,[Func]:%s [Line]:%d [Info]:"Msg"\033[0;39m\n",LevelStr, __FUNCTION__, __LINE__,## __VA_ARGS__); } while (0)
/* system is unusable	*/
#define SAMPLE_SVP_TRACE_FATAL(Msg,...)   SAMPLE_SVP_PRINTF_RED("Fatal",Msg,##__VA_ARGS__)
/* action must be taken immediately */
#define SAMPLE_SVP_TRACE_ALERT(Msg,...)   SAMPLE_SVP_PRINTF_RED("Alert",Msg,##__VA_ARGS__)
/* critical conditions */
#define SAMPLE_SVP_TRACE_CRIT(Msg,...)    SAMPLE_SVP_PRINTF_RED("Critical",Msg,##__VA_ARGS__)
/* error conditions */
#define SAMPLE_SVP_TRACE_ERR(Msg,...)     SAMPLE_SVP_PRINTF_RED("Error",Msg,##__VA_ARGS__)
/* warning conditions */
#define SAMPLE_SVP_TRACE_WARN(Msg,...)    SAMPLE_SVP_PRINTF("Warning",Msg,##__VA_ARGS__)
/* normal but significant condition  */
#define SAMPLE_SVP_TRACE_NOTICE(Msg,...)  SAMPLE_SVP_PRINTF("Notice",Msg,##__VA_ARGS__)
/* informational */
#define SAMPLE_SVP_TRACE_INFO(Msg,...)    SAMPLE_SVP_PRINTF("Info",Msg,##__VA_ARGS__)
/* debug-level messages  */
#define SAMPLE_SVP_TRACE_DEBUG(Msg, ...)  SAMPLE_SVP_PRINTF("Debug",Msg,##__VA_ARGS__)

#define SAMPLE_SVP_TRACE(Level,Msg, ...)                        \
do {                                                            \
	switch(Level) {                                             \
        case SAMPLE_SVP_ERR_LEVEL_DEBUG:                        \
            SAMPLE_SVP_TRACE_DEBUG(Msg,##__VA_ARGS__);          \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_INFO:                         \
            SAMPLE_SVP_TRACE_INFO(Msg,##__VA_ARGS__);           \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_NOTICE:                       \
            SAMPLE_SVP_TRACE_NOTICE(Msg,##__VA_ARGS__);         \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_WARNING:                      \
            SAMPLE_SVP_TRACE_WARN(Msg,##__VA_ARGS__);           \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_ERROR:                        \
            SAMPLE_SVP_TRACE_ERR(Msg,##__VA_ARGS__);            \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_CRIT:                         \
            SAMPLE_SVP_TRACE_CRIT(Msg,##__VA_ARGS__);           \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_ALERT:                        \
            SAMPLE_SVP_TRACE_ALERT(Msg,##__VA_ARGS__);          \
            break;                                              \
        case SAMPLE_SVP_ERR_LEVEL_FATAL:                        \
            SAMPLE_SVP_TRACE_FATAL(Msg,##__VA_ARGS__);          \
            break;                                              \
        default:                                                \
            break;                                              \
	}                                                           \
}while(0)

/****
*Expr is true,goto
*/
#define SAMPLE_SVP_CHECK_EXPR_GOTO(Expr, Label,Level,Msg, ...)  \
do{																\
	if(Expr) {                                                  \
		SAMPLE_SVP_TRACE(Level,Msg,## __VA_ARGS__);             \
		goto Label;                                             \
	}                                                           \
}while(0)
/****
*Expr is true,return void
*/
#define SAMPLE_SVP_CHECK_EXPR_RET_VOID(Expr,Level,Msg, ...)     \
do{                                                             \
	if(Expr) {                                                  \
		SAMPLE_SVP_TRACE(Level,Msg, ##__VA_ARGS__);             \
		return;                                                 \
	}                                                           \
}while(0)
/****
*Expr is true,return Ret
*/
#define SAMPLE_SVP_CHECK_EXPR_RET(Expr,Ret,Level,Msg, ...)      \
do{                                                             \
	if(Expr) {                                                  \
		SAMPLE_SVP_TRACE(Level,Msg, ##__VA_ARGS__);             \
		return Ret;                                             \
	}                                                           \
}while(0)
/****
*Expr is true,trace
*/
#define SAMPLE_SVP_CHECK_EXPR_TRACE(Expr,Level,Msg, ...)        \
do{                                                             \
	if(Expr)                                                    \
	{                                                           \
	   SAMPLE_SVP_TRACE(Level,Msg, ##__VA_ARGS__);              \
	}                                                           \
}while(0)

#define SAMPLE_SVP_ALIGN_16		   16
#define SAMPLE_SVP_ALIGN_32		   32
#define SAMPLE_SVP_D1_PAL_HEIGHT   576
#define SAMPLE_SVP_D1_PAL_WIDTH    704

//free mmz
#define SAMPLE_SVP_MMZ_FREE(phy,vir)                            \
do{                                                             \
	if ((0 != (phy)) && (0 != (vir))) {                         \
		HI_MPI_SYS_MmzFree((phy),(void*)(HI_UL)(vir));          \
		(phy) = 0;                                              \
		(vir) = 0;                                              \
	}                                                           \
}while(0)


#define SAMPLE_SVP_CLOSE_FILE(fp)                               \
do{                                                             \
	if (NULL != (fp)) {                                         \
		fclose((fp));                                           \
		(fp) = NULL;                                            \
	}                                                           \
}while(0)

// =================================================================
#ifdef __cplusplus
}
#endif // __cplusplus
#endif