// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#ifndef RTOS_INFERENCE_ENGINE_H_
#define RTOS_INFERENCE_ENGINE_H_

#include <cstdint>

#include "FlashLoader.hpp"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "xcore_interpreter.h"
#include "xcore_ops.h"
#include "xcore_rtos_ops.h"

#include "rtos_printf.h"

namespace xcore {
namespace rtos {

// typedefs
typedef tflite::Model model_t;
typedef tflite::MicroAllocator micro_allocator_t;
typedef tflite::SimpleMemoryAllocator simple_allocator_t;
typedef tflite::GreedyMemoryPlanner memory_planner_t;
typedef tflite::MicroErrorReporter error_reporter_t;
typedef tflite::MicroOpResolver micro_op_resolver_t;
typedef tflite::MicroProfiler tflite_profiler_t;
typedef tflite::micro::xcore::XCoreProfiler xcore_profiler_t;
typedef tflite::micro::xcore::XCoreInterpreter interpreter_t;

typedef enum InferenceEngineStatus {
  Ok = 0,
  ModelVersionError = 1,
  AllocateTensorsError = 2,
  InvokeError = 3
} InferenceEngineStatus;

typedef struct QuantizationParams {
  float scale;
  int32_t zero_point;
} QuantizationParams;

/**
 * RTOSInferenceEngine class
 *
 * RTOS implementation of the inference engine.
 */
/**
 * @brief Represents the inference engine of an RTOS application.
 *
 * @tparam NUM_OPERATORS    Number of unique operators in the model.
 * @tparam MAX_EVENT_COUNT  Maximum number of profiling events to track.
 *
 */
template <size_t NUM_OPERATORS, size_t MAX_EVENT_COUNT = 64>
class InferenceEngine {
public:
  typedef tflite::MicroMutableOpResolver<NUM_OPERATORS> resolver_t;

  /**
   * @brief Construct a `RTOSInferenceEngine`.
   */
  InferenceEngine()
      : arena_(nullptr), arena_size_(0), model_(nullptr), allocator_(nullptr),
        planner_(nullptr), profiler_(nullptr) {}

  /**
   * @brief Destruct a `RTOSInferenceEngine`.
   */
  ~InferenceEngine() {
    if (interpreter_) {
      delete interpreter_;
      interpreter_ = nullptr;
    }
  }

  /**
   * @brief Initialize a `RTOSInferenceEngine`.
   *
   * @param[in] arena        Array for scratch and activations.
   * @param[in] arena_size   Size (in bytes) of arena array
   */
  tflite::MicroMutableOpResolver<NUM_OPERATORS> *Initialize(uint8_t *arena,
                                                            size_t arena_size) {
    xassert(arena);
    xassert(arena_size > 0);

    arena_ = arena;
    arena_size_ = arena_size;

    // Set up simple allocator
    static simple_allocator_t simple_allocator_s(&reporter_, arena, arena_size);

    // Set up simple allocator
    if (planner_ == nullptr) {
      uint8_t *planner_buffer = simple_allocator_s.AllocateFromTail(
          sizeof(memory_planner_t), alignof(memory_planner_t));
      planner_ = new (planner_buffer) memory_planner_t();
    }

    // Set up micro allocator
    if (allocator_ == nullptr) {
      allocator_ =
          micro_allocator_t::Create(&simple_allocator_s, planner_, &reporter_);
    }

    return &resolver_;
  }

  /**
   * @brief Load a model.
   *
   * @param[in] model_content      Model contents
   */
  InferenceEngineStatus LoadModel(const uint8_t *model_content,
                                  FlashLoader *flash_loader = nullptr) {
    xassert(model_content);

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model_ = tflite::GetModel(model_content);
    if (model_->version() != TFLITE_SCHEMA_VERSION) {
      return ModelVersionError;
    }

#ifndef NDEBUG
    // Get model specific profiler
    if (profiler_ == nullptr) {
      // Set up profiling
      static xcore_profiler_t profiler_s;

      profiler_ = &profiler_s;
      profiler_->Init(allocator_, MAX_EVENT_COUNT);
    }
#endif

    // Build an interpreter to run the model with
    interpreter_ = new (interpreter_buffer_)
        interpreter_t(model_, resolver_, allocator_, &reporter_, planner_, true,
                      profiler_, flash_loader);

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_tensors_status = interpreter_->AllocateTensors();
    if (allocate_tensors_status != kTfLiteOk) {
      return AllocateTensorsError;
    }

    return Ok;
  }

