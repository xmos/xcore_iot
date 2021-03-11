// Copyright 2020-2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/**
 * This is the API for the RPC library used by the XCORE RTOS drivers.
 * Applications may also use this API directly. Note its usage by the
 * drivers for examples.
 */

#ifndef RTOS_RPC_H_
#define RTOS_RPC_H_

#include <stdint.h>

#include "rtos_intertile.h"

/**
 * Initializes a parameter descriptor for parameters of standard types.
 * For example char, short, int, long, uint32_t, etc. The type must have
 * a size of 1, 2, 4, or 8 bytes. These parameters are inputs to the function.
 *
 * \param param The variable that will be sent to the remote function.
 */
#define RPC_PARAM_TYPE(param)                 {sizeof(typeof(param)),              0, 1, 0}

/**
 * Initializes a parameter descriptor for function return values. The return
 * value must be of a standard type. The type must have a size of 1, 2, 4,
 * or 8 bytes. These parameters are outputs from the function.
 *
 * \param type The type of the return value.
 */
#define RPC_PARAM_RETURN(type)                {sizeof(type),                       0, 0, 1}

/**
 * Initializes a parameter descriptor for a buffer that is treated as const by
 * the remote function. The contents of the buffer will be sent to the remote function,
 * but will not be sent back to the caller.
 *
 * \param param  A pointer to the buffer that will be sent to the remote function.
 * \param length The number of bytes in the buffer that should be sent to the remote function.
 */
#define RPC_PARAM_IN_BUFFER(param, length)    {sizeof(typeof((param)[0])) * (length),  1, 1, 0}

/**
 * Initializes a parameter descriptor for a buffer that is treated as write only by
 * the remote function. The contents of the buffer on the the calling side will not
 * be sent to the remote function. The remote function will send back data to the caller
 * and will be written to this buffer by rpc_response_unmarshall()/rpc_response_unmarshall_va().
 *
 * \param param  A pointer to the buffer that will be written to by the remote function.
 * \param length The number of bytes in the buffer, and the maximum number of bytes that
 *               should be written to it by the remote function.
 */
#define RPC_PARAM_OUT_BUFFER(param, length)   {sizeof(typeof((param)[0])) * (length),  1, 0, 1}

/**
 * Initializes a parameter descriptor for a buffer that is used by the remote function.
 * The contents of the buffer is treated as read/write by the remote function. The contents
 * of the buffer will be sent to the remote function and will also be sent back data to the
 * caller.
 *
 * \param param  A pointer to the buffer that will be used by the remote function.
 * \param length The number of bytes in the buffer that will be sent to the remote function,
 *               and the maximum number of bytes that it should write to it.
 */
#define RPC_PARAM_INOUT_BUFFER(param, length) {sizeof(typeof((param)[0])) * (length),  1, 1, 1}

/**
 * Initializes the last parameter descriptor in a parameter descriptor list. Must be the last
 * item in a parameter descriptor list provided to rpc_request_marshall_va() and
 * rpc_response_unmarshall_va().
 */
#define RPC_PARAM_LIST_END                    {0,                                  0, 0, 0}

/**
 * RPC parameter descriptor. Initialize this with one of the RPC_PARAM macros.
 * It is used to describe the parameters of RPC functions.
 */
typedef struct {
    uint32_t length  : 24; /**< The length in bytes of the parameter itself, or the data it points to if ptr is 1 */
    uint8_t  ptr     :  1; /**< Set to 1 if the parameter is a pointer */
    uint8_t  input   :  1; /**< Set to 1 if the parameter data is input to the function */
    uint8_t  output  :  1; /**< Set to 1 if the parameter data is output from the function */
} rpc_param_desc_t;

/**
 * The structure describing both RPC request and response messages. It is
 * populated by rpc_request_parse() and rpc_response_parse().
 */
typedef struct {
    int fcode;        /**< An enumerator that identifies the function */
    int param_count;  /**< The number of parameters the function takes. Only populated by rpc_request_parse() */
    rpc_param_desc_t *param_desc; /**< A list of parameter descriptors for the function. Only populated by rpc_request_parse() */
    void *params;     /**< Pointer to the beginning of the receieved parameter values */
    void *msg_buf;    /**< Pointer to the beginning of the receieved RPC message buffer */
} rpc_msg_t;

