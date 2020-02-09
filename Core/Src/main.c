#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f30x.h"
#include "stm32f30x_gpio.h"
#include "core_cm4.h"

#include "OS/task.h"
#include "OS/schedule.h"
#include "OS/syscall.h"
#include "OS/lock.h"
#include "OS/isr_notify.h"

#include "basic_timer.h"

void toggle_led(void* params);
void increment_counter(void* params);

typedef struct {
  uint32_t led;
  uint32_t time_on;
  uint32_t time_off;
  TIM_TypeDef* timer;
  uint32_t irqn;
} LEDTaskParams;


  LEDTaskParams led9Params = {
    .led = GPIO_Pin_9,
    .time_on = 500,
    .time_off = 500,
    .timer = TIM7,
    .irqn = TIM7_IRQn
  };

  LEDTaskParams led8Params = {
    .led = GPIO_Pin_8,
    .time_on = 1000,
    .time_off = 1000,
    .timer = TIM6,
    .irqn = TIM6_DAC_IRQn
  };

  LEDTaskParams led10Params = {
    .led = GPIO_Pin_10,
    .time_on = 3000,
    .time_off = 3000
  };

static volatile uint32_t counter_val;
static OsMutex counter_mutex;

typedef struct {
  volatile uint32_t* val_ptr;
  uint32_t led;
  OsMutex* mutex;
} CounterTaskParams;

CounterTaskParams counter_params_0 = {
  .val_ptr = &counter_val,
  .led = GPIO_Pin_9,
  .mutex = &counter_mutex
};

CounterTaskParams counter_params_1 = {
  .val_ptr = &counter_val,
  .led = GPIO_Pin_8,
  .mutex = &counter_mutex
};

CounterTaskParams counter_params_2 = {
  .val_ptr = &counter_val,
  .led = GPIO_Pin_10,
  .mutex = &counter_mutex
};

int main(void)
{

  RCC->APB1ENR = 0xFFFFFFFF;
  RCC->APB2ENR = 0xFFFFFFFF;
  RCC->AHBENR  = 0xFFFFFFFF;


  GPIO_InitTypeDef ledGpio = {
    .GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_10 |
                GPIO_Pin_15 | GPIO_Pin_11 | GPIO_Pin_14 |
                GPIO_Pin_12 | GPIO_Pin_13,
    .GPIO_Mode = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_Level_1,
    .GPIO_OType = GPIO_OType_PP,
    .GPIO_PuPd = GPIO_PuPd_NOPULL
  };

  GPIO_Init(GPIOE, &ledGpio);

  GPIO_ResetBits(GPIOE, GPIO_Pin_9 | GPIO_Pin_8 | GPIO_Pin_10 |
                        GPIO_Pin_15 | GPIO_Pin_11 | GPIO_Pin_14 |
                        GPIO_Pin_12 | GPIO_Pin_13);

  mutex_init(&counter_mutex);

  asm("cpsid i");

  SysTick->LOAD = 9000;
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk;

  init_schedule();

  /*
  LEDTaskParams led15Params = {
    .led = GPIO_Pin_15,
    .time_on = 1300,
    .time_off = 1300 
  };

  LEDTaskParams led11Params = {
    .led = GPIO_Pin_11,
    .time_on = 1400,
    .time_off = 1400 
  };

  LEDTaskParams led14Params = {
    .led = GPIO_Pin_14,
    .time_on = 1500,
    .time_off = 1500 
  };

  LEDTaskParams led12Params = {
    .led = GPIO_Pin_12,
    .time_on = 1600,
    .time_off = 1600 
  };

  LEDTaskParams led13Params = {
    .led = GPIO_Pin_13,
    .time_on = 1700,
    .time_off = 1700 
  };*/

  create_task(toggle_led, 1, 1024, (void*)&led8Params);
  create_task(toggle_led, 5, 1024, (void*)&led9Params);
  // create_task(toggle_led, 1024, (void*)&led10Params);
  /*create_task(toggle_led, 1024, (void*)&led15Params);
  create_task(toggle_led, 1024, (void*)&led11Params);
  create_task(toggle_led, 1024, (void*)&led14Params);
  create_task(toggle_led, 1024, (void*)&led12Params);
  create_task(toggle_led, 1024, (void*)&led13Params);*/

  create_task(increment_counter, 0, 1024, (void*)&counter_params_0);
  create_task(increment_counter, 4, 1024, (void*)&counter_params_1);
  create_task(increment_counter, 1, 1024, (void*)&counter_params_2);

  asm("cpsie i");
  yield();

  while (1)
  {
  }
}

void toggle_led(void* params) {
  LEDTaskParams* ledParamsPtr = (LEDTaskParams*)params;
  LEDTaskParams ledParams = *ledParamsPtr;
  int i;
  uint32_t irqn = ledParams.irqn;
  TIM_TypeDef* timer = ledParams.timer;

  basic_timer_fire_every_ms(timer, irqn, ledParams.time_on);

  while (1) {
    GPIO_SetBits(GPIOE, ledParams.led);
    isr_notify_clear(irqn);
    //basic_timer_fire_in_ms(timer, irqn, ledParams.time_on);
    isr_notify_try_wait(irqn);
    //for (i = 5000000; i > 0; i--);
    // wait_for(ledParams.time_on);
    GPIO_ResetBits(GPIOE, ledParams.led);
    isr_notify_clear(irqn);
    //basic_timer_fire_in_ms(timer, irqn, ledParams.time_off);
    isr_notify_try_wait(irqn);
    //for (i = 5000000; i > 0; i--);
    // wait_for(ledParams.time_off);
  }
}

void increment_counter(void* params) {
  CounterTaskParams* counterParamsPtr = (CounterTaskParams*)params;

  GPIO_ResetBits(GPIOE, counterParamsPtr->led);

  int i;
  // for (i = 0; i < 5000000; i++) {
  for (i = 0; i < 5000000; i++) {
    mutex_spinlock(counterParamsPtr->mutex);
    (*counterParamsPtr->val_ptr)++;
    mutex_spinunlock(counterParamsPtr->mutex);
  }

  GPIO_SetBits(GPIOE, counterParamsPtr->led);

  while (1)
  {
    wait_for(1000);
  }

}

void SystemClock_Config(void)
{
}

void Error_Handler(void)
{
}

void assert_failed(uint8_t *file, uint32_t line)
{ 
}

void assert_param(uint32_t status) {

}