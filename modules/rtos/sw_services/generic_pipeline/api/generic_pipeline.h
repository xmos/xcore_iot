// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

#ifndef RTOS_SW_SERVICES_GENERIC_PIPELINE_H_
#define RTOS_SW_SERVICES_GENERIC_PIPELINE_H_

/**
 * \addtogroup generic_pipeline generic_pipeline
 *
 * The public API for using the generic pipeline implementation.
 * @{
 */

/**
 * Function pointer type for application provided generic pipeline input callback
 * functions.
 *
 * Called by the first generic_pipeline_stage() when the stage wants input data.
 * This data pointer is provided to the first stage function to be processed.
 *
 * \param  input_data      A pointer to application specific data
 *
 * \returns                A frame pointer to be used by the pipeline stages
 */
typedef void * (*pipeline_input_t)(void *input_data);

/**
 * Function pointer type for application provided generic pipeline output callback
 * functions.
 *
 * Called by the last generic_pipeline_stage() when the stage wants is done
 * processing the data.
 *
 * \param  data                A pointer to the processed data
 * \param  output_data         A pointer to application specific data
 *
 * \returns                    0, to take ownership of data pointer
 *                             otherwise, request the generic pipeline to free
 *                               data internally
 */
typedef int (*pipeline_output_t)(void *data, void *output_data);

/**
 * Function pointer type for application provided generic pipeline stage callback
 * functions.
 *
 * Called by each generic_pipeline_stage() after input data is received.
 *
 * \param  data     A pointer to the data.  This buffer is used for both
 *                  input and output.
 */
typedef void (*pipeline_stage_t)(void *data);

/**
 * Create a multistage generic pipeline.
 *
 * This function will create a multistage pipeline, creating a task per stage
 * and connecting them via queues.  Each stage task follows the convention:
 *   - Get input data
 *   - Process data
 *   - Push output data
 *
 * For the first stage, the input data are the provided by the input callback.
 * For the final stage, the output data are provided to the output callback.
 *
 * \param input                   A function pointer called to get input data
 * \param output                  A function pointer called to give output data
 * \param input_data              A pointer to application specific data to pass
 *                                to the input callback function
 * \param output_data             A pointer to application specific data to pass
 *                                to the output callback function
 * \param stage_functions         An array of stage function pointers
 * \param stage_stack_word_sizes  The stack size of each stage.
 *                                Note: For the first stage must contain enough
 *                                stack for the stage function + input function.
 *                                Likewise, the last stage must contain enough
 *                                stack for the stage function + output function.
 * \param pipeline_priority       The priority of all pipeline tasks
 * \param stage_count             The number of stages.  The limit is 10 stages.
 */
void generic_pipeline_init(
		const pipeline_input_t input,
		const pipeline_output_t output,
		void * const input_data,
		void * const output_data,
		const pipeline_stage_t * const stage_functions,
		const size_t * const stage_stack_word_sizes,
		const int pipeline_priority,
		const int stage_count);

/**@}*/

#endif /* RTOS_SW_SERVICES_GENERIC_PIPELINE_H_ */
