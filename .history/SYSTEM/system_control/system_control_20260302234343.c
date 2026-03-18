
#include "main.h"



uint32	sys_control_time = 0;  //��ʱ����ʱ��
 uint8	   sys_time_up	   = false ;   //�����־
 uint8	   sys_time_start = false;	//��������ʱ���־ 0 = not ,1 = yes

 
uint8 target_percent = 0; //�����趨Ŀ��������
uint8 now_percent = 0; //�����趨���ڵ�ʵʱ����

uint8 adc_sample_flag = 0; //adc ����ʱ���־

uint8 T_PERCENT = 0;
uint32_t BJ_TimeVar;//����ʱ�������

/*ʱ��ṹ�壬Ĭ��ʱ��2000-01-01 00:00:00*/
struct rtc_time systmtime=
{
	0,0,0,1,1,2000,0
};


UART_DATA U1_Inf;
UART_DATA U2_Inf;
UART_DATA U3_Inf;

UART_DATA U4_Inf;

UART_DATA U5_Inf;



SYS_INF sys_data;
SYS_COPY copy_sys;

LCD_MEM lcd_data;



Lcd_Read_Data read_lcd_data;//���ڼ�¼�˹����õ�ϵͳ����
SYS_WORK_TIME sys_time_inf;//��¯ϵͳ�ۼ�����ʱ�����
SYS_WORK_TIME Start_End_Time; //���ڼ�¼������ͣ������ʱ�����������������

SYS_WORK_TIME big_time_inf;//��¯С��������ʱ��
SYS_WORK_TIME small_time_inf;//��¯���������ʱ��

sys_flags sys_flag; //ϵͳ��Ҫʹ�õı�־������


SYS_CONFIG sys_config_data;//����ϵͳ���ô�С����ʱ�Ȳ���

SYS_ADMIN  Sys_Admin; //�������ù���Ա����

AB_EVENTS  Abnormal_Events;//����ϵͳ����ʱ���쳣��¼
BYTE_WORD4 Secret_uint; //���ڶ�4���ֽ�ת��Ϊ32λ����
BYTE_WORD1 Data_wordtobyte;//����1WORD    2���ֽ�ת��

FLP_INT  Float_Int;//���ڵ����ȸ��������ݵ�ת��
BYTE_INT32 Byte_To_Duint32;  //����4���ֽڵ�uint32������ת��


LCD_QuXian lcd_quxian_data;//����ˢ������ͳ�Ƶ�����
ERR_LCD  Err_Lcd_Code;//����ˢ��lcd��������
LCD_FLASH_STRUCT  Lcd_FlashD;


LCD_E_M  Err_Lcd_Memory[8];//���ڼ�¼8��������Ϣ��ʱ��͹���ԭ��
ERROR_DATE_STRUCT SPI_Error_Data;



IO_DATA IO_Status;
 Login_TT Login_D; //�����¼��Ϣ�����ṹ��

 Logic_Water Water_State;


 UPID PID; 



 JUMP_TYPE  Jump_Step = First;



 










uint8  Air_Door_Index = 0;//���ڴ��Է����쳣����ת״̬ʹ��
uint8  ab_index =0 ;
 



uint8 Self_Index = 0;
uint8 Sys_Staus = 0;
uint8	Sys_Launch_Index = 0;
uint8 Ignition_Index = 0;
uint8	Pressure_Index = 0;
uint8 IDLE_INDEX = 0;







void Get_IO_Inf(void)
{
	uint8  Error16_Time = 8;
	
	uint8  Error_Buffer = 0;
	static uint8 waterProtectFilterState = WATER_OK;
	static uint8 waterProtectLowHoldSec = 0;
	static uint8 waterProtectRecoverHoldSec = 0;
	static uint8 waterProtectFilterInit = 0;
	static uint8 lastPaiwuSecTick = 0xFF;
	uint8 currentPaiwuSecTick = 0;
	uint16 waterProtectLowThreshold = 0;
	uint16 waterProtectRecoverThreshold = 0;
	uint16 waterProtectHysteresis = 20;
		//�̶�һֱ����źţ� ȼ��ѹ������еʽѹ���������ź�

	/* ��ѹҺλ�ڲ�ˮʱ�����˲ʱ�³壬�������뼶�˲����ͻأ��������е�Һλͣ¯ */
	if(sys_flag.A3B3_ChaYaMokuai_TxRX_Count < 20)
		{
			if(waterProtectFilterInit == 0)
				{
					waterProtectFilterInit = 1;
					waterProtectFilterState = IO_Status.Target.water_protect;
					waterProtectLowHoldSec = 0;
					waterProtectRecoverHoldSec = 0;
					lastPaiwuSecTick = sys_flag.Paiwu_Secs;
				}

			waterProtectLowThreshold = (uint16)Sys_Admin.ChaYa_WaterLow_Set;
			waterProtectRecoverThreshold = waterProtectLowThreshold + waterProtectHysteresis;

			currentPaiwuSecTick = sys_flag.Paiwu_Secs;
			if(currentPaiwuSecTick != lastPaiwuSecTick)
				{
					lastPaiwuSecTick = currentPaiwuSecTick;
					if(waterProtectFilterState == WATER_OK)
						{
							if(sys_flag.LPWater_Value <= waterProtectLowThreshold)
								{
									waterProtectLowHoldSec++;
									if(waterProtectLowHoldSec >= 5)
										{
											waterProtectFilterState = WATER_LOSE;
											waterProtectLowHoldSec = 0;
											waterProtectRecoverHoldSec = 0;
										}
								}
							else
								{
									waterProtectLowHoldSec = 0;
								}
						}
					else
						{
							if(sys_flag.LPWater_Value >= waterProtectRecoverThreshold)
								{
									waterProtectRecoverHoldSec++;
									if(waterProtectRecoverHoldSec >= 3)
										{
											waterProtectFilterState = WATER_OK;
											waterProtectRecoverHoldSec = 0;
											waterProtectLowHoldSec = 0;
										}
								}
							else
								{
									waterProtectRecoverHoldSec = 0;
								}
						}
				}

			IO_Status.Target.water_protect = waterProtectFilterState;
		}
	else
		{
			waterProtectFilterInit = 0;
			waterProtectLowHoldSec = 0;
			waterProtectRecoverHoldSec = 0;
		}
	
		Error_Buffer = FALSE ;
		if (IO_Status.Target.water_high== WATER_OK)
			{
				if(IO_Status.Target.water_mid== WATER_LOSE ||IO_Status.Target.water_protect == WATER_LOSE)
					Error_Buffer = OK ;
			}
	
	
		if (IO_Status.Target.water_mid== WATER_OK)
			{
				if(IO_Status.Target.water_protect == WATER_LOSE)
					Error_Buffer = OK ;
			}
		
		if(Error_Buffer)
			{
				if(sys_flag.flame_state)
					{
						sys_flag.Force_Supple_Water_Flag = OK;
						sys_flag.Force_Flag =OK;
					}
					
				else
					sys_flag.Force_Supple_Water_Flag = 0;

				sys_flag.Error16_Flag = OK;
			}
		else
			{
				sys_flag.Force_Flag = FALSE; // 22.07.12����û�м�ʱ���ǿ�Ʋ�ˮ����
				sys_flag.Error16_Flag = 0;
				sys_flag.Error16_Count = 0;
			}
		//ǿ�Ʋ�ˮ12�룬Ȼ�����ǿ�Ʋ�ˮ�ı�־
		if(sys_flag.Force_Count >= 5)
			{
				 sys_flag.Force_Supple_Water_Flag = 0;
				 sys_flag.Force_Flag = FALSE;
				 sys_flag.Force_Count = 0;
			}
			
		
		
		
		if(sys_flag.Error16_Count >=  6)  //6��
			{
				if(sys_flag.flame_state && sys_data.Data_10H == 2)
					{
						sys_data.Data_12H = 6; // �¶ȸ����û��趨ֵ0.01kg
						Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼
					}
				else
					{
						if(sys_flag.Error_Code == 0)
							sys_flag.Error_Code = Error8_WaterLogic;
					}
				
				
				sys_flag.Error16_Flag = FALSE;
				sys_flag.Error16_Count = 0;
			}

	//�̶�һֱ����źţ� ȼ��ѹ������еʽѹ���������ź�
		 
		if(IO_Status.Target.hot_protect == THERMAL_BAD)
			{
				if(sys_flag.Error15_Flag == 0)
					sys_flag.Error15_Count = 0;
				
				sys_flag.Error15_Flag = OK;

				if(sys_flag.Error15_Count > 1)
					{
						if(sys_flag.Error_Code == 0 )
							sys_flag.Error_Code = Error15_RebaoBad;
					}
				
			}
		else
			{
				sys_flag.Error15_Flag = 0;
				sys_flag.Error15_Count = 0;
			}

		
		//��еʽѹ������ź�	
		if(IO_Status.Target.hpressure_signal == PRESSURE_ERROR)
			{
				if(sys_flag.Error1_Flag == 0)
					sys_flag.Error1_Count = 0;
				
				sys_flag.Error1_Flag = OK;		
				//������ѹ��������ȫ��Χ�����ϣ�����
				if(sys_flag.Error1_Count > 1)
					{
						 if(sys_flag.Error_Code == 0 )
							sys_flag.Error_Code = Error1_YakongProtect; //����ѹ��������ȫ��Χ����	
					}
			}
		else
			{
				sys_flag.Error1_Flag = 0;
				sys_flag.Error1_Count = 0;
			}
			

	
}



/**
  * @brief  �������ǰ�����׼������
  * @param  sys_flag.before_ignition_index
  * @retval ׼���÷���1�����򷵻�0
  */

uint8 Before_Ignition_Prepare(void)
{
		//1��ˮλ�źű�����                2�������źű�����
		//sys_flag.before_ignition_index
		uint8 func_state = 0;

		func_state = 0;
		switch (sys_flag.before_ignition_index)
			{
				case 0 :
						//������ŷ���ѭ���ã�sys_flag.Pai_Wu_Already���ˮλ�źž����Ƿ�����������Ʒ�
							 Send_Air_Open();  //�򿪷��ǰ��ɨ	
							 
							 PWM_Adjust(0); //�ȴ�5�����
							 Pai_Wu_Door_Close();
							 delay_sys_sec(12000);
							
							 
							sys_flag.before_ignition_index = 1;//��ת���¸�״̬
							sys_flag.FlameOut_Count = 0;
							sys_flag.XB_WaterLowAB_Count = 0;
							break;

				case 1: 

						if(sys_time_start == 0)
							sys_time_up = 1;
						if(sys_time_up)
							{
								sys_time_up = 0;
								sys_flag.Wts_Gas_Index =0;
								sys_flag.before_ignition_index = 2;//��ת���¸�״̬
								PWM_Adjust(100);
								
							}	
								
							break;

				case 2:
					
					

					sys_flag.before_ignition_index = 0;
					func_state = SUCCESS;	

					break;

			   default:
			   	sys_flag.before_ignition_index = 0;//�ָ�Ĭ��״̬
			   			sys_close_cmd();
			   			break;
			}

		

		return func_state ;//���ǰ׼����׼�����ˣ�����1
}



/**
  * @brief  ��鲢ת����IO��ˮλ��Ϣ���ȱ�������״̬
* @param  
  * @retval ��
  */
 void Self_Check_Function()
{
	
	
	Get_IO_Inf(); //��ȡIO��Ϣ

	
	
						 
}

/**
  * @brief  ϵͳ������
* @param   �����ɷ���1�����򷵻�0
  * @retval ��
  */
uint8  Sys_Ignition_Fun(void)
{
	uint8 First_Blow_Time = 0;
	uint8 Value_Buffer = 0;

	
		First_Blow_Time = Sys_Admin.First_Blow_Time / 1000;  //�������
		sys_data.Data_12H = 0x00; //�������У�û�ж��쳣���м��
		Abnormal_Events.target_complete_event = 0;
		switch(Ignition_Index)
		{
			case 0 : //  
						sys_flag.Ignition_Count = 0;
						sys_flag.FlameRecover_Time = 0; //�Ը�λʱ���������
						sys_flag.LianxuWorkTime = 0;  //�Ա��׶ι���ʱ������

						sys_flag.Pid_First_Start = 0;
						WTS_Gas_One_Close();
					
						/*******************PWM����*һ��������ɨ***********************************/
						Send_Air_Open();  //���ǰ��ɨ			
						PWM_Adjust(99);
						if(IO_Status.Target.water_high == WATER_LOSE)
							{
								sys_flag.Force_Supple_Water_Flag = OK;
								delay_sys_sec(100000); //60��ǿ�Ʋ�ˮ
							}

						
						Ignition_Index = 10; //�л����̣�
							
						
					break;

			case 10:
					Send_Air_Open();
					Send_Gas_Close();//ȼ������رգ��رգ��ر�
					WTS_Gas_One_Close();
					if(IO_Status.Target.water_high == WATER_OK)
						{
							sys_flag.Force_Supple_Water_Flag = FALSE;
							sys_time_up = 1;
						}

					
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							
						}
					if(sys_time_up)
						{
							if(sys_flag.AirWork_Time > First_Blow_Time)
								{
									delay_sys_sec(200);
								}
							else
								{
									//������ʱ��С��Ӧ�ô�ɨ��ʱ������
									Value_Buffer = First_Blow_Time - sys_flag.AirWork_Time;
									delay_sys_sec(Value_Buffer * 1000);
								}
							
							Ignition_Index = 1; //�л�����
						}
					else
						{
							
						}

					break;

		case 1:
					Send_Air_Open();
					Send_Gas_Close();//ȼ������رգ��رգ��ر�
					WTS_Gas_One_Close();
					Dian_Huo_OFF();  //�رյ��̵���
					//ʱ�䵽��Ҳ�����ִ�г���
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							
						}
					if(sys_time_up)
						{
							sys_time_up = 0;
						
							delay_sys_sec(100);  //����

							Ignition_Index = 2; //�л����̣�,����е�����
						}
					else
						{
							
						}

					
					break;

		
		 
		case 2://���з����л�,����������任��ע���鳬ѹͣ¯����
					Send_Air_Open();
					Send_Gas_Close();//ȼ������رգ��رգ��ر�
					WTS_Gas_One_Close();
					Dian_Huo_OFF();  //�رյ��̵���
					PWM_Adjust(99);//���ʰٷ�֮60
					
			
					if(sys_time_start == 0)
						sys_time_up = 1;
					if(sys_time_up)
						{
							sys_time_up = 0;
							
							sys_flag.Force_Supple_Water_Flag = FALSE; //������Ҫ��ǿ�Ʊ�־ȡ��

							
							if(IO_Status.Target.Air_Door == AIR_CLOSE)//���Źر��򱨾����ߵ�ƽ����
								sys_flag.Error_Code = Error9_AirPressureBad; //���Է��Ź���
							else
								{
									//NOP
								}
								
							
								
							PWM_Adjust(30);//��⹦��
							if(Sys_Admin.Fan_Speed_Check)
								delay_sys_sec(20000);  //�ȴ����ٱ仯ʱ�䣬��ʱ�򱨾�
							else
								delay_sys_sec(3000);
							Ignition_Index = 3;
							 
						}

					break;
						
	case 3://��ʽ��ʼ��𣬵�����ȿ�1.5s
					Send_Air_Open();  //���ű����
					Send_Gas_Close();//ȼ������رգ��رգ��ر�
					PWM_Adjust(30);//��⹦��
					Dian_Huo_OFF();  //�رյ��̵���
					sys_flag.Force_Supple_Water_Flag = FALSE;
					//���ǰȷ�ϣ�
					if (IO_Status.Target.water_protect== WATER_LOSE)
 						{
							sys_flag.Error_Code  = Error5_LowWater;
 						}
					if(Sys_Admin.Fan_Speed_Check)
						{
							if(sys_flag.Fan_Rpm > 1000 && sys_flag.Fan_Rpm < 6500)//��������1000ת��6500ת֮��
								{
									delay_sys_sec(1000);//���ӷ����ȶ���ʱ��
									Dian_Huo_Air_Level();//���Ƶ����ٳ���
									Ignition_Index = 4;
								}
							else
								{
									//NOP
								}

							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
									//NOP
								}
							if(sys_time_up)
								{
									sys_time_up = 0;
									//���ٿ���ʧ�鱨���ϣ�����
									sys_flag.Error_Code = Error13_AirControlFail; //���Է��Ź���//ϵͳ������־
								}
							else
								{
								//NOP
								}
						}
					else  //�������з��ټ��
						{
							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
									//NOP
								}
							if(sys_time_up)
								{
									Dian_Huo_Air_Level();//���Ƶ����ٳ���
									delay_sys_sec(3000); 
									Ignition_Index = 4;
								}
							else
								{
									//NOP
								}
						}
					
					
					
					
					break;

	case 4:
					Send_Air_Open();  //���ű����
					Send_Gas_Close();//ȼ������رգ��رգ��ر�
					Dian_Huo_Air_Level();//���Ƶ����ٳ���
					Dian_Huo_OFF();  //�رյ��̵���
		
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							//NOP
						}
					if(sys_time_up)
						{
							sys_time_up = 0;
							Dian_Huo_Start();//�������
							delay_sys_sec(1000);// 
							Ignition_Index = 5;
						}
					else
						{
							
						}
					
					break;
	case 5://��ȼ��2.5s
					Send_Air_Open();  //���ű����
					Dian_Huo_Air_Level();//���Ƶ����ٳ���
					Dian_Huo_Start();//�������
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							
						}
					if(sys_time_up)
						{
							 
							sys_time_up = 0;
							// Send_Gas_Open();
							WTS_Gas_One_Open();//ȼ����1
							delay_sys_sec(3500);
							
							Ignition_Index = 6;
						}
					else
						{
							
						}

				break;
					 
	case 6: //�����رգ��ȴ�3��������޻��棬��Ӳ���ӳ�
					Send_Air_Open();  //���ű����
					Dian_Huo_Air_Level();//���Ƶ����ٳ���
					Dian_Huo_Start();//�������
					 
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							//NOP
						}
					
					if(sys_time_up)
						{
							sys_time_up = 0;

							//Dian_Huo_OFF(); //2023��10��17��12:21:58 ע�͵����д���
							Send_Gas_Open();
							delay_sys_sec(4800);  //�޸� ��1���Ϊ1.5��
							Ignition_Index = 7;
						}
					else
						{
							
						}
		
					break;
	case 7://3��ʱ�䵽���л������»�һ��ʱ�䣬�޻����򱨾�
					 
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							//NOP
						}
					if(sys_time_up)
						{
							sys_time_up = 0;
							Dian_Huo_OFF();  //�رյ��̵���
							WTS_Gas_One_Close();
							 
							if(sys_flag.flame_state == FLAME_OK )  //�л���
							{
								 //���ɹ����������ԭ״̬���ȴ������ȶ�
								  delay_sys_sec(Sys_Admin.Wen_Huo_Time);  //�趨�ȶ�����ʱ��10sec��
								 Ignition_Index = 8;//�л����̣����ɹ�������ϵͳ��������״̬	
 
							}
							else  //�޻���
							{
								
								sys_flag.Ignition_Count ++;
								Send_Gas_Close();//�ر�ȼ������
								WTS_Gas_One_Close();
								Dian_Huo_OFF();  //�رյ��̵������������ͼ��תΪ��ɫ
								if(sys_flag.Ignition_Count < Max_Ignition_Times)
									{
										//ִ�еڶ��ε��
										Ignition_Index = 9;
										PWM_Adjust(99);
										sys_flag.AirWork_Time = 0;  //��Ҫִ�ж��δ�ɨ
										delay_sys_sec(Sys_Admin.First_Blow_Time);  //�趨�´ε��ʱ����Ϊ20sec + 10������٣�
				  					}
								else
									{
										sys_flag.Error_Code = Error11_DianHuo_Bad;//ϵͳ������־
										Ignition_Index = 0;
									}
									
							}
						}
					else
						{
							//NOP
						}
					
						
				break;
			case 8: //�ȴ��»���ʱ

					
					Dian_Huo_OFF();
					sys_flag.Force_UnSupply_Water_Flag = FALSE ;  //���Բ�ˮ
					 //��ֹû����ˮλ���ٿ�һ��
					if(sys_flag.flame_state == FLAME_OUT)//�Ȼ���̻���Ϩ��
						{ 
								sys_flag.Ignition_Count ++;
								Send_Gas_Close();//�ر�ȼ������
								WTS_Gas_One_Close();
								Dian_Huo_OFF();  //�رյ��̵������������ͼ��תΪ��ɫ
								if(sys_flag.Ignition_Count < Max_Ignition_Times)
									{
										//ִ�еڶ��ε��
										Ignition_Index = 9;
										PWM_Adjust(99);
										sys_flag.AirWork_Time = 0;  //��Ҫִ�ж��δ�ɨ
										delay_sys_sec(Sys_Admin.First_Blow_Time);  //�趨�´ε��ʱ����Ϊ20sec + 10������٣�
									}
								else
									{
										sys_flag.Error_Code = Error11_DianHuo_Bad;//ϵͳ������־
										Ignition_Index = 0;
									}
									
						}

					
						if(sys_time_start == 0)
							{
								sys_time_up = 1;
							}
						else
							{
							
							}
						if(sys_time_up)
						{
							sys_time_up = 0;//�����ȶ�ʱ�䵽
							
	/**************************************��ת���ڶ��׶β�������***START********************************************/
							 sys_flag.Ignition_Count = 0;
				
							return 1;
	/**************************************��ת���ڶ��׶β�������***END********************************************/
						}
						else
						{
							
						}
				
					break;

			case 9://���ʧ�ܣ��л���������
					Send_Gas_Close();//�ر�ȼ������
					Dian_Huo_OFF();
					WTS_Gas_One_Close();
					
					PWM_Adjust(99);
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							
						}
					if(sys_time_up)
						{
							sys_time_up = 0;
							Dian_Huo_Air_Level();//���Ƶ����ٳ���
							delay_sys_sec(6000);  //�趨�´ε��ʱ��Ϊ15sec��
							Ignition_Index =4;//�л����̣�׼���ٴε��,�����ͣ
						}  

					break;
			

			default:
					sys_close_cmd();
					break;
		}

		return 0;
		
}




