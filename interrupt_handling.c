#include "interrupt_handling.h"

#define ASSERT(cond) if ((cond) == 0) while (1) { /* Stay here forever */}

/* Whether the input IRQn is an exception */
#define IS_EXCEPTION_NUM(IRQn)     (((int16_t)(IRQn) >= -16) && ((int16_t)(IRQn) < 0))
/* Whether the input IRQn is an interrupt */
#define IS_INTERRUPT_NUM(IRQn)     (((int16_t)(IRQn) >= 0) && ((int16_t)(IRQn) < 0xF0))
/* Whether the input IRQn is an interrupt or an exception */
#define IS_IRQn(IRQn)              (IS_INTERRUPT_NUM(IRQn) || IS_EXCEPTION_NUM(IRQn))
/* Whether the input interrupt level is valid */
#define IS_INT_LVL_VALID(intLevel) (INTERRUPT_HIGHEST_PRIORITY < (intLevel) \
	                                  && (intLevel) <= INTERRUPT_LOWEST_PRIORITY)

#if (__CORTEX_M >= 3)
/* Hơw many bit should we left shift to reach the start of BASEPRI register */
#define BASEPRI_START_BIT          (8U - __NVIC_PRIO_BITS)

static int8_t basePriLevel = 3;
#else
static int8_t basePriLevel = -1;
#endif
/* Todo: write all descriptions (the same way as Fsoft Academy) */

/*

Notes:
	1. Some functions are __attribute__ (("weak")) so
	   that user can override them for specific usage.

*/

