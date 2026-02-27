#include "usart2.h"
#include "stdarg.h"	
 



 UtoSlave_Info UtoSlave;
 Duble5_Info Double5;

 UNION_FLAGS Union_Flag; 	 

LCD10Struct LCD10D;
UNION_GGA AUnionD;

LCD10_JZ_Struct LCD10JZ[13];  //用于机组的数据显示

ALCD10Struct UnionLCD;


LCD_WR  LCD10WR;

LCD4013_Struct LCD4013X;

LCD10K1_Struct LCD10X1;





void U2_send_byte(u8 data)
{
	
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);
		USART_SendData(USART2,data);
	 
}

//串口4发送s个字符
void U2_send_str(u8 *str,u8 s)
{
	u8 i;
	for(i=0;i<s;i++)
	{
		U2_send_byte(*str);
		str++;
	}
}


void u2_printf(char* fmt,...)  
{  
  	int len=0;
	int cnt=0;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)U2_Inf.TX_Data,fmt,ap);
	va_end(ap);
	len = strlen((const char*)U2_Inf.TX_Data);
	while(len--)
	  {
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=1); //等待发送结束
	  USART_SendData(USART2,U2_Inf.TX_Data[cnt++]);
	  }
}




///////////////////////////////////////USART2 初始化配置部分//////////////////////////////////	    
//功能：初始化IO 串口2
//输入参数
//bound:波特率
//输出参数
//无
//////////////////////////////////////////////////////////////////////////////////////////////	  
void uart2_init(u32 bound)
{  	 		 
	//GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能GPIOA时钟
	USART_DeInit(USART2);  //复位串口2

     //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  

    //Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 2;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	        //根据指定的参数初始化VIC寄存器
  
    //USART2 初始化设置
	USART_InitStructure.USART_BaudRate = bound;   //一般设置为115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;  //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;  //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式

    USART_Init(USART2, &USART_InitStructure);   //初始化串口
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  //开启中断
    USART_Cmd(USART2, ENABLE);                      //使能串口 
	
//	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);    //使能串口2的DMA发送
//	UART_DMA_Config(DMA1_Channel7,(u32)&USART2->DR,(u32)USART2_TX_BUF,1000);  //DMA1通道7,外设为串口2,存储器为USART2_TX_BUF,长度1000. 										  	
}



