// This is a TensorFlow Lite model runner interface that has been
// generated using the generate_model_runner tool.

#include "{header_file}"

#include "tensorflow/lite/micro/kernels/xcore/xcore_interpreter.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_ops.h"
#include "tensorflow/lite/micro/kernels/xcore/xcore_profiler.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/version.h"

tflite::ErrorReporter *reporter = nullptr;
tflite::Profiler *profiler = nullptr;
const tflite::Model *model = nullptr;
tflite::micro::xcore::XCoreInterpreter *interpreter = nullptr;

// static buffer for XCoreInterpreter class allocation
uint8_t interpreter_buffer[sizeof(tflite::micro::xcore::XCoreInterpreter)];

void model_runner_initialize(const uint8_t* model_content, uint8_t* arena, int arena_size)
{{
  // Set up error reporting
  static tflite::MicroErrorReporter error_reporter;
  if (reporter == nullptr) {{
    reporter = &error_reporter;
  }}
  // Set up profiling.
  static tflite::micro::xcore::XCoreProfiler xcore_profiler(reporter);
  if (profiler == nullptr) {{
    profiler = &xcore_profiler;
  }}

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
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
  if (interpreter) {{
    // We already have an interpreter so we need to explicitly call the
    // destructor here but NOT delete the object
    interpreter->~XCoreInterpreter();
  }}
  interpreter = new (interpreter_buffer) tflite::micro::xcore::XCoreInterpreter(
      model, resolver, arena, arena_size, reporter, true,
      profiler);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {{
    TF_LITE_REPORT_ERROR(reporter, "AllocateTensors() failed");
    exit(1);
  }}
}}

int8_t* model_runner_get_input()
{{
  return interpreter->input(0)->data.int8;
}}

size_t model_runner_get_input_size()
{{
  return interpreter->input(0)->bytes;
}}

void model_runner_get_input_quantization(float *scale, int *zero_point)
{{
  *scale = interpreter->input(0)->params.scale;
  *zero_point = interpreter->input(0)->params.zero_point;
}}

void model_runner_invoke()
{{
  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {{
    TF_LITE_REPORT_ERROR(reporter, "Invoke failed\n");
  }}
}}

int8_t* model_runner_get_output()
{{
  return interpreter->output(0)->data.int8;
}}

size_t model_runner_get_output_size()
{{
    return interpreter->output(0)->bytes;
}}

void model_runner_get_output_quantization(float *scale, int *zero_point)
{{
  *scale = interpreter->output(0)->params.scale;
  *zero_point = interpreter->output(0)->params.zero_point;
}}