/**
* @brief  �������ʱ������״̬������δ�����л��棬���Է��ţ�ȼ�ջ��ȱ�����¯�峬�µȽ��ǣ��쳣�������쳣������������
* @param   �����Ϻ��쳣���з��룬ȼ��ѹ����ϵͳ���к͵���м��
  * @retval ��
  */
void Auto_Check_Fun(void)
{

	uint8 Error_Buffer = 0;
		//***********��ȡ��ת���ڵ�����*************//
		Get_IO_Inf(); //��ȡIO��Ϣ
	
		//����ʱ����Ӧ�ñպϣ��������쳣��˵������û������
		 if(IO_Status.Target.Air_Door == AIR_CLOSE) 
		 	if(sys_flag.Error_Code  == 0 )
				sys_flag.Error_Code  = Error9_AirPressureBad;
		
		if(IO_Status.Target.gas_low_pressure == GAS_OUT)
			{
				//��ȼ��ѹ���ͣ����ϣ�����
				
				if(sys_flag.Error_Code  == 0 )
					sys_flag.Error_Code  = Error3_LowGas; //ȼ��ѹ���͹��ϱ���
			}
		
//��⼫��ˮλ����
		if (IO_Status.Target.water_protect== WATER_LOSE)
 			{
					Error_Buffer = OK;		
 			}

		if(Error_Buffer)
			sys_flag.Error5_Flag = OK;
		else
			{
				sys_flag.Error5_Flag = 0;
				sys_flag.Error5_Count = 0;
			}
			
	
		if(sys_flag.Error5_Count >= 5)  //ԭ�趨 7�룬 �ָĳ�10�� 2022��5��6��10:11:58
			{
				 //�����У�����ˮλȱˮ����	
				sys_data.Data_12H = 6; // �¶ȸ����û��趨ֵ0.01kg
				Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼

				sys_flag.Error5_Flag = 0;
				sys_flag.Error5_Count = 0;
			}
								
//����̽�������

		if(sys_flag.flame_state == FLAME_OUT) //����0ʱ�����޻����ź�
			{
					Send_Gas_Close();//ȼ������ر�

					sys_flag.FlameOut_Count++;
					if(sys_flag.FlameOut_Count >= 3)
						{
							sys_flag.Error_Code  = Error12_FlameLose;
						}
					else
						{
							sys_data.Data_12H = 6; // �¶ȸ����û��趨ֵ0.01kg
							Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼
						}
			}
		
		if(sys_flag.FlameOut_Count)
			{
				//�������ȼ�հ�Сʱ���Զ���Ϩ���¼����
				if(sys_flag.FlameRecover_Time >= 600)  //����ʱ�䣬10�����ȶ����оͲ���
					{
						sys_flag.FlameOut_Count = 0;
					}
					
			}
		
		

	 

		if(Temperature_Data.Pressure_Value >= (Sys_Admin.DeviceMaxPressureSet - 3))
			{
				 if(Temperature_Data.Pressure_Value <= 250)
				 	{
				 		sys_flag.Error_Code = Error2_YaBianProtect;
				 	}
				 else
				 	{
				 		sys_flag.Error_Code = Error4_YaBianLoss;
				 	}
					
			}
	

		 
}

	
/**
* @brief  �����ʱ������״̬����ȼ�ջ��ȱ�����ȼ��ѹ��״̬���Ƚ��ǹ��ϣ����뱨����ʾ��
* @param  ���������¯�峬��,���Է���
  * @retval ��
  */
void Ignition_Check_Fun(void)
{
		
		Get_IO_Inf(); //��ȡIO��Ϣ

		

	 	//ȼ��ѹ��״̬���
		if(IO_Status.Target.gas_low_pressure == GAS_OUT)
		{
				
				//��ȼ��ѹ���ͣ����ϣ�����
				
				sys_flag.Error_Code = Error3_LowGas;
				
		}
		
	


	

}
	
	

		

/**
  * @brief  ������ʱ������״̬������δ�����л��棬ȼ�ջ��ȱ�����¯�峬�µȽ��ǹ��ϣ����뱨����ʾ
* @param  �������Է��ź���������
  * @retval ��
  */
uint8 Idel_Check_Fun(void)
{
	//***********ˮλ�����һֱҪ���*************//
		
	 if(sys_flag.Error_Code )
	 		return 0;//����й��ϣ�ֱ���˳������ٽ��м��

	
	

	 
	  Get_IO_Inf(); //��ȡIO��Ϣ

	
	
	if (IDLE_INDEX == 0)
		{
		 if(sys_flag.flame_state == FLAME_OK)
			{
				if(sys_flag.Error_Code == 0 )
					sys_flag.Error_Code = Error7_FlameZiJian;
					 //����ʱ���϶�û�л��棬����̽�������ϱ���
			}
		
		}
		


	 
	 

		return 0 ;
		
}







uint8 System_Pressure_Balance_Function(void)
 	{
		

		static	uint16  Man_Set_Pressure = 0;  //1kg = 0.1Mpa  ����ϵͳȫ�ֱ������û��ɵ���,����ʾ�¶�ʱ��300 = 30.0��
		static  uint8 	air_min = 0;//��С����
		static  uint8   air_max = 0;//������
		static	uint16  	stop_wait_pressure = 0; //���ڴﵽĿ���趨ֵʱ��������ʼ�¼ 
		uint8  Tp_value = 0; //���ڷ�������м�ֵ
		 

	
	
		air_min = Sys_Admin.Dian_Huo_Power;//ȡ�����Ϊ��С���й���

		air_max = Sys_Admin.Max_Work_Power;  //���������й��ʽ��б߽籣��
		if(air_max >= 100)
			air_max = 100;

		if(air_min < 30)
			air_min = 30;

	Tp_value = sys_data.Data_1FH;
		
	//׷�٣��ܿط���Ĺ���	
	if(sys_flag.Pid_First_Start == 0)
		{
			PID.Out_Put = sys_data.Data_1FH *100;
			PID.Old_Put = PID.Out_Put;
			sys_flag.Pid_First_Start = OK;
		}
	else
		{
			Solo_Pid_Cal_Function();
		}
			
			Tp_value = PID.Out_Put / 100;  //ȡ��
		
		if(Tp_value >air_max)
			Tp_value = air_max;

		if(Tp_value < air_min)
			Tp_value = air_min;

	

		
		PWM_Adjust(Tp_value);

		if(Temperature_Data.Pressure_Value >= sys_config_data.Auto_stop_pressure)
			{
				sys_data.Data_12H = 1; // ѹ�������趨ֵ
				Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼
				sys_flag.Pid_First_Start = 0;
			}


		
		
 		return  OK;
 	}


uint8 XB_System_Pressure_Balance_Function(void)
 	{
		

		static	uint16  Man_Set_Pressure = 0;  //1kg = 0.1Mpa  ����ϵͳȫ�ֱ������û��ɵ���,����ʾ�¶�ʱ��300 = 30.0��
		static  uint8 	air_min = 0;//��С����
		static  uint8   air_max = 0;//������
		static	uint16  	stop_wait_pressure = 0; //���ڴﵽĿ���趨ֵʱ��������ʼ�¼ 
		uint8  Tp_value = 0; //���ڷ�������м�ֵ

/*************************����������**************************************************/
		uint16 Real_Pressure = 0;
		static uint8   Yacha_Value = 65;  //�̶�ѹ��0.45Mpa��ԭ��65�����ڵ�����
		uint16 Max_Pressure = 150;  //15����  1.50Mpa
/******************************************************************************************/
	if(Sys_Admin.Device_Style == 1 || Sys_Admin.Device_Style == 3) 
		{
			//�����������ѹ�������Ǹ����û����趨ѹ��������
			
			Yacha_Value = 65;
			
			Real_Pressure = Temperature_Data.Inside_High_Pressure;
		}
	else
		{
			//��������
			Yacha_Value = 0;
			Real_Pressure = Temperature_Data.Pressure_Value;
		}

	
		air_min = Sys_Admin.Dian_Huo_Power;//ȡ�����Ϊ��С���й���

		air_max = Sys_Admin.Max_Work_Power;  //���������й��ʽ��б߽籣��
		if(air_max >= 100)
			air_max = 100;

		if(air_max < 20)
			air_max = 25;


		
		Man_Set_Pressure =sys_config_data.zhuan_huan_temperture_value + Yacha_Value;   // 
		stop_wait_pressure = sys_config_data.Auto_stop_pressure + Yacha_Value;
		
	
		
		Tp_value = now_percent;	

		if(Temperature_Data.Pressure_Value < sys_config_data.zhuan_huan_temperture_value)
			{
				
				if(Real_Pressure < Man_Set_Pressure ) 
					{
					
						if(sys_flag.Pressure_ChangeTime > 6) //8�����ϱ仯0.01����0.1Mpa ��Ҫ100������ʱ��̫���� ����仯ʱ��̫�̣�����С��2��仯0.01��
							{
								sys_flag.get_60_percent_flag = OK; //��Ҫ����
							}
		
						if(sys_flag.Pressure_ChangeTime <= 5)
							{
								sys_flag.get_60_percent_flag = 0;  //����仯���ʷ���
							} 
		
						
						if(sys_flag.get_60_percent_flag)
							{
								if(sys_flag.Power_1_Sec)
									{
										sys_flag.Power_1_Sec = 0;
										Tp_value = Tp_value + 1;
									}
							}
						else
							{
								if(sys_flag.Power_5_Sec)
									{
										sys_flag.Power_5_Sec = 0;
										Tp_value = Tp_value + 1;
									}
							}
						
					}
					
			}


		
		
		if(Real_Pressure == Man_Set_Pressure)
			{
		
				if(now_percent > 80)//ǰ���Ǳ������40
					{
						Tp_value = 80;
					}
			/*��������д����ȱ�ݣ����������С��λ*/	

				
				sys_flag.get_60_percent_flag = 1;//ȼ��Ԥ�������
			}

		/********************�������ͣ���ѹ���ⲿѹ������Ҫ�Ƚ��趨ѹ��**********************************************/
		if(Real_Pressure > Man_Set_Pressure  || Temperature_Data.Pressure_Value >= sys_config_data.zhuan_huan_temperture_value)
			{
				//˥���ٶ�Ϊÿ���1

				if(Temperature_Data.Pressure_Value >= (sys_config_data.zhuan_huan_temperture_value ))
					{
						//��������ͬʱ���㣬����������
						if(Real_Pressure > Man_Set_Pressure)
							{
								//�������ߣ�Ҫ������
								if(now_percent > 80)//ǰ���Ǳ������40
									{
										Tp_value = 70;
									}
								if(sys_flag.Power_1_Sec)
									{
										sys_flag.Power_1_Sec = 0;
										Tp_value = Tp_value - 1;
									}
								
							}
						else
							{
								//�ڲ�ѹ��С�����ֵ�����ѹ���ߵ����ޣ����ʵ�������
								if(Temperature_Data.Pressure_Value >= (sys_config_data.zhuan_huan_temperture_value + 2 ))
									{
										if(sys_flag.Power_1_Sec)
											{
												sys_flag.Power_1_Sec = 0;
												Tp_value = Tp_value - 1;
											}
									}
								else
									{
										if(Real_Pressure <= (Man_Set_Pressure - 10) )
											{
												if(sys_flag.Power_1_Sec)
													{
														sys_flag.Power_1_Sec = 0;
														Tp_value = Tp_value + 1;
													}
											}
									}
								
								
							}
						
							
					}
				else
					{
						//û�е����û��趨ѹ���������ڲ�ѹ���Ѿ������趨ֵ����Ҳ��Ҫ������
							if(Real_Pressure > Man_Set_Pressure)
								{
									if(sys_flag.Power_1_Sec)
										{
											sys_flag.Power_1_Sec = 0;
											Tp_value = Tp_value - 1;
										}
								}
					}

				
			}	
			

		now_percent = Tp_value;

		if(now_percent >air_max)
			now_percent = air_max;

		if(now_percent < air_min)
			now_percent = air_min;

						
	 

		if(now_percent >= 70)
			sys_flag.get_60_percent_flag = 1;//ȼ��Ԥ�������

		
		PWM_Adjust(now_percent);

		//�������ѹ�������趨ѹ��0.05Mpa���ϣ���ͣ¯
		if(Real_Pressure >= stop_wait_pressure  || Temperature_Data.Pressure_Value >= sys_config_data.Auto_stop_pressure)
			{
				sys_data.Data_12H |= Set_Bit_4; // �¶ȸ����û��趨ֵ0.01kg
				Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼
			 
			}
		
 		return  OK;
 	}




/**
	 * @brief  ϵͳ���й����У��쳣��Ӧ�䴦�����쳣�����������ۼӣ�����Ӧ����������Ϊϵͳ����
	 * @param    ����ʱ����Ϩ���쳣
							 ¯�ڳ����쳣
							 �������رպ��쳣
							 ȼ�����ȱ��������쳣
  * @retval ��
  */
