#ifndef __BASIC_TIMER_H__
#define __BASIC_TIMER_H__

void basic_timer_fire_in_ms(TIM_TypeDef* timer, uint32_t irqn, uint32_t time_ms);
void basic_timer_fire_every_ms(TIM_TypeDef* timer, uint32_t irqn, uint32_t time_ms);

#endif