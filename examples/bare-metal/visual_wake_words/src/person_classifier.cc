// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "person_classifier.h"

#include <ctime>

#include <platform.h> // for PLATFORM_REFERENCE_MHZ
#include <xcore/assert.h>

#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "xcore_interpreter.h"
#include "xcore_ops.h"

constexpr int operator_count = 5;

// typedefs
typedef tflite::Model model_t;
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::SimpleMemoryAllocator simple_allocator_t;
typedef tflite::GreedyMemoryPlanner memory_planner_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::MicroOpResolver micro_op_resolver_t;
typedef tflite::MicroMutableOpResolver<operator_count> resolver_t;
typedef tflite::MicroProfiler tflite_profiler_t;
typedef tflite::micro::xcore::XCoreProfiler xcore_profiler_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;

// static variables
static resolver_t resolver_s;
static error_reporter_t error_reporter_s;

static const model_t *model = nullptr;
static resolver_t *resolver = nullptr;
static error_reporter_t *reporter = nullptr;
static micro_allocator_t *allocator = nullptr;
static memory_planner_t *planner = nullptr;
static xcore_profiler_t *profiler = nullptr;
static interpreter_t *interpreter = nullptr;

static int8_t* interpreter_buffer[sizeof(interpreter_t)];

extern "C" void DebugLog(const char *s) {
  while (*s) {
    putchar(*s);
    s++;
  }
} 

void person_classifier_init(uint8_t *arena, size_t arena_size) {
  xassert(arena);
  xassert(arena_size > 0);

  // Set up error reporting
  if (reporter == nullptr) {
    reporter = &error_reporter_s;
  }

  // Set up simple allocator
  static simple_allocator_t simple_allocator_s(reporter, arena, arena_size);

  // Set up simple allocator
  if (planner == nullptr) {
    uint8_t *planner_buffer = simple_allocator_s.AllocateFromTail(sizeof(memory_planner_t), alignof(memory_planner_t));
    planner = new (planner_buffer) memory_planner_t();    
  }

  // Set up micro allocator
  if (allocator == nullptr) {
    allocator = micro_allocator_t::Create(&simple_allocator_s, planner, reporter);
  }
}

ModelRunnerStatus person_classifier_allocate(const uint8_t *model_content) {
  xassert(model_content);

  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(model_content);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    return ModelVersionError;
  }

  // Setup op resolver
  if (resolver == nullptr) {
     resolver = &resolver_s;
     resolver->AddSoftmax();
     resolver->AddPad();
     resolver->AddConv2D();
     resolver->AddReshape();
     resolver->AddCustom(tflite::ops::micro::xcore::Conv2D_V2_OpCode,
                         tflite::ops::micro::xcore::Register_Conv2D_V2());
  }

  // Get model specific profiler
#ifndef NDEBUG  
  if (profiler == nullptr) {
    // Set up profiling
    static xcore_profiler_t profiler_s;

    profiler = &profiler_s;

  }
  profiler->Init(allocator, 32);
#endif

  // Build an interpreter to run the model with
  interpreter = new (interpreter_buffer)
      interpreter_t(model, *resolver, allocator, reporter, planner, true, profiler, nullptr);

  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_tensors_status = interpreter->AllocateTensors();
  if (allocate_tensors_status != kTfLiteOk) {
    return AllocateTensorsError;
  }

  return Ok;
}

int8_t *person_classifier_input_buffer_get() {
  return interpreter->input(0)->data.int8;
}

size_t person_classifier_input_size_get() {
  return interpreter->input(0)->bytes;
}

void person_classifier_input_quant_get(float *scale,
                                  int *zero_point) {
  xassert(scale);
  xassert(zero_point);

  *scale = interpreter->input(0)->params.scale;
  *zero_point = interpreter->input(0)->params.zero_point;
}

int8_t *person_classifier_output_buffer_get() {
  return interpreter->output(0)->data.int8;
}

size_t person_classifier_output_size_get() {
  return interpreter->output(0)->bytes;
}

void person_classifier_output_quant_get(float *scale,
                                   int *zero_point) {
  xassert(scale);
  xassert(zero_point);

  *scale = interpreter->output(0)->params.scale;
  *zero_point = interpreter->output(0)->params.zero_point;
}


#ifndef NDEBUG

void person_classifier_profiler_summary_print() {
  uint32_t count = 0;
  uint32_t total = 0;
  uint32_t time_us = 0;
  const uint32_t *durations = nullptr;
  const char *op_name;

  count = profiler->GetNumEvents();
  durations = profiler->GetEventDurations();

  size_t subgraph_idx = 0;
  const tflite::SubGraph *subgraph = model->subgraphs()->Get(subgraph_idx);
  auto *opcodes = model->operator_codes();
  uint32_t operators_size = NumSubgraphOperators(subgraph);
  const tflite::OpResolver *c_resolver =
      static_cast<const tflite::OpResolver *>(resolver);

  for (size_t i = 0; i < operators_size; ++i) {
    if (i < count) {
      const auto *op = subgraph->operators()->Get(i);
      const size_t index = op->opcode_index();
      const auto *opcode = opcodes->Get(index);
      const TfLiteRegistration *registration = nullptr;

      GetRegistrationFromOpCode(opcode, *c_resolver, reporter, &registration);

      if (registration->builtin_code == tflite::BuiltinOperator_CUSTOM) {
        op_name = registration->custom_name;
      } else {
        op_name = tflite::EnumNameBuiltinOperator(
            tflite::BuiltinOperator(registration->builtin_code));
      }
      time_us = durations[i] / PLATFORM_REFERENCE_MHZ;
      total += time_us;
      printf("Operator %d, %s took %lu microseconds\n", i, op_name, time_us);
    }
  }
  printf("TOTAL %lu microseconds\n", total);
}

#endif

ModelRunnerStatus person_classifier_invoke() {
  if (profiler) {
    // Reset the profiler
    profiler->ClearEvents();
  }

  // Run inference, and report any error
  TfLiteStatus invoke_status = interpreter->Invoke();

  if (invoke_status != kTfLiteOk) {
    return InvokeError;
  }

  return Ok;
}