/*
Copyright (c) 2020 SparkFun Electronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#if DEVICE_SPI

#include "spi_api.h"
#include "iom_api.h"
#include "PeripheralPins.h"
#include "mbed_assert.h"

#include <string.h>

#define DEFAULT_CLK_FREQ (4000000)
#define DEFAULT_SPI_MODE (AM_HAL_IOM_SPI_MODE_0)

#define standin_fn() printf("stand in for '%s', file: '%s', line: %d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__)

//static am_hal_iom_transfer_t xfer = {0};
static void* IOM0_HANDLE=NULL;
static void* IOM1_HANDLE=NULL;
static void* IOM2_HANDLE=NULL;
static void* IOM3_HANDLE=NULL;
static void* IOM4_HANDLE=NULL;
static void* IOM5_HANDLE=NULL;
#define IOM_PERIPHERAL_COUNT 4 //should be 6, but last 2 will never be used in our application
#define IOM_CQ_DEPTH 1 //only need 1 deep, might need 2
#define IOM_CQ_STRUCT_WORD_SIZE AM_HAL_IOM_CQ_ENTRY_SIZE // thought it was 22, may actually be 24
static uint32_t IOM_CQ_BUF[IOM_PERIPHERAL_COUNT][(IOM_CQ_STRUCT_WORD_SIZE) * IOM_CQ_DEPTH];
void xfer_complete_callback(void *pCallbackCtxt, uint32_t transactionStatus);

#define am_iomaster0_isr                                                          \
    am_iom_isr1(0)
    
#define am_iomaster1_isr                                                          \
    am_iom_isr1(1)
    
#define am_iomaster2_isr                                                          \
    am_iom_isr1(2)
    
#define am_iomaster3_isr                                                          \
    am_iom_isr1(3)
    
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr


//void am_iomaster0_isr(void);
//void am_iomaster1_isr(void);
//void am_iomaster2_isr(void);
//void am_iomaster3_isr(void);
//void am_iomaster4_isr(void);
//void am_iomaster5_isr(void);

void am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM0_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM0_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM0_HANDLE, ui32Status);
        }
    }
}

void am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM1_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM1_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM1_HANDLE, ui32Status);
        }
    }
}

void am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM2_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM2_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM2_HANDLE, ui32Status);
        }
    }
}

void am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM3_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM3_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM3_HANDLE, ui32Status);
        }
    }
}


/*
void am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM4_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM4_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM4_HANDLE, ui32Status);
        }
    }
}

void am_iomaster5_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(IOM5_HANDLE, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(IOM5_HANDLE, ui32Status);
            am_hal_iom_interrupt_service(IOM5_HANDLE, ui32Status);
        }
    }
}
*/
void xfer_complete_callback(void *pCallbackCtxt, uint32_t transactionStatus){
    //txn_stat = transactionStatus;
    //ISR_COUNT_OCCURANCES++;
}

SPIName spi_get_peripheral_name(PinName mosi, PinName miso, PinName sclk){
    uint32_t iom_mosi = pinmap_peripheral(mosi, spi_master_mosi_pinmap());
    uint32_t iom_miso = pinmap_peripheral(miso, spi_master_miso_pinmap());
    uint32_t iom_sclk = pinmap_peripheral(sclk, spi_master_clk_pinmap());

    uint32_t iom;

    if (miso == NC) {
        iom = pinmap_merge(iom_mosi, iom_sclk);
    } else if (mosi == NC) {
        iom = pinmap_merge(iom_miso, iom_sclk);
    } else {
        uint32_t iom_data = pinmap_merge(iom_mosi, iom_miso);
        iom = pinmap_merge(iom_data, iom_sclk);
    }

    if((int)iom == NC){
        return IOM_NUM;
    }

    return (SPIName)iom;
}