/**
 * Creates an RPC request message to call a remote function. See also rpc_client_call_generic().
 *
 * \param[out] msg        The RPC request message that can be sent to the remote tile. A buffer is allocated
 *                        for it and a pointer to it is returned via this parameter. It must be freed with
 *                        rtos_osal_free() when it is no longer needed.
 * \param[in]  fcode      The function code enumerator. This is used by the remote tile to determine which function
 *                        to execute.
 * \param[in]  param_desc Parameter descriptor list. This describes each of the remote function's parameters,
 *                        as well as its return value(s).
 * \param[in]  ap         The arguments to pass to the remote function. They must be in the same order and match
 *                        the parameters described in the list \p param_desc. Each must be a pointer to the argument
 *                        data.
 *
 * \returns The length in bytes of the RPC message created and returned via \p msg.
 *
 */
int rpc_request_marshall_va(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], va_list ap);

/**
 * This is the same as rpc_request_marshall_va(), except that it takes a variable number
 * of arguments for the remote function arguments, rather than a va_list of them.
 *
 * \param[out] msg        The RPC request message that can be sent to the remote tile. A buffer is allocated
 *                        for it and a pointer to it is returned via this parameter. It must be freed with
 *                        rtos_osal_free() when it is no longer needed.
 * \param[in]  fcode      The function code enumerator. This is used by the remote tile to determine which function
 *                        to execute.
 * \param[in]  param_desc Parameter descriptor list. This describes each of the remote function's parameters,
 *                        as well as its return value(s).
 * \param[in]  ...        The arguments to pass to the remote function. They must be in the same order and match
 *                        the parameters described in the list \p param_desc. Each must be a pointer to the argument
 *                        data.
 *
 * \returns The length in bytes of the RPC message created and returned via \p msg.
 *
 */
int rpc_request_marshall(uint8_t **msg, int fcode, const rpc_param_desc_t param_desc[], ...);

/**
 * Parses a received RPC request message and fills in a provided rpc_msg_t struct.
 *
 * \param[out] rpc_msg A pointer to an rpc_msg_t struct to fill in. This can subsequently be passed to either
 *                     rpc_request_unmarshall_va() or rpc_request_unmarshall().
 * \param[in]  msg_bug A pointer to the received RPC request message.
 */
void rpc_request_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf);

/**
 * Retrieves the arguments from a parsed RPC request message into a list of parameters.
 *
 * \param[in]  rpc_msg A pointer to an rpc_msg_t struct that has already been filled in by rpc_request_parse().
 * \param[out] ap      The arguments to pass to the function corresponding to rpc_msg->fcode. They must be in
 *                     the same order and match the parameters that will be passed to the function. Each must
 *                     be a pointer to the argument. Their values are populated with the argument data in the
 *                     received RPC request message.
 */
void rpc_request_unmarshall_va(rpc_msg_t *rpc_msg, va_list ap);

/**
 * This is the same as rpc_request_unmarshall_va(), except that it takes a variable number
 * of arguments for the function arguments, rather than a va_list of them.
 *
 * \param[in]  rpc_msg A pointer to an rpc_msg_t struct that has already been filled in by rpc_request_parse().
 * \param[out] ...     The arguments to pass to the function corresponding to rpc_msg->fcode. They must be in
 *                     the same order and match the parameters that will be passed to the function. Each must
 *                     be a pointer to the argument. Their values are populated with the argument data in the
 *                     received RPC request message.
 */
void rpc_request_unmarshall(rpc_msg_t *rpc_msg, ...);

/**
 * Creates an RPC response message to send back a function's return value and any output buffer data to the remote caller.
 *
 * \param[out] msg        The RPC response message that can be sent to the remote tile. A buffer is allocated
 *                        for it and a pointer to it is returned via this parameter. It must be freed with
 *                        rtos_osal_free() when it is no longer needed.
 * \param[in]  rpc_msg    A pointer to an rpc_msg_t struct that has already been filled in by rpc_request_parse().
 * \param[in]  ap         The arguments that were passed to the called function. They must be in the same order
 *                        and match the parameters that were passed to the function. Note that these should be the
 *                        arguments as passed to the function, not pointers to them.
 *
 * \returns The length in bytes of the RPC message created and returned via \p msg.
 *
 */
