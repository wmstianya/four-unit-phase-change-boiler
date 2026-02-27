#include "usart3.h"
#include "stdarg.h"

  



  	 
///////////////////////////////////////USART3 printf支持部分//////////////////////////////////
//串口2,u2_printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
////////////////////////////////////////////////////////////////////////////////////////////////
void u3_printf(char* fmt,...)  
{  
  
}
///////////////////////////////////////USART2 初始化配置部分//////////////////////////////////	    
//功能：初始化IO 串口2
//输入参数
//bound:波特率
//输出参数
//无
//////////////////////////////////////////////////////////////////////////////////////////////	  
void uart3_init(u32 bound)
{  	 		 
	//GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
	USART_DeInit(USART3);  //复位串口3

     //USART3_TX   PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);
   
    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);  

    //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	        //根据指定的参数初始化VIC寄存器
  
    //USART3 初始化设置
	USART_InitStructure.USART_BaudRate = bound;   //一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;  //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;  //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;  //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  //无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	  //收发模式

    USART_Init(USART3, &USART_InitStructure);   //初始化串口
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //开启中断
    USART_Cmd(USART3, ENABLE);                      //使能串口 
	
//	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE);    //使能串口2的DMA发送
//	UART_DMA_Config(DMA1_Channel7,(u32)&USART2->DR,(u32)USART2_TX_BUF,1000);  //DMA1通道7,外设为串口2,存储器为USART2_TX_BUF,长度1000. 										  	
}




uint8 FuNiSen_Read_WaterDevice_Function(void)
{
	static uint8  Jump_Index = 0;
	static uint8 Jump_Count = 0;
	uint8 Address = 8; //默认本机内地址为 8 ，用于进水调节阀
	uint16 check_sum = 0;
	uint16 Percent = 0;


	//200ms调节一次
	if(sys_flag.WaterAJ_Flag == 0)
		return 0 ;

	sys_flag.WaterAJ_Flag = 0;
	sys_flag.waterSend_Flag = OK;

	if(sys_flag.Water_Percent <= 100)
		Percent =  sys_flag.Water_Percent * 10;
	else
		Percent = 0 ; //值不对则关闭

	Jump_Count ++;
	if(Jump_Count > 5)
		{
			Jump_Index = 0;
			Jump_Count = 0;
		}
	else
		Jump_Index = 1;
		
	
	switch (Jump_Index)
			{
			case 0: //读取实时的开度数值
					U3_Inf.TX_Data[0] = Address;
					U3_Inf.TX_Data[1] = 0x03;// 
					
					U3_Inf.TX_Data[2] = 0x00;
					U3_Inf.TX_Data[3] = 0x00;//地址

					U3_Inf.TX_Data[4] = 0x00;
					U3_Inf.TX_Data[5] = 0x01;//读取数据个数
				 
					check_sum  = ModBusCRC16(U3_Inf.TX_Data,8);
					U3_Inf.TX_Data[6]  = check_sum >> 8 ;
					U3_Inf.TX_Data[7]  = check_sum & 0x00FF;
					
					Usart_SendStr_length(USART3,U3_Inf.TX_Data,8);
				   
					//Jump_Index = 1;
					break;
			case 1://写入开度的值
					//Jump_Index = 0;

					U3_Inf.TX_Data[0] = Address;
					U3_Inf.TX_Data[1] = 0x06;// 
					
					U3_Inf.TX_Data[2] = 0x00;
					U3_Inf.TX_Data[3] = 0x01;//地址

					U3_Inf.TX_Data[4] = Percent >> 8;
					U3_Inf.TX_Data[5] = Percent & 0x00FF;//写入的开度值
				 
					check_sum  = ModBusCRC16(U3_Inf.TX_Data,8);
					U3_Inf.TX_Data[6]  = check_sum >> 8 ;
					U3_Inf.TX_Data[7]  = check_sum & 0x00FF;			
					Usart_SendStr_length(USART3,U3_Inf.TX_Data,8);
					break;
			default:
					Jump_Index = 0;
					break;
			}
	
	return 0;
}