void spi_get_capabilities(PinName ssel, bool slave, spi_capabilities_t *cap){
    MBED_ASSERT(cap);

    SPIName iom_ssel = (SPIName)pinmap_peripheral(ssel, spi_master_cs_pinmap());

    cap->minimum_frequency = 0;
    cap->maximum_frequency = AM_HAL_IOM_MAX_FREQ;
    cap->word_length = 0x00000080;
    cap->slave_delay_between_symbols_ns = 0;
    cap->clk_modes = 0x0F;
    cap->support_slave_mode = (iom_ssel == IOM_ANY) ? true : false; 
    cap->hw_cs_handle = false;
    cap->async_mode = false;
    cap->tx_rx_buffers_equal_length = false;
}

void spi_init(spi_t *obj, PinName mosi, PinName miso, PinName sclk, PinName ssel){
    MBED_ASSERT(obj);

    MBED_ASSERT((int)ssel == NC);

    // iom determination
    SPIName iom = spi_get_peripheral_name(mosi, miso, sclk);
    SPIName iom_ssel = (SPIName)pinmap_peripheral(ssel, spi_master_cs_pinmap());
    MBED_ASSERT((int)iom != IOM_NUM);
    MBED_ASSERT((int)iom != IOM_ANY);

    // iom configuration
    obj->spi.iom_obj.iom.inst = (uint32_t)iom;
    obj->spi.iom_obj.iom.cfg.eInterfaceMode = AM_HAL_IOM_SPI_MODE;
    obj->spi.iom_obj.iom.cfg.ui32ClockFreq = AM_HAL_IOM_1MHZ;//AM_HAL_IOM_48MHZ;//DEFAULT_CLK_FREQ;
    obj->spi.iom_obj.iom.cfg.eSpiMode = DEFAULT_SPI_MODE;
    obj->spi.iom_obj.iom.cfg.pNBTxnBuf =&(IOM_CQ_BUF[iom][0]);
    obj->spi.iom_obj.iom.cfg.ui32NBTxnBufLength = IOM_CQ_STRUCT_WORD_SIZE * IOM_CQ_DEPTH;


    // pin configuration
    pinmap_config(sclk, spi_master_clk_pinmap());
    if((int)mosi != NC){ pinmap_config(mosi, spi_master_mosi_pinmap()); }
    if((int)miso != NC){ pinmap_config(miso, spi_master_miso_pinmap()); }
    if((int)ssel != NC){ pinmap_config(ssel, spi_master_cs_pinmap()); }

    // initialization
    iom_init(&obj->spi.iom_obj);
    //NEED TO SET UP THE GLOBAL HANDLES HERE:
    switch (obj->spi.iom_obj.iom.inst)
    	{
    	case 0:
    		IOM0_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	case 1:
    		IOM1_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	case 2:
    		IOM2_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	case 3:
    		IOM3_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	case 4:
    		IOM4_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	case 5:
    		IOM5_HANDLE=obj->spi.iom_obj.iom.handle;
    	break;
    	
    	default:
    	break;
    	
    	}
    
	//am_hal_iom_interrupt_enable(obj->spi.iom_obj.iom.handle, 0xFFFD);    
    
}

void spi_free(spi_t *obj){
    iom_deinit(&obj->spi.iom_obj);
}

void spi_format(spi_t *obj, int bits, int mode, int slave){
    MBED_ASSERT(obj);
    switch(mode){
        case 0:
            obj->spi.iom_obj.iom.cfg.eSpiMode = AM_HAL_IOM_SPI_MODE_0;
            break;
        case 1:
            obj->spi.iom_obj.iom.cfg.eSpiMode = AM_HAL_IOM_SPI_MODE_1;
            break;
        case 2:
            obj->spi.iom_obj.iom.cfg.eSpiMode = AM_HAL_IOM_SPI_MODE_2;
            break;
        case 3:
            obj->spi.iom_obj.iom.cfg.eSpiMode = AM_HAL_IOM_SPI_MODE_3;
            break;

    }
    iom_init(&obj->spi.iom_obj);
}

void spi_frequency(spi_t *obj, int hz) {
    MBED_ASSERT(obj);
    obj->spi.iom_obj.iom.cfg.ui32ClockFreq = (uint32_t)hz;
    iom_init(&obj->spi.iom_obj);
}