void  Abnormal_Events_Response(void)
{
		
	static uint16 Compare_Pressure = 0 ;
//�������쳣ʱ����һ����ִ�йر�ȼ�����飬�����ʱ��ɨ����ɨʱ�䣬�û��ɵ�
	if(LCD4013X.DLCD.UnionControl_Flag)
		{
			Compare_Pressure = sys_config_data.zhuan_huan_temperture_value;
		}
	else
		{
			Compare_Pressure = sys_config_data.Auto_start_pressure;
		}
	  
		
		if (sys_data.Data_12H)
			{
			switch (ab_index)
				{
					case 0:
						   Send_Air_Open(); 
						   Dian_Huo_OFF();
						   Send_Gas_Close();//�ر�ȼ������	
						   PWM_Adjust(99);
						   sys_flag.Pid_First_Start = 0;

						   sys_flag.AirWork_Time = 0; //��������ʱ��
						   if(sys_data.Data_12H == 6 || sys_data.Data_12H == 1 || sys_data.Data_12H == 5)
						   	{
						   		//����ˮλ��ع��ϻ������л���Ϩ���Զ���ת
						   		 delay_sys_sec(20000);//ִ�к�ɨ��ʱ5��
						   		 ab_index = 1; //��ת����
						   	}
						   else
						   	{	
						   		 if(sys_data.Data_12H == 3)
						   		 	{
						   		 		delay_sys_sec(6000);//�����Զ���������
						   		 		ab_index = 10; //��ת����
						   		 	}
								 else
								 	{
								 		//�����������������
								 		delay_sys_sec(6000);//�����Զ���������
						   		 		ab_index = 1; //��ת����
								 	}
						   		 
						   	}
						   
						   	
							break;
					case 1:
							//��ǿ��Ϩ��LCDͼ��
						
							Send_Gas_Close();//�ر�ȼ������
							Dian_Huo_OFF();
							PWM_Adjust(99);

							if(IO_Status.Target.water_high == WATER_LOSE)
								{
									sys_flag.Force_Supple_Water_Flag = OK;
								}
							else
								{
								//NOP
								}
					
								if(IO_Status.Target.water_high == OK)
									{
										sys_flag.Force_Supple_Water_Flag = 0;
									}
								else
									{
										//NOP
									}
	
							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
								//NOP
								}
								
							if(sys_time_up)
								{
									sys_time_up = 0;
									 delay_sys_sec(1000);
									ab_index = 2; //��ת����
									
								}
							else
								{
								//NOP
								}

							
							break;
					case 2:
							//��ǿ��Ϩ��LCDͼ��
						
							Send_Gas_Close();//�ر�ȼ������
							Dian_Huo_OFF();
							PWM_Adjust(33); //�󴵹��ʱ�ʶ
					
							if(IO_Status.Target.water_high == WATER_LOSE)
								{
									sys_flag.Force_Supple_Water_Flag = OK;
								}
							else
								{
								//NOP
								}
								
							
						
							if(IO_Status.Target.water_high == OK)
								{
									sys_flag.Force_Supple_Water_Flag = 0;
								}
							else
								{
									//NOP
								}
							
	
							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
								}
								
							if(sys_time_up)
								{
									sys_time_up = 0;
									ab_index = 3; //��ת����
									delay_sys_sec(500);//
									sys_flag.Force_Supple_Water_Flag = 0;
									
									/*��鼫��ˮλ��״��*/
									if (IO_Status.Target.water_protect== WATER_LOSE)
										{
											sys_flag.Error_Code  = Error5_LowWater;
										}
									else
										{
											
										}

									if(sys_flag.flame_state == FLAME_OK)
										{
											 //��ʱ���϶�û�л��棬����̽�������ϱ���
											sys_flag.Error_Code = Error7_FlameZiJian;
										}

									
									/***ˮλ�߼������ж�***/
									if (IO_Status.Target.water_high== WATER_OK)
										{
											if(IO_Status.Target.water_mid== WATER_LOSE ||IO_Status.Target.water_protect == WATER_LOSE)
												{
													if(sys_flag.Error_Code == 0)
														sys_flag.Error_Code = Error8_WaterLogic;
												}
												
										}
								
								
									if (IO_Status.Target.water_mid== WATER_OK)
										{
											if(IO_Status.Target.water_protect == WATER_LOSE)
												{
													if(sys_flag.Error_Code == 0)
														sys_flag.Error_Code = Error8_WaterLogic;
												}
												 
										}
									
								}
							else
								{
									
								}

							
							break;
					
					case 3:
						 
							Dian_Huo_OFF();
						    Send_Gas_Close();//�ر�ȼ������	
							PWM_Adjust(33);
						
							if(sys_flag.flame_state == FLAME_OK)
								{
									 //��ʱ���϶�û�л��棬����̽�������ϱ���
									sys_flag.Error_Code = Error7_FlameZiJian;
								}
							
							if(sys_time_start == 0)
								{
									sys_time_up = 1;	
								}
							else
								{
									
								}
								
							if(sys_time_up)
								{
									sys_time_up = 0;
									 
									ab_index = 4; //��ת����
									 
								}
							else
								{
									
								}
							
							break;

				case 10:
							 Send_Gas_Close();
							 
							if(Auto_Pai_Wu_Function())
								{
									ab_index = 4; 
									Abnormal_Events.target_complete_event = OK;
									sys_flag.Paiwu_Flag = 0;
								}
							else
								{
									
								}
							
							break;
					case 4:
								Send_Air_Open();  //���ǰ��ɨ
								Abnormal_Events.target_complete_event = OK;
								if (Abnormal_Events.target_complete_event)
									{
										//˫����Ҫ�򿪣������ر�
										if(Temperature_Data.Pressure_Value <= Compare_Pressure)
											{
												Dian_Huo_OFF();
												Send_Gas_Close();//�ر�ȼ������
												sys_data.Data_12H = 0 ;// �¶ȵ���ͣ¯ֵ
												Abnormal_Events.target_complete_event = 0;
												memset(&Abnormal_Events,0,sizeof(Abnormal_Events));//���쳣�ṹ������
												ab_index = 0;  //��index��ʼ���������´ν���
												sys_data.Data_10H = SYS_WORK;// ���빤��״̬
												Sys_Staus = 2; //ϵͳ������2�׶Σ���������
												Sys_Launch_Index = 1; //���е��ǰ���
												Ignition_Index = 0;  //������ת��ַ�����ǰһ�׶�
												Send_Air_Open();  //���ǰ��ɨ 						
												delay_sys_sec(1000);//�ӳ�12s
											}
											

									}
							
							break;
					default:
						sys_close_cmd();
						break;
				}
			}
		else
			{
				ab_index = 0;  //��index��ʼ���������´ν���
			}
			


		
	
	
	
}
/**
  * @brief  ϵͳ���г���
* @param   Sys_Launch_Index�������л�ϵͳ���в���
  * @retval ��
  */
void Sys_Launch_Function(void)
{
		switch(Sys_Launch_Index)
		{
			case  0: //ϵͳ�Լ�
						Self_Check_Function();//���ȼ��ѹ���ͻ�еʽѹ��������
						
						if(Temperature_Data.Pressure_Value >= (Sys_Admin.DeviceMaxPressureSet - 3))
							{
								 if(Temperature_Data.Pressure_Value <= 250)
								 	{
								 		sys_flag.Error_Code = Error2_YaBianProtect;
								 	}
								 else
								 	{
								 		sys_flag.Error_Code = Error4_YaBianLoss;
								 	}
									
							}
						if(Before_Ignition_Prepare())
						{
								Ignition_Index = 0;
								Sys_Launch_Index = 1;//��ת���¸����̣����׶�
								
						}
						
					break;
			
			case  1: //ϵͳ������
						
						Ignition_Check_Fun();
						if(Sys_Ignition_Fun())
							{
								Sys_Launch_Index = 2;//�л�ϵͳ���̵�������ת״̬
							
								Ignition_Index = 0; //��λ��������ת����

								delay_sys_sec(2000);//����Ҫ��û�У�����һ�׶εĳ���ִ�в����� 

								sys_data.Data_12H = 0; //���쳣����¼��λ
								Abnormal_Events.airdoor_event = 0;
								Abnormal_Events.burner_heat_protect_count = 0;
								Abnormal_Events.flameout_event = 0;
								Abnormal_Events.overheat_event = 0;

								sys_flag.WaterUnsupply_Count = 0; //��ʱ��δ��ˮ��־��ʱ����
							}
						Self_Index = 0;
						ab_index =0;
						Air_Door_Index = 0;

				break;
			
			case  2: //ϵͳ����
			
						sys_flag.Force_Supple_Water_Flag = FALSE; //����״̬�ر�ǿ�Ʋ�ˮ
						sys_flag.Already_Work_On_Flag = OK ;
								
					    if(sys_data.Data_12H == 0)
					    	{
					    		Auto_Check_Fun();  //��û���쳣ʱ��ִ��IO�͸��������
				   				System_Pressure_Balance_Function();
								
								
								if(sys_flag.Paiwu_Flag)
									sys_data.Data_12H = 3 ;//��Ҫ�������۵ı�־
					    	}
						else//�쳣״̬��һЩ״̬���ļ��
							{
								Abnormal_Check_Fun();
							}
	
						Abnormal_Events_Response(); //�쳣���
						
					break;
			
			default:
					sys_close_cmd();
					Sys_Launch_Index = 0;
					break;
		}	
}





void Abnormal_Check_Fun(void)
{
	//���ȼ��ѹ���Ƿ����������������Ƿ�������¯����ˮ�Ƿ���
	//	Get_IO_Inf();  //��ֹ�ظ����ˮλ

		if(IO_Status.Target.hot_protect == THERMAL_BAD)
			{
				if(sys_flag.Error15_Flag == 0)
					sys_flag.Error15_Count = 0;
				
				sys_flag.Error15_Flag = OK;

				if(sys_flag.Error15_Count > 1)
					{
						if(sys_flag.Error_Code == 0 )
							sys_flag.Error_Code = Error15_RebaoBad;
					}
				
			}
		else
			{
				sys_flag.Error15_Flag = 0;
				sys_flag.Error15_Count = 0;
			}

		
		//��еʽѹ������ź�	
		if(IO_Status.Target.hpressure_signal == PRESSURE_ERROR)
			{
				if(sys_flag.Error1_Flag == 0)
					sys_flag.Error1_Count = 0;
				
				sys_flag.Error1_Flag = OK;		
				//������ѹ��������ȫ��Χ�����ϣ�����
				if(sys_flag.Error1_Count > 1)
					{
						 if(sys_flag.Error_Code == 0 )
							sys_flag.Error_Code = Error1_YakongProtect; //����ѹ��������ȫ��Χ����	
					}
			}
		else
			{
				sys_flag.Error1_Flag = 0;
				sys_flag.Error1_Count = 0;
			}
	
		
		if(IO_Status.Target.gas_low_pressure == GAS_OUT)
			{
				//��ȼ��ѹ���ͣ����ϣ�����
				if(sys_flag.Error_Code  == 0 )
					sys_flag.Error_Code  = Error3_LowGas; //ȼ��ѹ���͹��ϱ���
				
			}
		


	
		if(Temperature_Data.Pressure_Value >= (Sys_Admin.DeviceMaxPressureSet - 3))
			{
				 if(Temperature_Data.Pressure_Value <= 250)
				 	{
				 		sys_flag.Error_Code = Error2_YaBianProtect;
				 	}
				 else
				 	{
				 		sys_flag.Error_Code = Error4_YaBianLoss;
				 	}
					
			}



	
	

	
		
}

//ˢ��LCD������Ϣ��¼����
void Lcd_Err_Refresh(void)
{
	
	
}

void Lcd_Err_Read(void)
{
	
	
}

 

void  Err_Response(void)
{
	static uint8 Old_Error = 0;
	//����й��ϱ�����ͣ¯��14H��15HΪ��������
	  if(sys_flag.Error_Code == 0)
	  	{
	  		if(sys_flag.Lock_Error)
				sys_flag.tx_hurry_flag = 1;//�����������ݸ�������
				
	  			sys_flag.Error_Code = 0;
	  			sys_flag.Lock_Error = 0;//�Թ��Ͻ���
				Beep_Data.beep_start_flag = 0;	//�����������
					
	  	}


	  
	 //������ǰ���Ǳ����ȼ���
	 if(sys_flag.Lock_Error == 0)
	 	{
	 		if(sys_flag.Error_Code )
				{
			 		sys_close_cmd();
			 		sys_flag.Lock_Error = 1;  //�Թ��Ͻ�������
			 		sys_flag.Alarm_Out = OK;
			 		Beep_Data.beep_start_flag = 1;//���Ʒ���������	
					
				}
			
	 	}
	 else
	 	{
	 		if(sys_flag.Error_Code )
				{
					if(sys_data.Data_10H == 2)
						{
							sys_close_cmd();
			 				sys_flag.Lock_Error = 1;  //�Թ��Ͻ�������
			 				sys_flag.Alarm_Out = OK;
			 				Beep_Data.beep_start_flag = 1;//���Ʒ���������	
						}
			 		
					
				}
	 		
	 		// sys_flag.Target_Page = 0; //�Ѿ�ǿ����ת��Ϣ������ת��
	 	}

	 	 
				
	  
}


void  IDLE_Err_Response(void)
{
	static uint8 Old_Error = 0;
	//����й��ϱ�����ͣ¯��
	  if(sys_flag.Error_Code == 0)
	  	{
	  		if(sys_flag.Lock_Error)
				sys_flag.tx_hurry_flag = 1;//�����������ݸ�������

			sys_flag.Error_Code = 0;
	  			sys_flag.Lock_Error = 0;  //�Թ��Ͻ���
					Beep_Data.beep_start_flag = 0;	//�����������
					
	  	}

	  
	/******************�����ظ����ϼ�¼��ʱ��************************/
	  if(sys_flag.Old_Error_Count >=1800)
	  	{
	  		Old_Error = 0; //�ٴμ�¼
	  		sys_flag.Old_Error_Count = 0;
	  	}
	  else
	  	{
	  	
	  	}


		//����й��ϱ�����ͣ¯��
		 if (sys_flag.Lock_Error == 0)
 		 	{	
		  		
  				if(sys_flag.Error_Code && sys_flag.Error_Code != 0xFF)
  					{
  						
						Sys_Staus = 0;  //ϵͳ���뱨������
						
						
						
						if(sys_data.Data_10H == 2)
							{
								sys_close_cmd();
							}
						else
							{
								
							}
						
						
						Beep_Data.beep_start_flag = 1;	
						sys_flag.Lock_Error = 1;  //�Թ��Ͻ�������
						sys_flag.Alarm_Out = OK;
						sys_flag.tx_hurry_flag = 1;//�����������ݸ�������
						//ˢ��LCD������Ϣ��¼����
						if(sys_flag.Error_Code != Old_Error)
							{
								Old_Error = sys_flag.Error_Code;
							
							}
						else
							{
							
							}

						sys_flag.Old_Error_Count = 0; //���ϼ�¼ʱ������
						
					
						
						
  					}
		  				
		  			
				
			}
	
	
	  
}



/**
* @brief  ϵͳ�������ر����е�����ȴ������������������ʹ���ָ��
* @param   
  * @retval ��
  */
void System_Idel_Function(void)
{
	//1��	�ùص�ȫ���ص� 
		if(sys_flag.Idle_AirWork_Flag)
			{
				 
				//Send_Air_Open();
				//PWM_Adjust(40);
			}
		else
			{
				
				
			}
		Send_Air_Close();
		PWM_Adjust(0);
		
 		Dian_Huo_OFF();//���Ƶ��̵����ر�
		Send_Gas_Close();//ȼ������ر�
		WTS_Gas_One_Close();

		Solo_Work_ZhiShiDeng_Close();
		
		Auto_Pai_Wu_Function();
	
		 
}

/**
* @brief  ϵͳ�ܿس���
* @param   
  * @retval ��
  */
void System_All_Control()
{
		Sys_Staus = sys_data.Data_10H;

		Water_Balance_Function();//���油ˮģʽ
	//��ˮ����

		if(sys_flag.Work_1S_Flag)
			{
				//ȡ������������е�ʱ�䣬Ȼ����������ڵ������еĴ�ɨʱ�����
				sys_flag.Work_1S_Flag = 0;
				if(sys_data.Data_1FH > 0)
					{
						sys_flag.AirWork_Time++;
					}
				else
					{
						sys_flag.AirWork_Time = 0;
					}
			}
		
		

		switch(Sys_Staus)
			{

					case 0 :	//ϵͳ����

						 switch(IDLE_INDEX)
						 	{
						 		case  0 : //��������״̬  ,, ע�����״̬ѭ��ˮ�õĿ��������ݻ�ˮ�¶�
						 				
						 				sys_flag.Ignition_Count = 0;//����ʱ�Ե���������
										sys_flag.last_blow_flag = 0;//��ɨ״̬������־
									
										 System_Idel_Function( );//�������ܴ���
										//��������������ʵʱ��ʾ�����ϣ�ֻ���Ѳ�ִ��
										 Idel_Check_Fun();
										 IDLE_Err_Response();
										 Sys_Launch_Index = 0;
										break;

								case  1: //�ȴ���ɨ��ʱ
									 
										Send_Gas_Close();//ȼ������ر�
									 	Dian_Huo_OFF();//���Ƶ��̵����ر�
										 
										Get_IO_Inf();
										
										sys_flag.Force_Supple_Water_Flag = 0;
										if(sys_time_start == 0)
											{
												sys_time_up = 1;
											}
										else
											{
												
											}
										if(sys_time_up)
										{
											sys_time_up = 0;
											IDLE_INDEX = 2;//������������״̬
											//�رշ������ɨ�������������
											Send_Air_Close();
											
										}
										break;
								case 2: //�ȴ������������£���ֹ���⣬���10������
									  Send_Air_Close();//�����Դ�ر�
									  Send_Gas_Close();//ȼ������ر�
									  Dian_Huo_OFF();//���Ƶ��̵����ر�
									 
									  Get_IO_Inf();
									  IDLE_Err_Response();

									  
									
	 									sys_time_up = 0;
	 									IDLE_INDEX = 0;//������������״̬
	 									Last_Blow_End_Fun();//��ɨ����ִ�е�λ
	 									sys_flag.Force_Supple_Water_Flag = 0;
										sys_flag.Force_UnSupply_Water_Flag = FALSE ;
	
										break;

								default :
										Sys_Staus = 0;
										IDLE_INDEX = 0;
										break;
						 	}
					
							
						break;
					
					case 2:		//ϵͳ����
						
						Sys_Launch_Function();
						Solo_Work_ZhiShiDeng_Open();
					
						 //���ڿ����ٶ�
						Err_Response();//���д���״̬��Ӧ
						break;
			
					case 3://�ֶ�����״̬
							//�ֶ�ģʽ��1�� ��ˮλ����ʧ���Զ���ˮ����ˮλ��ͣ
							
							//��ת�ٵ�ֵ����LCD��ʾ
							
							
							//Send_Gas_Close();//ȼ������ر�
							
							
			
							break;


					case 4://���ϱ���ģʽ
							
							if(sys_flag.Error_Code == 0)
									{
										if(sys_flag.Lock_Error)
											sys_flag.tx_hurry_flag = 1;//�����������ݸ�������
							
										sys_flag.Error_Code = 0;
										sys_flag.Lock_Error = 0;  //�Թ��Ͻ���
										Beep_Data.beep_start_flag = 0;	//�����������	

										//Ҫ����״̬��ת
									}

							break;


			
					
					default:
						Sys_Staus = 0;
						IDLE_INDEX = 0;
						break;
				
			}
			
			
}
 


uint8   sys_work_time_function(void)
{
//ϵͳ�ۼ�����ʱ��,��¯����ʱ��
	

	 return 0;
			

}


void copy_to_lcd(void)
{
	
	
	
}



