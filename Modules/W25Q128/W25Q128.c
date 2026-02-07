#include "stm32f10x.h"                  // Device header
#include "SPI.h"
#include "W25Q128_Ins.h"



/**
  * 函    数：MPU6050读取ID号
  * 参    数：MID 工厂ID，使用输出参数的形式返回
  * 参    数：DID 设备ID，使用输出参数的形式返回
  * 返 回 值：无
  */
void W25Q128_ReadID(uint8_t *MID, uint16_t *DID)
{
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_JEDEC_ID);			//交换发送读取ID的指令
	*MID = MySPI_SwapByte(W25Q128_DUMMY_BYTE);	//交换接收MID，通过输出参数返回
	*DID = MySPI_SwapByte(W25Q128_DUMMY_BYTE);	//交换接收DID高8位
	*DID <<= 8;									//高8位移到高位
	*DID |= MySPI_SwapByte(W25Q128_DUMMY_BYTE);	//或上交换接收DID的低8位，通过输出参数返回
	MySPI_Stop();								//SPI终止
}

/**
  * 函    数：W25Q128写使能
  * 参    数：无
  * 返 回 值：无
  */
void W25Q128_WriteEnable(void)
{
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_WRITE_ENABLE);		//交换发送写使能的指令
	MySPI_Stop();								//SPI终止
}

/**
  * 函    数：W25Q128等待忙
  * 参    数：无
  * 返 回 值：无
  */
void W25Q128_WaitBusy(void)
{
	uint32_t Timeout;
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_READ_STATUS_REGISTER_1);				//交换发送读状态寄存器1的指令
	Timeout = 5000000;							//给定超时计数时间
	while ((MySPI_SwapByte(W25Q128_DUMMY_BYTE) & 0x01) == 0x01)	//循环等待忙标志位
	{
		Timeout --;								//等待时，计数值自减
		if (Timeout == 0)						//自减到0后，等待超时
		{
			/*超时的错误处理代码，可以添加到此处*/
			break;								//跳出等待，不等了
		}
	}
	MySPI_Stop();								//SPI终止
}

uint8_t W25Q128_ReadSR1(void)
{
    uint8_t sr1;
    MySPI_Start();
    MySPI_SwapByte(W25Q128_READ_STATUS_REGISTER_1);
    sr1 = MySPI_SwapByte(W25Q128_DUMMY_BYTE);
    MySPI_Stop();
    return sr1;
}


uint8_t W25Q128_ReadSR2(void)
{
    uint8_t sr2;
    MySPI_Start();
    MySPI_SwapByte(W25Q128_READ_STATUS_REGISTER_2);
    sr2 = MySPI_SwapByte(W25Q128_DUMMY_BYTE);
    MySPI_Stop();
    return sr2;
}


void W25Q128_WriteSR(uint8_t sr1, uint8_t sr2)
{
    // WRSR(0x01) 写 SR1 + SR2（大多数 W25Q 系列如此）
    W25Q128_WriteEnable();

    MySPI_Start();
    MySPI_SwapByte(W25Q128_WRITE_STATUS_REGISTER);
    MySPI_SwapByte(sr1);
    MySPI_SwapByte(sr2);
    MySPI_Stop();

    W25Q128_WaitBusy();
}
/**
  * 函    数：W25Q128页编程
  * 参    数：Address 页编程的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray	用于写入数据的数组
  * 参    数：Count 要写入数据的数量，范围：0~256
  * 返 回 值：无
  * 注意事项：写入的地址范围不能跨页
  */
void W25Q128_PageProgram(uint32_t Address, uint8_t *DataArray, uint16_t Count)
{
	uint16_t i;
	
	W25Q128_WriteEnable();						//写使能
	
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_PAGE_PROGRAM);		//交换发送页编程的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	for (i = 0; i < Count; i ++)				//循环Count次
	{
		MySPI_SwapByte(DataArray[i]);			//依次在起始地址后写入数据
	}
	MySPI_Stop();								//SPI终止
	
	W25Q128_WaitBusy();							//等待忙
}

/**
  * 函    数：W25Q128扇区擦除（4KB）
  * 参    数：Address 指定扇区的地址，范围：0x000000~0x7FFFFF
  * 返 回 值：无
  */
void W25Q128_SectorErase(uint32_t Address)
{
	W25Q128_WriteEnable();						//写使能
	
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_SECTOR_ERASE_4KB);	//交换发送扇区擦除的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	MySPI_Stop();								//SPI终止
	
	W25Q128_WaitBusy();							//等待忙
}

/**
  * 函    数：W25Q128读取数据
  * 参    数：Address 读取数据的起始地址，范围：0x000000~0x7FFFFF
  * 参    数：DataArray 用于接收读取数据的数组，通过输出参数返回
  * 参    数：Count 要读取数据的数量，范围：0~0x800000
  * 返 回 值：无
  */
void W25Q128_ReadData(uint32_t Address, uint8_t *DataArray, uint32_t Count)
{
	uint32_t i;
	MySPI_Start();								//SPI起始
	MySPI_SwapByte(W25Q128_READ_DATA);			//交换发送读取数据的指令
	MySPI_SwapByte(Address >> 16);				//交换发送地址23~16位
	MySPI_SwapByte(Address >> 8);				//交换发送地址15~8位
	MySPI_SwapByte(Address);					//交换发送地址7~0位
	for (i = 0; i < Count; i ++)				//循环Count次
	{
		DataArray[i] = MySPI_SwapByte(W25Q128_DUMMY_BYTE);	//依次在起始地址后读取数据
	}
	MySPI_Stop();								//SPI终止
}


/**
  * 函    数：W25Q128初始化
  * 参    数：无
  * 返 回 值：无
  */
void W25Q128_Init(void)
{
	uint8_t sr1, sr2;

    MySPI_Init();

	// 注意需要显示解除写保护, 否则会是不是出现错误
    // 解除写保护（清掉 BP 位，避免擦写被拒绝）
    sr1 = W25Q128_ReadSR1();
    sr2 = W25Q128_ReadSR2();

    // SR1: BP0~BP2 通常在 bit2~bit4（0x1C），再保守一点把 0x7C 里的保护相关位清掉
    if (sr1 & 0x7C)
    {
        sr1 &= (uint8_t)~0x7C;
        W25Q128_WriteSR(sr1, sr2);
    }			//先初始化底层的SPI
}