uint8 FuNiSen_RecData_WaterDevice_Processs(void)
{
	uint16 checksum = 0;
	uint8 address = 0;
	uint8 i = 0;
	uint8 command = 0;
	uint8 Length = 0;
	uint16 Value_Buffer = 0;

	if(sys_flag.waterSend_Count >= 10)
	   sys_flag.Error_Code = Error19_SupplyWater_UNtalk;
	
	if(U3_Inf.Recive_Ok_Flag)
		{
			U3_Inf.Recive_Ok_Flag = 0;//不能少哦
			 //关闭中断
			USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); 
			 
			checksum  = U3_Inf.RX_Data[U3_Inf.RX_Length - 2] * 256 + U3_Inf.RX_Data[U3_Inf.RX_Length - 1];

			address = U3_Inf.RX_Data[0]; 
			command = U3_Inf.RX_Data[1];
			Length = U3_Inf.RX_Data[2];
		
			//01 03 06 07 CF 07 CF 00 00 45 99
			//01 06 00 02 07 CF 6A 6E 
			if(address == 0x08 &&checksum == ModBusCRC16(U3_Inf.RX_Data,U3_Inf.RX_Length) )
				{
					 sys_flag.waterSend_Flag = FALSE;
					 sys_flag.waterSend_Count = 0;
					if(command == 0x03 && Length == 0x02)
						{
						   //读取当前阀的状态
						  
						   
							Value_Buffer = U3_Inf.RX_Data[3] * 256 + U3_Inf.RX_Data[4];
							if(Value_Buffer == 0xEA)
								sys_flag.Water_Error_Code = OK; //故障状态
							else
							   sys_flag.Water_Error_Code = 0;

							if(sys_flag.Water_Error_Code == OK)
							   sys_flag.Error_Code = Error18_SupplyWater_Error;
							
						}
					
				}
				

		//对接收缓冲器清零
			for( i = 0; i < 100;i++ )
				U3_Inf.RX_Data[i] = 0x00;
		
		//重新开启中断
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); 
			
		}
	
	return 0;	
}



void ModBus_Uart3_LocalRX_Communication(void)
{
		
		uint8  i = 0;	
		uint8  Target_Address = 0;
		uint8 command = 0;
		uint8 Length = 0;
		uint16 Value_Buffer = 0;

		uint16 Value_A = 0;
		uint16 Value_B = 0;
		float Value20mA_A = 0;
		float Value20mA_B = 0;
		
		uint16 checksum = 0;

		uint16 HH_Buffer = 0;
		uint16 LL_Buffer = 0;
		

		//差压模块通信丢失保护
		if(sys_flag.A3B3_ChaYaMokuai_TxRX_Count >= 20)
			{
				sys_flag.A3B3_ChaYaMokuai_TxRX_Count = 20; //保持状态，等待通信恢复
				sys_flag.HPWater_Value = 0;
				sys_flag.LPWater_Value = 0;   //将需要读取的参数清零
				
			}

		
		// UnionLCD.UnionD.Steam_Speed = 2410;
		//  UnionLCD.UnionD.LL_Steam =2659 ;
		
		if(U3_Inf.Recive_Ok_Flag)
			{
				U3_Inf.Recive_Ok_Flag = 0;//不能少哦
				 //关闭中断
				USART_ITConfig(USART3, USART_IT_RXNE, DISABLE); 
				 
				checksum  = U3_Inf.RX_Data[U3_Inf.RX_Length - 2] * 256 + U3_Inf.RX_Data[U3_Inf.RX_Length - 1];
				command = U3_Inf.RX_Data[1];
				Length = U3_Inf.RX_Data[2];
				

				
				if(checksum == ModBusCRC16(U3_Inf.RX_Data,U3_Inf.RX_Length))
					{
						Target_Address = U3_Inf.RX_Data[0];
						

						if(Target_Address == 1) //涡街流量的默认地址为1
							{
								 Float_Int.byte4.data_LL = U3_Inf.RX_Data[6];
								 Float_Int.byte4.data_LH =U3_Inf.RX_Data[5];
								 Float_Int.byte4.data_HL = U3_Inf.RX_Data[4];
								 Float_Int.byte4.data_HH = U3_Inf.RX_Data[3];  //瞬时流量

								 UnionLCD.UnionD.Steam_Speed = Float_Int.value * 100;  //取整

								 HH_Buffer =  U3_Inf.RX_Data[17] *256 + U3_Inf.RX_Data[18];
								 LL_Buffer =  U3_Inf.RX_Data[19] *256 + U3_Inf.RX_Data[20];

								 UnionLCD.UnionD.HH_Steam = HH_Buffer;
								 UnionLCD.UnionD.LL_Steam = LL_Buffer;
								 
							}


						if(Target_Address == 10) //两路4-20mA扩展模块
							{
									sys_flag.A3B3_ChaYaMokuai_TxRX_Count = 0;
								 
									if(U3_Inf.RX_Data[1] == 0x04 )
										{
										   //读取当前阀的状态
										   
											Value_A = U3_Inf.RX_Data[3] * 256 + U3_Inf.RX_Data[4];
										    Value_B = U3_Inf.RX_Data[5] * 256 + U3_Inf.RX_Data[6];

											if(Value_A >= 819)
												{
													//Value_Buffer  = Value_Buffer - 819;  //4mA  = 819  20mA = 4095
													Value20mA_A = (float)Value_A * 20 /4095 - 4;  //换算成实际的mA值
												}
										   
											if(Value20mA_A > 0 )
												{
													sys_flag.HPWater_Value = Value20mA_A * 1500 / 16;  // 20 = 20mA , 75 = 1500量程/ 16mA
													 
												}
											else
												{
													sys_flag.HPWater_Value = 0;
												}

											if(Value_B >= 819)
												{
													//Value_Buffer  = Value_Buffer - 819;  //4mA  = 819  20mA = 4095
													Value20mA_B = (float)Value_B * 20 /4095 - 4;  //换算成实际的mA值
												}
										   
											if(Value20mA_B > 0 )
												{
													sys_flag.LPWater_Value = Value20mA_B * 1500 / 16;  // 20 = 20mA , 75 = 1500量程/ 16mA
													 
												}
											else
												{
													sys_flag.LPWater_Value = 0;
												}


											
										
											
											
										}
				
							}

						
							

					}
					
			 
				
			//对接收缓冲器清零
				for( i = 0; i < 200;i++ )
					U3_Inf.RX_Data[i] = 0x00;
			
			//重新开启中断
				USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); 
				
			}
}


