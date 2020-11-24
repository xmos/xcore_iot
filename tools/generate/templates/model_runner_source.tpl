// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#include "{header_file}"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

// shorthand typedefs
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::SimpleMemoryAllocator simple_allocator_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;
typedef tflite::micro::xcore::XCoreProfiler profiler_t;

// static variables
static micro_allocator_t *allocator = nullptr;
static error_reporter_t *reporter = nullptr;
static profiler_t *profiler = nullptr;

void model_runner_init(uint8_t* arena, int arena_size)
{{
  // Set up error reporting
  static error_reporter_t error_reporter_s;
  if (reporter == nullptr) {{
    reporter = &error_reporter_s;
  }}

  // Set up allocator
  static simple_allocator_t simple_allocator_s(reporter, arena, arena_size);
  if (allocator == nullptr) {{
    allocator = micro_allocator_t::Create(&simple_allocator_s, reporter);
  }}

  // Set up profiling
  static profiler_t profiler_s(reporter);
  if (profiler == nullptr) {{
    profiler = &profiler_s;
  }}
}}

void model_runner_create(model_runner_t *ctx, const uint8_t* model_content)
{{
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  const tflite::Model* model;
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {{
    TF_LITE_REPORT_ERROR(reporter,
                         "Model provided is schema version %d not equal "
                         "to supported version %d.",
                         model->version(), TFLITE_SCHEMA_VERSION);
    exit(1);
  }}

  // This pulls in all the operation implementations we need.
  static tflite::MicroMutableOpResolver<{operator_count}> resolver;
{operator_list}

  // Build an interpreter to run the model with
  void *interpreter_buffer =
      allocator->AllocatePersistentBuffer(sizeof(interpreter_t));
  interpreter_t *interpreter = new (interpreter_buffer) interpreter_t(
    model, resolver, allocator, reporter, true,
    profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {{
    TF_LITE_REPORT_ERROR(reporter, "AllocateTensors() failed");
    exit(1);
  }}

  ctx->handle = static_cast<void*>(interpreter);
}}

int8_t* model_runner_get_input(model_runner_t *ctx)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->input(0)->data.int8;
}}

size_t model_runner_get_input_size(model_runner_t *ctx)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->input(0)->bytes;
}}

void model_runner_get_input_quant(model_runner_t *ctx, float *scale, int *zero_point)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  *scale = interpreter->input(0)->params.scale;
  *zero_point = interpreter->input(0)->params.zero_point;
}}

void model_runner_invoke(model_runner_t *ctx)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {{
    TF_LITE_REPORT_ERROR(reporter, "Invoke failed\n");
  }}
}}

int8_t* model_runner_get_output(model_runner_t *ctx)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->output(0)->data.int8;
}}

size_t model_runner_get_output_size(model_runner_t *ctx)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  return interpreter->output(0)->bytes;
}}

void model_runner_get_output_quant(model_runner_t *ctx, float *scale, int *zero_point)
{{
  interpreter_t *interpreter = static_cast<interpreter_t *>(ctx->handle);
  *scale = interpreter->output(0)->params.scale;
  *zero_point = interpreter->output(0)->params.zero_point;
}}