void sys_control_config_function(void)
{

//���ÿ���ϵͳĬ�ϲ�������
	uint16  data_temp = 0;
	uint8 temp = 0;


	data_temp =  *(uint32 *)(CHECK_FLASH_ADDRESS);
	if(data_temp != FLASH_BKP_DATA) 
		{
			
			
			LCD10D.DLCD.YunXu_Flag = SlaveG[1].Key_Power; 


			Sys_Admin.ChaYa_WaterLow_Set = 100; //200mm
			Sys_Admin.ChaYa_WaterMid_Set = 180; // 
			Sys_Admin.ChaYa_WaterHigh_Set = 230; // 
			Sys_Admin.InsideWater_AlarmValue = 300;
			
			

			Sys_Admin.Device_Style  = 0;  //0 ���ǳ��浥��1��������1�������������
		
			
			Sys_Admin.LianXu_PaiWu_DelayTime = 10; //Ĭ��15���Ӷ���һ�Σ�ÿ��3��
			Sys_Admin.LianXu_PaiWu_OpenSecs = 4; //���ȵ�1s,Ĭ�Ͽ���3��

			Sys_Admin.Water_BianPin_Enabled = 0;  //Ĭ�ϲ��򿪱�Ƶ��ˮ����
			Sys_Admin.Water_Max_Percent = 45; 
			
			
			Sys_Admin.YuRe_Enabled  = 1; //Ĭ�ϴ򿪸��±���
			Sys_Admin.Inside_WenDu_ProtectValue  = 270;// �����¶�Ĭ��Ϊ270��

			 
		
			Sys_Admin.Special_Secs = 18;
			 
			sys_time_inf.UnPaiwuMinutes = 0;
		
			
			Sys_Admin.Balance_Big_Time = 90;
			Sys_Admin.Balance_Small_Time = 150;
		
			Sys_Admin.DeviceMaxPressureSet = 100; //Ĭ�϶ѹ����10����
			
		//��һ���� ����Ӧ�ṹ�帳ֵ
			Sys_Admin.First_Blow_Time = 25 * 1000;  //ǰ��ɨʱ��
	 	
	
			Sys_Admin.Last_Blow_Time = 20 *1000;//��ɨʱ��
			

			Sys_Admin.Dian_Huo_Power = 30;  //Ĭ�ϵ����Ϊ30% 
		
			Sys_Admin.Max_Work_Power = 100;  //Ĭ�������Ϊ100
			Sys_Admin.Wen_Huo_Time =5 * 1000;  //�ȶ�����ʱ�� 10��

			Sys_Admin.Fan_Speed_Check = 1;  //Ĭ���Ǽ�����	
			
			 Sys_Admin.Fan_Speed_Value = 4800; //Ĭ�Ϸ�����ת��Ϊ6600��

			 Sys_Admin.Fan_Pulse_Rpm = 3;   //Ĭ��ÿת������3����Amtek 

			 Sys_Admin.Fan_Fire_Value = 6500 ; //Ĭ�Ϸ�������ת��Ϊ3500rpm

			 Sys_Admin.Danger_Smoke_Value =  850; //�����¶�Ĭ��ֵΪ80��
			 Sys_Admin.Supply_Max_Time =  320; //��ˮ��ʱĬ�ϱ���ֵΪ300��
			
			 Sys_Admin.ModBus_Address = 0; //Ĭ�ϵ�ַΪ20

			 sys_config_data.Sys_Lock_Set = 0;  //Ĭ�ϲ�������ͣ����
 
		  
		   	sys_config_data.Auto_stop_pressure = 60; //������4kg,ͣ¯Ĭ��Ϊ5kg

			sys_config_data.Auto_start_pressure = 40; //������4kg,����ѹ����1kg������  
	 		sys_config_data.zhuan_huan_temperture_value = 50; //����Ŀ��ѹ��ֵ0.4Mpa
	 		
			
		
		//��һ���� д���ڲ�FLASH
			sys_flag.Lcd_First_Connect = OK;

			
	 	 	 
			 
			Write_Internal_Flash();
			Write_Admin_Flash();
			Write_Second_Flash();
			 
			
			
		}
	else  //˵���Ѿ�д�������������ڴ������������,�����ڲ�FLASH���ݣ���ֵ����Ӧ�ṹ��
		{
				
			Sys_Admin.Fan_Pulse_Rpm = *(uint32 *)(FAN_PULSE_RPM_ADDRESS);

			Sys_Admin.ChaYa_WaterLow_Set =  *(uint32 *)(CHAYA_WATERLOW_SET); //200mm
			Sys_Admin.ChaYa_WaterMid_Set = *(uint32 *)(CHAYA_WATERMID_SET); // 
			Sys_Admin.ChaYa_WaterHigh_Set = *(uint32 *)(CHAYA_WATERHIGH_SET); // 
			Sys_Admin.InsideWater_AlarmValue = *(uint32 *)(INSIDE_WATER_ALARM_VALUE_SET);
			 
			Sys_Admin.Device_Style =  *(uint32 *)(Device_Style_Choice_ADDRESS);
		
			Sys_Admin.Water_BianPin_Enabled = *(uint32 *)(WATER_BIANPIN_ADDRESS);
			Sys_Admin.Water_Max_Percent = *(uint32 *)(WATER_MAXPERCENT_ADDRESS);
			
		
			Sys_Admin.YuRe_Enabled  = *(uint32 *)(WENDU_PROTECT_ADDRESS);

			Sys_Admin.Inside_WenDu_ProtectValue  = *(uint32 *)(BENTI_WENDU_PROTECT_ADDRESS);//�����¶�
		
			Sys_Admin.Access_Data_Mode = *(uint32 *)(ACCESS_DATA_MODE_SET_ADDRESS);	
			
			Sys_Admin.DeviceMaxPressureSet = *(uint32 *)(DEVICE_MAX_PRESSURE_SET_ADDRESS);

			  Sys_Admin.Supply_Max_Time =*(uint32 *)(SUPPLY_MAX_TIME_ADDRESS); 
			
			Sys_Admin.First_Blow_Time = *(uint32 *)(FIRST_BLOW_ADDRESS);  //Ԥ��ɨʱ��
			
		
			Sys_Admin.Last_Blow_Time =  *(uint32 *)(LAST_BLOW_ADDRESS);//��ɨʱ��
			
			
			Sys_Admin.Dian_Huo_Power =  *(uint32 *)(DIAN_HUO_POWER_ADDRESS);  //�����

			Sys_Admin.Max_Work_Power = *(uint32 *)(MAX_WORK_POWER_ADDRESS);  //Ĭ�������Ϊ100
			
			Sys_Admin.Wen_Huo_Time = *(uint32 *)(WEN_HUO_ADDRESS);  //�ȶ�����ʱ��  

			Sys_Admin.Fan_Speed_Check = *(uint32 *)(FAN_SPEED_CHECK_ADDRESS);  //�Ƿ���з��ټ��
			
			Sys_Admin.Fan_Speed_Value = *(uint32 *)(FAN_SPEED_VALUE_ADDRESS);  //�Ƿ���з��ټ��
			Sys_Admin.Fan_Fire_Value = *(uint32 *)(FAN_FIRE_VALUE_ADDRESS);

			Sys_Admin.Danger_Smoke_Value = *(uint32 *)(DANGER_SMOKE_VALUE_ADDRESS);
			 
			
			 Sys_Admin.ModBus_Address = *(uint32 *)(MODBUS_ADDRESS_ADDRESS) ;  
			
			sys_config_data.wifi_record = *(uint32 *)(CHECK_WIFI_ADDRESS);  //ȡ��wifi��¼��ֵ

			sys_config_data.zhuan_huan_temperture_value = *(uint32 *)(ZHUAN_HUAN_TEMPERATURE); //����Ŀ��ѹ��ֵ0.4Mpa

			sys_config_data.Auto_stop_pressure = *(uint32 *)(AUTO_STOP_PRESSURE_ADDRESS); //ȡ���Զ�ͣ¯ѹ��

			sys_config_data.Auto_start_pressure = *(uint32 *)(AUTO_START_PRESSURE_ADDRESS);//ȡ���Զ���¯ѹ��

			

			/**********************��ʷ������Ϣ��ȡ  *************************************/
			
			/**********************��ʷ������Ϣ��ȡ  ����*************************************/		
			
		}

		
		
	    

	 

  //���գ������ݷ���LCDչʾ
	
	
}



//��������Ϣ������ת��Ϊbit,��������ˢ��lcd������
uint8  byte_to_bit(void)
{
	 

	


		return 0;
}













//����LCD����MCU������
void Load_LCD_Data(void)
{
	
}





void clear_struct_memory(void)
{
	uint8 temp = 0;
		//�Խṹ�������ʼ��	
	memset(&sys_data,0,sizeof(sys_data));	//��״̬��Ϣ�ṹ������
  	memset(&lcd_data,0,sizeof(lcd_data));	//��״̬��Ϣ�ṹ������
	memset(&sys_time_inf,0,sizeof(sys_time_inf));	//��״̬��Ϣ�ṹ������
	
	memset(&sys_config_data,0,sizeof(sys_config_data));	//��״̬��Ϣ�ṹ������
	
	
	memset(&Switch_Inf,0,sizeof(Switch_Inf));//��ϵͳ��־����������
	memset(&Abnormal_Events,0,sizeof(Abnormal_Events));//���쳣�ṹ������
	memset(&sys_flag,0,sizeof(sys_flag));//��ϵͳ��־����
	
	memset(&Flash_Data,0,sizeof(Flash_Data));
	memset(&Temperature_Data,0,sizeof(Temperature_Data));
	 
	
	
}








void One_Sec_Check(void)
{
	float Fbuffer = 3.3 ;
 	 
	 //�������Ч������֤��������������
	if(sys_flag.Relays_3Secs_Flag)
		{
			sys_flag.Relays_3Secs_Flag = 0;
		 
			Float_Int.byte4.data_LL = 0x19;
			Float_Int.byte4.data_LH =0xE0;
			Float_Int.byte4.data_HL = 0xC0;
			Float_Int.byte4.data_HH = 0X41;
		//	u1_printf("\n*�ѹ����ֵ��= %d\n",Sys_Admin.DeviceMaxPressureSet);
		//	u1_printf("\n*�洢����ֵ��= %d\n",sys_flag.Attention_Flag);
		//	u1_printf("\n* ����#1 ���ʹ���= %d,  ���մ���= %d\n", SlaveG[1].Send_Count,SlaveG[1].Rec_Count);
		//	u1_printf("\n* ����#2 ���ʹ���= %d,  ���մ���= %d\n", SlaveG[2].Send_Count,SlaveG[2].Rec_Count);
		//	u1_printf("\n* ���ٵı�־ = %d\n",PID.Down_Flag);
		//	u1_printf("\n* ��ǰ�׶� = %f \n",Float_Int.value);


			//u1_printf("\n*��ַ������= %d\n",sys_flag.Address_Number);
	//	u1_printf("\n* ����2�����ɵ�ʱ��ֵ = %d\n",SlaveG[2].Big_time);
	//	u1_printf("\n* ����1LOW���ɵ�ʱ��ֵ = %d\n",SlaveG[1].Small_time);
	//	u1_printf("\n* ����2LOW���ɵ�ʱ��ֵ = %d\n",SlaveG[2].Small_time);
		//	u4_printf("\n* ����������־ = %d\n",AUnionD.UnionStartFlag);
		//	u4_printf("\n* ���н׶� = %d\n",AUnionD.Mode_Index);

		
		

			
		}
	 
	
	
		



				



	

	
//��ӡ������Ϣ
	if(sys_flag.two_sec_flag)
		{
			sys_flag.two_sec_flag = 0;
			
			//sys_flag.LianxuWorkTime
			//u1_printf("\n* ���õ�ʱ��= %d\n",Sys_Admin.LianXu_PaiWu_DelayTime);
			//u1_printf("\n* �Ѿ����е�ʱ��= %d\n",sys_flag.LianxuWorkTime);
			//u1_printf("\n* s���ÿ�����ʱ��= %d\n",Sys_Admin.LianXu_PaiWu_OpenSecs);
			
			//u1_printf("\n* ������ʱ��= %d\n",sys_flag.Lianxu_OpenTime);
		//	u1_printf("\n* ��ˮ�ı�־= %d\n",Switch_Inf.water_switch_flag);

			
		}

		
	
	
}



uint8  sys_start_cmd(void)
{
		

		if(sys_flag.Lock_System)
			{
				//��ת�����Ͻ��棬���޹��ϴ�����ʾ
				
				return 0 ;
			}
		
		 
		if(sys_flag.Error_Code )
			{
					 	Sys_Staus = 0;  // 
						sys_data.Data_10H = 0x00;  //ϵͳֹͣ״̬
						sys_data.Data_12H = 0x00; //�Է��������쳣��������

						
						
						delay_sys_sec(100);// 
					
						IDLE_INDEX = 1; 

						sys_flag.Lock_Error = 1;  //�Թ��Ͻ�������
						sys_flag.tx_hurry_flag = 1;//�����������ݸ�������
						Beep_Data.beep_start_flag = 1;	
						
			}
		else
			{
				if(sys_data.Data_10H == 0)
					{
						IDLE_INDEX = 0;  //��ֹ�ں�ɨʱ�����
						Sys_Staus = 2;
						Sys_Launch_Index = 0;
						sys_flag.before_ignition_index = 0;
						Ignition_Index = 0;
						sys_time_up = 0;	

	   					 sys_data.Data_10H = 0x02;  //ϵͳ����״̬
					
						sys_flag.Paiwu_Flag = 0; //����д������ʲôԭ����
						
						
	    				sys_time_start = 0; //�������״̬�£����ܴ��ڵ���ʱ�ȴ�����ֹ�����ϵͳ
					/************�Դ���ѭ���ù���ʱ���������*****************8*/
						
						sys_flag.Already_Work_On_Flag = FALSE;
					
						sys_flag.get_60_percent_flag = 0;
						sys_flag.Pai_Wu_Idle_Index = 0;

						sys_flag.before_ignition_index = 0;	
						sys_flag.tx_hurry_flag = 1;//�����������ݸ�������											
	    				Dian_Huo_OFF();//���Ƶ��̵����ر�
	    				//LCD�л�����ҳ��
	    				
					}
				
				
			}
	    
		
	return 0;							
}


void sys_close_cmd(void)
{
			sys_data.Data_10H = 0x00;  //ϵͳֹͣ״̬
																		
			
			//ϵͳֹͣ���Թؼ����ݽ��д洢
		 	WTS_Gas_One_Close();
		  	
			sys_flag.Force_Supple_Water_Flag = 0;
			Abnormal_Events.target_complete_event = 0;
			Dian_Huo_OFF();//�رյ��̵���
			Send_Gas_Close();//�ر�ȼ������ 
			
			sys_flag.get_60_percent_flag = 0;
			
		  //���ϴγ����п��ܴ��ڵ��쳣״̬������0
		memset(&Abnormal_Events,0,sizeof(Abnormal_Events));	//��״̬��Ϣ�ṹ������			
														
		//���к�ɨ��ʱ
		//�򿪷��������ɨ��ʱ
		//�������״̬1
		//��׼��ת����
		sys_data.Data_10H = SYS_IDLE; // 
		Sys_Staus = 0; // 
		Sys_Launch_Index = 0;
		sys_flag.before_ignition_index = 0;
		Ignition_Index = 0;
		IDLE_INDEX = 1;
		Last_Blow_Start_Fun();
	
}


//��ɨ��ʼִ�г���
void Last_Blow_Start_Fun(void)
{
	//ȷ�Ϸ���Ѿ���
	Send_Air_Open();

	sys_flag.last_blow_flag = 1;//��ɨ״̬��ʼ��־
	
	PWM_Adjust(99);//90%�ķ������к�ɨ
	delay_sys_sec(15000);//���û�ɹ����ʹ���15��
}


/*�����ɨ������־��  ������������λ�����ʧ�ܹ��ϣ�ȼ������й¶���ϣ�ϵͳ�����л���Ϩ��*/

void Last_Blow_End_Fun(void)
{
	//ȷ�Ϸ���ر�
	
			Send_Air_Close();

	sys_flag.tx_hurry_flag = 1;//�����������ݸ�������
	 
	 
	 
	sys_flag.last_blow_flag = 0;//��ɨ״̬������־
}

 




/*��ֹ�û��л����ֶ�����ҳ�棬��ʱ��û���˳��ֶ����ԣ�10���Ӻ��˳��ֶ�����*/



//���ü̵����źſ���������������������ˮλ�ź���,�����ˮλ�߼�����
uint8  Water_Balance_Function(void)
{
	
	uint8 buffer = 0;
			
	
		
	lcd_data.Data_15H = 0;
	if (IO_Status.Target.water_protect== WATER_OK)
				buffer |= 0x01;
		else
				buffer &= 0x0E; 
	
		if (IO_Status.Target.water_mid== WATER_OK)
				buffer |= 0x02;
		else
				buffer &= 0x0D;
	
		
		if (IO_Status.Target.water_high== WATER_OK)
				buffer |= 0x04;
		else
				buffer &= 0x0B;
	
		


//������й����У�����ˮλ��̽���ˮ������
	


		lcd_data.Data_15L = buffer;
		LCD10D.DLCD.Water_State = buffer;

	//��ˮ��ʱ  �� ��ˮ��ʱ���ϴ���
	//��ˮ��ʱ�߼�

	
	if(sys_flag.Error_Code)//����ȱ����Ϻ�ˮλ�߼����󣬲���ˮ
		{
			Feed_Main_Pump_OFF();	
			Second_Water_Valve_Close();
			 return 0;
		}

	 if(sys_data.Data_10H == SYS_MANUAL)   //�ֶ�ģʽ��ˮ����
	 		return 0;



	  if(sys_data.Data_10H == SYS_IDLE)
	 	{
	 		
	 		if(sys_flag.last_blow_flag)
	 			{
	 				/*2023��3��10��09:20:37 �ɳ����źţ��ĳ����źţ���ֹˮ����*/
	 				if( IO_Status.Target.water_mid == WATER_LOSE)
	 					sys_flag.Force_Supple_Water_Flag = OK;

					if( IO_Status.Target.water_mid == WATER_OK)
						sys_flag.Force_Supple_Water_Flag = FALSE;
					
	 			}
			else
				{ 
					//������ɨ������û������ˮλ��ˮ�û��ڹ���������
					
					sys_flag.Force_Supple_Water_Flag = FALSE;
					
					
				}
			if(sys_flag.Force_Supple_Water_Flag) //ǿ�Ʋ�ˮ��־����ǿ�ƴ򿪲�ˮ����
				{
					Feed_Main_Pump_ON();
					Second_Water_Valve_Open();
					 
				}
			 if(sys_flag.Force_Supple_Water_Flag == 0)
			 	{
			 		Feed_Main_Pump_OFF();
					Second_Water_Valve_Close();
			 	}

			return 0;
		
	 		
	 	}
			  

	 if(sys_flag.Force_Supple_Water_Flag) //ǿ�Ʋ�ˮ��־����ǿ�ƴ򿪲�ˮ����
		{
			Feed_Main_Pump_ON();
			Second_Water_Valve_Open();
			return 0;
		}
	
/**************************************************************/
	
	 
	//������ˮλ������ǲ��ܻ��ģ��������ɿ���
	if(sys_flag.Error_Code == 0)
		{
	 		if(IO_Status.Target.water_mid == WATER_LOSE || IO_Status.Target.water_protect == WATER_LOSE)//��ˮλ�źŶ�ʧ�����벹ˮ
	 			{
						Feed_Main_Pump_ON();
						Second_Water_Valve_Open();
	 			}
	
			if(IO_Status.Target.water_high == WATER_OK && IO_Status.Target.water_mid == WATER_OK && IO_Status.Target.water_protect == WATER_OK )
				{
						Feed_Main_Pump_OFF();
						Second_Water_Valve_Close();
				}
				
		}
	else
		{
			Feed_Main_Pump_OFF();	
			Second_Water_Valve_Close();
		}
		

			
	return  0;	
}



