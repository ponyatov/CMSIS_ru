/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board UART driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

#include "uart-board.h"

UART_HandleTypeDef UartHandle;
uint8_t RxData = 0;

#if UART_DMA_ENABLE
__IO bool UartTxReady = 1;
__IO bool UartRxReady = 1;
static uint8_t tx_buf[UART_LENGTH];
static uint8_t rx_buf[UART_LENGTH];
#define UART_RX_ONE_BYTE 1
uint32_t LogLevel = LOG_LEVEL;
#else
static bool TxBusy = false;
#endif

static void printchar(char **str, int c)
{
	//extern int putchar(int c);
	if (str) {
		**str = c;
		++(*str);
	}
	else (void)putchar(c);
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(char **out, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if (width > 0) {
		register int len = 0;
		register const char *ptr;
		for (ptr = string; *ptr; ++ptr) ++len;
		if (len >= width) width = 0;
		else width -= len;
		if (pad & PAD_ZERO) padchar = '0';
	}
	if (!(pad & PAD_RIGHT)) {
		for ( ; width > 0; --width) {
			printchar (out, padchar);
			++pc;
		}
	}
	for ( ; *string ; ++string) {
		printchar (out, *string);
		++pc;
	}
	for ( ; width > 0; --width) {
		printchar (out, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

static int printi(char **out, int i, int b, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *s;
	register int t, neg = 0, pc = 0;
	register unsigned int u = i;

	if (i == 0) {
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints (out, print_buf, width, pad);
	}

	if (sg && b == 10 && i < 0) {
		neg = 1;
		u = -i;
	}

	s = print_buf + PRINT_BUF_LEN-1;
	*s = '\0';

	while (u) {
		t = u % b;
		if( t >= 10 )
			t += letbase - '0' - 10;
		*--s = t + '0';
		u /= b;
	}

	if (neg) {
		if( width && (pad & PAD_ZERO) ) {
			printchar (out, '-');
			++pc;
			--width;
		}
		else {
			*--s = '-';
		}
	}

	return pc + prints (out, s, width, pad);
}

static int print(char **out, int *varg)
{
	register int width, pad;
	register int pc = 0;
	register char *format = (char *)(*varg++);
	char scr[2];

	for (; *format != 0; ++format) {
		if (*format == '%') {
			++format;
			width = pad = 0;
			if (*format == '\0') break;
			if (*format == '%') goto out;
			if (*format == '-') {
				++format;
				pad = PAD_RIGHT;
			}
			while (*format == '0') {
				++format;
				pad |= PAD_ZERO;
			}
			for ( ; *format >= '0' && *format <= '9'; ++format) {
				width *= 10;
				width += *format - '0';
			}
			if( *format == 's' ) {
				register char *s = *((char **)varg++);
				pc += prints (out, s?s:"(null)", width, pad);
				continue;
			}
			if( *format == 'd' ) {
				pc += printi (out, *varg++, 10, 1, width, pad, 'a');
				continue;
			}
			if( *format == 'x' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'X' ) {
				pc += printi (out, *varg++, 16, 0, width, pad, 'A');
				continue;
			}
			if( *format == 'u' ) {
				pc += printi (out, *varg++, 10, 0, width, pad, 'a');
				continue;
			}
			if( *format == 'c' ) {
				/* char are converted to int then pushed on the stack */
				scr[0] = *varg++;
				scr[1] = '\0';
				pc += prints (out, scr, width, pad);
				continue;
			}
		}
		else {
		out:
			printchar (out, *format);
			++pc;
		}
	}
	if (out) **out = '\0';
	return pc;
}

uint8_t CheckLogLevel( const char *in )
{
    // Not to show "--> ..." logs when log level is INFO
    if (LogLevel == INFO && (*(in+0) == 0x2D)) {
        if ((*(in+1) == 0x2D) && (*(in+2) == 0x3E))
            return 1;
    }
    return 0;
}

/*!
 * If last byte of a string is not "NL", add "NL"
 */
static void append_new_line(char* out)
{
    size_t len;
    len = strlen(out);
    if (len != 0 && (out[len-1] != 0xA)) {
        strcat(out, "\n");
    }
}

static uint8_t out1[UART_LENGTH];
static uint32_t value1;
static uint8_t **pout1 = (uint8_t**)&value1;
void UartPrintLF(const char *format, ...)
{
#if UART_ENABLE
    //uint8_t out[UART_LENGTH];
    //uint8_t **pout = (uint8_t**)malloc(sizeof(uint8_t*));
    *pout1 = &out1[0];

    memset(out1, 0x00, UART_LENGTH);
    register int *varg = (int *)(&format);
    //Copy strings into character array
    print((char**)pout1, varg);
    //Add a LF char at the end of "out" if no LF
    append_new_line((char*)out1);
#if UART_DMA_ENABLE
    //Store into UART Tx Buffer
    UartPutBuffer(&Uart, out1, strlen((const char*)out1));
    UartTxDma();
#else
    //Store into UART Tx Buffer
    UartPutBuffer(&Uart, out, strlen((const char*)out));
    //Pop Tx Bufer and trigger UART Tx Interrupt
    HAL_UART_TxCpltCallback(&UartHandle);
#endif
    //free(pout);
#else
    UNUSED(format);
#endif
}

static uint8_t out2[UART_LENGTH];
static uint32_t value2;
static uint8_t **pout2 = (uint8_t**)&value2;
void UartPrint(const char *format, ...)
{
#if UART_ENABLE
    uint32_t prim;
    if(CheckLogLevel(format))
        return;

    //uint8_t out[UART_LENGTH];
    //uint8_t **pout = (uint8_t**)malloc(sizeof(uint8_t*));
    *pout2 = &out2[0];

    memset(out2, 0x00, UART_LENGTH);
    register int *varg = (int *)(&format);
    //Copy strings into character array
    print((char**)pout2, varg);
#if UART_DMA_ENABLE
    prim = __get_PRIMASK();
    __disable_irq( );
    //Store into UART Tx Buffer
    UartPutBuffer(&Uart, out2, strlen((const char*)out2));
    if(!prim)
        __enable_irq( );
    UartTxDma();
#else
    //Store into UART Tx Buffer
    UartPutBuffer(&Uart, out, strlen((const char*)out));
    //Pop Tx Bufer and trigger UART Tx Interrupt
    HAL_UART_TxCpltCallback(&UartHandle);
#endif
    //free(pout);
#else
    UNUSED(format);
#endif
}

uint32_t UartScan(uint8_t *buffer)
{
#if UART_ENABLE
    uint32_t readBytes = 0;
    //Clear UART buffer for Rx
    memset(buffer, 0x00, UART_LENGTH);
    //Read n Bytes from UART Rx Fifo
    UartGetBuffer(&Uart, buffer, UART_LENGTH, (uint16_t*)&readBytes);
    return readBytes;
#else
    UNUSED(buffer);
#endif
}

void UartMcuInit( Uart_t *obj, uint8_t uartId, PinNames tx, PinNames rx )
{
    obj->UartId = uartId;

#if defined(STM32F072xB)
    __HAL_RCC_USART2_FORCE_RESET( );
    __HAL_RCC_USART2_RELEASE_RESET( );
    __HAL_RCC_USART2_CLK_ENABLE( );

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF1_USART2 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF1_USART2 );
#elif defined(STM32F401xC)
    __HAL_RCC_USART2_FORCE_RESET( );
    __HAL_RCC_USART2_RELEASE_RESET( );
    __HAL_RCC_USART2_CLK_ENABLE( );

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART2 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF7_USART2 );
#elif defined(STM32L073xx)
    __HAL_RCC_USART1_FORCE_RESET( );
    __HAL_RCC_USART1_RELEASE_RESET( );
    __HAL_RCC_USART1_CLK_ENABLE( );

#if UART_DMA_ENABLE
    __HAL_RCC_DMA1_CLK_ENABLE();
#endif

    GpioInit( &obj->Tx, tx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF4_USART1 );
    GpioInit( &obj->Rx, rx, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF4_USART1 );
#endif
}

void UartMcuConfig( Uart_t *obj, UartMode_t mode, uint32_t baudrate, WordLength_t wordLength, StopBits_t stopBits, Parity_t parity, FlowCtrl_t flowCtrl )
{
#if defined(STM32L073xx)
    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;
#endif

#if defined(STM32F072xB) || defined(STM32F401xC)
    UartHandle.Instance = USART2;
#elif defined(STM32L073xx)
    UartHandle.Instance = USART1;
#endif
    UartHandle.Init.BaudRate = baudrate;

    if( mode == TX_ONLY )
    {
        if( obj->FifoTx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_TX;
    }
    else if( mode == RX_ONLY )
    {
        if( obj->FifoRx.Data == NULL )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_RX;
    }
    else if( mode == RX_TX )
    {
        if( ( obj->FifoTx.Data == NULL ) || ( obj->FifoRx.Data == NULL ) )
        {
            assert_param( FAIL );
        }
        UartHandle.Init.Mode = UART_MODE_TX_RX;
    }
    else
    {
       assert_param( FAIL );
    }

    if( wordLength == UART_8_BIT )
    {
        UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
    }
    else if( wordLength == UART_9_BIT )
    {
        UartHandle.Init.WordLength = UART_WORDLENGTH_9B;
    }

    switch( stopBits )
    {
    case UART_2_STOP_BIT:
        UartHandle.Init.StopBits = UART_STOPBITS_2;
        break;
    case UART_1_STOP_BIT:
    default:
        UartHandle.Init.StopBits = UART_STOPBITS_1;
        break;
    }

    if( parity == NO_PARITY )
    {
        UartHandle.Init.Parity = UART_PARITY_NONE;
    }
    else if( parity == EVEN_PARITY )
    {
        UartHandle.Init.Parity = UART_PARITY_EVEN;
    }
    else
    {
        UartHandle.Init.Parity = UART_PARITY_ODD;
    }

    if( flowCtrl == NO_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    }
    else if( flowCtrl == RTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS;
    }
    else if( flowCtrl == CTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_CTS;
    }
    else if( flowCtrl == RTS_CTS_FLOW_CTRL )
    {
        UartHandle.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS;
    }

    UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if( HAL_UART_Init( &UartHandle ) != HAL_OK )
    {
        while( 1 );
    }

#if UART_DMA_ENABLE
    /* Configure the DMA handler for reception process */
    hdma_rx.Instance                 = USARTx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Init.Request             = DMA_REQUEST_3;

    HAL_DMA_Init(&hdma_rx);

    /* Associate the initialized DMA handle to the the UART handle */
    __HAL_LINKDMA(&UartHandle, hdmarx, hdma_rx);

    /*###### Configure the DMA ##################################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx.Instance                 = USARTx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.Request             = DMA_REQUEST_3;

    HAL_DMA_Init(&hdma_tx);

    /* Associate the initialized DMA handle to the UART handle */
    __HAL_LINKDMA(&UartHandle, hdmatx, hdma_tx);

    /*###### Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (USART1_TX) */
    HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);

    /* NVIC configuration for DMA transfer complete interrupt (USART1_RX) */
    HAL_NVIC_SetPriority(DMA1_Channel4_5_6_7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_5_6_7_IRQn);
#endif

#if defined(STM32F072xB) || defined(STM32F401xC)
    HAL_NVIC_SetPriority( USART2_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( USART2_IRQn );
#elif defined(STM32L073xx)
    HAL_NVIC_SetPriority( USART1_IRQn, 0, 0 );
    HAL_NVIC_EnableIRQ( USART1_IRQn );
#endif

    /* Enable the UART Data Register not empty Interrupt */
#if UART_DMA_ENABLE
    __HAL_UART_FLUSH_DRREGISTER(&UartHandle);
    memset(rx_buf, 0x00, UART_LENGTH);
#if UART_RX_ONE_BYTE
    HAL_UART_Receive_DMA( &UartHandle, rx_buf, 1 );
#else
    HAL_UART_Receive_DMA( &UartHandle, rx_buf, UART_LENGTH );
#endif //UART_RX_ONE_BYTE
#else
    HAL_UART_Receive_IT( &UartHandle, &RxData, 1 );
#endif //UART_DMA_ENABLE
}

void UartMcuDeInit( Uart_t *obj )
{
#if defined(STM32F072xB) || defined(STM32F401xC)
    __HAL_RCC_USART2_FORCE_RESET( );
    __HAL_RCC_USART2_RELEASE_RESET( );
    __HAL_RCC_USART2_CLK_DISABLE( );
#elif defined(STM32L073xx)
    __HAL_RCC_USART1_FORCE_RESET( );
    __HAL_RCC_USART1_RELEASE_RESET( );
    __HAL_RCC_USART1_CLK_DISABLE( );
#endif

    GpioInit( &obj->Tx, obj->Tx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Rx, obj->Rx.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t UartMcuPutChar( Uart_t *obj, uint8_t data )
{
#if UART_DMA_ENABLE
    //uint32_t prim;
    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        //prim = __get_PRIMASK();
        //__disable_irq( );
        FifoPush( &obj->FifoTx, data );
        //if(!prim)
        //    __enable_irq( );

        return 0; // OK
    }
    return 1; // Busy
#else
    if( IsFifoFull( &obj->FifoTx ) == false )
    {
        if ( TxBusy == true ) {
            __disable_irq( );
            FifoPush( &obj->FifoTx, data );
            __enable_irq( );
            // Enable the USART Transmit interrupt
            //AcSiP(-), for fixing UART Tx in driver 1.4
            //__HAL_UART_ENABLE_IT( &UartHandle, USART_IT_TXE );
        }
        else {
            TxBusy = true;
            // Enable the USART Transmit interrupt
            HAL_UART_Transmit_IT(&UartHandle, &data, 1 );
        }
        return 0; // OK
    }
    return 1; // Busy
#endif
}

uint8_t UartMcuGetChar( Uart_t *obj, uint8_t *data )
{
    uint32_t prim;
    if( IsFifoEmpty( &obj->FifoRx ) == false )
    {
        prim = __get_PRIMASK();
        __disable_irq( );
        *data = FifoPop( &obj->FifoRx );
        if(!prim)
            __enable_irq( );
        return 0;
    }
    return 1;
}

void HAL_UART_TxCpltCallback( UART_HandleTypeDef *UartHandle )
{
#if UART_DMA_ENABLE
    UartTxReady = 1;
    //Send UART Tx Data if Tx Fifo has data.
    UartTxDma();
#else
    uint8_t data;

    if( IsFifoEmpty( &Uart.FifoTx ) == false )
    {
        data = FifoPop( &Uart.FifoTx );
        //  Write one byte to the transmit data register
        HAL_UART_Transmit_IT( UartHandle, &data, 1 );
    }
    else
    {
        // Disable the USART Transmit interrupt
        //AcSiP(-), for keeping UART Tx
        //HAL_NVIC_DisableIRQ( USART2_IRQn );
        TxBusy = false;
    }
#endif
    if( Uart.IrqNotify != NULL )
    {
        Uart.IrqNotify( UART_NOTIFY_TX );
    }
}

void HAL_UART_RxCpltCallback( UART_HandleTypeDef *UartHandle )
{
#if UART_DMA_ENABLE
    UartRxReady = 1;
    //Receive UART Rx Data if Rx Fifo has data.
    UartRxDma();
#else
    if( IsFifoFull( &Uart.FifoRx ) == false )
    {
        // Read one byte from the receive data register
        FifoPush( &Uart.FifoRx, RxData );
    }
    if( Uart.IrqNotify != NULL )
    {
        Uart.IrqNotify( UART_NOTIFY_RX );
    }

    __HAL_UART_FLUSH_DRREGISTER( UartHandle );
    HAL_UART_Receive_IT( UartHandle, &RxData, 1 );
#endif
}

void HAL_UART_ErrorCallback( UART_HandleTypeDef *UartHandle )
{
#if UART_DMA_ENABLE
    __HAL_UART_CLEAR_OREFLAG(UartHandle);
    __HAL_UART_CLEAR_NEFLAG(UartHandle);
    __HAL_UART_CLEAR_FEFLAG(UartHandle);
#endif
}

#if defined(STM32F072xB) || defined(STM32F401xC)
void USART2_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UartHandle );
}
#elif defined(STM32L073xx)
void USART1_IRQHandler( void )
{
    HAL_UART_IRQHandler( &UartHandle );
}

#if UART_DMA_ENABLE
uint8_t UartSendDma( void )
{
    uint8_t data = 0;
    uint32_t len = 0;
    uint32_t prim = 0;

    if (UartTxReady == 1) {
        //Clear local memory Tx buffer
        memset(tx_buf, 0x00, UART_LENGTH);

        prim = __get_PRIMASK();
        __disable_irq( );
        //Fetch data from tx fifo to local memory
        while ( IsFifoEmpty( &Uart.FifoTx ) == false ) {
            data = FifoPop( &Uart.FifoTx );
            tx_buf[len++] = data;
        }
        if(!prim)
            __enable_irq( );
        //Launch UART Tx DMA if data is ready to be sent.
        if (len > 0) {
            //if( HAL_UART_Transmit_DMA(&UartHandle, aTxBuffer, TXBUFFERSIZE) != HAL_OK )
            if( HAL_UART_Transmit_DMA(&UartHandle, tx_buf, len) != HAL_OK )
                return 1;
            //Uart Tx DMA launched, Set flag as "busy"
            UartTxReady = 0;
        }
    }
    return 0;
}

uint8_t UartReceiveDma( void )
{
    uint32_t len = 0;
    uint32_t prim = 0;

    if ( UartRxReady == 1 ) {
#if UART_RX_ONE_BYTE
        prim = __get_PRIMASK();
        __disable_irq( );
        //Save one byte data from rx buffer into rx fifo
        if ( IsFifoFull( &Uart.FifoRx ) == false ) {
            //Check if needs to stop fifo pushing if 0x00 meets
            if ( rx_buf[len] != 0x00 ) {
                FifoPush( &Uart.FifoRx, rx_buf[len] );
                ++len;
            }
        }

        if(!prim)
            __enable_irq( );

        if ( HAL_UART_Receive_DMA(&UartHandle, rx_buf, 1 ) != HAL_OK )
            return 1;
#else
        prim = __get_PRIMASK();
        __disable_irq( );
        //Save data from rx buffer into rx fifo
        while ( IsFifoFull( &Uart.FifoRx ) == false ) {
            //Check if needs to stop fifo pushing if 0x00 meets
            if ( rx_buf[len] == 0x00 )
                break;

            FifoPush( &Uart.FifoRx, rx_buf[len] );
            ++len;
        }
        //Clear local memory Rx buffer
        memset(rx_buf, 0x00, UART_LENGTH);

        if(!prim)
            __enable_irq( );

        if ( HAL_UART_Receive_DMA(&UartHandle, rx_buf, UART_LENGTH ) != HAL_OK )
            return 1;
#endif
    }
    return 0;
}

void DMA1_Channel4_5_6_7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(UartHandle.hdmatx);
    HAL_DMA_IRQHandler(UartHandle.hdmarx);
}
#endif

#endif
