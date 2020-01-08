// Copyright (c) 2014-2016, XMOS Ltd, All rights reserved
#ifndef _spi_h_
#define _spi_h_
#include <xs1.h>
#include <stdint.h>
#include <stddef.h>

/** This type indicates what mode an SPI component should use */
typedef enum spi_mode_t {
  SPI_MODE_0, /**< SPI Mode 0 - Polarity = 0, Clock Edge = 1 */
  SPI_MODE_1, /**< SPI Mode 1 - Polarity = 0, Clock Edge = 0 */
  SPI_MODE_2, /**< SPI Mode 2 - Polarity = 1, Clock Edge = 0 */
  SPI_MODE_3, /**< SPI Mode 3 - Polarity = 1, Clock Edge = 1 */
} spi_mode_t;


/** This interface allows clients to interact with SPI master task. */
typedef interface spi_master_if {

  /** Begin a transaction.
   *
   *  This will start a transaction on the bus. During a transaction, no
   *  other client to the SPI component can send or receive data. If
   *  another client is currently using the component then this call
   *  will block until the bus is released.
   *
   *  \param device_index  the index of the slave device to interact with.
   *  \param speed_in_khz  The speed that the SPI bus should run at during
   *                       the transaction (in kHZ).
   *  \param mode          The mode of spi transfers during this transaction.
   */
  [[guarded]]
  void begin_transaction(unsigned device_index,
                         unsigned speed_in_khz, spi_mode_t mode);

  /** End a transaction.
   *
   *  This ends a transaction on the bus and releases the component to other
   *  clients.
   *
   *  \param ss_deassert_time  The minimum time in reference clock ticks between
   *                           assertions of the selected slave select. This
   *                           time will be ignored if the next transaction is
   *                           to a different slave select.
   */
  void end_transaction(unsigned ss_deassert_time);

  /** Transfer a byte over the spi bus.
   *
   *  This function will transmit and receive 8 bits of data over the SPI
   *  bus. The data will be transmitted least-significant bit first.
   *
   *  \param data          the data to transmit the MOSI port.
   *
   *  \returns       the data read in from the MISO port.
   */
  uint8_t transfer8(uint8_t data);

  /** Transfer a 32-bit word over the spi bus.
   *
   *  This function will transmit and receive 32 bits of data over the SPI
   *  bus. The data will be transmitted least-significant bit first.
   *
   *  \param data    the data to transmit the MOSI port.
   *
   *  \returns       the data read in from the MISO port.
   */
  uint32_t transfer32(uint32_t data);
} spi_master_if;

/** Task that implements the SPI proctocol in master mode that is
    connected to a multiple slaves on the bus.

    Each slave must be connected to using the same SPI mode.

    You can access different slave devices over the interface connection
    using the device_index parameter of the interface functions.
    The task will allocate the device indices in the order of the supplied
    array of slave select ports.

    \param i             an array of interface connection to the
                         clients of the task.
    \param num_clients   the number of clients connected to the task.
    \param clk           a clock block used by the task.
    \param sclk          the SPI clock port.
    \param mosi          the SPI MOSI (master out, slave in) port.
    \param miso          the SPI MISO (master in, slave out) port.
    \param p_ss          an array of ports connected to the slave select signals
                         of the slave.
    \param num_slaves    The number of slave devices on the bus.
    \param clk           a clock for the component to use.
*/
[[distributable]]
void spi_master(server interface spi_master_if i[num_clients],
        static const size_t num_clients,
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 ?miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock ?clk);

/** Asynchronous interface to an SPI component.
 *
 *  This interface allows programs to offload SPI bus transfers to another
 *  task. An asynchronous notification occurs when the transfer is complete.
 */
typedef interface spi_master_async_if  {
  /** Begin a transaction.
   *
   *  This will start a transaction on the bus. During a transaction, no
   *  other client to the SPI component can send or receive data. If
   *  another client is currently using the component then this call
   *  will block until the bus is released.
   *
   *  \param device_index  the index of the slave device to interact with.
   *  \param speed_in_khz  The speed that the SPI bus should run at during
   *                       the transaction (in kHZ)
   *  \param mode          The mode of spi transfers during this transaction
   */
  void begin_transaction(unsigned device_index,
                         unsigned speed_in_khz, spi_mode_t mode);

  /** End a transaction.
   *
   *  This ends a transaction on the bus and releases the component to other
   *  clients.
   *
   *  \param ss_deassert_time  The minimum time in reference clock ticks between
   *                           assertions of the selected slave select. This
   *                           time will be ignored if the next transaction is
   *                           to a different slave select.
   */
  void end_transaction(unsigned ss_deassert_time);

  /** Initialize Transfer an array of bytes over the spi bus.
   *
   *  This function will initialize a transmit of 8 bit data
   *  over the SPI bus.
   *
   *  \param inbuf    A *movable* pointer that is moved to the other task
   *                  pointing to the buffer area to fill with data. If this
   *                  parameter is NULL then the incoming data of the transfer
   *                  will be discarded.
   *  \param outbuf   A *movable* pointer that is moved to the other task
   *                  pointing to the buffer area to with data to transmit.
   *                  If this parameter is NULL then the outgoing data of the
   *                  transfer will consist of undefined values.
   *  \param nbytes   The number of bytes to transfer over the bus.
   */
  void init_transfer_array_8(uint8_t * movable inbuf,
                             uint8_t * movable outbuf,
                             size_t nbytes);

  /** Initialize Transfer an array of bytes over the spi bus.
   *
   *  This function will initialize a transmit of 32 bit data
   *  over the SPI bus.
   *
   *  \param inbuf    A *movable* pointer that is moved to the other task
   *                  pointing to the buffer area to fill with data. If this
   *                  parameter is NULL then the incoming data of the transfer
   *                  will be discarded.
   *  \param outbuf   A *movable* pointer that is moved to the other task
   *                  pointing to the buffer area to with data to transmit.
   *                  If this parameter is NULL then the outgoing data of the
   *                  transfer will consist of undefined values.
   *  \param nwords   The number of words to transfer over the bus.
   */
  void init_transfer_array_32(uint32_t * movable inbuf,
                              uint32_t * movable outbuf,
                              size_t nwords);


  /** Transfer completed notification.
   *
   *  This notification occurs when a transfer is completed.
   */
  [[notification]]
  slave void transfer_complete(void);

  /** Retrieve transfer buffers.
   *
   *  This function should be called after the transfer_complete() notification
   *  and will return the buffers given to the other task by
   *  init_transfer_array_8().
   *
   *  \param inbuf    A movable pointer that will be set to the buffer
   *                  pointer that was filled during the transfer.
   *  \param outbuf   A movable pointer that will be set to the buffer
   *                  pointer that was transmitted during the transfer.
   */
  [[clears_notification]]
  void retrieve_transfer_buffers_8(uint8_t * movable &inbuf,
                                   uint8_t * movable &outbuf);


  /** Retrieve transfer buffers.
   *
   *  This function should be called after the transfer_complete() notification
   *  and will return the buffers given to the other task by
   *  init_transfer_array_32().
   *
   *  \param inbuf    A movable pointer that will be set to the buffer
   *                  pointer that was filled during the transfer.
   *  \param outbuf   A movable pointer that will be set to the buffer
   *                  pointer that was transmitted during the transfer.
   */
  [[clears_notification]]
  void retrieve_transfer_buffers_32(uint32_t * movable &inbuf,
                                    uint32_t * movable &outbuf);
} spi_master_async_if;