//�����ֶ�ģʽһЩ���ܵĴ���
uint8 Manual_Realys_Function(void)
{
	
	
	
	
	//��ˮ��ʱ��ʾ����ֹˮ�������ٲ�
	
	
	return 0;
}

void Check_Config_Data_Function(void)
{
	float ResData = 0;
	
	

	
	
}



void Fan_Speed_Check_Function(void)
{
	
	//Fan_Rpm = (1000/(2* fan_count)) / 3(ÿ������3ת) *60�� = 100000 / sys_flag.Fan_count


		 
		static uint8 Pulse = 2;    //���ַ��ÿת5������
		 
		uint32 All_Fan_counts = 0;
			
		
			//G1G170   0.5T���	ÿת3�����壬  Ametek  0.5T��� ÿת2������
			//G3G250   1T����Ĳ��� ÿת3������
			//G3G315   2T����Ĳ���  ÿת 5������
			if(sys_flag.Rpm_1_Sec)
				{
					sys_flag.Rpm_1_Sec = FALSE;

			

					//����PB0�������жϣ�����
				//	if(Sys_Admin.Fan_Pulse_Rpm >=  10  || Sys_Admin.Fan_Pulse_Rpm == 0)
							Sys_Admin.Fan_Pulse_Rpm = 2; //�������������

				//���NXK83-1100-FZ01   ���ת�ٴﵽ10000ת����

					if(sys_flag.Fan_count > 0 )
						{
							sys_flag.Fan_Rpm = sys_flag.Fan_count * 60 / Sys_Admin.Fan_Pulse_Rpm;
							sys_flag.Fan_count = 0;
							
						}
						  //��������/5��  *60	��60��ָ60�룬����5 ��3��ÿת5������
					else
						{
							sys_flag.Fan_count = 0;
							sys_flag.Fan_Rpm = 0;
						}
						
				
				}


	
}


/*���ھ����̹����������������е�ʱ��*/
uint8 Admin_Work_Time_Function(void)
{
	//�漰���ı�����Flash_Data.Admin_Work_Time��systmtime
	
	
	
	return 0 ;
}









void HardWare_Protect_Relays_Function(void)
{
 	 
 }



uint8 Power_ON_Begin_Check_Function(void)
{
	uint8 Return_Value = 0;
	switch (sys_flag.PowerOn_Index)
		 {
			case 0:
					delay_sys_sec(10000);
					sys_flag.PowerOn_Index = 1;
					break;
			case 1:
					ModBus2LCD4013_Lcd7013_Communication();
					
					if(sys_time_start == 0)
						{
							sys_time_up = 1;
						}
					else
						{
							
						}

					
					if(sys_time_up)
						{
							sys_time_up = 0;
							sys_flag.PowerOn_Index = 2;
							if(sys_flag.Lcd4013_OnLive_Flag && Sys_Admin.ModBus_Address)
								{
									sys_flag.Address_Number = Sys_Admin.ModBus_Address;
								}
							else
								{
									sys_flag.Address_Number = 0;  //��ⲻ��С������������
								}

							
						}
					else
						{
							
						}
					
					break;
			case 2:
					Return_Value= OK;
				
					break;

			default:
					Return_Value= OK;
				
					break;
		 }

	return Return_Value;
}

uint8 IDLE_Auto_Pai_Wu_Function(void)
{
	 
	
	return 0;
}

uint8 Auto_Pai_Wu_Function(void)
{
	static uint8 OK_Pressure = 5;
	static uint8 PaiWu_Count = 0;
	uint8  Paiwu_Times = 3;  //4�ν�ѹ����
	//���������ѹ��С�ڰ빫��ʱ���Զ�����һ��
    
	uint8  Time = 15;//����ѹ����ˮ30��

	uint8 	Ok_Value = 0;
	
	
		//1�� Ҫ��¯Ҫ���й���2���Զ����۹��ܣ�Ҫ����
		
				if(sys_flag.Paiwu_Flag)
					{
						switch (sys_flag.Pai_Wu_Idle_Index)
							{
								case 0:
										
									//	Pai_Wu_Door_Close();
										Pai_Wu_Door_Open();
									if(Temperature_Data.Pressure_Value > OK_Pressure)
										{
											delay_sys_sec(25000);
											
										}
									else
										{
											delay_sys_sec(40000); //��ѹ�������ʱ��
										}
										sys_flag.Pai_Wu_Idle_Index = 2;
										

										break;
								
								case 2:  //��⼫��ˮλ�ж��Ƿ����
										if(sys_time_start == 0)
											{
												sys_time_up = 1;
											}
										else
											{
												
											}
										
										if ( IO_Status.Target.water_protect== WATER_LOSE ) 
											{
												sys_flag.Pai_Wu_Idle_Index = 3;
												delay_sys_sec(100000);//������ˮλ��ˮ��ʱ��
												 Pai_Wu_Door_Close();
											}

										
										if(sys_time_up)
											{
												sys_time_up = 0;
												sys_flag.Force_Supple_Water_Flag = FALSE;
												//	 Pai_Wu_Door_Close();
												 delay_sys_sec(3000); //������ˮλ��ˮ��ʱ��
												sys_flag.Pai_Wu_Idle_Index = 21;
											}
										else
											{
												
											}

										break;
								case 21: 
										//��⵽��ˮλ�󣬵ȴ�3����رշ���
										if(sys_time_start == 0)
											{
												sys_time_up = 1;
											}
										else
											{
												
											}
										
										if(sys_time_up)
											{
												sys_time_up = 0;
												sys_flag.Force_Supple_Water_Flag = FALSE;
												 Pai_Wu_Door_Close();
												 delay_sys_sec(100000); //������ˮλ��ˮ��ʱ��
												sys_flag.Pai_Wu_Idle_Index = 3;
											}
										else
											{
												
											}
											

										break;
								case 3:
										Pai_Wu_Door_Close();
										if(sys_time_start == 0)
											{
												sys_time_up = 1;
											}
										else
											{
												
											}
										if ( IO_Status.Target.water_mid== WATER_OK ) 
											{
												sys_flag.Pai_Wu_Idle_Index = 4;
											}

										if(sys_time_up)
											{
												sys_time_up = 0;
												sys_flag.Force_Supple_Water_Flag = FALSE;
												 Pai_Wu_Door_Close();
												sys_flag.Pai_Wu_Idle_Index = 4;
											}
										else
											{
												
											}

										break;
								
								case 4:
										Pai_Wu_Door_Close();
										sys_flag.Force_Supple_Water_Flag  = 0;
										sys_flag.Paiwu_Flag = FALSE;
										sys_flag.Pai_Wu_Idle_Index = 0;
										Ok_Value = OK;  //�����Զ����۳���
										break;
								
								default:
									sys_flag.Paiwu_Flag = FALSE;
									sys_flag.Pai_Wu_Idle_Index =0;
									Ok_Value = OK; 
										break;
							}
					}
				else
					{
						sys_flag.Pai_Wu_Idle_Index = 0;
						sys_flag.Force_Supple_Water_Flag = FALSE;
						Ok_Value = OK; 
						Pai_Wu_Door_Close();

					}
			
		
		
	return Ok_Value;
}



uint8 YunXingZhong_TimeAdjustable_PaiWu_Function(void)
{
	//�豸���й�����ʹ�øù���
	uint8  set_flag = 0;
	
		


	return set_flag;
}


uint8 PaiWu_Warnning_Function(void)
{
	//���ۼ�ʱ����2E       2F ,30��
	static uint16 Max_Time = 480 ;  //���ʱ��ʱ8Сʱ
	static uint16 Max_Value = 1439; //�����ʾ��ʱ��Ϊ23:59
	static uint8 Low_Flag = 0;

	if(sys_data.Data_10H == SYS_WORK)
		{
			if(sys_flag.Paiwu_Secs >= 60)
				{
					sys_flag.Paiwu_Secs = 0;
					sys_time_inf.UnPaiwuMinutes ++;
					 
				}
				
		}
	else
		{
			sys_flag.Paiwu_Secs = 0;
		}
		
	if(sys_time_inf.UnPaiwuMinutes > Max_Value)
		sys_time_inf.UnPaiwuMinutes = Max_Value;

	if(sys_time_inf.UnPaiwuMinutes > Max_Time)
		{
			lcd_data.Data_2EH = 0;
			lcd_data.Data_2EL = OK; //������ʾͼ���ɫ
			//sys_flag.Paiwu_Alarm_Flag  = OK;
		}
	else
		{
			lcd_data.Data_2EH = 0;
			lcd_data.Data_2EL = 0; //������ʾͼ���ɫ
			//sys_flag.Paiwu_Alarm_Flag  = FALSE;
		}

	
	if(Low_Flag == 0)
		sys_flag.Low_Count = 0;
	if(sys_time_inf.UnPaiwuMinutes > 1) //δ����ʱ�䳬��10���ӣ�ˮλ��ʧ�󣬻������ʱ������
		{
			if (IO_Status.Target.water_protect == WATER_LOSE)
				{
					Low_Flag = OK;
					if(sys_flag.Low_Count >= 3)//�����ˮλ����20����������
						{
							Low_Flag = 0;
							sys_time_inf.UnPaiwuMinutes = 0;
							Write_Second_Flash();
						}
				}
					
		}

	lcd_data.Data_2FH = 0;
	lcd_data.Data_2FL = sys_time_inf.UnPaiwuMinutes / 60; //δ����ʱ�䣺 Сʱ��ʾ
	lcd_data.Data_30H = 0;
	lcd_data.Data_30L = sys_time_inf.UnPaiwuMinutes % 60; //δ����ʱ�䣺 ������ʾ

	
	return 0;
}


uint8 Special_Water_Supply_Function(void)
{
	static uint8 High_Flag = 0;
	//���½�ˮ��ŷ� ���漰�����»�ˮ��ŷ�
	 



	return 0 ;
}



//��ʱ�����ó���Ͷ��ʹ��
uint8 WaterLevel_Unchange_Check(void)
{
	

	return 0;
}


uint8  Water_BianPin_Function(void)
{
	
	uint8 buffer = 0;

	static uint8 Water_Mid_MaxTime = 5;  //5�룬����6���رյ�ŷ�
	static uint8 Water_Mid_Time = 0;
	static uint8 Max_Wait_Time = 10;  //����ˮλ���ȴ�10�� �Ͳ��䣬
	static uint8 Water_High_Flag = 0; //�����ˮλ��־������ɵ�ˮλ������
			
	
		
	lcd_data.Data_15H = 0;
	if (IO_Status.Target.water_protect== WATER_OK)
				buffer |= 0x01;
		else
				buffer &= 0x0E; 
	
		if (IO_Status.Target.water_mid== WATER_OK)
				buffer |= 0x02;
		else
				buffer &= 0x0D;
	
		
		if (IO_Status.Target.water_high== WATER_OK)
				buffer |= 0x04;
		else
				buffer &= 0x0B;
	
		


//������й����У�����ˮλ��̽���ˮ������
	


		lcd_data.Data_15L = buffer;
		LCD10D.DLCD.Water_State = buffer;

	//��ˮ��ʱ  �� ��ˮ��ʱ���ϴ���
	//��ˮ��ʱ�߼�

	
	if(sys_flag.Error_Code)//����ȱ����Ϻ�ˮλ�߼����󣬲���ˮ
		{
			Feed_Main_Pump_OFF();	
			Second_Water_Valve_Close();
			 return 0;
		}

	 if(sys_data.Data_10H == SYS_MANUAL)   //�ֶ�ģʽ��ˮ����
	 		return 0;



	  if(sys_data.Data_10H == SYS_IDLE)
	 	{
	 		
	 		if(sys_flag.last_blow_flag)
	 			{
	 				/*2023��3��10��09:20:37 �ɳ����źţ��ĳ����źţ���ֹˮ����*/
	 				if( IO_Status.Target.water_mid == WATER_LOSE)
	 					sys_flag.Force_Supple_Water_Flag = OK;

					if( IO_Status.Target.water_mid == WATER_OK)
						sys_flag.Force_Supple_Water_Flag = FALSE;
					
	 			}
			else
				{ 
					//������ɨ������û������ˮλ��ˮ�û��ڹ���������
					
					sys_flag.Force_Supple_Water_Flag = FALSE;
					
					
				}
			if(sys_flag.Force_Supple_Water_Flag) //ǿ�Ʋ�ˮ��־����ǿ�ƴ򿪲�ˮ����
				{
					Feed_Main_Pump_ON();
					Second_Water_Valve_Open();
					 
				}
			 if(sys_flag.Force_Supple_Water_Flag == 0)
			 	{
			 		Feed_Main_Pump_OFF();
					Second_Water_Valve_Close();
			 	}

			return 0;
		
	 		
	 	}
			  

	 if(sys_flag.Force_Supple_Water_Flag) //ǿ�Ʋ�ˮ��־����ǿ�ƴ򿪲�ˮ����
		{
			Feed_Main_Pump_ON();
			Second_Water_Valve_Open();
			return 0;
		}
	
/**************************************************************/
	
	 
	//������ˮλ������ǲ��ܻ��ģ��������ɿ���
	if(sys_flag.Error_Code == 0)
		{
	 		if(IO_Status.Target.water_mid == WATER_LOSE || IO_Status.Target.water_protect == WATER_LOSE)//��ˮλ�źŶ�ʧ�����벹ˮ
	 			{
	 				if(Switch_Inf.water_switch_flag == 0)
	 					{
	 						//˵����Ƶʱ��̫��������Ҫ���
	 						Max_Wait_Time = Max_Wait_Time - 1;
							if(Max_Wait_Time < 5)
								{
									Max_Wait_Time = 5;
								}
								
							
	 					}
						Feed_Main_Pump_ON();
						Second_Water_Valve_Open();
						Water_High_Flag = FALSE;
						Water_Mid_Time = 0;
	 			}

			if(IO_Status.Target.water_mid == WATER_OK )
				{
					if(Water_High_Flag == 0)
						{
							if(sys_flag.Water_1s_Flag)
								{
									sys_flag.Water_1s_Flag = 0;
									Water_Mid_Time++;
								}
							if(Switch_Inf.water_switch_flag)
								{
									//����ˮ�ÿ���ʱ����鲹ˮ�ÿ���ʱ�䣬������ˮλ����ʱ����ֹͣˮ�ã���ˮλ����ʱ������
									if(Water_Mid_Time > Water_Mid_MaxTime)
										{
											Water_Mid_Time = 0;
											Feed_Main_Pump_OFF();
											Second_Water_Valve_Close();
										}
								}
							else
								{
									//����ˮ��ֹͣ����ʱ�����ˮ�ùص�ʱ�䣬����ˮ�ùص�ʱ�䣬���ڱ�Ƶʱ�䣬������
									if(Water_Mid_Time > Max_Wait_Time)
										{
											Water_Mid_Time = 0;
											Feed_Main_Pump_ON();
											Second_Water_Valve_Open();
										}
								}
							
							

							
						}
					
				}
	
			if(IO_Status.Target.water_high == WATER_OK && IO_Status.Target.water_mid == WATER_OK && IO_Status.Target.water_protect == WATER_OK )
				{		
						if(Switch_Inf.water_switch_flag)
							{
								Max_Wait_Time  = Max_Wait_Time + 1;
								if(Max_Wait_Time >= 20)
									{
										Max_Wait_Time = 20;
									}
							}
						Feed_Main_Pump_OFF();
						Second_Water_Valve_Close();
						Water_High_Flag = OK;
				}
				
		}
	else
		{
			Feed_Main_Pump_OFF();	
			Second_Water_Valve_Close();
		}
		
	 
	

			
	return  0;	
}