  /**
   * @brief Invoke inference.
   */
  InferenceEngineStatus Invoke() {
    if (profiler_) {
      // Reset the profiler
      profiler_->ClearEvents();
    }

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter_->Invoke();

    if (invoke_status != kTfLiteOk) {
      return InvokeError;
    }

    return Ok;
  }

  /**
   * @brief Print the profile summary.
   */
  void PrintProfilerSummary() {
#ifndef NDEBUG
    uint32_t count = 0;
    uint32_t total = 0;
    uint32_t time_us = 0;
    const uint32_t *durations = nullptr;
    const char *op_name;

    count = profiler_->GetNumEvents();
    durations = profiler_->GetEventDurations();

    size_t subgraph_idx = 0;
    const tflite::SubGraph *subgraph = model_->subgraphs()->Get(subgraph_idx);
    auto *opcodes = model_->operator_codes();
    uint32_t operators_size = NumSubgraphOperators(subgraph);
    const tflite::OpResolver *c_resolver =
        static_cast<const tflite::OpResolver *>(&resolver_);

    for (size_t i = 0; i < operators_size; ++i) {
      if (i < count) {
        const auto *op = subgraph->operators()->Get(i);
        const size_t index = op->opcode_index();
        const auto *opcode = opcodes->Get(index);
        const TfLiteRegistration *registration = nullptr;

        GetRegistrationFromOpCode(opcode, *c_resolver, &reporter_,
                                  &registration);

        if (registration->builtin_code == tflite::BuiltinOperator_CUSTOM) {
          op_name = registration->custom_name;
        } else {
          op_name = tflite::EnumNameBuiltinOperator(
              tflite::BuiltinOperator(registration->builtin_code));
        }
        time_us = durations[i] / PLATFORM_REFERENCE_MHZ;
        total += time_us;
        rtos_printf("Operator %d, %s took %lu microseconds\n", i, op_name,
                    time_us);
      }
    }
    rtos_printf("TOTAL %lu microseconds\n", total);
#endif
  }

  /** Get the model input buffer.
   *
   * @param[in] index      Input index
   *
   * @return    Pointer to model input buffer
   */
  int8_t *GetInputBuffer(size_t index = 0) {
    return interpreter_->input(index)->data.int8;
  }

  /** Get the model input size
   *
   * @param[in] index      Input index
   *
   * @return    Input size (in bytes)
   */
  size_t GetInputSize(size_t index = 0) const {
    return interpreter_->input(index)->bytes;
  }

  /** Get the model input dimension
   *
   * @param[in] dim        Input dimension
   * @param[in] index      Input index
   *
   * @return    Dimension size
   */
  size_t GetInputDimension(size_t dim_index, size_t input_index = 0) const {
    return interpreter_->input(input_index)->dims->data[dim_index];
  }

  /** Get the model input quantization parameters
   *
   * @param[in] index      Input index
   *
   * @return    QuantizationParams struct
   */
  QuantizationParams GetInputQuantization(size_t index = 0) {
    QuantizationParams params;
    params.scale = interpreter_->input(index)->params.scale;
    params.zero_point = interpreter_->input(index)->params.zero_point;

    return params;
  }

  /** Get the model output buffer.
   *
   * @param[in] index      Output index
   *
   * @return    Pointer to model output buffer
   */
  int8_t *GetOutputBuffer(size_t index = 0) {
    return interpreter_->output(index)->data.int8;
  }

  /** Get the model output size
   *
   * @param[in] index      Output index
   *
   * @return    Output size (in bytes)
   */
  size_t GetOutputSize(size_t index = 0) const {
    return interpreter_->output(index)->bytes;
  }

  /** Get the model output quantization parameters
   *
   * @param[in] index      Output index
   *
   * @return    QuantizationParams struct
   */
  QuantizationParams GetOutputQuantization(size_t index = 0) {
    QuantizationParams params;
    params.scale = interpreter_->output(index)->params.scale;
    params.zero_point = interpreter_->output(index)->params.zero_point;

    return params;
  }

private:
  uint8_t *arena_;
  size_t arena_size_;
  const model_t *model_;
  micro_allocator_t *allocator_;
  memory_planner_t *planner_;
  xcore_profiler_t *profiler_;
  interpreter_t *interpreter_;
  resolver_t resolver_;

  error_reporter_t reporter_;
  uint64_t interpreter_buffer_
      [(sizeof(tflite::micro::xcore::XCoreInterpreter) + sizeof(uint64_t) - 1) /
       sizeof(uint64_t)]; // This needs to be aligned on a double word boundary
};

} // namespace rtos
} // namespace xcore

#endif // RTOS_INFERENCE_ENGINE_H_