int spi_master_write(spi_t *obj, int value) {
    uint32_t rxval = 0;
    spi_master_block_write(obj, (const char *)&value, 1, (char*)&rxval, 1, 0x00);
    return rxval;
}
//---------------------------------------------------------------------------------------------------------------------------------
int spi_master_non_block_read(spi_t *obj, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, char write_fill)
{
    MBED_ASSERT(obj);

	am_hal_iom_transfer_t my_xfer = {0};
    	int chars_handled = 0;
  
   my_xfer.ui8RepeatCount  = 0;
    my_xfer.ui32PauseCondition = 0;
    my_xfer.ui32StatusSetClr = 0;
    my_xfer.ui8Priority     = 1;  

    my_xfer.eDirection = AM_HAL_IOM_RX;
    my_xfer.ui32NumBytes =tx_length;
    //xfer.uPeerInfo.ui32SpiChipSelect = 0;
    my_xfer.pui32RxBuffer = (uint32_t *)rx_buffer;
    my_xfer.pui32TxBuffer = NULL;
    my_xfer.bContinue=false;
    
    my_xfer.ui32InstrLen    = 0;
    my_xfer.ui32Instr       = 0;

	

    if(my_xfer.ui32NumBytes){
    	
    	//turn off full-duplex, because xfer.eDirection = AM_HAL_IOM_TX; doesn't seem to work..  why???
    	IOMn(obj->spi.iom_obj.iom.inst)->MSPICFG &= 0xFFFFFFFB;
        uint32_t status = am_hal_iom_nonblocking_transfer(obj->spi.iom_obj.iom.handle, &my_xfer, xfer_complete_callback, NULL);
 
        if(AM_HAL_STATUS_SUCCESS != status)
        	{
            	return 0;
        	}
        chars_handled += my_xfer.ui32NumBytes;
    }
    return chars_handled;

}
//---------------------------------------------------------------------------------------------------------------------------------
int spi_master_non_block_read_cb(spi_t *obj, const char *rx_buffer, int rx_length, am_hal_iom_callback_t pfnCallback, void *pCallbackCtxt)
{
    MBED_ASSERT(obj);

	am_hal_iom_transfer_t my_xfer = {0};
    	int chars_handled = 0;
  
   my_xfer.ui8RepeatCount  = 0;
    my_xfer.ui32PauseCondition = 0;
    my_xfer.ui32StatusSetClr = 0;
    my_xfer.ui8Priority     = 1;  

    my_xfer.eDirection = AM_HAL_IOM_RX;
    my_xfer.ui32NumBytes =rx_length;
    //xfer.uPeerInfo.ui32SpiChipSelect = 0;
    my_xfer.pui32RxBuffer = (uint32_t *)rx_buffer;
    my_xfer.pui32TxBuffer = NULL;
    my_xfer.bContinue=false;
    
    my_xfer.ui32InstrLen    = 0;
    my_xfer.ui32Instr       = 0;

	

    if(my_xfer.ui32NumBytes){
    	
    	//turn off full-duplex, because xfer.eDirection = AM_HAL_IOM_TX; doesn't seem to work..  why???
    	IOMn(obj->spi.iom_obj.iom.inst)->MSPICFG &= 0xFFFFFFFB;
        uint32_t status = am_hal_iom_nonblocking_transfer(obj->spi.iom_obj.iom.handle, &my_xfer, pfnCallback, pCallbackCtxt);
 
        if(AM_HAL_STATUS_SUCCESS != status)
        	{
            	return 0;
        	}
        chars_handled += my_xfer.ui32NumBytes;
    }
    return chars_handled;

}
//---------------------------------------------------------------------------------------------------------------------------------
int spi_master_non_block_write_cb(spi_t *obj, const char *tx_buffer, int tx_length, am_hal_iom_callback_t pfnCallback, void *pCallbackCtxt)
{
    MBED_ASSERT(obj);

    am_hal_iom_transfer_t my_xfer = {0};
    int chars_handled = 0;
    uint32_t addr_check=0;
    my_xfer.ui8RepeatCount  = 0;
    my_xfer.ui32PauseCondition = 0;
    my_xfer.ui32StatusSetClr = 0;
    my_xfer.ui8Priority     = 1;  

    my_xfer.eDirection = AM_HAL_IOM_TX;
    my_xfer.pui32RxBuffer = NULL;
    my_xfer.bContinue=false;
    
    

//here is a hack for dealing with non aligned buffers on write.  if address ends in any of last 2 bits being non-zero, 
//then stack portions in command and set command 
//offset length.  This will transfer the non aligned portions with the aligned portions.
//The corner case is transactions less than 4 bytes.  handle those

    addr_check = ((uint32_t)tx_buffer & 0x00000003);
    
    switch(addr_check)
    	{
    		case 1: //ugly because of extra access on less than 3, tx_buffer[N], but should be ok
    			//we are one over, so we need to send 3 bytes as command to re-align
    			my_xfer.pui32TxBuffer= (uint32_t *)&(tx_buffer[3]);
    			my_xfer.ui32InstrLen    = (tx_length < 3 ) ? tx_length : 3; //if tx_length is less than 3, then we are only writing
    			my_xfer.ui32Instr       = ((uint8_t)tx_buffer[0]);
    			my_xfer.ui32Instr<<=8;
    			my_xfer.ui32Instr       |= ((uint8_t)tx_buffer[1]);
    			my_xfer.ui32Instr<<=8;
    			my_xfer.ui32Instr       |= ((uint8_t)tx_buffer[2]);
    			my_xfer.ui32NumBytes =(tx_length < 3) ? 0 : (tx_length - 3);
    			
    		break;
    		
    		case 2://ugly because of extra access on less than 2, tx_buffer[N], but should be ok
    			my_xfer.pui32TxBuffer= (uint32_t *)&(tx_buffer[2]);
    			my_xfer.ui32InstrLen    = (tx_length < 2 ) ? tx_length : 2;
    			my_xfer.ui32Instr       = ((uint8_t)tx_buffer[0]);
    			my_xfer.ui32Instr<<=8;
    			my_xfer.ui32Instr       |= ((uint8_t)tx_buffer[1]);
    			my_xfer.ui32NumBytes =(tx_length < 2) ? 0 : (tx_length - 2);
    		break;
    		
    		case 3://still ugly.
    			my_xfer.pui32TxBuffer= (uint32_t *)&(tx_buffer[1]);
    			my_xfer.ui32InstrLen    = 1;
    			my_xfer.ui32Instr       = ((uint8_t)tx_buffer[0]);
    			my_xfer.ui32NumBytes =(tx_length < 1) ? 0 : (tx_length - 1);
    		break;
    		
    		case 0://already aligned!
    		default:
    			my_xfer.pui32TxBuffer = (uint32_t *)tx_buffer;
    			my_xfer.ui32InstrLen    = 0;
    			my_xfer.ui32Instr       = 0;
    			my_xfer.ui32NumBytes =tx_length;
    		
    		break;
    			
    	}
    	
	

    if(tx_length)
    {
    	
    	//turn off full-duplex, because xfer.eDirection = AM_HAL_IOM_TX; doesn't seem to work..  why???
    	IOMn(obj->spi.iom_obj.iom.inst)->MSPICFG &= 0xFFFFFFFB;
        uint32_t status = am_hal_iom_nonblocking_transfer(obj->spi.iom_obj.iom.handle, &my_xfer, pfnCallback, pCallbackCtxt);
 
      
        if(AM_HAL_STATUS_SUCCESS != status)
        	{
            	return 0;
        	}
        chars_handled += tx_length;
    }
    return chars_handled;

}
//---------------------------------------------------------------------------------------------------------------------------------
int spi_master_non_block_write(spi_t *obj, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, char write_fill)
{
    uint32_t status = spi_master_non_block_write_cb(obj, tx_buffer, tx_length, xfer_complete_callback, NULL);
    return status;
}
//--------------------------------------------------------------------------------------------------------------------------------
int spi_master_block_write(spi_t *obj, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, char write_fill){
    MBED_ASSERT(obj);

    int chars_handled = 0;
    am_hal_iom_transfer_t xfer = {0};
    xfer.ui32InstrLen = 0;
    xfer.ui32Instr = 0;
    xfer.bContinue = false;
    xfer.ui8RepeatCount = 0;
    xfer.ui8Priority = 1;
    xfer.ui32PauseCondition = 0;
    xfer.ui32StatusSetClr = 0;
	
    // perform a duplex xfer for the smaller of the two buffers
    xfer.eDirection = AM_HAL_IOM_FULLDUPLEX;
    xfer.ui32NumBytes = (tx_length > rx_length) ? rx_length : tx_length;
    xfer.pui32RxBuffer = (uint32_t *)rx_buffer;
    xfer.pui32TxBuffer = (uint32_t *)tx_buffer;

    if(xfer.ui32NumBytes){
        uint32_t status = am_hal_iom_spi_blocking_fullduplex(obj->spi.iom_obj.iom.handle, &xfer);
        if(AM_HAL_STATUS_SUCCESS != status){
            return 0;
        }
        chars_handled += xfer.ui32NumBytes;
    }

    // handle difference between buffers
    if(tx_length != rx_length){
        bool Rw = (rx_length >= tx_length);

        // set up common config
        xfer.eDirection = (Rw) ? AM_HAL_IOM_RX : AM_HAL_IOM_TX;
        xfer.ui32NumBytes = (Rw) ? (rx_length - tx_length) : (tx_length - rx_length);
        xfer.pui32RxBuffer = (Rw) ? (uint32_t*)(rx_buffer + chars_handled) : NULL;
        xfer.pui32TxBuffer = (Rw) ? NULL : (uint32_t*)(tx_buffer + chars_handled);
        
        uint32_t status = AM_HAL_STATUS_SUCCESS;
        if(!Rw || (write_fill == 0x00)){
            // when transmitting (w) or reading with a zero fill just use a simplex transfer
            status = am_hal_iom_blocking_transfer(obj->spi.iom_obj.iom.handle, &xfer);
            if(AM_HAL_STATUS_SUCCESS != status){
                return chars_handled;
            }
            chars_handled += xfer.ui32NumBytes;
        }else{
            // when reading with a nonzero fill use a duplex transfer
            uint8_t fill[xfer.ui32NumBytes];
            memset(fill, write_fill, xfer.ui32NumBytes);
            xfer.eDirection = AM_HAL_IOM_FULLDUPLEX;
            xfer.pui32TxBuffer = (uint32_t*)&fill;
            uint32_t status = am_hal_iom_spi_blocking_fullduplex(obj->spi.iom_obj.iom.handle, &xfer);
            if(AM_HAL_STATUS_SUCCESS != status){
                return chars_handled;
            }
            chars_handled += xfer.ui32NumBytes;
        }
    }

    return chars_handled;
}