uint8 LianXu_Paiwu_Control_Function(void)
{
	uint32 Dealy_Time = 0;
	uint16 Open_Time = 0; //�������۷���ʵ�ʿ���ʱ���趨����ȷ��0.1s

	uint16 Cong_Work_Time = 0;
	static uint8 Time_Ok = 0;  //����ʱ�䵽�ı�־����̬����
	
	//�������ۿ�����־����������ʱ�������������ۿ���ʱ����

	//������4����ѹ���£�����2�룬��ˮ����1L

	//Sys_Admin.LianXu_PaiWu_Enabled 
	//Sys_Admin.LianXu_PaiWu_DelayTime //���ȵ�0.1Сʱ
	//Sys_Admin.LianXu_PaiWu_OpenSecs //���ȵ�1s

	//ADouble5[1].True.LianXuTime_H���ӻ���ǰ�Ѿ�������ʱ��
	//************��Ҫ�������ӻ�ͬʱ���ۣ���ô���������������ӣ��ӻ�����ԭ��ʱ���趨�������ӳ������ӣ�Ҫ��Ҫ������ۣ�
	//��Ҫ�Ѵӻ�����������ʱ�䣬ͬ����������

	//������Ҫ��ˮ�ò�ˮ�������ܴ�

	//sys_flag.LianXu_1sFlag
	Dealy_Time = Sys_Admin.LianXu_PaiWu_DelayTime * 1 * 60; //0.1h * min  * 60sec/min
	

	Open_Time = Sys_Admin.LianXu_PaiWu_OpenSecs * 10; //�����100ms��λ�����㾫׼����ʱ��

	if(Sys_Admin.Device_Style == 1 || Sys_Admin.Device_Style == 3)
		{
			//�����飬�ü̵����������йѹ
			return 0 ;
		}
	
	if(sys_data.Data_10H == 3)
		return 0;
	

	//����״̬���л���ı�־���ŶԹ�����ʱ�����ͳ��
	if(sys_data.Data_10H == 2)
		{
			if(sys_flag.flame_state)
				if(sys_flag.LianXu_1sFlag)
					{
						sys_flag.LianxuWorkTime ++;//���
						sys_flag.LianXu_1sFlag = 0;
					}
		}


	 

	//��鹤���ĵ�ʱ�䣬��û�дﵽ�趨��ֵ
	if(sys_flag.LianxuWorkTime >= Dealy_Time)
		{
			sys_flag.LianxuWorkTime = 0; //��������
			sys_flag.Lianxu_OpenTime  = 0;
		
			Time_Ok = OK;//�����������۱�־
		}

	//������ʱ�䵽���Ҵ��ڲ�ˮ״̬������������۷�����鷧�ſ�����ʱ��
	if(Time_Ok)
		{
			
			if(sys_flag.Lianxu_OpenTime < Open_Time)
				{
					 if( Switch_Inf.water_switch_flag)//  ����Ƶ��ˮ�������Ǹ�ˮ���������������ź�����
					 	{
					 		LianXu_Paiwu_Open();
							if(sys_flag.LianXu_100msFlag)
								{
									sys_flag.LianXu_100msFlag = 0;
									sys_flag.Lianxu_OpenTime++;
								}
							
					 	}
					 else
					 	LianXu_Paiwu_Close();
				}
			else
				{
					Time_Ok = FALSE; //ʱ�䵽�ı�־����
					
				}
			
		}
	else
		{
			sys_flag.Lianxu_OpenTime  = 0; //����ϴ�ʹ�õı�����־
			LianXu_Paiwu_Close();
		}
	
	

	return 0;
}



uint8 Auto_StartOrClose_Process_Function(void)
{
	
	

	return 0;
}


void JTAG_Diable(void)
{
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);
	
}




uint8 Speed_Pressure_Function(void)
{
	static uint16 Old_Pressure = 0; //���ڱ����ϸ��׶ε�����ֵ
	uint16 New_Pressure =0;
	static uint16 TimeCount = 0;
	uint8 Chazhi = 0;

	//
	if(Sys_Admin.Device_Style == 1 || Sys_Admin.Device_Style == 3) 
		{
			//������ʹ���ڲ�ѹ����Ϊ׷��Ŀ��
			New_Pressure = Temperature_Data.Inside_High_Pressure;  //����һ�β��ѹ����ΪĿ��
		}
	else
		{
			New_Pressure = Temperature_Data.Pressure_Value;   //���β�ѹ����Ϊ׷��Ŀ��
		}

	
	
	if(sys_flag.Pressure_1sFlag)
		{
			sys_flag.Pressure_1sFlag = 0;
			
			if(sys_flag.flame_state)
				{
					TimeCount ++;
					if(New_Pressure > Old_Pressure)
						{
							Chazhi = New_Pressure - Old_Pressure;
							Old_Pressure = New_Pressure;
							sys_flag.Pressure_ChangeTime = TimeCount;
							sys_flag.Pressure_ChangeTime = sys_flag.Pressure_ChangeTime / Chazhi;
							TimeCount = 0;
						}


					if(New_Pressure < Old_Pressure)
						{
							Chazhi = Old_Pressure - New_Pressure;
							
							Old_Pressure = New_Pressure;
							sys_flag.Pressure_ChangeTime = TimeCount;
							sys_flag.Pressure_ChangeTime = sys_flag.Pressure_ChangeTime / Chazhi;
							
							TimeCount = 0;
						}
				}
			else   //û�л���ʱ��״̬����
				{
					Old_Pressure = New_Pressure;
					TimeCount = 0;
					sys_flag.Pressure_ChangeTime = 0;
				}
		}

	

		return 0;
}

uint8 Wifi_Lock_Time_Function(void)
{
	
	
	
	return 0 ;
}

uint8 XiangBian_Steam_AddFunction(void)
{

	uint16 Protect_Pressure = 165;  //1.65Mpa

	Sys_Admin.Device_Style = 1; //ǿ��Ϊ������


	if(sys_flag.HPWater_Value < LCD4013X.DLCD.InsideWater_AlarmValue)
		{
			IO_Status.Target.XB_WaterLow = FALSE;
		}
	else
		{
			IO_Status.Target.XB_WaterLow = OK;
		}
	
	if(Sys_Admin.Device_Style == 1 || Sys_Admin.Device_Style == 3)  //��������������
		{
			if(sys_data.Data_10H == 2)
				{
					if(Temperature_Data.Inside_High_Pressure >=Protect_Pressure) //����15���ֱ�ӱ���
						{
							if(sys_flag.Error_Code == 0 )
								sys_flag.Error_Code  = Error20_XB_HighPressureYabian_Bad;
						}

					
				}


			switch (sys_data.Data_10H)
				{
					case 0:  //����״̬,�����ּ���ˮλ������������
							if(IO_Status.Target.XB_WaterLow == FALSE)
								{
									if(sys_flag.XB_WaterLow_Flag == 0)
										{
											sys_flag.XB_WaterLow_Flag = OK;
											sys_flag.XB_WaterLow_Count = 0;
										}

									if(sys_flag.XB_WaterLow_Count > 15)
										{
											//sys_flag.Error_Code = Error22_XB_HighPressureWater_Low; 
										}
								}
							else
								{
									sys_flag.XB_WaterLow_Flag = 0;
									sys_flag.XB_WaterLow_Count = 0;
								}
							
							break;
					case 2://����״̬
							if(sys_flag.flame_state == OK)
								{
									//���ּ���ˮλ�����ұ����¶ȳ���230�ȣ���ͣ��ת��ɨ������4�κ��򱨾�
									if(IO_Status.Target.XB_WaterLow == FALSE && sys_flag.Protect_WenDu >= 200)
										{
											if(sys_flag.XB_WaterLow_Flag == 0)
												{
													sys_flag.XB_WaterLow_Flag = OK;
													sys_flag.XB_WaterLow_Count = 0;
												}
											if(sys_data.Data_12H == 0)
												{
													sys_flag.XB_WaterLowAB_Count ++;
												}
											
											if(sys_flag.XB_WaterLowAB_Count >= 4)
												{
													sys_flag.Error_Code = Error22_XB_HighPressureWater_Low; 
												}
											else
												{
													//ת���쳣״̬
													sys_data.Data_12H = 5; //  
													Abnormal_Events.target_complete_event = 1;//�쳣�¼���¼
												}
										}
									else
										{
												sys_flag.XB_WaterLow_Flag = 0;
												sys_flag.XB_WaterLow_Count = 0;

												if(sys_flag.XB_WaterLowAB_Count)
													{
														//�������ȼ�հ�Сʱ���Զ���Ϩ���¼����
														if(sys_flag.XB_WaterLowAB_RecoverTime >= 1800)//30min�������У�����Ϊ��������
															sys_flag.XB_WaterLowAB_Count = 0;
													}
										}
								}
							else
								{
									//�豸������״̬����ֹ�մ������쳣��ˮλ��û�ȶ����������������
									//�豸��ǰ��ɨ�����У���⵽ȱˮ��Ҳ��ֱ�ӱ���
								
									if(sys_data.Data_12H == 0)
										{
											//���쳣״̬����ֱ�ӱ���
											if(IO_Status.Target.XB_WaterLow == FALSE)
												{
													if(sys_flag.XB_WaterLow_Flag == 0)
														{
															sys_flag.XB_WaterLow_Flag = OK;
															sys_flag.XB_WaterLow_Count = 0;
														}

													if(sys_flag.XB_WaterLow_Count > 10)
														{
															sys_flag.Error_Code = Error22_XB_HighPressureWater_Low; 
														}
													
												}
											else
												{
													sys_flag.XB_WaterLow_Flag = 0;
													sys_flag.XB_WaterLow_Count = 0;
												}

											
										}
								}
							break;
					default:

							break;
				}

			

			

			if(IO_Status.Target.XB_Hpress_Ykong == PRESSURE_ERROR)
				{
					 if(sys_flag.Error_Code == 0 )
						sys_flag.Error_Code = Error21_XB_HighPressureYAKONG_Bad; //����ѹ��������ȫ��Χ����	
				}

			if(Temperature_Data.Inside_High_Pressure >= 2)//0.02Mpa
	 			{
	 				//LianXu_Paiwu_Close();
	 			}
		}
		 


	return 0;	
}


uint8 GetOut_Mannual_Function(void)
{
	Feed_Main_Pump_OFF();
	 
	Second_Water_Valve_Close();
	Pai_Wu_Door_Close();
	Send_Air_Close();
	LianXu_Paiwu_Close();
	WTS_Gas_One_Close();
	PWM_Adjust(0);



		return 0;
}




void Union_Check_Config_Data_Function(void)
{
	float Resdata  = 0;
	uint8 Address = 0;
	//1�� ǰ��ɨ���30--120s
	 
	if(Sys_Admin.First_Blow_Time > 100000 ||Sys_Admin.First_Blow_Time < 10000) //��������趨��Χ����ֵ׷��
		Sys_Admin.First_Blow_Time =15000 ;
	
	//2�� ��ɨ���30--120s	
	 
	if(Sys_Admin.Last_Blow_Time > 100000 ||Sys_Admin.Last_Blow_Time < 10000) //��������趨��Χ����ֵ׷��
		Sys_Admin.Last_Blow_Time =15000 ;
	
	//3�� �����20--35%
	 
	if(Sys_Admin.Dian_Huo_Power > Max_Dian_Huo_Power ||Sys_Admin.Dian_Huo_Power < Min_Dian_Huo_Power) //��������趨��Χ����ֵ׷��
		Sys_Admin.Dian_Huo_Power =25 ;
	

	//4�� �������й��ʼ��30--100%
	if(Sys_Admin.Max_Work_Power > 100 ||Sys_Admin.Max_Work_Power < 30)
		Sys_Admin.Max_Work_Power = 100;

	if(Sys_Admin.Max_Work_Power < Sys_Admin.Dian_Huo_Power)
		Sys_Admin.Max_Work_Power = Sys_Admin.Dian_Huo_Power;

	


	 
	if(Sys_Admin.Fan_Speed_Check > 1)
		Sys_Admin.Fan_Speed_Check = 1; //Ĭ���Ǽ����ٵ�
	
		
	 
	if(Sys_Admin.Danger_Smoke_Value > 2000 && Sys_Admin.Danger_Smoke_Value < 600)
		Sys_Admin.Danger_Smoke_Value = 800;
	
	
	 
	if(sys_config_data.zhuan_huan_temperture_value < 10|| sys_config_data.zhuan_huan_temperture_value > Sys_Admin.DeviceMaxPressureSet)
		sys_config_data.zhuan_huan_temperture_value = 55; //������ޣ�Ĭ��5.5����
	

	
	if(Sys_Admin.DeviceMaxPressureSet > 160) //25�������Ҫ��������
		{
			Sys_Admin.DeviceMaxPressureSet = 80;
			 
		}
		



	UnionLCD.UnionD.UnionStartFlag = AUnionD.UnionStartFlag;

	
	//�����¶ȱ�����������ʾ����*********************************
	UnionLCD.UnionD.PaiYan_WenDu = Temperature_Data.Smoke_Tem / 10;
	UnionLCD.UnionD.PaiYan_AlarmValue = Sys_Admin.Danger_Smoke_Value / 10;
	if(UnionLCD.UnionD.PaiYan_AlarmValue < 85 )
		{
			UnionLCD.UnionD.PaiYan_AlarmValue = 85;
			
		}
	
	if(sys_flag.PaiYanAlarm_Flag)
		{
			if(UnionLCD.UnionD.PaiYan_WenDu  > UnionLCD.UnionD.PaiYan_AlarmValue)
				{
					 AUnionD.UnionStartFlag = 0; //����豸�����У���ȫ��ֹͣ

					 
					UnionLCD.UnionD.Union_Error = 2;//�����¶ȳ����趨ֵ
					if(UnionLCD.UnionD.PaiYan_WenDu  > 300)
		 				{
		 					UnionLCD.UnionD.Union_Error = 1; //�����¶�δ���Ӻ�
						}
				}
		}
	
	
//	Resdata = Temperature_Data.Pressure_Value;
//	UnionLCD.UnionD.Big_Pressure =  Resdata / 100;
//	AUnionD.Big_Pressure = UnionLCD.UnionD.Big_Pressure;
		
	Resdata = sys_config_data.zhuan_huan_temperture_value ;	
	AUnionD.Target_Value = Resdata / 100;
	UnionLCD.UnionD.Target_Value = AUnionD.Target_Value ;

	
	Resdata = sys_config_data.Auto_stop_pressure ;	
	UnionLCD.UnionD.Stop_Value  = Resdata / 100;
	AUnionD.Stop_Value = UnionLCD.UnionD.Stop_Value;


	 

	Resdata = sys_config_data.Auto_start_pressure ;	
	UnionLCD.UnionD.Start_Value  = Resdata / 100;
	AUnionD.Start_Value = UnionLCD.UnionD.Start_Value;

	Resdata = Sys_Admin.DeviceMaxPressureSet  ;	
	UnionLCD.UnionD.Max_Pressure  = Resdata / 100;
	AUnionD.Max_Pressure = UnionLCD.UnionD.Max_Pressure;

	 UnionLCD.UnionD.Need_Numbers = AUnionD.Need_Numbers;
	
	UnionLCD.UnionD.AliveOK_Numbers = AUnionD.AliveOK_Numbers;

	UnionLCD.UnionD.Mode_Index = AUnionD.Mode_Index;

	UnionLCD.UnionD.PID_Next_Time = AUnionD.PID_Next_Time;
	UnionLCD.UnionD.PID_Pvalue = AUnionD.PID_Pvalue;
	UnionLCD.UnionD.PID_Ivalue = AUnionD.PID_Ivalue;
	UnionLCD.UnionD.PID_Dvalue = AUnionD.PID_Dvalue;

	UnionLCD.UnionD.Union16_Flag = AUnionD.Union16_Flag;

	UnionLCD.UnionD.A1_WorkTime = AUnionD.A1_WorkTime;
    SlaveG[1].Work_Time = AUnionD.A1_WorkTime;
    UnionLCD.UnionD.A2_WorkTime = AUnionD.A2_WorkTime;
    SlaveG[2].Work_Time = AUnionD.A2_WorkTime;
	UnionLCD.UnionD.A3_WorkTime = AUnionD.A3_WorkTime;
    SlaveG[3].Work_Time = AUnionD.A3_WorkTime;
	UnionLCD.UnionD.A4_WorkTime = AUnionD.A4_WorkTime;
    SlaveG[4].Work_Time = AUnionD.A4_WorkTime;

	UnionLCD.UnionD.A5_WorkTime = AUnionD.A5_WorkTime;
    SlaveG[5].Work_Time = AUnionD.A5_WorkTime;
	UnionLCD.UnionD.A6_WorkTime = AUnionD.A6_WorkTime;
    SlaveG[6].Work_Time = AUnionD.A6_WorkTime;
	UnionLCD.UnionD.A7_WorkTime = AUnionD.A7_WorkTime;
    SlaveG[7].Work_Time = AUnionD.A7_WorkTime;
	UnionLCD.UnionD.A8_WorkTime = AUnionD.A8_WorkTime;
    SlaveG[8].Work_Time = AUnionD.A8_WorkTime;
	UnionLCD.UnionD.A9_WorkTime = AUnionD.A9_WorkTime;
    SlaveG[9].Work_Time = AUnionD.A9_WorkTime;

	UnionLCD.UnionD.A10_WorkTime = AUnionD.A10_WorkTime;
    SlaveG[10].Work_Time = AUnionD.A10_WorkTime;

	
	JiZu[1].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0001 ;
	JiZu[2].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0002 ;
	JiZu[3].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0004 ;
	JiZu[4].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0008 ;
	
	JiZu[5].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0010;
	JiZu[6].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0020;
	JiZu[7].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0040;
	JiZu[8].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0080;	
	JiZu[9].Slave_D.UnionOn_Flag = AUnionD.Union16_Flag & 0x0100;
	JiZu[10].Slave_D.UnionOn_Flag =AUnionD.Union16_Flag & 0x0200;


	//����ܿر�����־��������־��ȡ
	sys_flag.PaiYanAlarm_Flag = UnionLCD.UnionD.Alarm_Allow_Flag & 0x0001 ;

	
	for(Address = 1; Address <= 10; Address ++)
		{
			if(JiZu[Address].Slave_D.UnionOn_Flag > 0)
				JiZu[Address].Slave_D.UnionOn_Flag = OK;
		}

	UnionLCD.UnionD.Devive_Style = AUnionD.Devive_Style;
	UnionLCD.UnionD.Max_Address = AUnionD.Max_Address;
	AUnionD.Sys_Version = Soft_Version;
	UnionLCD.UnionD.Sys_Version = AUnionD.Sys_Version;

	AUnionD.ModBus_Address = Sys_Admin.ModBus_Address;
	UnionLCD.UnionD.ModBus_Address = AUnionD.ModBus_Address;

	UnionLCD.UnionD.OFFlive_Numbers = AUnionD.OFFlive_Numbers;

	UnionLCD.UnionD.Data14 = SlaveG[1].LianxuFa_Data;
	UnionLCD.UnionD.Data15 = SlaveG[2].LianxuFa_Data;
	UnionLCD.UnionD.Data16 = SlaveG[3].LianxuFa_Data;
	UnionLCD.UnionD.Data17 = SlaveG[4].LianxuFa_Data;

	UnionLCD.UnionD.HPWater_Value_A1 = LCD10X1.HP_WaterValue[1];
	AUnionD.HPWater_Value_A1 =  LCD10X1.HP_WaterValue[1];

	UnionLCD.UnionD.HPWater_Value_A2 = LCD10X1.HP_WaterValue[2];
	AUnionD.HPWater_Value_A2 =  LCD10X1.HP_WaterValue[2];

	UnionLCD.UnionD.HPWater_Value_A3 = LCD10X1.HP_WaterValue[3];
	AUnionD.HPWater_Value_A3 =  LCD10X1.HP_WaterValue[3];

	UnionLCD.UnionD.HPWater_Value_A4 = LCD10X1.HP_WaterValue[4];
	AUnionD.HPWater_Value_A4 =  LCD10X1.HP_WaterValue[4];

	UnionLCD.UnionD.LPWater_Value_A1 = LCD10X1.LP_WaterValue[1];
	AUnionD.LPWater_Value_A1 =  LCD10X1.LP_WaterValue[1];

	UnionLCD.UnionD.LPWater_Value_A2 = LCD10X1.LP_WaterValue[2];
	AUnionD.LPWater_Value_A2 =  LCD10X1.LP_WaterValue[2];

	UnionLCD.UnionD.LPWater_Value_A3 = LCD10X1.LP_WaterValue[3];
	AUnionD.LPWater_Value_A3 =  LCD10X1.LP_WaterValue[3];

	UnionLCD.UnionD.LPWater_Value_A4 = LCD10X1.LP_WaterValue[4];
	AUnionD.LPWater_Value_A4 =  LCD10X1.LP_WaterValue[4];
	 
	
}