int rpc_response_marshall_va(uint8_t **msg, const rpc_msg_t *rpc_msg, va_list ap);

/**
 * This is the same as rpc_response_marshall_va(), except that it takes a variable number
 * of arguments for the function arguments, rather than a va_list of them.
 *
 * \param[out] msg        The RPC response message that can be sent to the remote tile. A buffer is allocated
 *                        for it and a pointer to it is returned via this parameter. It must be freed with
 *                        rtos_osal_free() when it is no longer needed.
 * \param[in]  rpc_msg    A pointer to an rpc_msg_t struct that has already been filled in by rpc_request_parse().
 * \param[in]  ap         The arguments that were passed to the called function. They must be in the same order
 *                        and match the parameters that were passed to the function. Note that these should be the
 *                        arguments as passed to the function, not pointers to them.
 *
 * \returns The length in bytes of the RPC message created and returned via \p msg.
 *
 */
int rpc_response_marshall(uint8_t **msg, const rpc_msg_t *rpc_msg, ...);

/**
 * Parses a received RPC response message and fills in a provided rpc_msg_t struct. See also rpc_client_call_generic().
 *
 * \param[out] rpc_msg A pointer to an rpc_msg_t struct to fill in. This can subsequently be passed to either
 *                     rpc_response_unmarshall_va() or rpc_response_unmarshall().
 * \param[in]  msg_bug A pointer to the received RPC response message.
 */
void rpc_response_parse(rpc_msg_t *rpc_msg, uint8_t *msg_buf);

/**
 * Retrieves the return value and output buffer data from a parsed RPC response message into a list of parameters.
 * See also rpc_client_call_generic().
 *
 * \param[in]  rpc_msg A pointer to an rpc_msg_t struct that has already been filled in by rpc_response_parse().
 * \param[out] ap      The arguments passed to the remote function. They must be in the same order and match
 *                     the parameters described in the parameter descriptor list passed to rpc_request_marshall_va()
 *                     or rpc_request_marshall(). Each must be a pointer to the argument data. This must be the
 *                     same list as passed to rpc_request_marshall_va() or rpc_request_marshall().
 */
void rpc_response_unmarshall_va(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], va_list ap);

/**
 * This is the same as rpc_response_unmarshall_va(), except that it takes a variable number
 * of arguments for the remote function arguments, rather than a va_list of them.
 *
 * \param[in]  rpc_msg A pointer to an rpc_msg_t struct that has already been filled in by rpc_response_parse().
 * \param[out] ...     The arguments passed to the remote function. They must be in the same order and match
 *                     the parameters described in the parameter descriptor list passed to rpc_request_marshall_va()
 *                     or rpc_request_marshall(). Each must be a pointer to the argument data. This must be the
 *                     same list as passed to rpc_request_marshall_va() or rpc_request_marshall().
 */
void rpc_response_unmarshall(const rpc_msg_t *rpc_msg, const rpc_param_desc_t param_desc[], ...);

/**
 * Calls a remote function given a function code enumerator, a parameter descriptor list, and an intertile instance
 * that has already been started.
 *
 * This function may be used to call a remote function rather than the three separate functions rpc_request_marshall_va(),
 * rpc_response_parse(), and rpc_response_unmarshall_va(), as the sequence is generic enough to handle most remote functions.
 *
 * \param[in] intertile_ctx An intertile driver instance that has already been initialized and started, and is connected
 *                          to the tile that hosts the remote function.
 * \param[in] port          The intertile port to send the request to, and listen for the response from.
 * \param[in] fcode         The function code enumerator. This is used by the remote tile to determine which function
 *                          to execute.
 * \param[in,out] ...       The arguments to pass to the remote function. They must be in the same order and match
 *                          the parameters described in the list \p param_desc. Each must be a pointer to the argument
 *                          data. Input argument data will be copied into the request message and sent to the remote
 *                          function. Output argument data will be received and copied to the argument pointers.
 */
void rpc_client_call_generic(rtos_intertile_t *intertile_ctx, uint8_t port, int fcode, const rpc_param_desc_t param_desc[], ...);

#endif /* RTOS_RPC_H_ */