void Union_ModBus2_Communication(void)
{
		
		uint8  i = 0;	
		uint8 index = 0;
		uint8 Bytes = 0;

		uint8 Modbus_Address = 0;
		
		uint16 checksum = 0;
		uint8  Cmd_Data = 0;
		uint16 Data_Address = 0;
		uint16 Buffer_Data16 = 0;
		uint16 Data_Length = 0;
		uint8  Index_Address = 0;

		uint16 Buffer_Int1 = 0;

		
		if(U2_Inf.Recive_Ok_Flag)
			{
				U2_Inf.Recive_Ok_Flag = 0;//不能少哦
				 //关闭中断
				USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); 
				 
				checksum  = U2_Inf.RX_Data[U2_Inf.RX_Length - 2] * 256 + U2_Inf.RX_Data[U2_Inf.RX_Length - 1];
				
			 	
				if(checksum == ModBusCRC16(U2_Inf.RX_Data,U2_Inf.RX_Length))
					{	
						Modbus_Address = U2_Inf.RX_Data[0];
						Cmd_Data = U2_Inf.RX_Data[1];
						
						
						
						if(Cmd_Data == 0x03 && Modbus_Address == 1)
							{
								sys_flag.LCD10_Connect = OK;
								
								Data_Address = U2_Inf.RX_Data[2] * 256 + U2_Inf.RX_Data[3];
								Buffer_Data16 = U2_Inf.RX_Data[4] * 256 + U2_Inf.RX_Data[5];
								Data_Length = Buffer_Data16;
								switch (Data_Address)
									{
									 case 0x0000:
									 		
									 
									 			Bytes = sizeof(UnionLCD.Datas);
												U2_Inf.TX_Data[0] = 0x01;
												U2_Inf.TX_Data[1]= 0x03;
									 			U2_Inf.TX_Data[2] = Bytes; //根据数据长度改编
									 			
											
									 			for(index = 3; index < (Bytes + 3); index ++)
													U2_Inf.TX_Data[index] = UnionLCD.Datas[index -3];
												
											
									 			checksum  = ModBusCRC16(U2_Inf.TX_Data,Bytes + 5);
												U2_Inf.TX_Data[Bytes + 3] = checksum >> 8;
												U2_Inf.TX_Data[Bytes + 4] = checksum & 0x00FF;
												
									 			Usart_SendStr_length(USART2,U2_Inf.TX_Data,Bytes +5);

											
									 			break;

									case 0x0063://取机组1的数据值
												Bytes = sizeof(JiZu[1].Datas);
												U2_Inf.TX_Data[0] = 0x01;
												U2_Inf.TX_Data[1]= 0x03;
									 			U2_Inf.TX_Data[2] = Bytes; //根据数据长度改编

												
									 			for(index = 3; index < (Bytes + 3); index ++)
													U2_Inf.TX_Data[index] = JiZu[1].Datas[index -3]; //跟机组地址对齐
													
									 			checksum  = ModBusCRC16(U2_Inf.TX_Data,Bytes + 5);
												U2_Inf.TX_Data[Bytes + 3] = checksum >> 8;
												U2_Inf.TX_Data[Bytes + 4] = checksum & 0x00FF;
												
									 			Usart_SendStr_length(USART2,U2_Inf.TX_Data,Bytes +5);

												break;
									case 0x0077://取机组2的数据值
												Jizu_ReadResponse(2);
												break;

									case 0x008B://取机组3的数据值
												Jizu_ReadResponse(3);
												break;

									case 0x009F://取机组4的数据值
												Jizu_ReadResponse(4);
												break;
									case 0x00B3://取机组5的数据值
												Jizu_ReadResponse(5);
												break;
									case 0x00C7://取机组6的数据值
												Jizu_ReadResponse(6);
												break;
									case 0x00DB://取机组7的数据值
												Jizu_ReadResponse(7);
												break;
									case 0x00EF://取机组8的数据值
												Jizu_ReadResponse(8);
												break;
									case 0x0103://取机组9的数据值
												Jizu_ReadResponse(9);
												break;
									case 0x0117://取机组10的数据值
												Jizu_ReadResponse(10);
												break;
									default:
										

											break;
									}
							}

						if(Cmd_Data == 0x10 && Modbus_Address == 1)
							{
								 //01  10  00 A4	00 02  04  00 0F 3D DD 18 EE 写32位数据指令格式
					  			 //响应： 01 10 00 A4	  00 02 crc
					  			 sys_flag.LCD10_Connect = OK;
								Data_Address = U2_Inf.RX_Data[2] * 256 + U2_Inf.RX_Data[3];
								Data_Length = U2_Inf.RX_Data[4] * 256 + U2_Inf.RX_Data[5];
								Buffer_Data16 = U2_Inf.RX_Data[7]  + U2_Inf.RX_Data[8] * 256;  //高低字节的顺序颠倒  //高低字节的顺序颠倒
						
								switch (Data_Address)
									{
									 case 0x0000:  //联控启动开关指令

													switch (Buffer_Data16)
														{
														case 0: //待机命令
																if(AUnionD.UnionStartFlag == 1)  //设备在运行，则执行关闭
																	{
																		AUnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.UnionStartFlag = Buffer_Data16;
																	}
																if(AUnionD.UnionStartFlag == 3)  //设备手动模式，则执行退出手动，进行待机
																	{
																		for(Index_Address = 1; Index_Address <= 10; Index_Address ++)
											 								{
											 									JiZu[Index_Address].Slave_D.StartFlag = 0; //各机组全都进入待机模式
											 									JiZu[Index_Address].Slave_D.Realys_Out = 0;  //初始化所有继电器
											 									SlaveG[Index_Address].Out_Power = 0;
											 								}
																		AUnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.ZongKong_RelaysOut = 0;
																	}

																break;
														case 1:
																if(AUnionD.UnionStartFlag == 0)  //设备在待机，则执行启动命令
																	{
																		AUnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.UnionStartFlag = Buffer_Data16;
																	}

																break;
														case 3:
																if(AUnionD.UnionStartFlag == 0)  //设备待机中模式，则执行进入手动，
																	{
																		for(Index_Address = 1; Index_Address <= 10; Index_Address ++)
											 								{
											 									JiZu[Index_Address].Slave_D.StartFlag = 3; //各机组全都进入手动模式
											 									JiZu[Index_Address].Slave_D.Realys_Out = 0;  //初始化所有继电器
											 									SlaveG[Index_Address].Out_Power = 0;
											 								}
																		AUnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.UnionStartFlag = Buffer_Data16;
																		UnionLCD.UnionD.ZongKong_RelaysOut = 0;
																	}

																break;
														default:
																break;
														}
									 			
									 			  ModuBus2LCD_Write0x10Response(Data_Address,Data_Length);
									 		
									 			break;
									case 0x0001:  //正常使用台数
												if(Buffer_Data16 <= 10)  //设置的参数必须小于10
									 				UnionLCD.UnionD.Need_Numbers = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.Need_Numbers;

												AUnionD.Need_Numbers = UnionLCD.UnionD.Need_Numbers;


									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;

									 			break;
									case 0x0005:  //PID转换时间
												
												if(Buffer_Data16 <= 100 && Buffer_Data16 >= 5)  //设置的参数必须小于100,大于5
													UnionLCD.UnionD.PID_Next_Time = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.PID_Next_Time;
												
												AUnionD.PID_Next_Time = UnionLCD.UnionD.PID_Next_Time;
													
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;

									case 0x0006:  //PID值P
												
												if(Buffer_Data16 <= 50)  //设置的参数必须小于50
													UnionLCD.UnionD.PID_Pvalue = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.PID_Pvalue;
												
												AUnionD.PID_Pvalue = UnionLCD.UnionD.PID_Pvalue;
													
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;

									case 0x0007:  //PID值I
												
												if(Buffer_Data16 <= 10)  //设置的参数必须小于10
													UnionLCD.UnionD.PID_Ivalue = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.PID_Ivalue;
												
												AUnionD.PID_Ivalue = UnionLCD.UnionD.PID_Ivalue;
													
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;
									case 0x0008:  //PID值D
												
												if(Buffer_Data16 <= 30)  //设置的参数必须小于30
													UnionLCD.UnionD.PID_Dvalue = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.PID_Dvalue;
												
												AUnionD.PID_Dvalue = UnionLCD.UnionD.PID_Dvalue;
													
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;

									case 0x0009:  //16台设备允许使用标志
												
												if(Buffer_Data16 <= 65535)  //设置的参数必须小于65535
													UnionLCD.UnionD.Union16_Flag = Buffer_Data16;
												else
													Buffer_Data16 = UnionLCD.UnionD.Union16_Flag;
												
												AUnionD.Union16_Flag = UnionLCD.UnionD.Union16_Flag;
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 		
									 			break;



												
									case 0x000C:  //目标压力 浮点数 01 10 00 0C 00 02 04                14 AE 07 3F     D5 CB 
												if(Data_Length == 0x02)
													{
														Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
														Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
														Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
														Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
														UnionLCD.UnionD.Target_Value = Float_Int.value;
														
														Buffer_Int1 = UnionLCD.UnionD.Target_Value * 100;
														
														if(Buffer_Int1 >= 10 && Buffer_Int1 <=  Sys_Admin.DeviceMaxPressureSet)
															{
																 sys_config_data.zhuan_huan_temperture_value = Buffer_Int1;
																 UnionLCD.UnionD.Target_Value = Float_Int.value;
																 if(sys_config_data.zhuan_huan_temperture_value > sys_config_data.Auto_stop_pressure)
																 	{
																 		sys_config_data.Auto_stop_pressure = sys_config_data.zhuan_huan_temperture_value + 10;
																		if(sys_config_data.Auto_stop_pressure >= Sys_Admin.DeviceMaxPressureSet )
																			sys_config_data.Auto_stop_pressure = Sys_Admin.DeviceMaxPressureSet - 1;

																		UnionLCD.UnionD.Stop_Value = (float) sys_config_data.Auto_stop_pressure / 100;
																 	}
															
																  Write_Internal_Flash();
																

																 
																 
																
															}
														
														
														ModuBus2LCD_Write0x10Response(Data_Address,Data_Length);	
													}

												break;
									case 0x000E:  //停止压力 浮点数
												if(Data_Length == 0x02)
													{
														Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
														Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
														Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
														Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
														UnionLCD.UnionD.Stop_Value = Float_Int.value;
														Buffer_Int1 = UnionLCD.UnionD.Stop_Value * 100;

													//	u1_printf("\n*停止压力值 = %d   \n",Buffer_Int1);

														if(Buffer_Int1 >= sys_config_data.zhuan_huan_temperture_value && Buffer_Int1 <=  Sys_Admin.DeviceMaxPressureSet)
															{
																sys_config_data.Auto_stop_pressure = Buffer_Int1;
															 
																Write_Internal_Flash();
																 
															}
														
														ModuBus2LCD_Write0x10Response(Data_Address,Data_Length);	
													}

												break;
									case 0x0010:  //启动压力 浮点数
												if(Data_Length == 0x02)
													{
														Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
														Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
														Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
														Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
														UnionLCD.UnionD.Start_Value = Float_Int.value;
														
														Buffer_Int1 = UnionLCD.UnionD.Start_Value * 100;
														if( Buffer_Int1 <  sys_config_data.Auto_stop_pressure)
															{
																sys_config_data.Auto_start_pressure = Buffer_Int1;
																UnionLCD.UnionD.Start_Value = Float_Int.value;
																Write_Internal_Flash();
															}
														ModuBus2LCD_Write0x10Response(Data_Address,Data_Length);	
													}

												break;
									case 0x0012:  //额定压力参数设置
												if(Data_Length == 0x02)
													{
														Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
														Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
														Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
														Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
														UnionLCD.UnionD.Max_Pressure = Float_Int.value;
														
														Buffer_Int1 = UnionLCD.UnionD.Max_Pressure * 100;
														
														AUnionD.Max_Pressure = UnionLCD.UnionD.Max_Pressure;
														
														if( Buffer_Int1 <= 250) //最大额定压力小于2.5Mpa
															{
																 
																if(Sys_Admin.DeviceMaxPressureSet != Buffer_Int1)
																	{
																		Sys_Admin.DeviceMaxPressureSet = Buffer_Int1;
																		
																		//u1_printf("\n*额定的压力值：= %d\n",Sys_Admin.DeviceMaxPressureSet);
																		Write_Admin_Flash();
																		//u1_printf("\n*CCCCCCCCCCC：= %d\n",sys_flag.Attention_Flag);
																	}
																
																
															}
														ModuBus2LCD_Write0x10Response(Data_Address,Data_Length);	
													}

												break;
									case 20:  //A1工作时间
												UnionLCD.UnionD.A1_WorkTime = Buffer_Data16;												
												AUnionD.A1_WorkTime = UnionLCD.UnionD.A1_WorkTime;
												SlaveG[1].Work_Time = AUnionD.A1_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 21:  //A2工作时间
												UnionLCD.UnionD.A2_WorkTime = Buffer_Data16;												
												AUnionD.A2_WorkTime = UnionLCD.UnionD.A2_WorkTime;
												SlaveG[2].Work_Time = AUnionD.A2_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 22:  //A3工作时间
												UnionLCD.UnionD.A3_WorkTime = Buffer_Data16;												
												AUnionD.A3_WorkTime = UnionLCD.UnionD.A3_WorkTime;
												SlaveG[3].Work_Time = AUnionD.A3_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 23:  //A4工作时间
												UnionLCD.UnionD.A4_WorkTime = Buffer_Data16;												
												AUnionD.A4_WorkTime = UnionLCD.UnionD.A4_WorkTime;
												SlaveG[4].Work_Time = AUnionD.A4_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 24:  //A5工作时间
												UnionLCD.UnionD.A5_WorkTime = Buffer_Data16;												
												AUnionD.A5_WorkTime = UnionLCD.UnionD.A5_WorkTime;
												SlaveG[5].Work_Time = AUnionD.A5_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 25:  //A6工作时间
												UnionLCD.UnionD.A6_WorkTime = Buffer_Data16;												
												AUnionD.A6_WorkTime = UnionLCD.UnionD.A6_WorkTime;
												SlaveG[6].Work_Time = AUnionD.A6_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 26:  //A7工作时间
												UnionLCD.UnionD.A7_WorkTime = Buffer_Data16;												
												AUnionD.A7_WorkTime = UnionLCD.UnionD.A7_WorkTime;
												SlaveG[7].Work_Time = AUnionD.A7_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 27:  //A8工作时间
												UnionLCD.UnionD.A8_WorkTime = Buffer_Data16;												
												AUnionD.A8_WorkTime = UnionLCD.UnionD.A8_WorkTime;
												SlaveG[8].Work_Time = AUnionD.A8_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 28:  //A9工作时间
												UnionLCD.UnionD.A9_WorkTime = Buffer_Data16;												
												AUnionD.A9_WorkTime = UnionLCD.UnionD.A9_WorkTime;
												SlaveG[9].Work_Time = AUnionD.A9_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 29:  //A10工作时间
												UnionLCD.UnionD.A10_WorkTime = Buffer_Data16;												
												AUnionD.A10_WorkTime = UnionLCD.UnionD.A10_WorkTime;
												SlaveG[10].Work_Time = AUnionD.A10_WorkTime;
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;

									case 30:  //故障报警复位指令
												UnionLCD.UnionD.Error_Reset = Buffer_Data16;												
												AUnionD.Error_Reset = UnionLCD.UnionD.Error_Reset;
												UnionLCD.UnionD.Union_Error = 0;  //总控故障码也需要清零

												for(index = 1;index <= 10; index ++)
													{
														if(JiZu[index].Slave_D.Error_Code)
															{
																SlaveG[index].Command_SendFlag = 3; //连续发三次
																JiZu[index].Slave_D.Error_Reset = OK;
																
															 
															}
													}

												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 31:  //故障报警声音解除
												UnionLCD.UnionD.Alarm_OFF = Buffer_Data16;												
												AUnionD.Alarm_OFF = UnionLCD.UnionD.Alarm_OFF;
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 32:  //设备类型
									
												UnionLCD.UnionD.Devive_Style = Buffer_Data16;												
												AUnionD.Devive_Style = UnionLCD.UnionD.Devive_Style;
												for(index = 1;index <= 10; index ++)
													{
														if(SlaveG[index].Alive_Flag)
															{
																if(JiZu[index].Slave_D.Error_Code)
																	{
																		SlaveG[index].Command_SendFlag = 3; //连续发三次
																		JiZu[index].Slave_D.Error_Code = 0;
																	}
																	
															}
														
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 33:  //额定设备数量
												if(Buffer_Data16 <= 10)
													{
														UnionLCD.UnionD.Max_Address = Buffer_Data16;												
														AUnionD.Max_Address = UnionLCD.UnionD.Max_Address;
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 35:  //ModBus通信地址
												if(Buffer_Data16 <= 250)
													{
														UnionLCD.UnionD.ModBus_Address = Buffer_Data16;												
														AUnionD.ModBus_Address = UnionLCD.UnionD.ModBus_Address;
														Sys_Admin.ModBus_Address = UnionLCD.UnionD.ModBus_Address;

														 
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;

									case 36:  //禁止设备数量
												if(Buffer_Data16 <= 10)
													{
														UnionLCD.UnionD.OFFlive_Numbers = Buffer_Data16;												
														AUnionD.OFFlive_Numbers = UnionLCD.UnionD.OFFlive_Numbers;
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;

									case 38:  //排烟温度报警值
												if(Buffer_Data16 <= 200 && Buffer_Data16 >= 80)
													{
														UnionLCD.UnionD.PaiYan_AlarmValue = Buffer_Data16;												
														AUnionD.PaiYan_AlarmValue = UnionLCD.UnionD.PaiYan_AlarmValue;
														Sys_Admin.Danger_Smoke_Value = Buffer_Data16 *10;
														 Write_Admin_Flash();
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									case 40:  //总控相关报警允许标志
												
												UnionLCD.UnionD.Alarm_Allow_Flag = Buffer_Data16;												
												AUnionD.Alarm_Allow_Flag = UnionLCD.UnionD.Alarm_Allow_Flag;
									
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;

									case 41:  //总控继电器输出
												
												UnionLCD.UnionD.ZongKong_RelaysOut = Buffer_Data16;												
												AUnionD.ZongKong_RelaysOut = UnionLCD.UnionD.ZongKong_RelaysOut;
												if(Buffer_Data16 & 0x0001)   //风机标志位
													{
														
														ZongKong_YanFa_Open();
														
													}
												else
													{
														ZongKong_YanFa_Close();
														 
													}
												
									 			ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									 			break;
									
									case 101:// A1功率调整输出指令，在手动模式下有效
																					
												 
												SlaveG[1].Out_Power = Buffer_Data16;
												JiZu[1].Slave_D.Power = Buffer_Data16;
												 
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;
									case 108:// A1要进行排污的指令
																					
												JiZu[1].Slave_D.PaiwuFa_State = Buffer_Data16;
												SlaveG[1].Paiwu_Flag = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;
									
									case 113:// A1继电器输出指令，在手动模式下有效

												JiZu[1].Slave_D.Realys_Out = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);

											break;
									case 114:// A1点火功率保护值
												if(Buffer_Data16 < 60 &&Buffer_Data16 >=30)
													{
														JiZu[1].Slave_D.DianHuo_Value = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 115:// A1最大功率保护值
												if(Buffer_Data16 >=30 && Buffer_Data16 <= 100)
													{
														JiZu[1].Slave_D.Max_Power = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 116:// A1本体温度保护值
												if(Buffer_Data16 < 350)
													{
														JiZu[1].Slave_D.Inside_WenDu_Protect = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;


									//*****************A222222机组相关的控制指令*********************8		
									case 121:// A2功率调整输出指令，在手动模式下有效
									
												SlaveG[2].Out_Power = Buffer_Data16;
												JiZu[2].Slave_D.Power = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;

									case 128:// A2要进行排污的指令
																					
												JiZu[2].Slave_D.PaiwuFa_State = Buffer_Data16;
												SlaveG[2].Paiwu_Flag = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;
									case 133:// A1继电器输出指令，在手动模式下有效

												JiZu[2].Slave_D.Realys_Out = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);

											break;
									case 134:// A2点火功率保护值
												if(Buffer_Data16 < 60 &&Buffer_Data16 >=30)
													{
														JiZu[2].Slave_D.DianHuo_Value = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 135:// A2最大功率保护值
												if(Buffer_Data16 >=30 && Buffer_Data16 <= 100)
													{
														JiZu[2].Slave_D.Max_Power = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 136:// A2本体温度保护值
												if(Buffer_Data16 < 350)
													{
														JiZu[2].Slave_D.Inside_WenDu_Protect = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;

									//*****************A3333333机组相关的控制指令*********************8		
									case 141:// A3功率调整输出指令，在手动模式下有效
									
												SlaveG[3].Out_Power = Buffer_Data16;
												JiZu[3].Slave_D.Power = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;

									case 148:// A3要进行排污的指令
																					
												JiZu[3].Slave_D.PaiwuFa_State = Buffer_Data16;
												SlaveG[3].Paiwu_Flag = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;
									case 153:// A3继电器输出指令，在手动模式下有效

												JiZu[3].Slave_D.Realys_Out = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);

											break;
									
									case 154:// A3点火功率保护值
												if(Buffer_Data16 < 60 &&Buffer_Data16 >=30)
													{
														JiZu[3].Slave_D.DianHuo_Value = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 155:// A3最大功率保护值
												if(Buffer_Data16 >=30 && Buffer_Data16 <= 100)
													{
														JiZu[3].Slave_D.Max_Power = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 156:// A3本体温度保护值
												if(Buffer_Data16 < 350)
													{
														JiZu[3].Slave_D.Inside_WenDu_Protect = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									
									//*****************A4444444机组相关的控制指令*********************8		
									case 161:// A4功率调整输出指令，在手动模式下有效
									
												SlaveG[4].Out_Power = Buffer_Data16;
												JiZu[4].Slave_D.Power = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;

									case 168:// A4要进行排污的指令
																					
												JiZu[4].Slave_D.PaiwuFa_State = Buffer_Data16;
												SlaveG[4].Paiwu_Flag = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
									
											break;
									case 173:// A4继电器输出指令，在手动模式下有效

												JiZu[4].Slave_D.Realys_Out = Buffer_Data16;
												ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);

											break;
									
									case 174:// A4点火功率保护值
												if(Buffer_Data16 < 60 &&Buffer_Data16 >=30)
													{
														JiZu[4].Slave_D.DianHuo_Value = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 175:// A4最大功率保护值
												if(Buffer_Data16 >=30 && Buffer_Data16 <= 100)
													{
														JiZu[4].Slave_D.Max_Power = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									case 176:// A4本体温度保护值
												if(Buffer_Data16 < 350)
													{
														JiZu[4].Slave_D.Inside_WenDu_Protect = Buffer_Data16;
														ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
													}

											break;
									
									

											

									default:

											break;

									}
							}

							
						
						
					}
					
				 
				
			//对接收缓冲器清零
				for( i = 0; i < 200;i++ )
					U2_Inf.RX_Data[i] = 0x00;
			
			//重新开启中断
				USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); 
				
			}

		
}


void ModBus2LCD_Communication(void)
{
		
		
		
}

uint8 ModuBus2LCD_Write0x10Response(uint16 address,uint16 Data16)
{
	uint16 check_sum = 0;
	
	U2_Inf.TX_Data[0] = 0x01;
	U2_Inf.TX_Data[1]= 0x10;

	
	U2_Inf.TX_Data[2] = address >> 8;    // 地址高字节
	U2_Inf.TX_Data[3] = address & 0x00FF;  //地址低字节
	

	U2_Inf.TX_Data[4] = Data16 >> 8;  //数据高字节
	U2_Inf.TX_Data[5] = Data16 & 0x00ff;   //数据低字节

	check_sum  = ModBusCRC16(U2_Inf.TX_Data,8);   //这个根据总字节数改变
	U2_Inf.TX_Data[6]  = check_sum >> 8 ;
	U2_Inf.TX_Data[7]  = check_sum & 0x00FF;

	Usart_SendStr_length(USART2,U2_Inf.TX_Data,8);

	return 0;
}


uint8 ModuBus2LCD4013_Write0x10Response(uint16 address,uint16 Data16)
{
	uint16 check_sum = 0;
	
	U2_Inf.TX_Data[0] = 0x02;
	U2_Inf.TX_Data[1]= 0x10;

	
	U2_Inf.TX_Data[2] = address >> 8;    // 地址高字节
	U2_Inf.TX_Data[3] = address & 0x00FF;  //地址低字节
	

	U2_Inf.TX_Data[4] = Data16 >> 8;  //数据高字节
	U2_Inf.TX_Data[5] = Data16 & 0x00ff;   //数据低字节

	check_sum  = ModBusCRC16(U2_Inf.TX_Data,8);   //这个根据总字节数改变
	U2_Inf.TX_Data[6]  = check_sum >> 8 ;
	U2_Inf.TX_Data[7]  = check_sum & 0x00FF;

	Usart_SendStr_length(USART2,U2_Inf.TX_Data,8);

	return 0;
}


uint8 ModuBus2LCD_WriteAdress0x0000Response(uint16 Buffer_Data16)
{

		switch (Sys_Admin.Device_Style)
			{
				case 0:
				case 1:  //单机模式
						
						if(Buffer_Data16 == 1) 
							{
								if(sys_data.Data_10H == 0)
									sys_start_cmd();
								 
							}
						if(Buffer_Data16 == 0) 
							{
								if(sys_data.Data_10H == 2)
									sys_close_cmd();
			
								if(sys_data.Data_10H == 3)
									{
										sys_data.Data_10H = 0;
										GetOut_Mannual_Function(); 
									}
								 
							}
						if(Buffer_Data16 == 3) 
							{
								if(sys_data.Data_10H == 0)
									{
										sys_data.Data_10H = 3 ;
										GetOut_Mannual_Function();
									}
							}
			
						break;
				case 2: //双拼模式   要考虑 手动模式和 启动联控的命令
				case 3:
						if(Buffer_Data16 == 1) 
							{
								//启动联控
								UnionD.UnionStartFlag = OK;
								
								//if(sys_data.Data_10H == 0)
								//	sys_start_cmd();
								//if(LCD10JZ[2].DLCD.Device_State == 1)
								//	{
								//		SlaveG[2].Startclose_Sendflag = 3;
								//		SlaveG[2].Startclose_Data = Buffer_Data16;
								//	}
								 
							}
						if(Buffer_Data16 == 0) 
							{
								if(sys_data.Data_10H == 2)
									sys_close_cmd();

								
								//关闭联控
								UnionD.UnionStartFlag = FALSE;
								
								SlaveG[2].Startclose_Sendflag = 3;
								SlaveG[2].Startclose_Data = Buffer_Data16;
									
								
								if(sys_data.Data_10H == 3)
									{
										sys_data.Data_10H = 0;
										GetOut_Mannual_Function(); 
									}
							}
					

						if(Buffer_Data16 == 3) 
							{
								UnionD.UnionStartFlag  = 3;
								
								if(sys_data.Data_10H == 0)
									{
										sys_data.Data_10H = 3 ;
										GetOut_Mannual_Function();
									}
								SlaveG[2].Startclose_Sendflag = 3;
								SlaveG[2].Startclose_Data = Buffer_Data16;
							}
			
						break;
				default:
			
						break;
			}


	return 0;
}



uint8 Jizu_ReadResponse(uint8 address)
{
	uint8 Bytes = 0;
	uint8 index = 0;
	uint16 checksum = 0;
	Bytes = sizeof(JiZu[address].Datas);
	U2_Inf.TX_Data[0] = 0x01;
	U2_Inf.TX_Data[1]= 0x03;
	U2_Inf.TX_Data[2] = Bytes; //根据数据长度改编
	for(index = 3; index < (Bytes + 3); index ++)
		U2_Inf.TX_Data[index] = JiZu[address].Datas[index -3]; //跟机组地址对齐
		
	checksum  = ModBusCRC16(U2_Inf.TX_Data,Bytes + 5);
	U2_Inf.TX_Data[Bytes + 3] = checksum >> 8;
	U2_Inf.TX_Data[Bytes + 4] = checksum & 0x00FF;
	Usart_SendStr_length(USART2,U2_Inf.TX_Data,Bytes +5);

		return 0;
}



uint8 LCD4013_MmodBus2_Communicastion( )
{
	uint8 LCD4013_Address = 2;  //屏4013的地址
	uint8 Bytes = 0;
	uint8 index = 0;
	uint16 checksum = 0;
	
	uint8 Cmd_Data = 0;
	uint16 Data_Address = 0;
	uint16 Buffer_Data16 = 0;
	uint16 Data_Length = 0;
	Cmd_Data  = U2_Inf.RX_Data[1];

	switch (Cmd_Data)
		{
			case 0x03:
				Data_Address = U2_Inf.RX_Data[2] * 256 + U2_Inf.RX_Data[3];
				Buffer_Data16 = U2_Inf.RX_Data[4] * 256 + U2_Inf.RX_Data[5];
				switch (Data_Address)
					{
					case 0x0000:
							Bytes = sizeof(LCD4013X.Datas);
							U2_Inf.TX_Data[0] = LCD4013_Address;
							U2_Inf.TX_Data[1]= 0x03;
				 			U2_Inf.TX_Data[2] = Bytes; //根据数据长度改编,不能超过200
				 			
						
				 			for(index = 3; index < (Bytes + 3); index ++)
								U2_Inf.TX_Data[index] = LCD4013X.Datas[index -3];
							
						
				 			checksum  = ModBusCRC16(U2_Inf.TX_Data,Bytes + 5);
							U2_Inf.TX_Data[Bytes + 3] = checksum >> 8;
							U2_Inf.TX_Data[Bytes + 4] = checksum & 0x00FF;
							
				 			Usart_SendStr_length(USART2,U2_Inf.TX_Data,Bytes +5);

							break;
					default:
							break;
								
								
					}

				
				break;
			case 0x10:
					Data_Address = U2_Inf.RX_Data[2] * 256 + U2_Inf.RX_Data[3];
					Data_Length = U2_Inf.RX_Data[4] * 256 + U2_Inf.RX_Data[5];
					Buffer_Data16 = U2_Inf.RX_Data[7]  + U2_Inf.RX_Data[8] * 256;
					switch (Data_Address)
						{
						case 0x0000:
									
									switch (Buffer_Data16)
										{
										case 0:
												if(sys_data.Data_10H == 2)
													{
														sys_close_cmd();
														LCD4013X.DLCD.Relays_Out = 0;
														LCD4013X.DLCD.Start_Close_Cmd = 0;
													}
												if(sys_data.Data_10H == 3)
													{
														//退出手动模式
														sys_data.Data_10H = 0;
														LCD4013X.DLCD.Relays_Out = 0;
														GetOut_Mannual_Function();
														LCD4013X.DLCD.Start_Close_Cmd = 0;
													}
												ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);

												break;
										case 1: //启动命令
												if(sys_data.Data_10H == 0)
													{
														sys_start_cmd();
														LCD4013X.DLCD.Start_Close_Cmd = 2;
													}
												ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);

												break;
										case 3://手动模式
												//只能在待机情况下进入手动
												if(sys_data.Data_10H == 0)
													{
														LCD4013X.DLCD.Relays_Out = 0;
														GetOut_Mannual_Function();
														sys_data.Data_10H = 3;
														LCD4013X.DLCD.Start_Close_Cmd = 3;
													}
												ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);

												break;
										default:
												break;
										}
									 

									break;
						case 0x0003: //故障复位
									if(Buffer_Data16)
										{
											LCD4013X.DLCD.Error_Code = 0;
											sys_flag.Error_Code = 0; //故障复位 
											 
											ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
										}

									break;

						case 0x0004:  //手动风机功率
									if(Buffer_Data16 <= 100)
										{
											LCD4013X.DLCD.Air_Power = Buffer_Data16;
											PWM_Adjust(Buffer_Data16);
											 
											ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
										}

									break;
						case 0x000E: //点火功率
									if(Buffer_Data16 < 60 &&Buffer_Data16 >=30)
										{
											LCD4013X.DLCD.Dian_Huo_Power = Buffer_Data16;
											Sys_Admin.Dian_Huo_Power = Buffer_Data16;
											Write_Admin_Flash();
											ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
										}

									break;
						case 0x000F: //最大功率
									if(Buffer_Data16 <= 100 &&Buffer_Data16 >=30)
										{
											LCD4013X.DLCD.Max_Work_Power = Buffer_Data16;
											Sys_Admin.Max_Work_Power = Buffer_Data16;
											Write_Admin_Flash();
											ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
										}

									break;
						case 0x0012: //本体温度保护值
									if(Buffer_Data16 <= 350 &&Buffer_Data16 >=200)
										{
											LCD4013X.DLCD.Inside_WenDu_ProtectValue = Buffer_Data16;
											Sys_Admin.Inside_WenDu_ProtectValue = Buffer_Data16;
											Write_Admin_Flash();
											ModuBus2LCD_Write0x10Response(Data_Address,Buffer_Data16);
										}

									break;
						case 0x0016: //手动模式下，继电器输出测试
									
									//在手动模式下，根据标志位开相应的继电器
										LCD4013X.DLCD.Relays_Out = 	Buffer_Data16;										
										if(Buffer_Data16 & 0x0001)	 //风机标志位
											{
												
												Send_Air_Open();
												
											}
										else
											{
												Send_Air_Close();
												 
											}

										if(Buffer_Data16 & 0x0002)	 //水泵，补水阀标志位
											{
												Feed_Main_Pump_ON();
												Second_Water_Valve_Open();
											}
										else
											{
												 Feed_Main_Pump_OFF();
												 Second_Water_Valve_Close();
											}

										if(Buffer_Data16 & 0x0004)	 //排水阀
											{
												Pai_Wu_Door_Open();
											}
										else
											{
												Pai_Wu_Door_Close();
											}
										if(Buffer_Data16 & 0x0008)	 //明火阀
											{
												WTS_Gas_One_Open();
											}
										else
											{
												WTS_Gas_One_Close();
											}

										if(Buffer_Data16 & 0x0010)	 //高压侧补水阀
											{ 
												LianXu_Paiwu_Open();
											}
										else
											{
												LianXu_Paiwu_Close();
											}

									break;
						case 25:  //差压低液位值校订值
								if(Buffer_Data16 <= 1500 &&Buffer_Data16 >=10)
									{
										LCD4013X.DLCD.ChaYa_WaterLow_Set = Buffer_Data16;
										Sys_Admin.ChaYa_WaterLow_Set = Buffer_Data16;
										Write_Admin_Flash();
										ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
									}

								 break;
						case 26:  //差压中液位值校订值
								if(Buffer_Data16 <= 1500 &&Buffer_Data16 >=10)
									{
										LCD4013X.DLCD.ChaYa_WaterMid_Set = Buffer_Data16;
										Sys_Admin.ChaYa_WaterMid_Set = Buffer_Data16;
										Write_Admin_Flash();
										ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
									}

								 break;
						case 27:  //差压高液位值校订值
								if(Buffer_Data16 <= 1500 &&Buffer_Data16 >=10)
									{
										LCD4013X.DLCD.ChaYa_WaterHigh_Set = Buffer_Data16;
										Sys_Admin.ChaYa_WaterHigh_Set = Buffer_Data16;
										Write_Admin_Flash();
										ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
									}

								 break;

						case 28:  //一次侧低液位报警值
								if(Buffer_Data16 <= 1500 &&Buffer_Data16 >=10)
									{
										LCD4013X.DLCD.InsideWater_AlarmValue = Buffer_Data16;
										Sys_Admin.InsideWater_AlarmValue = Buffer_Data16;
										Write_Admin_Flash();
										ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
									}

								 break;
						case 0x0020: //本体地址设置
									if(Buffer_Data16 <= 6 &&Buffer_Data16 >=1)
										{
											LCD4013X.DLCD.Address = Buffer_Data16;
											Sys_Admin.ModBus_Address = Buffer_Data16;
											Write_Admin_Flash();
											ModuBus2LCD4013_Write0x10Response(Data_Address,Buffer_Data16);
										}
									break;
						case 0x002A:
							
							if(Data_Length == 0x02)
								{
									Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
									Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
									Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
									Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
									LCD4013X.DLCD.Target_Pressure = Float_Int.value;
									
									Buffer_Data16 = LCD4013X.DLCD.Target_Pressure * 100;
									
									if(Buffer_Data16 >= 10 && Buffer_Data16 <=	Sys_Admin.DeviceMaxPressureSet)
										{
											sys_config_data.zhuan_huan_temperture_value = Buffer_Data16;
											LCD4013X.DLCD.Target_Pressure = Float_Int.value;
											 if(sys_config_data.zhuan_huan_temperture_value > sys_config_data.Auto_stop_pressure)
												{
													sys_config_data.Auto_stop_pressure = sys_config_data.zhuan_huan_temperture_value + 10;
													if(sys_config_data.Auto_stop_pressure >= Sys_Admin.DeviceMaxPressureSet )
														sys_config_data.Auto_stop_pressure = Sys_Admin.DeviceMaxPressureSet - 1;

													LCD4013X.DLCD.Stop_Pressure = (float) sys_config_data.Auto_stop_pressure / 100;
												}
										
											  Write_Internal_Flash();
											 
										}
									
									
									ModuBus2LCD4013_Write0x10Response(Data_Address,Data_Length);	
								}

							break;
						case 0x002C:  //停止压力 浮点数
									if(Data_Length == 0x02)
										{
											Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
											Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
											Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
											Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
											LCD4013X.DLCD.Stop_Pressure = Float_Int.value;
											Buffer_Data16 = LCD4013X.DLCD.Stop_Pressure * 100;

											if(Buffer_Data16 >= sys_config_data.zhuan_huan_temperture_value && Buffer_Data16 <  Sys_Admin.DeviceMaxPressureSet)
												{
													sys_config_data.Auto_stop_pressure = Buffer_Data16;
													Write_Internal_Flash();
													 
												}
											
											ModuBus2LCD4013_Write0x10Response(Data_Address,Data_Length);	
										}

									break;
						case 0x002E:  //启动压力 浮点数
									if(Data_Length == 0x02)
										{
											Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
											Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
											Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
											Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
											LCD4013X.DLCD.Start_Pressure = Float_Int.value;
											Buffer_Data16 = LCD4013X.DLCD.Start_Pressure * 100;
											if( Buffer_Data16 <  sys_config_data.Auto_stop_pressure)
												{
													sys_config_data.Auto_start_pressure = Buffer_Data16;
													LCD4013X.DLCD.Start_Pressure = Float_Int.value;
													Write_Internal_Flash();
												}
											ModuBus2LCD4013_Write0x10Response(Data_Address,Data_Length);	
										}
									break;
						case 0x0030:  //额定压力参数设置
										if(Data_Length == 0x02)
											{
												Float_Int.byte4.data_LL = U2_Inf.RX_Data[7];
												Float_Int.byte4.data_LH = U2_Inf.RX_Data[8];
												Float_Int.byte4.data_HL = U2_Inf.RX_Data[9];
												Float_Int.byte4.data_HH = U2_Inf.RX_Data[10];
												LCD4013X.DLCD.Max_Pressure = Float_Int.value;
												Buffer_Data16 = LCD4013X.DLCD.Max_Pressure * 100;
												
												if( Buffer_Data16 <= 250) //最大额定压力小于2.5Mpa
													{
														Sys_Admin.DeviceMaxPressureSet = Buffer_Data16;
														Write_Admin_Flash();
													}
												ModuBus2LCD4013_Write0x10Response(Data_Address,Data_Length);	
											}

										break;
							
							default:

									break;
						}
				break;
			default:

					break;
		}


	return 0;
}

uint8  ModBus2LCD4013_Lcd7013_Communication(void)
{
	uint8 Index = 0;
	uint8 Modbus_Address = 0;
	uint16 checksum = 0;


	

	   LCD4013_Data_Check_Function();



	
		if(U2_Inf.Recive_Ok_Flag)
			{
				U2_Inf.Recive_Ok_Flag = 0;//不能少哦
				 //关闭中断
				USART_ITConfig(USART2, USART_IT_RXNE, DISABLE); 
				 
				checksum  = U2_Inf.RX_Data[U2_Inf.RX_Length - 2] * 256 + U2_Inf.RX_Data[U2_Inf.RX_Length - 1];
				
			 	
				if(checksum == ModBusCRC16(U2_Inf.RX_Data,U2_Inf.RX_Length))
					{
						Modbus_Address = U2_Inf.RX_Data[0];
						if(Modbus_Address == 2 )
							{	
								sys_flag.Lcd4013_OnLive_Flag = OK;
								LCD4013_MmodBus2_Communicastion();
							}
					}

				//对接收缓冲器清零
				for( Index = 0; Index < 200;Index++ )
					U2_Inf.RX_Data[Index] = 0x00;
			
				//重新开启中断
				USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); 
			}
	


	return 0;
}



uint8  LCD4013_Data_Check_Function(void)
{
	float ResData = 0.0;


	LCD4013X.DLCD.Soft_Version = Soft_Version;  //软件版本


	if(sys_data.Data_10H == 0)
		{
			LCD4013X.DLCD.Start_Close_Cmd = 0;
		}
	else
		{
			LCD4013X.DLCD.Start_Close_Cmd = 1;
		}
	

	LCD4013X.DLCD.Address = Sys_Admin.ModBus_Address;

	sys_flag.Address_Number = LCD4013X.DLCD.Address;
	
	LCD4013X.DLCD.Device_State = sys_data.Data_10H;
	//联机标志在串口4，检查是否收到联机信息

	LCD4013X.DLCD.Error_Code = sys_flag.Error_Code;

	LCD4013X.DLCD.Air_Power = sys_data.Data_1FH;
	LCD4013X.DLCD.Flame_State = sys_flag.flame_state;
	LCD4013X.DLCD.Pump_State = Switch_Inf.Water_Valve_Flag;
	LCD4013X.DLCD.Water_State = LCD10D.DLCD.Water_State;
	LCD4013X.DLCD.Paiwu_State = Switch_Inf.pai_wu_flag;
	LCD4013X.DLCD.Air_State = Switch_Inf.air_on_flag;
	LCD4013X.DLCD.Air_Speed = sys_flag.Fan_Rpm;
	LCD4013X.DLCD.Dian_Huo_Power = Sys_Admin.Dian_Huo_Power;
	LCD4013X.DLCD.Max_Work_Power = Sys_Admin.Max_Work_Power;
	LCD4013X.DLCD.Inside_WenDu_ProtectValue = Sys_Admin.Inside_WenDu_ProtectValue;
	LCD4013X.DLCD.Inside_WenDu = sys_flag.Protect_WenDu;

	LCD4013X.DLCD.HPWater_Value = sys_flag.HPWater_Value;
	LCD4013X.DLCD.LPWater_Value = sys_flag.LPWater_Value;

	LCD4013X.DLCD.ChaYa_WaterLow_Set = Sys_Admin.ChaYa_WaterLow_Set;
	LCD4013X.DLCD.ChaYa_WaterMid_Set = Sys_Admin.ChaYa_WaterMid_Set;
	LCD4013X.DLCD.ChaYa_WaterHigh_Set = Sys_Admin.ChaYa_WaterHigh_Set;
	LCD4013X.DLCD.InsideWater_AlarmValue = Sys_Admin.InsideWater_AlarmValue;

	
	
	ResData = Temperature_Data.Pressure_Value;
	LCD4013X.DLCD.Steam_Pressure = ResData / 100; //当前蒸汽的压力

	ResData = sys_config_data.zhuan_huan_temperture_value;
	LCD4013X.DLCD.Target_Pressure = ResData / 100; //目标压力

	ResData = sys_config_data.Auto_stop_pressure;
	LCD4013X.DLCD.Stop_Pressure = ResData / 100; //停机压力

	ResData = sys_config_data.Auto_start_pressure;
	LCD4013X.DLCD.Start_Pressure = ResData / 100; //启动压力

	ResData = Sys_Admin.DeviceMaxPressureSet;
	LCD4013X.DLCD.Max_Pressure = ResData / 100; //额定蒸汽压力

	return 0 ;
}