uint8 JiaYao_Supply_Function(void)
{
	uint8 address = 0;
	uint8 Supply_Flag = 0;
	for(address = 1; address <= 10; address ++)
		{
			if(JiZu[address].Slave_D.Pump_State)
				{
					Supply_Flag = OK;
				}
		}
	
	if(Supply_Flag)
		{
			Feed_Main_Pump_ON();
		}
	else
		{
			Feed_Main_Pump_OFF();
		}
	
	
	return 0;
}

uint8 D50L_Union_MuxJiZu_Control_Function(void)
{
	
	

	return 0;
}



uint16  Pid_Cal_Function(void)
{
	

	
	

	return 0;
}

uint8  Auto_Baudrate_check_Function(void)
{
	uint8 static Jump_Index  = 0;

	
	if(sys_flag.Lcd4013_OnLive_Flag) //��С�����Զ���������Ĳ�����
		{
			return 0; //С���Ĳ�����9600
		}

	
	sys_flag.Check_Finsh = OK;

	
	while(sys_flag.Check_Finsh)
		{
			IWDG_Feed();
			
			switch (Jump_Index)
				{
					case 0:
							delay_sys_sec(5000);  //ע��ʱ��
							Jump_Index = 1;

							break;
					case 1:
							//�ȼ��9600������

							Union_ModBus2_Communication();
							
							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
									
								}

							if(sys_flag.LCD10_Connect)
								{
									sys_time_up = 1;  //����⵽�����Զ������ʱ
									//u1_printf("\n*������9600�ɹ�AAAA��= %d\n",sys_flag.Address_Number);
								}
							if(sys_time_up)
								{
									sys_time_up = 0;
									


									if(sys_flag.LCD10_Connect)
										{
											Jump_Index = 2;  //  ֱ�ӵ��������˳�
											sys_flag.Check_Finsh = FALSE;  
										}
									else
										{
											Jump_Index = 2;  //���ż��115200������
											delay_sys_sec(5000); 
											uart2_init(115200);  //�������ʸ�Ϊ115200
											
										}
								}
							break;

					case 2:
							Union_ModBus2_Communication();
							
							if(sys_time_start == 0)
								{
									sys_time_up = 1;
								}
							else
								{
									
								}

							if(sys_flag.LCD10_Connect)
								{
									sys_time_up = 1;  //����⵽�����Զ������ʱ

									//u1_printf("\n*������115200�ɹ�@@@@��= %d\n",sys_flag.Address_Number);
								}
							if(sys_time_up)
								{
									sys_time_up = 0;
									
									Jump_Index = 3;  //  ֱ�ӵ��������˳�
									sys_flag.Check_Finsh = FALSE; 
								}


							break;

					case 3:
							sys_flag.Check_Finsh = FALSE;  

							break;

					default:


							break;
				}
		}


	return 0;

	
}




uint16  Solo_Pid_Cal_Function(void)
{
	static uint8 Vspeed = 30;  //��׼�ٶ�Ϊ40������ÿ��仯0.004,100��仯0.4Mpa
	uint8  Vbuffer = 0;
	uint16 Abs_Value = 0;

	uint16 Limit_PowerMIN = 0;
	uint16 Limit_PowerMAX = 0;


	
	

	
	//PID.Out_Put = 10000 ;
//	�ݶ�PID.P = 5,  ��Χ1--10������ÿ���������5%����С1%����仯��Χ�е��
	//PID.I = 0.2
	//PID.D = 7   ��Χ1--10������ȡֵ5-10

	PID.Proportion =8;  //5
	PID.Integral = 3;  //0.2
	PID.Derivative = 10; //7

	

	Limit_PowerMIN = 3000; //����������30%Ϊ��С����
	Limit_PowerMAX = 10000;  //�����

	if(PID.Old_Put == 0)
		{
			PID.Old_Put = Limit_PowerMIN;
		}
	
	
	//ѹ��������Χ�����ڡ�0.03Mpa���ڣ�change_Speed��Ҫ�Ŵ�10�����ý��п��ƣ����������

	
	//��Change_Speed = 1������10��仯0.01Mpa,,30��仯 ��0.03Mpa��
	//��change_Speed = 2, ����5��仯0.01Mpa��15��仯��0.03Mpa,Ҳ��
	//��change_Speed = 3, ����3.3��,�仯0.01,10��仯��0.03Mpa     ��   �ڵ���ѹ����Χ�ڣ��򲻵ñ仯����3����ѹ������
	//��change_Speed = 4, ����2.5��,�仯0.01,7.5��仯��0.03Mpa��25��仯0.1
	//��change_Speed = 5, ����2.0��,�仯0.01,6��仯��0.03Mpa ��20��仯0.1
	//��change_Speed = 6, ����1.6��,�仯0.01,5��仯��0.03Mpa ��16��仯0.1
	
	PID.SetPoint = sys_config_data.zhuan_huan_temperture_value * 10;  //0.50Mpa = 500

	//ע��PID,��������޵��������

	

	 

	//100ms ���һ��
	if(PID.Flag_100ms == OK)
		{
			PID.Flag_100ms = 0;

			//�ҳ��䶯��ֵ
			PID.Unchange_Count100ms ++;

			if(PID.Down_Flag)
				{
					PID.Down_Time++;
				}
			else
				{
					PID.Down_Time = 0;
				}
			
			if(PID.Unchange_Count100ms >= 30)
				{
					//��ÿ0.1���һ�Σ����ÿ����һ����û�б仯
					 
					if(PID.Real_Value !=  PID.LastValue)
						{	
							if(PID.Real_Value >= PID.LastValue)
								Abs_Value = PID.Real_Value - PID.LastValue;
							else
								Abs_Value = PID.LastValue - PID.Real_Value;
							 
							if(Abs_Value >= 2)
								{

									//�ҳ��䶯�ķ��򣬱䶯����
									if(PID.Real_Value > PID.LastValue)
										{
											PID.Up_Flag = OK;//�仯�����־
											PID.Down_Flag = 0;
											PID.Change_Speed = 10*10 * (PID.Real_Value - PID.LastValue) / (PID.Unchange_Count100ms);  //0.001 /��ı仯�ٶȣ����Ŵ�10��
										}

									if(PID.Real_Value < PID.LastValue)
										{
											PID.Up_Flag = 0;
											PID.Down_Flag = OK;//�仯�����־
											PID.Change_Speed = 10*10 * (PID.LastValue - PID.Real_Value) / (PID.Unchange_Count100ms);  //ÿ��ı仯�ٶ�

											
										}
								//	u1_printf("\n*ABS�Ĳ�ֵ = %d   \n",Abs_Value);
								//	u1_printf("\n*����ѹ����ֵ = %d   \n",PID.Real_Value);
								//	u1_printf("\n*�ϴ�ѹ����ֵ = %d   \n",PID.LastValue);
								//	u1_printf("\n*���α仯��ʱ�� = %d   \n",PID.Unchange_Count100ms);
								//	u1_printf("\n*ѹ���任�ٶ� = %d   \n",PID.Change_Speed);
									
									PID.Unchange_Count100ms = 0; //�ȴ��´μ���

									
									PID.LastValue = PID.Real_Value; //���䶯��ֵ������

									
								}
							else
								{
									
									 	
								}

							
							if(PID.Unchange_Count100ms >= 50)
								{
									PID.LastValue = PID.Real_Value; //���5�벻�仯��Ҳ��ֵһ��
									PID.Unchange_Count100ms = 0;
									PID.Change_Speed = 0;
									PID.Up_Flag = OK;
									PID.Down_Flag = 0;
								}
							
						}
					else
						{
							//��� 1��δ�䶯�������ʲô�أ������5��δ�䶯������ʲô�أ�
							//���3��һ�����ڣ�
							//δ�仯����ȣ�������1����δ�仯
							 if(PID.Change_Speed >= 10)
								PID.Change_Speed = 2;  //������С�仯���� 

							 if(PID.Unchange_Count100ms >= 50)
								{
									PID.LastValue = PID.Real_Value; //���5�벻�仯��Ҳ��ֵһ��
									PID.Unchange_Count100ms = 0;
									PID.Change_Speed = 0;
									PID.Up_Flag =OK;
									PID.Down_Flag = 0;
								}

								
						}
						
				}
			

			



  //��һ���� ����֤��ѹ����
		//��ǰֵС�� �趨ֵ
		if(PID.Real_Value >= PID.SetPoint)
			Abs_Value = PID.Real_Value - PID.SetPoint;
		else
			Abs_Value = PID.SetPoint - PID.Real_Value;


		  
		
			if(PID.Real_Value < PID.SetPoint)
				{
			
					if(Abs_Value >= 160)  //����Ŀ��ֵ����0.16Mpa
						{
							//����仯����С�� 5 p/s ��0.16Mpa = 160  /     5 = 32 Ҳ����Ҫ����32����ܵ����趨ѹ��
							if(PID.Change_Speed <  Vspeed)
								{
									//Ҫ���������
									
									PID.Out_Put  = PID.Old_Put + PID.Proportion * 2 + PID.Integral + 2*PID.Derivative ;  //����׶ΰ���2����ϵ�����������ϱ������� ����������0.1--10�� ÿ0.1s �� 1����1��仯1%
									//��λ�þ�����ѹ�Χ��ʱ���������ߣ�
								}
							else
								{
									//5���ϵı仯�ٶȣ��仯̫�죬�����
									//�ݶ�PID.D = 8
									//�仯�ٶȿ죬����Ҫ�����ٶȣ�������5���£����㻺��,�����ٶȸ�PD�й�ϵ��PD�ķ�ΧֵҲ����5--10֮�䣬PID.P��PID.D�����߶�Ϊ10
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put + PID.Proportion + Vspeed - PID.Change_Speed ;//ע��˳��ͷ���5-5 =0�������1
										}
									if(PID.Down_Flag)
										{
											PID.Out_Put  = PID.Old_Put + 3*PID.Proportion * 2 + PID.Integral +3* PID.Derivative  ;//ע��˳��ͷ���5-5 =0�������1
										}
									
								}
						}
					if(Abs_Value > 100 && Abs_Value < 160)
						{
							//����Ҫ������30����
							if(PID.Change_Speed  >=30)
								{
									//Ҫ����
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  - PID.Proportion - PID.Derivative;
										}

									if(PID.Down_Flag)
										{
											//ѹ��ֵ���½����ƣ������ӹ���
											PID.Out_Put  = PID.Old_Put + PID.Proportion * 2 + PID.Integral + 2*PID.Derivative ;
										}
									
								}
							else
								{
									if(PID.Up_Flag)
										{
											if(PID.Change_Speed < 10)
												{
													PID.Out_Put  = PID.Old_Put + PID.Proportion * 2;
												}
											else
												{
													PID.Out_Put  = PID.Old_Put + PID.Proportion + PID.Integral;
												}
										}
											//Ҫ����
										
										if(PID.Down_Flag)
										{
											//ѹ��ֵ���½����ƣ������ӹ���
											PID.Out_Put  = PID.Old_Put + PID.Proportion * 2 + PID.Integral + 2*PID.Derivative ;
										}
										
								}
						}

					if(Abs_Value > 30 && Abs_Value < 100)
						{
							//����Ҫ������20����
							if(PID.Change_Speed  >=20)
								{
									//Ҫ����
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  - PID.Proportion ;
										}

									if(PID.Down_Flag)
										{
											//ѹ��ֵ���½����ƣ������ӹ���
											PID.Out_Put  = PID.Old_Put + PID.Proportion * 3 + PID.Integral + 2*PID.Derivative ;
										}
									
									
								}
							else
								{
									if(PID.Change_Speed < 10)
										{
											//Ҫ����
											if(PID.Up_Flag)
												{
													PID.Out_Put  = PID.Old_Put  + PID.Proportion + PID.Integral;
												}

											if(PID.Down_Flag)
												{
													PID.Out_Put  = PID.Old_Put  + 3*PID.Proportion + PID.Derivative;
												}
										}
									else
										{
											if(PID.Up_Flag)
												{
													PID.Out_Put  = PID.Old_Put  + PID.Proportion  ;
												}

											if(PID.Down_Flag)
												{
													PID.Out_Put  = PID.Old_Put  + 2*PID.Proportion + PID.Derivative + PID.Integral;
												}
										}
								}
						}

					if(Abs_Value <= 30)
						{
							//����Ҫ������10���ڣ������0
							if(PID.Change_Speed  >=10)
								{
									//Ҫ����
									if(PID.Up_Flag)
										{
											//ѹ��ֵ���Ͻ����ƣ��򽵵͹���
											PID.Out_Put  = PID.Old_Put  - 2*PID.Proportion ;
										}

									if(PID.Down_Flag)
										{
											//ѹ��ֵ���½����ƣ������ӹ���
											PID.Out_Put  = PID.Old_Put  + 3* PID.Proportion  + PID.Integral + PID.Derivative;
										}
									
										
									
								}
							else
								{
									
											//���ٲ�������ֹ���½������ƣ��������Ӷ���
											if(PID.Down_Flag)
												{
													PID.Out_Put  = PID.Old_Put + PID.Proportion * 3 + PID.Integral;
												}
											

											if(PID.Up_Flag)
												{
													//ѹ��ֵ���Ͻ����ƣ��򽵵͹���
													PID.Out_Put  = PID.Old_Put + PID.Proportion;
												}
											
										
								}
						}
					
					
				}
			else
				{
					//�����趨ֵʱ��
					if(Abs_Value <= 20)
						{
							
							if(PID.Change_Speed  >=20)
								{
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  - 3* PID.Proportion - PID.Integral - 2*PID.Derivative;
										}

									if(PID.Down_Flag)
										{
											PID.Out_Put  = PID.Old_Put  + 3*PID.Proportion + PID.Integral + 2*PID.Derivative;
										}
								}
							else
								{
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  -2* PID.Proportion - 2*PID.Derivative - PID.Integral;
										}

									if(PID.Down_Flag)
										{
											if(PID.Change_Speed > 10)
												PID.Out_Put  = PID.Old_Put  +2* PID.Proportion + PID.Integral + 2*PID.Derivative;
											else
												PID.Out_Put  = PID.Old_Put  + PID.Proportion + PID.Integral + PID.Derivative;
										}
								}
						}

					if(Abs_Value > 20 && Abs_Value <= 40)
						{
							
							if(PID.Change_Speed  >=20)
								{
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  -  PID.Proportion * 5  - PID.Integral - PID.Derivative*2  ;  //ע������
										}

									if(PID.Down_Flag)
										{
											PID.Out_Put  = PID.Old_Put  + 2*PID.Proportion + PID.Derivative*2  ;
										}
								}
							else
								{
									if(PID.Up_Flag)
										{
											PID.Out_Put  = PID.Old_Put  - PID.Proportion * 3 - PID.Derivative*2  -PID.Integral;
										}

									if(PID.Down_Flag)
										{
											PID.Out_Put  = PID.Old_Put  + 2*PID.Proportion + PID.Integral ;
										}
									
								}
						}

					if(Abs_Value > 40)
						{
							if(PID.Up_Flag)
								{
									PID.Out_Put  = PID.Old_Put  - PID.Proportion *4  - PID.Integral - PID.Derivative*2;
										
								}

							if(PID.Down_Flag)
								{
									if(PID.Change_Speed  >=10)
										PID.Out_Put  = PID.Old_Put  + 2*PID.Proportion ;  //���ݽ����ٶȣ�����
									else
										PID.Out_Put  = PID.Old_Put  + PID.Proportion + PID.Integral ;
								}
							
						}
					
				}


				if(PID.Out_Put < Limit_PowerMIN)
					PID.Out_Put = Limit_PowerMIN;
				if(PID.Out_Put > Limit_PowerMAX)
				PID.Out_Put = Limit_PowerMAX;

				 PID.Old_Put  = PID.Out_Put;
		}

	
	

	return 0;
}




