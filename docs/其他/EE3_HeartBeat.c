/**
 * \file      	EE3_SendFconData.c
 * \author    	Leo Bai
 * \version   	1.08
 * \date      	2012.07.13
 * \brief     	FconMode��JPEGͼ�����߳�
**/

#include "EE3_common.h"
#include "EE3_FtpUpload.h"

#define	CMDHEADLEN	6
#define SEVER_IP_ADDR "192.168.1.90"

//#define SEVER_IP_ADDR "210.73.152.201"
/**************************************************************************/
//�ⲿ����
extern RoseekJPGEncBufStruct	g_JpgEncBufStruct;
extern volatile	Bool	g_bIsFconClientConnect;
/**************************************************************************/
HEARTBEATSENDPACKET HeartBeatSendPacket;
//HEARTBEATRECPACKET   HeartBeatRecPacket;
extern void TimeProcess(Uint8 *time_buf);
extern Uint8   g_ui8SPT[8];
extern Uint8 OSD_Buf[256];



extern volatile	Bool		g_bIsStopTrgSvr;
extern volatile	Bool		g_bIsStopTrgSvr;
extern volatile	Bool		g_bTrgSvrIsLink;
extern RoseekImgInfStruct	 	g_ImgInfStruct;
extern RoseekTrgCtrlInfStruct 	g_TrgCtrlInfStruct;
extern AT88SC_Config AT88SC25616C_ConfigZone;
extern volatile Uint8 g_ui8SDFSCMD;
extern char g_szParam1[];
extern char g_szParam2[];
extern volatile Uint8 g_ui8SDFSCMD;
extern char g_szParam1[];
extern char g_szParam2[];

//�ļ��ֲ�����---����gelcmd
extern Uint8 cmdhead[6];
extern Uint8 bufSnd[256];
extern Uint8 bufRcv[5000];
static  Uint32 g_ui32FlashPulsWidth = 0;
static volatile Bool g_bPolarizer = FALSE;
extern void Roseek_Set_TrgCtrlParam( Uint8* pui8ParamSur );


