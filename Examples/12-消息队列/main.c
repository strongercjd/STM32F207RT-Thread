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
 * ��Ϣ��������
 *
 * �������ᴴ��2����̬�̣߳�һ���̻߳����Ϣ��������ȡ��Ϣ��һ���̻߳ᶨʱ����
 * Ϣ���з��� ��ͨ��Ϣ�ͽ�����Ϣ��
 */

/* ��Ϣ���п��ƿ� */
static struct rt_messagequeue mq;
/* ��Ϣ�������õ��ķ�����Ϣ���ڴ�� */
static rt_uint8_t msg_pool[2048];

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
/* �߳�1��ں��� */
static void thread1_entry(void *parameter)
{
    char buf[6];
    rt_uint8_t cnt = 0;
		buf[5] = 0;
    while (1)
    {
        /* ����Ϣ�����н�����Ϣ */
        if (rt_mq_recv(&mq, &buf, 5, RT_WAITING_FOREVER) == RT_EOK)
        {
            printf("thread1: recv msg from msg queue, the content:%s\n", buf);
            if (cnt == 19)
            {
                break;
            }
        }
        /* ��ʱ50ms */
        cnt++;
        rt_thread_mdelay(50);
    }
    printf("thread1: detach mq \n");
    rt_mq_detach(&mq);
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
/* �߳�2��� */
static void thread2_entry(void *parameter)
{
    int result;
    char buf = 'A';    
    rt_uint8_t cnt = 0;
    
    while (1)
    {
        if (cnt == 8)
        {
            /* ���ͽ�����Ϣ����Ϣ������ */
            result = rt_mq_urgent(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                printf("rt_mq_urgent ERR\n");
            }
            else
            {
                printf("thread2: send urgent message - %c\n", buf);
            }
        }
        else if (cnt >= 20)/* ����20����Ϣ֮���˳� */
        {
            printf("message queue stop send, thread2 quit\n");
            break;
        }
        else
        {
            /* ������Ϣ����Ϣ������ */
            result = rt_mq_send(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                printf("rt_mq_send ERR\n");
            }

            printf("thread2: send message - %c\n", buf);
        }
        buf++;
        cnt++;
        /* ��ʱ5ms */
        rt_thread_mdelay(5);
    }
}

/* ��Ϣ����ʾ���ĳ�ʼ�� */
int msgq_sample(void)
{
    rt_err_t result;

    /* ��ʼ����Ϣ���� */
    result = rt_mq_init(&mq,
                        "mqt",
                        &msg_pool[0],               /* �ڴ��ָ��msg_pool */
                        8,                          /* ÿ����Ϣ�Ĵ�С�� 8 �ֽ� */
                        sizeof(msg_pool),           /* �ڴ�صĴ�С��msg_pool�Ĵ�С */
                        RT_IPC_FLAG_FIFO);          /* ����ж���̵߳ȴ������������ȵõ��ķ���������Ϣ */

    if (result != RT_EOK)
    {
        printf("init message queue failed.\n");
        return -1;
    }

    rt_thread_init(&thread1,
                   "thread1",
                   thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 25, 5);
    rt_thread_startup(&thread1);

    rt_thread_init(&thread2,
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 25, 5);
    rt_thread_startup(&thread2);

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
  
  msgq_sample();
  
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
