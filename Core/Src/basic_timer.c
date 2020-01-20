

#include <stdint.h>

#include "stm32f30x.h"

#include "basic_timer.h"

#include "OS/isr_notify.h"


void basic_timer_fire_in_ms(TIM_TypeDef* timer, uint32_t irqn, uint32_t time_ms) {

    timer->CR1 = TIM_CR1_OPM |
                 TIM_CR1_URS;

    // Clear interrupt status
    timer->SR = 0;

    timer->DIER = TIM_DIER_UIE;

    timer->PSC = 36000;

    timer->ARR = time_ms * 2;

    timer->EGR = TIM_EGR_UG;

    NVIC_EnableIRQ(irqn);

    timer->CR1 |= TIM_CR1_CEN;
}

void basic_timer_fire_every_ms(TIM_TypeDef* timer, uint32_t irqn, uint32_t time_ms) {

    timer->CR1 = TIM_CR1_URS;

    // Clear interrupt status
    timer->SR = 0;

    timer->DIER = TIM_DIER_UIE;

    timer->PSC = 36000 - 1;

    timer->ARR = (time_ms * 2) - 1;

    timer->EGR = TIM_EGR_UG;

    NVIC_EnableIRQ(irqn);

    timer->CR1 |= TIM_CR1_CEN;
}

void TIM7_IRQHandler(void) {
    isr_notify_notify_ISR(TIM7_IRQn);
    TIM7->SR = 0;
}

void TIM6_DAC_IRQHandler(void) {
    isr_notify_notify_ISR(TIM6_DAC_IRQn);
    TIM6->SR = 0;
}