uint8 Modbus3_UnionTx_Communication(void)
{
	static uint8 Address = 1;
	uint16 check_sum = 0;	

	Sys_Admin.Device_Style = 0;  //仅用于支持相变液位高度检测
	if(Sys_Admin.Device_Style == 0 || Sys_Admin.Device_Style == 1)
		{
			switch(Address)
				{
					case 1:
							U3_Inf.TX_Data[0] = 10; // 地址默认为10
							U3_Inf.TX_Data[1] = 0x04;// 
							
							U3_Inf.TX_Data[2] = 0x00;
							U3_Inf.TX_Data[3] = 0x00;//地址

							U3_Inf.TX_Data[4] = 0x00;
							U3_Inf.TX_Data[5] = 0x02;//读取数据个数
						 
							check_sum  = ModBusCRC16(U3_Inf.TX_Data,8);
							U3_Inf.TX_Data[6]  = check_sum >> 8 ;
							U3_Inf.TX_Data[7]  = check_sum & 0x00FF;
							
							Usart_SendStr_length(USART3,U3_Inf.TX_Data,8);

							sys_flag.A3B3_ChaYaMokuai_TxRX_Count ++;
							sys_flag.Special_100msFlag = 0;

							Address ++;
							break;
					case 2:
							if(sys_flag.Special_100msFlag)
								{
									sys_flag.Special_100msFlag = 0;
									Address = 1;
								}

							break;

					default:
							Address = 1;

							break;
				}
		}

	
		
	
		

		return 0;
}

uint8 Modbus3_UnionRx_DataProcess(uint8 Cmd,uint8 address)
{
		

		return 0;
}




uint8 ModBus3_RTU_Read03(uint8 Target_Address,uint16 Data_Address,uint8 Data_Length )
{
		uint16 Check_Sum = 0;
		U3_Inf.TX_Data[0] = Target_Address;
		U3_Inf.TX_Data[1] = 0x03;

		U3_Inf.TX_Data[2]= Data_Address >> 8;
		U3_Inf.TX_Data[3]= Data_Address & 0x00FF;

		
		U3_Inf.TX_Data[4]= Data_Length >> 8;
		U3_Inf.TX_Data[5]= Data_Length & 0x00FF;

		
		Check_Sum = ModBusCRC16(U3_Inf.TX_Data,8);
		U3_Inf.TX_Data[6]= Check_Sum >> 8;
		U3_Inf.TX_Data[7]= Check_Sum & 0x00FF;
		
		Usart_SendStr_length(USART3,U3_Inf.TX_Data,8);


	return 0;
}

uint8 ModBus3_RTU_Write06(uint8 Target_Address,uint16 Data_Address,uint16 Data16)
{
		uint16 Check_Sum = 0;
		U3_Inf.TX_Data[0] = Target_Address;
		U3_Inf.TX_Data[1] = 0x06;

		U3_Inf.TX_Data[2]= Data_Address >> 8;
		U3_Inf.TX_Data[3]= Data_Address & 0x00FF;

		U3_Inf.TX_Data[4]= Data16 >> 8;
		U3_Inf.TX_Data[5]= Data16 & 0x00FF;

		Check_Sum = ModBusCRC16(U3_Inf.TX_Data,8);
		U3_Inf.TX_Data[6]= Check_Sum >> 8;
		U3_Inf.TX_Data[7]= Check_Sum & 0x00FF;

		Usart_SendStr_length(USART3,U3_Inf.TX_Data,8);

		

		return 0;
}

uint8 ModBus3_RTU_Write10(uint8 Target_Address,uint16 Data_Address)
{
		

		

		return 0;
}


uint8 Union_MuxJiZu_Control_Function(void)
{
	
	

		return 0;
}



