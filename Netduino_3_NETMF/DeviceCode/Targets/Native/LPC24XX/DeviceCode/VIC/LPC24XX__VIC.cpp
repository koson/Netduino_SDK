//-----------------------------------------------------------------------------
// Software that is described herein is for illustrative purposes only  
// which provides customers with programming information regarding the  
// products. This software is supplied "AS IS" without any warranties.  
// NXP Semiconductors assumes no responsibility or liability for the 
// use of the software, conveys no license or title under any patent, 
// copyright, or mask work right to the product. NXP Semiconductors 
// reserves the right to make changes in the software without 
// notification. NXP Semiconductors also make no representation or 
// warranty that such application will be suitable for the specified 
// use without further testing or modification. 
//-----------------------------------------------------------------------------

#include <tinyhal.h>
#include "..\LPC24XX.h"

////////////////////////////////////////////////////////////////////////////////
#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rwdata = "s_IsrTable_LPC24XX"
#endif

#define DEFINE_IRQ(index) { index, { NULL, (void*)(size_t)index } }

LPC24XX_VIC_Driver::IRQ_VECTORING __section(rwdata) LPC24XX_VIC_Driver::s_IsrTable[] =
{
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_WDT        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_SW         ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_DBG_COM_RX ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_DBG_COM_TX ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_TIMER0     ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_TIMER1     ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_UART0      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_UART1      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_PWM_0_1    ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_I2C0       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_SPI_SSP0   ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_SSP1       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_PLL        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_RTC        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_EINT0      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_EINT1      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_EINT2_LCD  ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_EINT3      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_ADC0       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_I2C1       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_BOD        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_EMAC       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_USB        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_CAN        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_SD         ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_DMA        ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_TIMER2     ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_TIMER3     ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_UART2      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_UART3      ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_I2C2       ),
    DEFINE_IRQ( LPC24XX_VIC::c_IRQ_INDEX_I2S        ),
                             
    DEFINE_IRQ( LPC24XX_VIC_Driver::c_VECTORING_GUARD     ),
};

#undef DEFINE_IRQ

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rwdata
#endif

//--//

void CPU_INTC_Initialize()
{
    LPC24XX_VIC_Driver::Initialize();
}

BOOL CPU_INTC_ActivateInterrupt( UINT32 Irq_Index, HAL_CALLBACK_FPN ISR, void* ISR_Param )
{
    return LPC24XX_VIC_Driver::ActivateInterrupt( Irq_Index, FALSE, ISR, ISR_Param );
}

BOOL CPU_INTC_DeactivateInterrupt( UINT32 Irq_Index )
{
    return LPC24XX_VIC_Driver::DeactivateInterrupt( Irq_Index );
}

BOOL CPU_INTC_InterruptEnable( UINT32 Irq_Index )
{
    return LPC24XX_VIC_Driver::InterruptEnable( Irq_Index );
}

BOOL CPU_INTC_InterruptDisable( UINT32 Irq_Index )
{
    return LPC24XX_VIC_Driver::InterruptDisable( Irq_Index );
}

BOOL CPU_INTC_InterruptEnableState( UINT32 Irq_Index )
{
    return LPC24XX_VIC_Driver::InterruptEnableState( Irq_Index );
}

BOOL CPU_INTC_InterruptState( UINT32 Irq_Index )
{
    return LPC24XX_VIC_Driver::InterruptState( Irq_Index );
}


////////////////////////////////////////////////////////////////////////////////

void __irq IRQ_Handler()

{
    LPC24XX_VIC_Driver::IRQ_Handler();
}

void LPC24XX_VIC_Driver::Initialize()
{
    LPC24XX_VIC& VIC = LPC24XX::VIC();

    // disable all interrupts
    VIC.INTENCLR = 0xFFFFFFFF;

    IRQ_VECTORING* IsrVector = s_IsrTable;

    while(IsrVector->Index != c_VECTORING_GUARD)
    {
        IsrVector->Handler.Initialize( STUB_ISRVector, (void*)(size_t)IsrVector->Index );

        IsrVector++;
    }
}