uint8 D50L_SoloPressure_Union_MuxJiZu_Control_Function(void)
{
	static uint16 Allneed_Power = 0;  //��������
static 	uint8 Need_Devices = 0;  //��Ҫ��̨��
	uint8 Need_Buffer = 0;
	uint8 Address = 0;  //���ڵ�ַ
	uint8 AliveOk_Numbres = 0;  //����OK���豸����
	uint8 Device_ErrorNumbers = 0;//���ϻ��������
static	uint8 Already_WorkNumbers = 0; //���Ѿ������е��豸����ͳ��
	uint8 AliveOK[13] = {0};    //�������豸�ĵ�ַ����ͳ��,1---10
static uint8 WorkOk_Address[13] = {0}; //�������е��豸��ַͳ��
		uint8 IndexAdd = 0;  //��������
static uint8 Second_Start_Flag = 0;  //���������ı�־

	uint8 AllPower_WorkDevices = 0;
	uint8 LowPower_WorkDevices = 0;

	float Resdata = 0;

	 
	 uint32 Max_time = 0;
static	uint32 Max_Address = 0;
	 uint32 Min_Time = 0;
static 	uint32 Min_Address = 0;

	uint8 Need_flag = 0;
	uint8 Loss_flag = 0;

	static uint8 Loop_Command_10secCount = 0;  //10�뷢��ָ��һ�Σ���ȫ��ָ���ͻ
	static uint8 Loop_Command_10secCount1 = 0;

	static uint8 FengFa_Close_Count = 0;  //���ڷ緧�ر��ж���ʱ��

	
	static uint8 Time_Count = 0;//������
	static uint8  ONTime_Flag = 0;   //�����־
	static uint8 Compare_Value = 0;

	static uint16 All_Work_Power = 0;


	uint8 Min_Power = 30;  //��С������30%
	uint16 Value_Buffer = 0;  //��������

	

	PID.Next_Time = AUnionD.PID_Next_Time;
//��һ�׶Σ� �ҵ��������ߵĻ�����ȷ�����߻�����̨����ȷ����һ��Ҫ�������豸��رյ��豸
  if(sys_flag.Union_1_Sec)
  	{
  		sys_flag.Union_1_Sec = 0; //ÿ����һ��

		Loop_Command_10secCount ++;  // 10��ı�־
		Loop_Command_10secCount1 ++;

		FengFa_Close_Count++;

		//������ʱ���������ӳ�PID������
		Time_Count ++;
		if(Time_Count >= Compare_Value)
			{
				ONTime_Flag = OK;
				Compare_Value = 0; 
			}
		else
			{
				ONTime_Flag = 0;
			}
			
		
		All_Work_Power = 0;
		Min_Address = 0;
		Max_Address = 0;  //���ݸ���
		Already_WorkNumbers = 0;
		Device_ErrorNumbers = 0;
		IndexAdd = 1; //��1��ʼ
		for(Address = 1; Address <= 10; Address ++)
			{
				WorkOk_Address[Address] = 0;  //��ʼ��
			}
		for(Address = 1; Address <= 10; Address ++)
				{
				 
					if(JiZu[Address].Slave_D.UnionOn_Flag) //ȡ���صı�־
						{
							if(SlaveG[Address].Alive_Flag)//��������
								{
									if(JiZu[Address].Slave_D.Error_Code == 0) //û�й���
										{
											AliveOk_Numbres ++;
											AliveOK[AliveOk_Numbres] = Address; //


											//�ҳ���ǰ״̬���������е�ѹ�����ֵ
											if(JiZu[Address].Slave_D.Dpressure <= 2.5)
												{
													if(Resdata < JiZu[Address].Slave_D.Dpressure)
														{
															Resdata = JiZu[Address].Slave_D.Dpressure;
														}
												}
											

											//�ҳ������ģ�˭����ʱ�����٣������´�����ʹ��
											if(JiZu[Address].Slave_D.Device_State == 1 && AUnionD.UnionStartFlag !=3)
												{
													SlaveG[Address].Out_Power  = 0;  //������������
													SlaveG[Address].Big_time = 0;
													SlaveG[Address].Small_time = 0;
													if(Min_Address == 0)//��ʼֵ
														{
															Min_Address = Address;
															 
															Min_Time= SlaveG[Address].Work_Time;
														}
													else
														{
															if(Min_Time > SlaveG[Address].Work_Time)
																{
																	Min_Time = SlaveG[Address].Work_Time;
																	Min_Address = Address;
																}
														}
													
												}
											
										}
									else
										{
											//���豸����ʱ��ֱ�ӽ���������
											Device_ErrorNumbers++;
											if(AUnionD.UnionStartFlag !=3 )  //���ֶ�ģʽ����£���������
												SlaveG[Address].Out_Power  = 0;  //������������
										}

									
		
									if(JiZu[Address].Slave_D.Device_State == 2)
										{
											Already_WorkNumbers ++;
											WorkOk_Address[Already_WorkNumbers] = Address; //���������豸��˳���źõ�ַ

											if(JiZu[Address].Slave_D.Flame )
												{
													//********************�ۼ��ܵ����й���
													if(JiZu[Address].Slave_D.Power >= JiZu[Address].Slave_D.Max_Power)
														{
															All_Work_Power = All_Work_Power + 100 ;
														}
													else
														{
															All_Work_Power = All_Work_Power + JiZu[Address].Slave_D.Power ;   
														}
													
												}

											if(JiZu[Address].Slave_D.Power >= JiZu[Address].Slave_D.Max_Power)  //�û����Ѿ�����������
												{
													if(JiZu[Address].Slave_D.Flame )
														{
															SlaveG[Address].Big_time ++;
															SlaveG[Address].Small_time = 0;
														}
													
													
													if(SlaveG[Address].Big_time >= PID.Next_Time)
														{
															AllPower_WorkDevices++;
														}
												}
											else
												{
													
													if(JiZu[Address].Slave_D.Power >= 70)
														{
															if(JiZu[Address].Slave_D.Flame )
																{
																	SlaveG[Address].Big_time ++;
																	SlaveG[Address].Small_time = 0;
																}
													
													
															if(SlaveG[Address].Big_time >= PID.Next_Time)
																{
																	AllPower_WorkDevices++;
																}		
														}
												}

											if(JiZu[Address].Slave_D.Power <= (JiZu[Address].Slave_D.DianHuo_Value + 10))  //�û����Ѿ��͸�������
												{
													if(JiZu[Address].Slave_D.Flame )
														{
															SlaveG[Address].Small_time ++;
															SlaveG[Address].Big_time = 0;
														}
													
													//С���ɣ����豸���е�ʱ��
													if(SlaveG[Address].Small_time >= (PID.Next_Time * 3)) //�������趨ʱ��
														{
															LowPower_WorkDevices++;
														}
												}

											//��������е�˭ʱ���
											if(Max_Address == 0)
												{
													Max_Address = Address;
													Max_time = SlaveG[Address].Work_Time;
												}
											else
												{
													if(Max_time < SlaveG[Address].Work_Time )
														{
															Max_Address = Address;
															Max_time = SlaveG[Address].Work_Time;
														}
												}
											 
										}
								}
							else
								{
									//�豸�����ߣ���ô��������������
								}
						}
					else
						{
							SlaveG[Address].Small_time= 0;
							SlaveG[Address].Big_time = 0;
							SlaveG[Address].Zero_time = 0;

							//�����ر�־ȡ�������豸������״̬ʱ��ֱ�ӹر�

							if(JiZu[Address].Slave_D.Device_State == 2 )  
								{
									//ȫ���ر�
									SlaveG[Address].Command_SendFlag = 3; //����������
									JiZu[Address].Slave_D.StartFlag = 0; //�رոû���
								}

							if(JiZu[Address].Slave_D.StartFlag == 1)
								{
									JiZu[Address].Slave_D.StartFlag = 0;
								}
						}
					
				}


		//��ȡȼ��ƽ���Ĺ���ֵ
		if(All_Work_Power == 0)
			{
				sys_flag.Pingjun_Power = 0;
			}
		else
			{
				if(Already_WorkNumbers > 0 )
					{
						sys_flag.Pingjun_Power = All_Work_Power / AliveOk_Numbres; 
					}
				else
					{
						sys_flag.Pingjun_Power = 0;
					}
				
			}
		

		//ȡ���������ߵĻ���ѹ�������ֵ
		UnionLCD.UnionD.Big_Pressure =  Resdata ;
		AUnionD.Big_Pressure = UnionLCD.UnionD.Big_Pressure;
		sys_flag.Already_WorkNumbers = Already_WorkNumbers; //���ڴ��ݲ���
		sys_flag.Device_ErrorNumbers = Device_ErrorNumbers; 

		AUnionD.AliveOK_Numbers = AliveOk_Numbres;  //ͳ���������ߵ��������й��ϵĳ���

		if(AUnionD.UnionStartFlag == 1)
		{
			//��û���豸����ʱ��������������صı�־��������
			if(AUnionD.AliveOK_Numbers == 0)
				{
					AUnionD.UnionStartFlag = 0;
					AUnionD.Mode_Index = 0;
					Second_Start_Flag = 0;
					Allneed_Power = 0;
					PID.Old_Put = 0;
					//��Ҫ���Ƿ���Ҫ��أ��ȴ����еķ���ر����ٹر�
					if(sys_flag.YanDao_FengFa_Index)
						{
							sys_flag.YanDao_FengFa_Index = 0;
							FengFa_Close_Count = 0;
						}
					else
						{
							sys_flag.YanDao_FengFa_Index = 0;
						}
					
					if(FengFa_Close_Count >= 15) //�ȴ�15�����ִ�йرն���
						{
							FengFa_Close_Count = 15;
							//RELAY3_OFF;
							ZongKong_YanFa_Close();
						}
				}
			
		}

		
  	}



  
	
//�ڶ��׶Σ�������������������������������������������Ҫ�Ļ���̨����������ѹ����������ر�

	if(AUnionD.UnionStartFlag == 0)
		{
			AUnionD.Mode_Index = 0;
			Second_Start_Flag = 0;
			Allneed_Power = 0;
			PID.Old_Put = 0;
			//��Ҫ���Ƿ���Ҫ��أ��ȴ����еķ���ر����ٹر�
			if(sys_flag.YanDao_FengFa_Index)
				{
					sys_flag.YanDao_FengFa_Index = 0;
					FengFa_Close_Count = 0;
				}
			else
				{
					sys_flag.YanDao_FengFa_Index = 0;
				}
			
			if(FengFa_Close_Count >= 15) //�ȴ�15�����ִ�йرն���
				{
					FengFa_Close_Count = 15;
					//RELAY3_OFF;
					ZongKong_YanFa_Close();
					for(Address = 1; Address <= 10; Address ++)
						{
							SlaveG[Address].Idle_AirWork_Flag = FALSE;
						}
					
				}
			
			
			
		}

	
	switch (AUnionD.Mode_Index)
		{
			case 0:
				
					//״̬0���������ߵĻ���ȫ���رգ�1���ȴ�������־��2�ȴ�ѹ������
					Need_Devices = 0;
					if(Loop_Command_10secCount1 >= 10)
						{
							Loop_Command_10secCount1 = 0;

							for(Address = 1; Address <= 10; Address ++)
								{
									//����һֱ�������и����ؽڵ�********************************
									//�����е�״̬,�����״̬������
									if(JiZu[Address].Slave_D.Device_State == 2 || JiZu[Address].Slave_D.Device_State == 3)  
										{
											//ȫ���ر�
											SlaveG[Address].Command_SendFlag = 3; //����������
											JiZu[Address].Slave_D.StartFlag = 0; //�رոû���
										}

									if(JiZu[Address].Slave_D.StartFlag == 1)
										{
											JiZu[Address].Slave_D.StartFlag = 0;
										}
										
								}
						}

					if(AUnionD.UnionStartFlag == 1)
						{
							switch (sys_flag.YanDao_FengFa_Index)
								{
									case 0 :
											//RELAY3_ON; //ֱ���̵����ſ���
											ZongKong_YanFa_Open();
											
											delay_sys_sec(10000);//�ӳ�10�룬�ȴ��緧�Ŀ���
											sys_flag.YanDao_FengFa_Index = 1;
										break;
									case 1:	
											//RELAY3_ON; //ֱ���̵����ſ���
											ZongKong_YanFa_Open();
											if(sys_time_start == 0)
												{
													sys_time_up = 1;
												}
											else
												{
													
												}
											if(sys_time_up)
												{
													sys_flag.YanDao_FengFa_Index = 2;
													//���������ʱ�������ֹ��������������
													for(Address = 1; Address <= 10; Address ++)
														{
															SlaveG[Address].Idle_AirWork_Flag = OK;
														}
													 
												}
											else
												{
													
												}

										break;
									case 2:				
										
										if(AUnionD.UnionStartFlag == 1) //����ָ�== 3 �����ֶ�ģʽ�����ܳ�ͻ
											{
												//����������־
												if(Second_Start_Flag == OK)
													{
														if(AUnionD.Big_Pressure <= AUnionD.Start_Value)
															{
																if(AUnionD.AliveOK_Numbers >= 1)  //��ֹһ̨�豸��������
																	{
																		AUnionD.Mode_Index = 1; //��ת״̬
																		Need_Devices = 1;  //������ʼ����
																	}
				
																Allneed_Power = 0; //��ֹ������������
																
																//��ʼ��һ����ʱ��
																Time_Count = 0;
																Compare_Value = PID.Next_Time;	 //����һ��������ͨ����Ļ����
																ONTime_Flag = 0;  //��ʼ��һ����ʱ��
															}
													}
												else
													{
													
														if(AUnionD.Big_Pressure <= AUnionD.Target_Value)
															{
																if(AUnionD.AliveOK_Numbers >= 1)  //��ֹһ̨�豸��������
																	{
																		AUnionD.Mode_Index = 1; //��ת״̬
																		Need_Devices = 1;  //������ʼ����
																	}
			
																//��ʼ��һ����ʱ��
																Time_Count = 0;
																Compare_Value = PID.Next_Time;	 //����һ��������ͨ����Ļ����
																ONTime_Flag = 0;  //��ʼ��һ����ʱ��
															}
				
														
													}
												
											}

											break;
									default:
										sys_flag.YanDao_FengFa_Index = 0;
										break;
								}
						}
					
					
					break;

			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
					//������һ̨�豸
					Second_Start_Flag = OK;
					
					if(Already_WorkNumbers == Need_Devices)
						{
							//���ڵĻ������������Ѿ����еĻ�����
							if(AUnionD.AliveOK_Numbers >= Already_WorkNumbers)
								{
									//��ת����һ�׶Σ���Ҫ����
									if(AUnionD.AliveOK_Numbers <= AUnionD.Need_Numbers)
										{
											AUnionD.Mode_Index = AUnionD.AliveOK_Numbers;
											Need_Devices = AUnionD.Mode_Index;
											 
											
										}
									else
										{
											//�ҵ�ǰ�����еģ������ɵ�ʱ�䣬����������ô������رջ���
											 
											if(Already_WorkNumbers > AUnionD.Need_Numbers )
												{
													if(LowPower_WorkDevices >= 2)
														{
															//����̨�豸��ʱ��͸������У������һ̨
															Need_Devices = Already_WorkNumbers - 1;
															//����Ӧ��ʱ���������
															for(Address = 1; Address <= 10; Address ++)
																{
																	SlaveG[Address].Small_time = 0;
																	SlaveG[Address].Big_time = 0;
																}
															sys_flag.Find_Flag = 3;  //*************************����ʹ��
														}
												}
											
											//���ߵ��豸����Ҫ�����Ѿ������е��豸����
											if(AliveOk_Numbres > Already_WorkNumbers )
												{
													if(AllPower_WorkDevices == Already_WorkNumbers)
														{
															Need_Devices = Already_WorkNumbers + 1;
															//����Ӧ��ʱ���������
															for(Address = 1; Address <= 10; Address ++)
																{
																	SlaveG[Address].Small_time = 0;
																	SlaveG[Address].Big_time = 0;
																}
														}
													
													
												}
										}
									
								}
							else
								{
									//���ڵĻ�������С���Ѿ����еĻ�����
									 
									AUnionD.Mode_Index = AUnionD.AliveOK_Numbers;
									Need_Devices = AUnionD.Mode_Index;
									
									
									
								}
						}
					else
						{
							if(Already_WorkNumbers > Need_Devices)
								{
									if(AUnionD.AliveOK_Numbers <= AUnionD.Need_Numbers)
										{
											AUnionD.Mode_Index = AUnionD.AliveOK_Numbers;
											Need_Devices = AUnionD.Mode_Index;
											 
											
										}
									else
										{
											//�ҵ�ǰ�����еģ������ɵ�ʱ�䣬����������ô������رջ���
											 
											if(Already_WorkNumbers > AUnionD.Need_Numbers )
												{
													if(LowPower_WorkDevices >= 2)
														{
															//����̨�豸��ʱ��͸������У������һ̨
															Need_Devices = Already_WorkNumbers - 1;
															//����Ӧ��ʱ���������
															for(Address = 1; Address <= 10; Address ++)
																{
																	SlaveG[Address].Small_time = 0;
																	SlaveG[Address].Big_time = 0;
																}
														}
												}
											
											//���ߵ��豸����Ҫ�����Ѿ������е��豸����
											if(AliveOk_Numbres > Already_WorkNumbers )
												{
													if(AllPower_WorkDevices == Already_WorkNumbers)
														{
															Need_Devices = Already_WorkNumbers + 1;
															//����Ӧ��ʱ���������
															for(Address = 1; Address <= 10; Address ++)
																{
																	SlaveG[Address].Small_time = 0;
																	SlaveG[Address].Big_time = 0;
																}
														}
												}
										}
								}
							else
								{

										AUnionD.Mode_Index = Need_Devices;
										Time_Count = 0;
										Compare_Value = PID.Next_Time;
										ONTime_Flag = 0;  //��ʼ��һ����ʱ��
								}
							//Need_Devices = AUnionD.Mode_Index;
						
						}

					if(AUnionD.Mode_Index > AUnionD.AliveOK_Numbers)
						AUnionD.Mode_Index = AUnionD.AliveOK_Numbers;


					//��Ŀ��ѹ������ͣ��ѹ��ʱ��ת�ɵȴ�״̬
					if(AUnionD.Big_Pressure >= AUnionD.Stop_Value)
						{
						    
							AUnionD.Mode_Index = 0;
							Need_Devices = 0;
							Loop_Command_10secCount1 = 10; //ǿ�ƹر�һ�Σ���10���������ر�״̬
						}

					break;

			default:

					break;
		}
	

	for(Address = 1; Address <= 10; Address ++)
		{
			//��ֹ������ʾ��100,
			if(SlaveG[Address].Out_Power > 100)
				{
					SlaveG[Address].Out_Power = 100;
				}
		}


	
//�����׶Σ��������������������������������������رջ�������Ӧ�Ļ���

		//�����������Ǽ��٣���Ϊ��ѯ�����������10�룬��Ҫ����10���ж�һ�Σ�����Ҫ���ǣ�PID�Ǳ߻᲻���ٳ��������ʣ�Ҫ��Ҫÿ����һ̨������PIDҪ�����µļ�������

		if(Loop_Command_10secCount >= 3 ) //ѭ���رգ�6̨����Ҫ18��
			{
				Loop_Command_10secCount = 0;  //10�����¼�������Ϊͨ�ţ�ָ��ͷ��������ͺ���
				
				
				if(Already_WorkNumbers > Need_Devices)
					{
						//�Ѿ�������̨�����������̨���������
						Loss_flag = OK;

						
					}
			
				if(Already_WorkNumbers < Need_Devices)
					{
						//�Ѿ�������̨��С�������̨����������
						Need_flag = OK;
					}

				if(Need_flag)
					{
						//���Ӵ����У���û�������е�ʱ����̻���������
						Need_flag = FALSE;
						SlaveG[Min_Address].Command_SendFlag = 3; //����������
						JiZu[Min_Address].Slave_D.StartFlag = OK; //�����û���

						 

					}

				if(Loss_flag)
					{
						//���٣���˭���е�ʱ������ر�
						Loss_flag = FALSE;
						SlaveG[Max_Address].Command_SendFlag = 3; //����������
						JiZu[Max_Address].Slave_D.StartFlag = 0; //�رոû���
					}


				
			}

  
	

	return 0;
}