/** SPI master component for asynchronous API.
 *
 * This component implements SPI and allows a client to connect using the
 * asynchronous SPI master interface.
 *
 *  \param i             an array of interface connection to the
 *                       clients of the task.
 *  \param num_clients   the number of clients connected to the task.
 *  \param sclk          the SPI clock port.
 *  \param mosi          the SPI MOSI (master out, slave in) port.
 *  \param miso          the SPI MISO (master in, slave out) port.
 *  \param p_ss          an array of ports connected to the slave select signals
 *                       of the slave.
 *  \param num_slaves    The number of slave devices on the bus.
 *  \param clk0           a clock for the component to use.
 *  \param clk1           a clock for the component to use.
 */
[[combinable]]
void spi_master_async(server interface spi_master_async_if i[num_clients],
        static const size_t num_clients,
        out buffered port:32 sclk,
        out buffered port:32 ?mosi,
        in buffered port:32 miso,
        out port p_ss[num_slaves],
        static const size_t num_slaves,
        clock clk0,
        clock clk1);

/**** SLAVE ****/

/** This interface allows clients to interact with SPI slave tasks by
 *  completing callbacks that show how to handle data.
 */
typedef interface spi_slave_callback_if {

  /** This callback will get called when the master de-asserts on the slave
   *  select line to end a transaction.
   */
  void master_ends_transaction(void);

  /** This callback will get called when the master initiates a bus transfer
   *  or when more data is required during a transaction.
   *  The application must supply the data to transmit to the master. If
   *  the spi slave component is set to ``SPI_TRANSFER_SIZE_32`` mode then
   *  this callback will not be called and master_requires_data32() will
   *  be called instead. Data is transmitted for the least significant bit
   *  first.  If the master completes the transaction before 8 bits are
   *  transferred the remaining bits are discarded.
   *
   *  \returns the 8-bit value to transmit.
   */
  uint32_t master_requires_data(void);

  /** This callback will get called after a transfer. It will occur after
   *  every 8 bits transferred if the slave component is set to
   *  ``SPI_TRANSFER_SIZE_8``. If the component is set to
   *  ``SPI_TRANSFER_SIZE_32`` then it will occur if the master ends the
   *  transaction before 32 bits are transferred.
   *
   *  \param datum the data received from the master.
   *  \param valid_bits the number of valid bits of data received from the master.
   */
  void master_supplied_data(uint32_t datum, uint32_t valid_bits);

} spi_slave_callback_if;

/** This type specifies the transfer size from the SPI slave component
    to the application */
typedef enum spi_transfer_type_t {
  SPI_TRANSFER_SIZE_8, ///< Transfers should by 8-bit.
  SPI_TRANSFER_SIZE_32 ///< Transfers should be 32-bit.
} spi_transfer_type_t;

/** SPI slave component.
 *
 *  This function implements an SPI slave bus.
 *
 *  \param spi_i   The interface to connect to the user of the component.
 *             The component acts as the client and will make callbacks to
 *             the application.
 *  \param p_sclk        the SPI clock port.
 *  \param p_mosi        the SPI MOSI (master out, slave in) port.
 *  \param p_miso        the SPI MISO (master in, slave out) port.
 *  \param p_ss          the SPI SS (slave select) port.
 *  \param clk           clock to be used by the component.
 *  \param mode          the SPI mode of the bus.
 *  \param transfer_type the type of transfer the slave will perform: either
 *                       ``SPI_TRANSFER_SIZE_8`` or ``SPI_TRANSFER_SIZE_32``.
 */
 [[combinable]]
  void spi_slave(client spi_slave_callback_if spi_i,
                 in port p_sclk,
                 in buffered port:32 p_mosi,
                 out buffered port:32 ?p_miso,
                 in port p_ss,
                 clock clk,
                 static const spi_mode_t mode,
                 static const spi_transfer_type_t transfer_type);

#endif // _spi_h_