/* Notes:

When a CRITICAL section is entered, all interrupts (except HardFault exception and
Non-Maskable Interrupt) are disabled.

We enter CRITICAL section by setting the PRIMASK register, a 1-bit wide interrupt
mask register, it blocks all exceptions (including interrupts) apart from the
Non-Maskable Interrupt (NMI) and the HardFault exception.

To carry out a critical task:
	1. Save the state of PRIMASK;
	2. Disable interrupts by set PRIMASK to 1;
	3. Execute the task;
	4. Restore PRIMASK state.

For example:

{
    uint32_t irqState;
    irqState = PRIMASK_EnterCritical();
    {
      // Your critical section ;
    }
    PRIMASK_ExitCritical(irqState);
}

*/

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK uint32_t PRIMASK_EnterCriticalSection(void)
{
  uint32_t irqState = __get_PRIMASK();

  __disable_irq();

  return irqState;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void PRIMASK_ExitCriticalSection(uint32_t irqState)
{
  if (irqState == 0U)
  {
    __enable_irq();
  }
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void PRIMASK_DisableIrq(void)
{
  __disable_irq();
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void PRIMASK_EnableIrq(void)
{
  __enable_irq();
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void PRIMASK_TriggerPendingInterrupts(void)
{
  if ((__get_PRIMASK() & 1U) != 0U)
  {
  	/* Execute all pending interupt handlers that are blocked by
  	 * the fact the PRIMASK is set. */
    __enable_irq();
    /*
     * We know that there are 3 instructions in the CPU at a time: being
     * fetched, decoded and executed. Pipeline flushing means re-fetch
     * all these instructions into CPU registers from memory or cache.
     *
     * For example, let's say that one of the executed interrupt handlers
     * changes the system control registers value. This change
     * may affect the instructions that are currently begin in the CPU.
     * To make sure changes made by interrupt handlers take effects, we re-fetch
		 * these instructions from cache or memory by calling __ISB().
     * */
    __ISB();
    /* Restore the state of PRIMASK. */
    __disable_irq();
  }
}

/* Notes:

When a ATOMIC section is entered, interrupts with priority lower or equal to
BASE_PRIORITY are disabled.

This feature is only supported in ARMv7-M architecture as it provides the BASEPRI
register. The width of the BASEPRI register is determined by the microcontroller vendors.
Most Cortex-M3 or Cortex-M4 microcontrollers have 8 or 16 programmable exception priority levels
and in these cases the width of BASEPRI will be 3 bits or 4 bits, respectively.

When BASEPRI is set to 0, it is disabled. When it is set to a non-zero value, it blocks exceptions
(including interrupts) that have the same or lower priority level, while still allowing exceptions
with a higher priority level to be accepted by the processor.

For older ARM architectures, ATOMIC section will be entered the same way as CRITICAL SECTION.

To carry out an atomic task:
	1. Save the state of BASEPRI;
	2. Disable interrupts equal to or lower than BASE_PRIORITY
	   by set BASEPRI to BASE_PRIORITY << BASEPRI_START_BIT;
	   Explannation: __NVIC_PRIO_BITS is the NVIC interrupt priority bits. Doing the
	   above expression means that we are left-shifting BASE_PRIORITY
	   to proper BASEPRI bits that are defined by microcontroller vendors.
	3. Execute the task;
	4. Restore BASEPRI state.

For example:

{
    uint32_t irqState;
    irqState = BASEPRI_EnterThresholdedInterruptsDisabledSection();
    {
      // Your critical section ;
    }
    BASEPRI_ExitThresholdedInterruptsDisabledSection(irqState);
}

*/

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
bool BASEPRI_SetPriorityLevelThreshold(uint8_t inputBasePriLevel)
{
	bool isSetSuccessfully = false;

#if (__CORTEX_M >= 3)
	if (IS_INT_LVL_VALID(inputBasePriLevel))
	{
		basePriLevel = inputBasePriLevel;
		isSetSuccessfully = true;
	}
#endif

	return isSetSuccessfully;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
int8_t BASEPRI_GetPriorityLevelThreshold(void)
{
  return basePriLevel;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK uint32_t BASEPRI_EnterInterruptsDisabledByThresholdSection(void)
{
	uint32_t irqState = 0;

#if (__CORTEX_M >= 3)
  irqState = __get_BASEPRI();
  __set_BASEPRI(basePriLevel << BASEPRI_START_BIT);
#else
  irqState = PRIMASK_EnterCriticalSection();
#endif

  return irqState;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void BASEPRI_ExitInterruptsDisabledByThresholdSection(uint32_t irqState)
{
#if (__CORTEX_M >= 3)
  __set_BASEPRI(irqState);
#else
  PRIMASK_ExitCriticalSection(irqState);
#endif
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void BASEPRI_DisableIrqByThreshold(void)
{
#if (__CORTEX_M >= 3)
  __set_BASEPRI(basePriLevel << BASEPRI_START_BIT);
#else
  PRIMASK_DisableIrq();
#endif
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void BASEPRI_EnableIrqByThreshold(void)
{
#if (__CORTEX_M >= 3)
  __set_BASEPRI(0);
#else
  PRIMASK_EnableIrq();
#endif
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK void BASEPRI_TriggerPendingInterruptsByThreshold(void)
{
#if (__CORTEX_M >= 3)
  uint32_t irqState = __get_BASEPRI();

  /* See PRIMASK_TriggerPendingInterrupts explannation */
  if (irqState >= (basePriLevel << BASEPRI_START_BIT))
  {
    __set_BASEPRI(0);
    __ISB();
    __set_BASEPRI(irqState);
  }
#else
  PRIMASK_TriggerPendingInterrupts();
#endif
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_EnterSpecificInterruptDisabledSection(NVIC_Mask_t *nvicState,
                             const NVIC_Mask_t *disable)
{
  NO_INTERRUPTS_SECTION
	(
    *nvicState = *(NVIC_Mask_t*)((uint32_t)&NVIC->ICER[0]);
    *(NVIC_Mask_t*)((uint32_t)&NVIC->ICER[0]) = *disable;
  )
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_ExitSpecificInterruptDisabledSection(const NVIC_Mask_t *disable)
{
	NVIC_DisableSpecificInterrupts(disable);
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_DisableSpecificInterrupts(const NVIC_Mask_t *disable)
{
  NO_INTERRUPTS_SECTION
	(
    *(NVIC_Mask_t*)((uint32_t)&NVIC->ICER[0]) = *disable;
  )
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_EnableSpecificInterrupts(const NVIC_Mask_t *enable)
{
  NO_INTERRUPTS_SECTION
	(
    *(NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]) = *enable;
  )
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_TriggerSpecificPendingInterrupts(const NVIC_Mask_t *enable)
{
  NVIC_Mask_t nvicMask      = {0};
  bool        shouldTrigger = false;

  // Get current NVIC enable mask.
  NO_INTERRUPTS_SECTION
	(
    nvicMask = *(NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]);
  )

  for (size_t i; i < MAX_NVIC_REG_WORDS; i++)
	{
  	nvicMask.reg[i] &= enable->reg[i];
		nvicMask.reg[i] = ~nvicMask.reg[i] & enable->reg[i];
		if (nvicMask.reg[i] != 0)
		{
			shouldTrigger = true;
		}
	}

	if (shouldTrigger)
	{
    *(NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]) = nvicMask;
    __ISB();
    *(NVIC_Mask_t*)((uint32_t)&NVIC->ICER[0]) = nvicMask;
  }
}

/**
	\brief      		 Set an bit in an NVIC mask.
	\details    		 Set an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_SetSpecificInterruptInAMask(IRQn_Type irqNum, NVIC_Mask_t *mask)
{
  if (IS_INTERRUPT_NUM(irqNum))
  {
  	mask->reg[(uint8_t)irqNum >> 5] |= 1UL << ((uint8_t)irqNum & 0x1FUL);
  }
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_ClearSpecificInterruptInAMask(IRQn_Type irqNum, NVIC_Mask_t *mask)
{
	if (IS_INTERRUPT_NUM(irqNum))
	{
		mask->reg[(uint8_t)irqNum >> 5] &= ~(1UL << ((uint8_t)irqNum & 0x1FUL));
	}
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK bool IRQ_IsInIrqContext(void)
{
	/* Reading VECTACTIVE to determine the currently executing
	 * exception/interrupt number. The result of not being zero means
	 * we are in an interrupt context. */
  return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0U;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK bool IRQ_IsIRQnBlocked(IRQn_Type irqNum)
{
  uint32_t irqPri, activeIrq;

#if (__CORTEX_M >= 3)
  uint32_t basepri;

  ASSERT((irqNum >= MemoryManagement_IRQn)
             && (irqNum < (IRQn_Type)0xF0));
#else
  ASSERT((irqNum >= SVCall_IRQn) && ((IRQn_Type)irqNum < 0xF0));
#endif

  /* Return true if all interrupts are disabled */
  if ((__get_PRIMASK() & 1U) != 0U)
  {
    return true;
  }

  /* Return true if the interrupt is disabled */
  if (NVIC_IsIRQnDisabled(irqNum))
  {
    return true;
  }

  irqPri = NVIC_GetPriority(irqNum);
#if (__CORTEX_M >= 3)
  basepri = __get_BASEPRI();
  /* Return true if the interrupt has lower priority than the threshold */
  if ((basepri != 0U) && (irqPri >= (basepri >> BASEPRI_START_BIT)))
  {
    return true;
  }
#endif

  // Check if already in an interrupt handler. If so, an interrupt with a
  // higher priority (lower priority value) can preempt.
  activeIrq = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos;
  if (activeIrq != 0U) {
    if (irqPri >= NVIC_GetPriority((IRQn_Type)(activeIrq - 16U))) {
      return true;                          // The IRQ in question has too low
    }                                       // priority vs. current active IRQ
  }

  return false;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
__WEAK bool IRQ_AreAllIRQnsDisabled(void)
{
	bool isAllIRQnsDisabled = false;

#if (__CORTEX_M >= 3)
	isAllIRQnsDisabled = ((__get_PRIMASK() & 1U) == 1U)
                       || (__get_BASEPRI() >= (basePriLevel << BASEPRI_START_BIT));
#else
	isAllIRQnsDisabled = (__get_PRIMASK() & 1U) == 1U;
#endif

  return isAllIRQnsDisabled;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_GetNvicIserMask(NVIC_Mask_t *mask)
{
  NO_INTERRUPTS_SECTION
	(
    *mask = *(NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]);
  )
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
bool NVIC_IsNvicIserMaskDisabled(const NVIC_Mask_t *mask)
{
  NVIC_Mask_t nvicMask       = {0};
  bool        isIserDisabled = true;

  NO_INTERRUPTS_SECTION
	(
    nvicMask = *(NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]);
  )

  for (size_t i; i < MAX_NVIC_REG_WORDS; i++)
  {
  	if ((mask->reg[i] & nvicMask.reg[i]) != 0U)
  	{
  		isIserDisabled = false;
  		break;
  	}
  }

  return isIserDisabled;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
bool NVIC_IsIRQnDisabled(IRQn_Type irqNum)
{
  bool        isDisabled = true;
  NVIC_Mask_t *mask      = NULL;

  if (IS_INTERRUPT_NUM(irqNum))
  {
  	/* Get Interrupt Set Enable Registers mask */
  	mask = (NVIC_Mask_t*)((uint32_t)&NVIC->ISER[0]);
  	/* Find and check whether the bit corresponding to input irqNum
  	   is equal to 0. */
  	isDisabled = (mask->reg[(((uint8_t)irqNum) >> 5UL)]    \
								  & (1UL << ((uint8_t)irqNum & 0x1FUL)))   \
								  == 0UL;
  }

  return isDisabled;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void *NVIC_GetIRQnHandler(IRQn_Type irqNum)
{
	void *handler = NULL;

  /* Input interrupt number can be an interrupt or an exception */
  if (IS_IRQn(irqNum))
  {
  	/*
			 1. SCB->VTOR is the value of Vector Table Offset Register
					in System Control Block. The value is start the address of NVIC.
			 2. For 32-bit MCU, the value is of type uint32_t. We do type casting
					to transform it to uint32_t* so that we can use [] operator
					to find the position of target handler in NVIC.
			 3. "+16" because we are finding the interrupt handlers,
					not exception handlers.
			 4. It is a convention that a type-casting operation should change only
					1 type attribute per operation. For example:
						+ DO:   uint8_t  <=> uint8_t* (Non-pointer to pointer with the same type)
						+ DO:   uint32_t <=> uint8_t  (Non-pointer to non-pointer with different type)
						+ DONT: uint32_t <=> uint8_t* (Non-pointer to pointer with different type)
  	*/
  	handler = (void*)((uint32_t*)(((uint32_t*)SCB->VTOR)[(int16_t)irqNum + 16]));
  }

  return handler;
}

/**
	\brief      		 Clear an bit in an NVIC mask.
	\details    		 Clear an IRQn bit in an NVIC mask corresponding to the input IRQn.
	\param [in]      irqNum: Device specific interrupt number.
	\param [in, out] mask:   The mask to set the interrupt bit.
	\note       		 IRQn must be an interrupt number (not exception number).
									 If input IRQn is invalid or it does not defined by the MCU vendor,
									 this function will have no effect.
 */
void NVIC_SetIRQnHandler(IRQn_Type irqNum, void *handler)
{
	if (IS_IRQn(irqNum))
	{
		((uint32_t*)SCB->VTOR)[(int16_t)irqNum + 16] = (uint32_t)((uint32_t*)handler);
	}
}
