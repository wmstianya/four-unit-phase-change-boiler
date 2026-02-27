/**
  ******************************************************************************
  * @file    usr_c210.c
  * @author  heic
  * @version V1.0
  * @date    2017-xx-xx
  * @brief   usr_c210 WIFI模组 接口定义及应用函数
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
#include "main.h"

extern uint16 USART2_RX_STA;




 WWW_DATA Talk;






 ///////////////////////////////////////服务器命令解析////////////////////////////////// 	 
 //功能：对服务器发来的数据进行分析
 //输入参数
 //输出参数
 //无
 //////////////////////////////////////////////////////////////////////////////////////////////    
 void Server_Cmd_Analyse(void)
 {
 	
 }

 
 //MCU定时往WIFI发送数据


 uint8  New_Server_Cmd_Analyse(void)
 {
 	


	return 0 ;
 }

 void Mcu_mail_Wifi(void)
 {
 	uint16 checksum = 0;

	
	
	
 
 
 }


uint8 ModuBusRTU_WriteResponse(uint16 address,uint16 Data16)
{
	uint16 check_sum = 0;
	uint8 Device_ID = 1;

		if(U1_Inf.RX_Data[0] > 1)
		{
			//当查询地址大于1时，自动切换为对方询问的地址
			Device_ID = Sys_Admin.ModBus_Address;
		}
	U1_Inf.TX_Data[0] = Device_ID;
	 
	U1_Inf.TX_Data[1]= 0x06;

	
	U1_Inf.TX_Data[2] = address >> 8;    // 地址高字节
	U1_Inf.TX_Data[3] = address & 0x00FF;  //地址低字节
	

	U1_Inf.TX_Data[4] = Data16 >> 8;  //数据高字节
	U1_Inf.TX_Data[5] = Data16 & 0x00FF;   //数据低字节

	check_sum  = ModBusCRC16(U1_Inf.TX_Data,8);   //这个根据总字节数改变
	U1_Inf.TX_Data[6]  = check_sum >> 8 ;
	U1_Inf.TX_Data[7]  = check_sum & 0x00FF;

	Usart_SendStr_length(USART1,U1_Inf.TX_Data,8);

	return 0;
}



uint8 ModuBusRTU_Write0x10Response(uint16 address,uint16 Data16)
{
	uint16 check_sum = 0;
	uint8 Device_ID = 1;

	if(U1_Inf.RX_Data[0] >= 1)
		{
			//当查询地址大于1时，自动切换为对方询问的地址
			Device_ID = Sys_Admin.ModBus_Address;
		}

	U1_Inf.TX_Data[0] = Device_ID; 
	U1_Inf.TX_Data[1]= 0x10;
	
	U1_Inf.TX_Data[2] = address >> 8;    // 地址高字节
	U1_Inf.TX_Data[3] = address & 0x00FF;  //地址低字节

	U1_Inf.TX_Data[4] = Data16 >> 8;  // 
	U1_Inf.TX_Data[5] = Data16 & 0X00FF;    

	check_sum  = ModBusCRC16(U1_Inf.TX_Data,8);   //这个根据总字节数改变
	U1_Inf.TX_Data[6]  = check_sum >> 8 ;
	U1_Inf.TX_Data[7]  = check_sum & 0x00FF;

	Usart_SendStr_length(USART1,U1_Inf.TX_Data,8);


	

		return 0;
}



uint8 Boss_Lock_Function(void)
 	{
		
			
 			return 0;
 	}



void NewSHUANG_PIN_Server_Cmd_Analyse(void)
{
   
   
  
}


uint32 Char_to_Int_6(uint32 number_H,uint32 number_L) 
{
    uint8 Code[6] ={0};
	uint32 Return_Value = 0;



	Code[0] = number_L & 0x000000FF - 0x30;
	number_L  = number_L >> 8;
	Code[1] = number_L & 0x000000FF - 0x30;
	 
	Code[2] = number_H & 0x000000FF - 0x30;
	number_H  = number_H >> 8;
	Code[3] = number_H & 0x000000FF - 0x30;
	number_H  = number_H >> 8;
	Code[4] = number_H & 0x000000FF - 0x30;
	number_H  = number_H >> 8;
	Code[5] = number_H & 0x000000FF - 0x30;
	
	 Return_Value =Code[0] + Code[1]* 10 + Code[2]* 100 + Code[3]* 1000 + Code[4]* 10000 + Code[5]* 100000;
	
	
	
    return Return_Value; // 返回转换后的BCD码
}






