/*
 * rtos_irq.h
 *
 *  Created on: Nov 19, 2019
 *      Author: mbruno
 */


#ifndef RTOS_IRQ_H_
#define RTOS_IRQ_H_

#include "xcore_c.h"

typedef void (*rtos_irq_isr_t)( void *param );
#define RTOS_IRQ_ISR_ATTR __attribute__((fptrgroup("rtos_irq_isr")))

/**
 * This function sends an IRQ to an RTOS core. It may be called both by
 * RTOS cores and non-RTOS cores. It must be called by a core on the
 * same tile as the core being interrupted.
 *
 * \param core_id        The core ID of the RTOS core to interrupt. The core must have
 *                       previously called rtos_irq_enable.
 * \param source_id      The ID of source of the IRQ. When called by an RTOS core,
 *                       this must be the core ID of the calling core.
 *                       When called by a non-RTOS core then this must be an ID returned
 *                       by rtos_irq_source_register().
 */
void rtos_irq(int core_id, int source_id);
//void vPortInterruptCore( int xOtherCoreID, chanend xSourceChanend, int xSourceID );


/**
 * This function sends an IRQ to a peripheral on a non-RTOS core.
 * It must be called by an RTOS core. The non-RTOS core does not
 * need to be on the same tile as the RTOS core.
 *
 * \param dest_chanend  The channel end used by the peripheral to receive
 *                      the interrupt.
 */
void rtos_irq_peripheral(chanend dest_chanend);
//void vPortInterruptXcore( chanend xDestChanend );

/**
 * This function registers a non-RTOS IRQ source. This must be
 * called by an RTOS core, preferably during initialization prior
 * to starting the scheduler. The source ID returned must be passed
 * to the non-RTOS peripheral that will be generating the IRQs.
 * The peripheral can then subsequently send IRQs with rtos_irq().
 *
 * \param isr            The interrupt service routine to run when the IRQ is received.
 * \param param          A pointer to user data to pass to the ISR.
 * \param source_chanend The channel end to use to send the IRQ.
 *
 * \returns the IRQ source ID that may be passed to rtos_irq() when the
 * peripheral needs to send an IRQ.
 */

int rtos_irq_register(rtos_irq_isr_t isr, void *data, chanend source_chanend);
//int xPortICIISRRegister( rtos_ici_cb_t ici_cb, void *param );

/**
 * This function enables the calling core to receive RTOS IRQs. It
 * should be called once during initialization by each RTOS core.
 */
void rtos_irq_enable( int xCoreID );
//void vPortEnableICI( int xCoreID );

#endif /* RTOS_IRQ_H_ */
