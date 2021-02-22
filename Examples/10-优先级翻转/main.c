/**
  ******************************************************************************
  * @file    Project/STM32F2xx_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    13-April-2012
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <rtthread.h>

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

void UART_Init(void);

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/*
 * ������ʹ������
 *
 * ������ӽ����� 3 ����̬�߳��Լ����л�����ʱ�����е��߳����ȼ��Ƿ�
 * ���������ȴ��߳����ȼ��е�������ȼ���
 *
 * �߳� 1��2��3 �����ȼ��Ӹߵ��ͷֱ𱻴�����
 * �߳� 3 �ȳ��л������������߳� 2 ��ͼ���л���������ʱ�߳� 3 �����ȼ�Ӧ��
 * ������Ϊ���߳� 2 �����ȼ���ͬ���߳� 1 ���ڼ���߳� 3 �����ȼ��Ƿ�����
 * Ϊ���߳� 2�����ȼ���ͬ��
 */

/* ָ���߳̿��ƿ��ָ�� */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;


#define THREAD_PRIORITY       10
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* �߳� 1 ��� */
static void thread1_entry(void *parameter)
{
	while(1){
    /* ���õ����ȼ��߳����� */
    rt_thread_mdelay(200);

    /* ��ʱ thread3 ���� mutex������ thread2 �ȴ����� mutex */

    /* ��� printf("the producer generates a number: %d\n", array[set%MAXSEM]); �� thread3 �����ȼ���� */
    if (tid2->current_priority != tid3->current_priority)
    {
        /* ���ȼ�����ͬ������ʧ�� */
        printf("the priority of thread2 is: %d\n", tid2->current_priority);
        printf("the priority of thread3 is: %d\n", tid3->current_priority);
        printf("test failed.\n");
        
    }
    else
    {
        printf("the priority of thread2 is: %d\n", tid2->current_priority);
        printf("the priority of thread3 is: %d\n", tid3->current_priority);
        printf("test OK.\n");
    }
	}
}

/* �߳� 2 ��� */
static void thread2_entry(void *parameter)
{
    rt_err_t result;

	  /* ���õ����ȼ��߳����� */
    rt_thread_mdelay(100);
	
    printf("the priority of thread2 is: %d\n", tid2->current_priority);




    /*
     * ��ͼ���л���������ʱ thread3 ���У�Ӧ�� thread3 �����ȼ�����
     * �� thread2 ��ͬ�����ȼ�
     */
    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);

    if (result == RT_EOK)
    {
        /* �ͷŻ����� */
        rt_mutex_release(mutex);
    }
}

/* �߳� 3 ��� */
static void thread3_entry(void *parameter)
{
    rt_tick_t tick;
    rt_err_t result;

    printf("the priority of thread3 is: %d\n", tid3->current_priority);

    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        printf("thread3 take a mutex, failed.\n");
    }

    /* ��һ����ʱ���ѭ����500ms */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) ;

    rt_mutex_release(mutex);
		while(1){
			rt_thread_mdelay(50);
	}
}

int pri_inversion(void)
{
    /* ���������� */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        printf("create dynamic mutex failed.\n");
        return -1;
    }

    /* �����߳� 1 */
    tid1 = rt_thread_create("thread1",
                            thread1_entry, 
                            RT_NULL,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
         rt_thread_startup(tid1);
 
    /* �����߳� 2 */
    tid2 = rt_thread_create("thread2",
                            thread2_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    /* �����߳� 3 */
    tid3 = rt_thread_create("thread3",
                            thread3_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
        rt_thread_startup(tid3);

    return 0;
}

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  GPIO_InitTypeDef  GPIO_Init_s;
  
  /* ʹ��GPIOE�˿�ʱ�� */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  
  /* ����LED�ܽ� */
  GPIO_Init_s.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init_s.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init_s.GPIO_OType = GPIO_OType_PP;
  GPIO_Init_s.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init_s.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_Init_s);
  
  GPIO_SetBits(GPIOE,GPIO_Pin_4);//Ϩ��LED��
  
  UART_Init();
  
  printf("\r\n======================================================================");
  printf("\r\n=               (C) COPYRIGHT 2021                                   =");
  printf("\r\n=                                                                    =");
  printf("\r\n=                ST207 RTT thread                                    =");
  printf("\r\n=                                                                    =");
  printf("\r\n=                                           By Firefly               =");
  printf("\r\n======================================================================");
  printf("\r\n\r\n");
  
  pri_inversion();
  
  while (1)
  {
    GPIO_SetBits(GPIOE,GPIO_Pin_4);  //Ϩ��LED��
    rt_thread_mdelay(500);
    GPIO_ResetBits(GPIOE,GPIO_Pin_4);//����LED��
    rt_thread_mdelay(500);//��ʱ500ms
  }
}
void UART_Init(void)
{
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  /* Enable UART1 clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  /* Connect PXx to USARTx_Tx*/
  GPIO_PinAFConfig(GPIOA, 9, GPIO_AF_USART1);
  
  /* Connect PXx to USARTx_Rx*/
  GPIO_PinAFConfig(GPIOA, 10, GPIO_AF_USART1);
  
  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  
  /* USART configuration */
  USART_Init(USART1, &USART_InitStructure);
  
  /* Enable USART */
  USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  USART_SendData(USART1, (uint8_t) ch);

  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}

  return ch;
}





/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