BOOL LPC24XX_VIC_Driver::ActivateInterrupt( UINT32 Irq_Index, BOOL Fast, HAL_CALLBACK_FPN ISR, void* ISR_Param )
{
    // figure out the interrupt
    IRQ_VECTORING* IsrVector = IRQToIRQVector( Irq_Index ); if(!IsrVector) return FALSE;

    {
        GLOBAL_LOCK(irq);

        LPC24XX_VIC& VIC = LPC24XX::VIC();

        // disable this interrupt while we change it
        VIC.INTENCLR = 1 << IsrVector->Index;

        // set the vector
        IsrVector->Handler.Initialize( ISR, ISR_Param );
        
        // Use Vector Address register to identify the source of interrupt
        VIC.VECTADDR[Irq_Index] = Irq_Index;

        // enable the interrupt if we have a vector
        VIC.INTENABLE = 1 << IsrVector->Index;
    }

    return TRUE;
}

BOOL LPC24XX_VIC_Driver::DeactivateInterrupt( UINT32 Irq_Index )
{
    // figure out the interrupt
    IRQ_VECTORING* IsrVector = IRQToIRQVector( Irq_Index ); if(!IsrVector) return FALSE;

    {
        GLOBAL_LOCK(irq);

        LPC24XX_VIC& VIC = LPC24XX::VIC();

        // disable this interrupt while we change it
        VIC.INTENCLR = 1 << IsrVector->Index;

        IsrVector->Handler.Initialize( STUB_ISRVector, (void*)(size_t)IsrVector->Index );
    }

    return TRUE;
}

#pragma arm section code = "SectionForInternalSRAM"
void LPC24XX_VIC_Driver::IRQ_Handler()
{
    UINT32 index;

    LPC24XX_VIC& VIC = LPC24XX::VIC();

    // set before jumping elsewhere or allowing other interrupts
    SystemState_SetNoLock( SYSTEM_STATE_ISR              );
    SystemState_SetNoLock( SYSTEM_STATE_NO_CONTINUATIONS );


    index = VIC.ADDRESS;

    IRQ_VECTORING* IsrVector = &s_IsrTable[ index ];
        
    // In case the interrupt was forced, remove the flag.
    VIC.RemoveForcedInterrupt( index );


    IsrVector->Handler.Execute();

    SystemState_ClearNoLock( SYSTEM_STATE_NO_CONTINUATIONS ); // nestable
    SystemState_ClearNoLock( SYSTEM_STATE_ISR              ); // nestable

    // Reset VIC priority hw logic.
    VIC.ADDRESS = 0xFF;
}
#pragma arm section code

BOOL LPC24XX_VIC_Driver::InterruptEnable( UINT32 Irq_Index  )
{
    IRQ_VECTORING* IsrVector = IRQToIRQVector( Irq_Index );

    if(!IsrVector) return FALSE;

    LPC24XX_VIC& VIC = LPC24XX::VIC();

    GLOBAL_LOCK(irq);

    BOOL WasEnabled = VIC.IsInterruptEnabled( Irq_Index );

    VIC.INTENABLE = 1 << IsrVector->Index;

    return WasEnabled;
}


BOOL LPC24XX_VIC_Driver::InterruptDisable( UINT32 Irq_Index )
{
    IRQ_VECTORING* IsrVector = IRQToIRQVector( Irq_Index );

    if(!IsrVector) return 0;

    LPC24XX_VIC& VIC = LPC24XX::VIC();

    GLOBAL_LOCK(irq);

    BOOL WasEnabled = VIC.IsInterruptEnabled( Irq_Index );

    VIC.INTENCLR = 1 << IsrVector->Index;

    return WasEnabled;
}


BOOL LPC24XX_VIC_Driver::InterruptEnableState( UINT32 Irq_Index )
{
    LPC24XX_VIC& VIC = LPC24XX::VIC();

    return VIC.IsInterruptEnabled( Irq_Index );
}


BOOL  LPC24XX_VIC_Driver::InterruptState( UINT32 Irq_Index )
{
    LPC24XX_VIC& VIC = LPC24XX::VIC();

    return VIC.GetInterruptState( Irq_Index );
}

LPC24XX_VIC_Driver::IRQ_VECTORING* LPC24XX_VIC_Driver::IRQToIRQVector( UINT32 IRQ )
{
    IRQ_VECTORING* IsrVector = s_IsrTable;

    while(IsrVector->Index != c_VECTORING_GUARD)
    {
        if(IsrVector->Index == IRQ) return IsrVector;

        IsrVector++;
    }

    return NULL;
}

void LPC24XX_VIC_Driver::STUB_ISRVector( void* Param )
{
    lcd_printf( "\fSTUB_ISR %08x\r\n", (size_t)Param );
    HARD_BREAKPOINT();
}