int spi_slave_receive(spi_t *obj) {
    standin_fn();
    MBED_ASSERT(0);
    return 0;
}

int spi_slave_read(spi_t *obj) {
    standin_fn();
    MBED_ASSERT(0);
    return 0;
}

void spi_slave_write(spi_t *obj, int value) {
    standin_fn();
    MBED_ASSERT(0);
}

int spi_busy(spi_t *obj) {
    standin_fn();
    MBED_ASSERT(0);
    return 0;
}

const PinMap *spi_master_mosi_pinmap(){
    return PinMap_SPI_MOSI;
}

const PinMap *spi_master_miso_pinmap(){
    return PinMap_SPI_MISO;
}

const PinMap *spi_master_clk_pinmap(){
    return PinMap_SPI_SCLK;
}

const PinMap *spi_master_cs_pinmap(){
    return PinMap_SPI_SSEL;
}

const PinMap *spi_slave_mosi_pinmap(){
    return PinMap_SPI_MOSI;
}

const PinMap *spi_slave_miso_pinmap(){
    return PinMap_SPI_MISO;
}

const PinMap *spi_slave_clk_pinmap(){
    return PinMap_SPI_SCLK;
}

const PinMap *spi_slave_cs_pinmap(){
    return PinMap_SPI_SSEL;
}

#endif // DEVICE_SPI