/**
 * \function 	SendFconDataPort
 * \brief    	ʹ��TCP/IPЭ��ʵ��FconMode��JPEGͼ����
 * \			Ĭ����Ϊ�������ˣ�Port = 55000		
 * \
**/	
void SendHeartBeatPort()
{
	SOCKET	HeartBeatListen;
	struct	sockaddr_in addr;
	int status = 0;
	int len =0 ;

	int i,cnt,paramlen;
	Uint8 RecvBuf[256];
	Bool	bIsError = FALSE;
	static  Bool bThereIsNoCMD = FALSE;
	Uint32	temp = 0;
	static	Uint8 state = 0;


	//Ϊ��ǰ�����������л���
	fdOpenSession( TaskSelf() );



	TSK_sleep( 15000 );
	while( 1 )
	{
	//	printf("sem_HeartBeat\n");

	//��������socket����
		HeartBeatListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if( HeartBeatListen == INVALID_SOCKET ){
			Roseek_Reset();//���������������ʧ�ܣ��������
		}
		bzero( &addr, sizeof(struct sockaddr_in) );
		addr.sin_family	= AF_INET;
		addr.sin_len    = sizeof(addr);
		addr.sin_addr.s_addr = inet_addr(SEVER_IP_ADDR); 
		addr.sin_port	= htons( 44444 );


		status = connect(HeartBeatListen,(PSA)&addr,sizeof(addr));
		if( status != 0)
		{
			fdClose(HeartBeatListen);
			continue;
		}
		while(1)
		{
			SEM_pend(&sem_HeartBeat,SYS_FOREVER);
			if( send( HeartBeatListen, (char *)&HeartBeatSendPacket, sizeof(HEARTBEATSENDPACKET), 0 ) < 0 )
			{	
				break;			
			}	
		//	i = 0;
		//	while( i < 3 ){	
				cnt = recv( HeartBeatListen, RecvBuf, 3, 0 );
		//		if( cnt <= 0 ){
		//			continue;
		//		}
		//		i += cnt;
		//	}
			if((RecvBuf[0]!=0xaa) || (RecvBuf[1]!=0x55) )
			{
				goto rst;
			}
			if(RecvBuf[2]==0x00)
			{
				goto rst;
			}
			else
			{
				//��������ͷ���������ID���Ƿ��Ӧ��־�Ͳ�������
				i = 0;
				while( i < CMDHEADLEN ){
					cnt = recv( HeartBeatListen, (char*)cmdhead, CMDHEADLEN-i, 0 );
					if( cnt <= 0 ){
						fdClose(HeartBeatListen);
						bIsError = TRUE;
						break;
					}
					i += cnt;
				}
				if( bIsError ){
					//����������ݳ�����ȴ��µ���������
					bIsError = FALSE;
					continue;
				}
				paramlen = (cmdhead[5]<<24) + (cmdhead[4]<<16) + (cmdhead[3]<<8) + cmdhead[2];
				//��ʼ�����������
				i = 0;
				if( paramlen > 0 ){
					while( i < paramlen ){
						cnt = recv( HeartBeatListen, (char*)bufRcv, paramlen-i, 0 );
						if( cnt <= 0 ){
							fdClose(HeartBeatListen);
							bIsError = TRUE;
							break;
						}
						i += cnt;
					}
					if( bIsError ){
						//����������ݳ�����ȴ��µ���������
						bIsError = FALSE;
						continue;
					}
				}
				bThereIsNoCMD = FALSE;
				switch( cmdhead[0] )
				{
					case 0x00 :
					{
						//ϵͳ��λ
						Roseek_Reset();
						break;
					}
					case 0x01 :
					{
						//�л�����ģʽ
						temp = bufRcv[0];
						switch(temp)
						{
							case 0:
								Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, EE3_335_DRCLK_NORMAL );
								break;
							case 1:
								if(state==0)
								{
									Roseek_Set_TrgFconMode_ScaleLevel( FconMode, TRGFCON_SCALE_LEVEL_1 );
									state =1;
								}
								else
								{
									Roseek_Set_TrgFconMode_ScaleLevel( FconMode, TRGFCON_SCALE_LEVEL_0 );
									state = 0;
								}
								Roseek_Mode_Switch_Ctrl( (enum Roseek_SensorRunMode)temp, EE3_335_DRCLK_NORMAL );
								break;
							default:
								Roseek_Mode_Switch_Ctrl( FconMode, EE3_335_DRCLK_NORMAL );		
						}

						break;
					}
					case 0x02 :
					{
						//��λ������ץȡһ��ͼ��
						Roseek_Grab_Single_Frame();
						break;
					}
					case 0x03 :
					{
						/////////////////////��ȡ����Ŀ���״̬\\\\\\\\\\\\\\\\\\\\\\\
						//1.��ȡ��ǰ�������ź�����ֵ
						*(float*)(bufSnd+4) = g_CameraGeneralCtrlParams.fCurSignalGain;
						//2.��ȡ��ǰ���Ź���ʱ��װ��ֵ
						*(Uint32*)(bufSnd+8) = g_CameraGeneralCtrlParams.ui32WDTValue;
						//3.��ȡ�����ͬ�����ʹ�ܱ�־
						*(bufSnd+12) = 1;
						//4.��ȡ�л�������ģʽ���Ƿ���������Զ�����ץȡһ��ͼ����Ʊ�־
						if( g_CameraGeneralCtrlParams.bAllowAutoTrgAfterModeSwitch ){
							*(bufSnd+13) = 1;
						}
						else{
							*(bufSnd+13) = 0;
						}
						//5.��ȡ�Ƿ�����ͼ��LUTӳ��任��־
						if( g_CameraGeneralCtrlParams.bIsEnableImgLUT ){
							*(bufSnd+14) = 1;
						}
						else{
							*(bufSnd+14) = 0;
						}
						//6.��ȡ�̵���״̬
						*(bufSnd+15) = 0;
						//7.��ȡ���Ź�ʹ�ܱ�־
						if( g_CameraGeneralCtrlParams.bIsEnableWatchDog ){
							*(bufSnd+16) = 1;
						}
						else{
							*(bufSnd+16) = 0;
						}
						//8.��ȡ��ǰ��ƽ����ջ���
						*(bufSnd+17) = 0;
						//9.��ȡ��ǰ����ģʽ
						switch(g_CameraGeneralCtrlParams.CurRunModeFlag ){
							case TrgMode:
								*(bufSnd+18) = 0;
								break;
							case FconMode:
								*(bufSnd+18) = 1;
								break;
							default:
								break;
						}
						//10.��ȡ��ǰͼ��ɼ���ʽ
						switch(g_CameraGeneralCtrlParams.CurImgFormat ){
							case RGB_ARRAY:
								*(bufSnd+19) = 0;
								break;
							case YUV_ARRAY:
								*(bufSnd+19) = 1;
								break;
							case BAYER_ARRAY:
								*(bufSnd+19) = 2;
								break;
							default:
								break;
						}
						//11.��ȡ�ɱ��I/O�ķ���
						*(Uint32*)(bufSnd+20) = 0x00;
						//12.��ȡ�ɱ��I/O�����ֵ
						*(Uint32*)(bufSnd+24) =0x00;
						//���ûش���������
						*(Uint32*)bufSnd = 24;
						break;
					}
					case 0x04 :
					{
						//���ô���ģʽ�µ��ع�ʱ��
						Roseek_Set_TrgCtrlParam( bufRcv );
						break;
					}
					case 0x05 :
					{
						//����ȫ�ֱ�������ģʽ�µ��ع�ʱ��
						temp = bufRcv[0] + (bufRcv[1]<<8) + (bufRcv[2]<<16);
						Roseek_Set_FConShutter_Time( temp );
						break;
					}
					case 0x06 :
					{
									break;
					}
					case 0x07 :
					{
						//���ô������ź�����
						if( *(float*)bufRcv >= PGA_LOWLIMIT && *(float*)bufRcv <= PGA_HIGHLIMIT ){
							Roseek_Set_Afe_Pga( g_CameraGeneralCtrlParams.CurRunModeFlag,*(float*)bufRcv );
							break;
						}
						break;
					}
					case 0x08 :
					{
						//���ð�ƽ���������
						switch( bufRcv[0]){
							case 0:	//Daylight
								g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
								Roseek_Set_WBEvr( DAYLIGHT );
								break;
							case 1: //Fluorescence
								g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
								Roseek_Set_WBEvr( FLUORESCENCE );
								break;
							case 2: //without WhiteBalance
								g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
								Roseek_Set_WBEvr( WITHOUTWB );
								break;
							default:
								break;
						}
						break;
					}
					case 0x09 :
					{
						//�Ƿ��Զ���ƽ��
						if( bufRcv[0] ){
							g_CameraGeneralCtrlParams.bRunWhitebalance = TRUE;
						}
						else{
							g_CameraGeneralCtrlParams.bRunWhitebalance = FALSE;
						}
						break;
					}
					case 0x0a :
					{
						//�����Ƿ�ʹ��ͼ��LUTӳ��任
						if( bufRcv[0] ){
							Roseek_Enable_Image_LUT( );
						}
						else{
							if( bufRcv[1] ){
								Roseek_Disable_Image_LUT( );
							}
							else{
								Roseek_Disable_Image_LUT( );
							}
						}
						break;
					}
					case 0x0b :
					{
						//����ͼ��LUTӳ���
						Roseek_Set_Image_LUT( bufRcv, 0 );
						break;
					}
					case 0x0c :
					{
						//���������ͬ�����ʹ��
						temp = bufRcv[0];
						if( temp==1 ){
							Roseek_Enable_TrgFlashlamp_Syn_Out();
						}
						else{
							Roseek_Disable_TrgFlashlamp_Syn_Out();
						}
						break;
					}
					case 0x0d :
					{
						//���ÿɱ��I/O�ڵķ���
						break;
					}
					case 0x0e :
					{
						//���ÿɱ��I/O�����ֵ
						//temp��D0~D9 -> Port1~Port10��1Ϊ�ߵ�ƽ��0Ϊ�͵�ƽ�������������Ч
						temp = (bufRcv[1]<<8) + bufRcv[0];
		//				Roseek_Set_Pio_Outvalue( temp );
						break;
					}
					case 0x0f :
					{
						//����ȫ�ֱ���ģʽ����ֵ������ֵ
						if( bufRcv[0]==1){
							g_FconBrightnessAdj_Params.bIsAllowBrightnessAdj = TRUE;
							Roseek_Set_Afe_Pga( FconMode,g_FconBrightnessAdj_Params.fCurrentAfePga_db );
							Roseek_Set_FConShutter_Time( g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us );
						}
						else{
							g_FconBrightnessAdj_Params.bIsAllowBrightnessAdj = FALSE;
						}
						break;
					}
					case 0x10 :
					{
						//����ȫ�ֱ�������ģʽ�µ�������������Զ����ڵ��ع�ʱ������
						g_FconBrightnessAdj_Params.ui32PIShutterWidthHigh_us = (bufRcv[2]<<16) + (bufRcv[1]<<8) + (bufRcv[0]);
						g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us = g_FconBrightnessAdj_Params.ui32PIShutterWidthHigh_us;
						g_FconBrightnessAdj_Params.fCurrentAfePga_db = PGA_LOWLIMIT;
						Roseek_Set_Afe_Pga( FconMode,g_FconBrightnessAdj_Params.fCurrentAfePga_db );
						Roseek_Set_FConShutter_Time( g_FconBrightnessAdj_Params.ui32CurrentShutterWidth_us );
						break;
					}
					case 0x11 :
					{
									break;
					}
					case 0x12 :
					{
										break;
					}
					case 0x13 :
					{
						//����������������Զ����ڵ��ź���������
						g_TrgBrightnessAdj_Params.fPIPgaHigh_db = *(float*)bufRcv;
						g_FconBrightnessAdj_Params.fPIPgaHigh_db = *(float*)bufRcv;
						break;
					}
					case 0x14 :
					{
						//����ȫ�ֱ�������ģʽ��������������Զ����ڵ�ͼ��������ֵ��Χ
						g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold = (Uint8)bufRcv[0];
						if( g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold > 235){
							g_FconBrightnessAdj_Params.ui32PIYmeanHighThreshold = 255;
						}
						else{
							g_FconBrightnessAdj_Params.ui32PIYmeanHighThreshold = g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold + 15;
						}
						if( g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold < 30){
							g_FconBrightnessAdj_Params.ui32PIYmeanLowThreshold = 10;
						}
						else{
							g_FconBrightnessAdj_Params.ui32PIYmeanLowThreshold = g_FconBrightnessAdj_Params.ui32PICurYmeanThreshold - 15;
						}
						break;
					}
					case 0x15 :
					{
						break;
					}
					case 0x16 :
					{
						//�����Ƿ�ʹ�ܿ��Ź�
						if( bufRcv[0] ){
							Roseek_Watchdog_Open();
							Roseek_KickWatchdog();
						}
						else{
							Roseek_Watchdog_Close();
						}
						break;
					}
					case 0x17 :
					{
						//���ÿ��Ź���ʱ��
						Roseek_Set_WatchdogTimer( bufRcv[0] );
						break;
					}
					case 0x18 :
					{
						//���������ʵʱʱ��ʱ��
						bufRcv[7] = 0;
						Roseek_RTC_SetTime( bufRcv );
						break;
					}
					case 0x19 :
					{
						//��ȡ�����ʵʱʱ��ʱ��
						Roseek_RTC_ReadTime( bufSnd+4 );
						*(Uint32*)bufSnd = 7;
						Roseek_Waitusec( 10 );
						break;
					}
					case 0x1a :
					{
						//�޸������IP��ַ
						temp = (bufRcv[3]<<24) + (bufRcv[2]<<16) + (bufRcv[1]<<8) + (bufRcv[0]);
						Roseek_Save_IP_Address( temp );
						break;
					}
					case 0x1b :
					{
						//�޸������MAC��ַ
						bufRcv[6] = 0x00;
						bufRcv[7] = 0x00;
						Roseek_Save_MAC_Address( bufRcv );
						Roseek_Waitusec( 15000);
						break;
					}
					case 0x1c :
					{
						//��ȡ�����MAC��ַ
						Roseek_Read_MAC_Address( bufSnd+4 );
						*(Uint32*)bufSnd = 6;
						Roseek_Waitusec( 10 );
						break;
					}
					case 0x1d :
					{
						//���ü̵����պϻ�Ͽ���1Ϊ�պϣ�0Ϊ�Ͽ�
						break;
					}
					case 0x1e :
					{
						//��ȡ����¶�
						*(float*)( bufSnd+4 ) = Roseek_Read_CurTemperature();
						*(Uint32*)bufSnd = 4;
						Roseek_Waitusec( 10 );
						break;
					}
					case 0x1f :
					{
						//�����л�������ģʽ���Ƿ���������Զ�����ץȡһ��ͼ��
						if( bufRcv[0] ){
							Roseek_Enable_RunModeSwitch_AutoTrg( TRUE );
						}
						else{
							Roseek_Enable_RunModeSwitch_AutoTrg( FALSE );
						}
						break;
					}
					case 0x20 :
					{
						//�л�ͼ��Ĳɼ���ʽ
						switch( bufRcv[0] ){
							case 0:
								Roseek_Set_ImgAcquisition_Format( RGB_ARRAY );
								break;
							case 1:
								Roseek_Set_ImgAcquisition_Format( YUV_ARRAY );
								break;
							case 2:
								Roseek_Set_ImgAcquisition_Format( BAYER_ARRAY );
								break;
							default:
								break;
						}
						break;
					}
					case 0x21 :
					{
						//ֹͣ����ģʽ�µ��ϴ�����
						g_bIsStopTrgSvr = TRUE;
						SEM_post( &sem_TrgDataReady );
						break;
					}
					case 0x22 :
					{
						//���ô���ģʽ�µĿ����ٶ��Ƿ�����������ģʽ����һ��
						if( bufRcv[0] ){
							Roseek_SetTrgShutterTimeCtrlMode( TRUE );
						}
						else{
							Roseek_SetTrgShutterTimeCtrlMode( FALSE );
						}
						break;
					}
					case 0x23 :
					{
						//��ⴥ��ģʽ�ϴ��������Ƿ�����
						if( g_bTrgSvrIsLink ){
							g_bIsStopTrgSvr = TRUE;
							SEM_post( &sem_TrgDataReady );
						}
						break;
					}
					case 0x24 :
					{
										break;
					}
					case 0x25 :
					{
										break;
					}
					case 0x26 :
					{
								break;
					}
					case 0x27 :
					{
										break;
					}
					case 0x2E :
					{
						//��ȡ�û�����оƬ����������
						if( Roseek_Encryption_Read_Configuration( bufRcv[0], bufRcv[1], bufRcv[2] )){
							memcpy(bufSnd+4,&AT88SC25616C_ConfigZone,sizeof(AT88SC_Config));
							*(Uint32*)bufSnd = 256;
						}
						else{
							*(Uint32*)bufSnd = 4;
						}
						break;
					}
					case 0x2F :
					{
						//�����û�����оƬ����������
						memcpy( &AT88SC25616C_ConfigZone, bufRcv+4, sizeof(AT88SC_Config));
						if( Roseek_Encryption_Initialization( bufRcv[0], bufRcv[1], bufRcv[2] )){
							*(Uint32*)bufSnd = 8;
						}
						else{
							*(Uint32*)bufSnd = 4;
						}
						break;
					}
					case 0x30:  //��ѧ�����л�����
					{       
						temp = bufRcv[0];       
						if(temp==0)       
						{            
							g_bPolarizer = FALSE;            
							Roseek_Drive_LF_Switch(SWITCH_TO_GENERAL_FILTER);       
						}       
						else       
						{            
							g_bPolarizer = TRUE;            
							Roseek_Drive_LF_Switch(SWITCH_TO_POLARIZER_FILTER);
						}       
						break; 
					} 
					case 0x31:  //������������ 
					{       
						temp = bufRcv[0];       
						if(temp==0)       
						{             
							Roseek_Disable_TrgFlashlamp_Syn_Out();             
							Roseek_Disable_FconFlashlamp_Syn_Out();       
						}       
						else       
						{             
							Roseek_Enable_TrgFlashlamp_Syn_Out();             
							Roseek_Enable_FconFlashlamp_Syn_Out();       
						}       
						break; 
					}
					case 0x32:   //���ò�����������Ч���� 
					{       
						g_ui32FlashPulsWidth = bufRcv[0]*255+bufRcv[1];       
						if(g_ui32FlashPulsWidth>=799)       
						{          
							g_ui32FlashPulsWidth =799;       
						}       
						Roseek_Set_FlashSynWidth(g_ui32FlashPulsWidth);       
						break; 
					}
					case 0x33:  //���ӻ���LUT���� 
					{       
						if(bufRcv[0] ==1)       
						{            
							Roseek_Set_Image_LUT( bufRcv+1, 1);//����LUT ������洢��EEPROM����       
						}       
						else       
						{            
							Roseek_Set_Image_LUT( bufRcv+1, 0);//����LUT ���洢       
						}        
						break; 
					} 
					case 0x34: 
					{       
						Roseek_Resume_Image_LUT();//�ָ�LUTΪ����Ĭ��ֵ       
						break; 
					}
					case 0x35:
					{
						Roseek_Get_CPU_ID(bufSnd+4);
						*(Uint32*)bufSnd = 6;
						break; 
					}
					case 0xff :
					{
						g_ui8SDFSCMD = (Uint8)bufRcv[0];
						memcpy(g_szParam1, bufRcv + 1, 17);
						memcpy(g_szParam2, bufRcv + 18, 17);
						g_szParam1[16] = '\0';
						g_szParam2[16] = '\0';
						
						break;
					}
					default :
					{
						bThereIsNoCMD = TRUE;
						break;
					}
				}
			}
	rst:	SEM_reset( &sem_HeartBeat, 0 ); 
		}
		fdClose(HeartBeatListen);
	}
	
}


void HeartBeatTskFun(void)
{
	Roseek_RTC_ReadTime(g_ui8SPT);
	TimeProcess(g_ui8SPT);
	memcpy(HeartBeatSendPacket.real_time,OSD_Buf,16);
	SEM_post(&sem_HeartBeat);

}

