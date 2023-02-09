#ifndef INTERRUPT_HANDLING_H
#define INTERRUPT_HANDLING_H

#include "stm32f4xx.h"
#include <stdbool.h>

/* Specify ARM architecture to determine whether we have BASEPRI register */
#if (__CORTEX_M >= 3)

#define INTERRUPT_LOWEST_PRIORITY  7u
#define INTERRUPT_HIGHEST_PRIORITY 0u
#define MAX_NVIC_REG_WORDS         8u

#else

#define INTERRUPT_LOWEST_PRIORITY  3u
#define INTERRUPT_HIGHEST_PRIORITY 0u
#define MAX_NVIC_REG_WORDS         8u

#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_IRQ_STATE   uint32_t irqState
#define DECLARE_NVIC_MASK  NVIC_Mask_t nvicMask

#define ENTER_NO_INTERRUPTS_SECTION()    irqState = PRIMASK_EnterNoInterruptsSection()
#define EXIT_NO_INTERRUPTS_SECTION()     PRIMASK_ExitNoInterruptsSection(irqState)
#define NO_INTERRUPTS_SECTION(inputSection) \
  {                                         \
    DECLARE_IRQ_STATE;                      \
    ENTER_NO_INTERRUPTS_SECTION();          \
    {                                       \
      inputSection                          \
    }                                       \
		EXIT_NO_INTERRUPTS_SECTION();           \
  }

#define ENTER_THREAD_SAFE_SECTION()      irqState = BASEPRI_EnterThreadSafeSection()
#define EXIT_THREAD_SAFE_SECTION()       BASEPRI_ExitThreadSafeSection(irqState)
#define THREAD_SAFE_SECTION(inputSection) \
  {                                       \
    DECLARE_IRQ_STATE;                    \
    ENTER_THREAD_SAFE_SECTION();          \
    {                                     \
      inputSection                        \
    }                                     \
		EXIT_THREAD_SAFE_SECTION();           \
  }

#define ENTER_SPECIFIC_INTERRUPT_DISABLED_SECTION(mask)    NVIC_EnterSpecificInterruptDisabledSection(&nvicMask, (mask))
#define EXIT_SPECIFIC_INTERRUPT_DISABLED_SECTION()         NVIC_ExitSpecificInterruptDisabledSection(&nvicMask)
#define SPECIFIC_INTERRUPT_DISABLED_SECTION(mask, inputSection) \
  {                                                             \
    DECLARE_NVIC_STATE;                                         \
    ENTER_SPECIFIC_INTERRUPT_DISABLED_SECTION(mask);            \
    {                                                           \
      inputSection                                              \
    }                                                           \
		EXIT_SPECIFIC_INTERRUPT_DISABLED_SECTION();                 \
  }

typedef struct
{
  uint32_t reg[MAX_NVIC_REG_WORDS];
} NVIC_Mask_t;

__WEAK uint32_t PRIMASK_EnterNoInterruptsSection(void);
__WEAK void     PRIMASK_ExitNoInterruptsSection(uint32_t irqState);
__WEAK void     PRIMASK_TriggerPendingInterrupts(void);
__WEAK void     PRIMASK_DisableIrq(void);
__WEAK void     PRIMASK_EnableIrq(void);

bool            BASEPRI_SetPriorityLevelThreshold(uint8_t inputBasePriLevel);
int8_t          BASEPRI_GetPriorityLevelThreshold(void);
__WEAK uint32_t BASEPRI_EnterInterruptsDisabledByThresholdSection(void);
__WEAK void     BASEPRI_ExitInterruptsDisabledByThresholdSection(uint32_t irqState);
__WEAK void     BASEPRI_TriggerPendingInterruptsByThreshold(void);
__WEAK void     BASEPRI_DisableIrqByThreshold(void);
__WEAK void     BASEPRI_EnableIrqByThreshold(void);

__WEAK bool IRQ_IsInIrqContext(void);
__WEAK bool IRQ_IsIRQnBlocked(IRQn_Type irqNum);
__WEAK bool IRQ_AreAllIRQnsDisabled(void);

void  NVIC_EnterSpecificInterruptDisabledSection(NVIC_Mask_t *nvicState,
                              									 const NVIC_Mask_t *disable);
void  NVIC_ExitSpecificInterruptDisabledSection(const NVIC_Mask_t *disable);
void  NVIC_TriggerSpecificPendingInterrupts(const NVIC_Mask_t *enable);
void  NVIC_DisableSpecificInterrupts(const NVIC_Mask_t *disable);
void  NVIC_EnableSpecificInterrupts(const NVIC_Mask_t *enable);
void  NVIC_SetSpecificInterruptInAMask(IRQn_Type irqNum, NVIC_Mask_t *mask);
void  NVIC_ClearSpecificInterruptInAMask(IRQn_Type irqNum, NVIC_Mask_t *mask);
void  NVIC_GetNvicIserMask(NVIC_Mask_t *mask);
bool  NVIC_IsNvicIserMaskDisabled(const NVIC_Mask_t *mask);
bool  NVIC_IsIRQnDisabled(IRQn_Type irqNum);
void* NVIC_GetIRQnHandler(IRQn_Type irqNum);
void  NVIC_SetIRQnHandler(IRQn_Type irqNum, void *handler);

#ifdef __cplusplus
}
#endif

#endif /* INTERRUPT_HANDLING_H */
