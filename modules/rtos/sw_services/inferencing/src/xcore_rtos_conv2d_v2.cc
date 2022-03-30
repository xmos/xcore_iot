// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.
#include "Conv2d.hpp"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/kernels/internal/tensor_ctypes.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/micro/kernels/kernel_util.h"
#include "xcore_custom_options.h"
#include "xcore_interpreter.h"
#include "xcore_utils.h"
extern "C" {
#include "nn_operator.h"
}

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
namespace rtos {
namespace conv_v2 {

// -------------------------------------------------------------------- //
// thread data type and worker functions
// -------------------------------------------------------------------- //

struct Conv2DShared {
  int8_t *X;
  int8_t *Y;
  nn::AbstractKernel *f;
};

extern "C" {
// TODO
#pragma stackfunction 1000
void rtos_conv2d_v2_thread_worker(void *shard, void *scrtch, void *kp) {
  nn::AbstractKernel::Params *kparams = (nn::AbstractKernel::Params *)kp;
  auto scratch = static_cast<int8_t *>(scrtch);
  auto shared = static_cast<Conv2DShared *>(shard);
  execute(shared->Y, shared->X, shared->f, kparams, scratch);
}
}

// -------------------------------------------------------------------- //
// op data types
// -------------------------------------------------------------------- //

enum KernelType {
  Conv2DValidDirect_t,
  Conv2DValidIndirect_t,
  Conv2DPaddedIndirect_t,
  DepthwiseConv2DValidDirect_t,
  DepthwiseConv2DPaddedIndirect_t,
  BNNConv2DValidDirectBinary_t,
  BNNConv2DValidIndirectBinary_t,
  BNNConv2DValidDirectInt8_t,
  BNNConv2DValidIndirectInt8_t
};

/**
 * @brief This describes the memory requirements of a worker thread. It also
 * includes an array of the work to be done by said worker.
 *
 */
struct Conv2DThreadInfo {
  size_t scratch_size;     // Each thread needs a scratch
  int stack_scratch_index; // All threads stack and scratch consolidated into a
                           // single scratch buffer
  // TODO: Clean up
  // Using AbstractKernel to be able to assign Filter2D or Filter2D_DW
  nn::AbstractKernel::Params *kparams;
};

// This is the struct that contains the data required to fully describe the work
// that the operator will perform. It needs to descibe the work for T threads.
// That means it must contain:
// - T sets of work, i.e. a list of jobs for each thread.
// - T scratch allocations, i.e. an amount of scratch memory for each thread.
struct Conv2DOpData
    : XCoreOpData { // Inherits the operator name field from XCoreOpData
  size_t thread_count;
  Conv2DThreadInfo *threads;
  KernelType kt;
  nn::AbstractKernel *filter2D; // The job to be done by this thread
};

// -------------------------------------------------------------------- //
// op function implementations
// -------------------------------------------------------------------- //

template <typename T>
T *getDeserializedParams(TfLiteContext *context, const uint8_t *data) {
  char *allocated_memory;
  int allocationByteCount = sizeof(T);
  allocated_memory =
      (char *)context->AllocatePersistentBuffer(context, allocationByteCount);
  T *param = T::template deserialise<T>(allocated_memory, (const char *)data);
  return param;
}

// Construct Filter2D threads
template <typename Conv2DType, typename MfType, typename AggType,
          typename OtType, typename AkType>
void ConstructFilter2DsImpl(Conv2DOpData *op_data, TfLiteContext *context,
                            const int scratch_size,
                            const uint8_t *memcpy_fn_data,
                            const uint8_t *agg_fn_data, OtType *ot,
                            flexbuffers::Vector &ak_params_vec) {
  typename MfType::Params *mf_params =
      getDeserializedParams<typename MfType::Params>(context, memcpy_fn_data);
  typename AggType::Params *af_params =
      getDeserializedParams<typename AggType::Params>(context, agg_fn_data);
  auto memcpy = new (context->AllocatePersistentBuffer(context, sizeof(MfType)))
      MfType(mf_params);
  auto aggregator =
      new (context->AllocatePersistentBuffer(context, sizeof(AggType)))
          AggType(af_params);

  // For each thread, we have a different set of abstract kernel params which we
  // extract here
  // We reuse the other params
  auto conv2d =
      new (context->AllocatePersistentBuffer(context, sizeof(Conv2DType)))
      Conv2DType(memcpy, aggregator, ot);
  op_data->filter2D = conv2d;
  for (int t = 0; t < op_data->thread_count; ++t) {
    op_data->threads[t].scratch_size = scratch_size;
    typename AkType::Params *ak_params =
        getDeserializedParams<typename AkType::Params>(
            context, ak_params_vec[t].AsBlob().data());
    op_data->threads[t].kparams = ak_params;
  }
}

// Specialised for the binary output cases
// Forwards into the ConstructFilter2DsImpl implementation function
template <typename Conv2DType, typename MfType, typename AggType,
          typename OtType, typename AkType, bool binaryOutput>
void ConstructFilter2Ds(Conv2DOpData *op_data, TfLiteContext *context,
                        const int scratch_size,
                        const uint8_t *memcpy_fn_data,
                        const uint8_t *agg_fn_data, const uint8_t *ot_fn_data,
                        flexbuffers::Vector &ak_params_vec) {
  // For binary output, we don't have output transform function params
  auto ot =
      new (context->AllocatePersistentBuffer(context, sizeof(OtType))) OtType();
  ConstructFilter2DsImpl<Conv2DType, MfType, AggType, OtType, AkType>(
      op_data, context, scratch_size, memcpy_fn_data, agg_fn_data, ot,
      ak_params_vec);
}

// Forwards into the ConstructFilter2DsImpl implementation function
// For binary output, we don't have output transform params, and so we need to
// specialise that case separately
template <typename Conv2DType, typename MfType, typename AggType,
          typename OtType, typename AkType>
void ConstructFilter2Ds(Conv2DOpData *op_data, TfLiteContext *context,
                        const int scratch_size,
                        const uint8_t *memcpy_fn_data,
                        const uint8_t *agg_fn_data, const uint8_t *ot_fn_data,
                        flexbuffers::Vector &ak_params_vec) {
  typename OtType::Params *ot_params =
      getDeserializedParams<typename OtType::Params>(context, ot_fn_data);
  auto ot = new (context->AllocatePersistentBuffer(context, sizeof(OtType)))
      OtType(ot_params);
  ConstructFilter2DsImpl<Conv2DType, MfType, AggType, OtType, AkType>(
      op_data, context, scratch_size, memcpy_fn_data, agg_fn_data, ot,
      ak_params_vec);
}

void *Init(TfLiteContext *context, const char *buffer, size_t length) {
  TFLITE_DCHECK(buffer != nullptr);
  auto op_data = construct_persistent_object<Conv2DOpData>(context);
  auto parser = CustomOptionParser(buffer, length);

  KernelType kt = (KernelType)parser.parseNamedCustomOption("kt").AsInt32();
  const uint8_t *memcpy_fn_data =
      parser.parseNamedCustomOption("mp").AsBlob().data();
  const uint8_t *agg_fn_data =
      parser.parseNamedCustomOption("aggp").AsBlob().data();
  const uint8_t *ot_fn_data =
      parser.parseNamedCustomOption("otp").AsBlob().data();
  int32_t scratch_size = parser.parseNamedCustomOption("scratch").AsInt32();
  auto ak_params_vec = parser.parseNamedCustomOption("akp").AsVector();

  auto thread_count = ak_params_vec.size();
  op_data->kt = kt;
  op_data->thread_count = thread_count;
  op_data->threads =
      static_cast<Conv2DThreadInfo *>(context->AllocatePersistentBuffer(
          context, op_data->thread_count * sizeof(Conv2DThreadInfo)));

  switch (kt) {
  case Conv2DValidDirect_t: {
    ConstructFilter2Ds<nn::Conv2dValidDirect, nn::DerefInputFn,
                       nn::MatMulDirectFn, nn::OT_int8, nn::Filter2D>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_Conv2DValidDir";
  } break;
  case Conv2DValidIndirect_t: {
    ConstructFilter2Ds<nn::Conv2dValidIndirect, nn::ImToColValid,
                       nn::MatMulInt8, nn::OT_int8, nn::Filter2D>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_Conv2DValidInd";
  } break;
  case Conv2DPaddedIndirect_t: {
    ConstructFilter2Ds<nn::Conv2dPaddedInDirect, nn::ImToColPadded,
                       nn::MatMulInt8, nn::OT_int8, nn::Filter2D>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_Conv2DPadInd";
  } break;
  case DepthwiseConv2DValidDirect_t: {
    ConstructFilter2Ds<nn::Conv2dDepthwiseValidDirect, nn::DerefInputFn,
                       nn::MatMulDirectFn_DW, nn::OT_int8, nn::Filter2D_DW>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_DWConv2DValidInd";
  } break;
  case DepthwiseConv2DPaddedIndirect_t: {
    ConstructFilter2Ds<nn::Conv2dDepthwisePaddedIndirect, nn::ImToColPadded,
                       nn::MatMulDirectFn_DW, nn::OT_int8, nn::Filter2D_DW>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_DWConv2DPadInd";
  } break;
  case BNNConv2DValidDirectBinary_t: {
    ConstructFilter2Ds<nn::BNNConv2dValidDirectBinary, nn::DerefInputFn,
                       nn::MatMulBinaryDirectFn, nn::OT_binary, nn::Filter2D,
                       /*binaryOutput=*/true>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_BNNValidDirBin";
  } break;
  case BNNConv2DValidIndirectBinary_t: {
    ConstructFilter2Ds<nn::BNNConv2dValidIndirectBinary, nn::ImToColValid,
                       nn::MatMulBinary, nn::OT_binary, nn::Filter2D,
                       /*binaryOutput=*/true>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_BNNValidIndBin";
  } break;
  case BNNConv2DValidDirectInt8_t: {
    ConstructFilter2Ds<nn::BNNConv2dValidDirectInt8, nn::DerefInputFn,
                       nn::MatMulBinaryDirectFn, nn::OT_int8_clamped,
                       nn::Filter2D>(op_data, context, scratch_size,
                                     memcpy_fn_data, agg_fn_data, ot_fn_data,
                                     ak_params_vec);
    op_data->name = "XC_BNNValidDirInt8";
  } break;
  case BNNConv2DValidIndirectInt8_t: {
    ConstructFilter2Ds<nn::BNNConv2dValidIndirectInt8, nn::ImToColValid,
                       nn::MatMulBinary, nn::OT_int8_clamped, nn::Filter2D>(
        op_data, context, scratch_size, memcpy_fn_data, agg_fn_data,
        ot_fn_data, ak_params_vec);
    op_data->name = "XC_BNNValidIndInt8";
  } break;
  }
  return op_data;
}

TfLiteStatus Prepare(TfLiteContext *context, TfLiteNode *node) {
  auto *op_data = reinterpret_cast<Conv2DOpData *>(node->user_data);
  for (int t = 0; t < op_data->thread_count; ++t) {
    TF_LITE_ENSURE_STATUS(context->RequestScratchBufferInArena(
        context, op_data->threads[t].scratch_size, &op_data->threads[t].stack_scratch_index));
  }
  return kTfLiteOk;
}

TfLiteStatus Eval(TfLiteContext *context, TfLiteNode *node) {
  const TfLiteEvalTensor *input = tflite::micro::GetEvalInput(context, node, 0);
  TfLiteEvalTensor *output = tflite::micro::GetEvalOutput(context, node, 0);
  const TfLiteEvalTensor *weights_tensor =
      tflite::micro::GetEvalInput(context, node, 1);
  const TfLiteEvalTensor *multipliers_and_biases_tensor =
      tflite::micro::GetEvalInput(context, node, 2);
  tflite::micro::xcore::XCoreInterpreter *xint =
      reinterpret_cast<tflite::micro::xcore::XCoreInterpreter *>(
          context->impl_);

  auto *op_data = reinterpret_cast<Conv2DOpData *>(node->user_data);
  int n_threads = op_data->thread_count;

  int8_t *weights =
      (int8_t *)tflite::micro::GetTensorData<int8_t>(weights_tensor);
  int16_t *multipliers_and_biases =
      (int16_t *)tflite::micro::GetTensorData<int16_t>(
          multipliers_and_biases_tensor);

  int8_t *thread_scratch[n_threads];
  Conv2DShared shared_data;
  shared_data.X = (int8_t *)tflite::micro::GetTensorData<int8_t>(input);
  shared_data.Y = (int8_t *)tflite::micro::GetTensorData<int8_t>(output);
  shared_data.f = op_data->filter2D;
  for (int t = 0; t < n_threads; ++t) {
    thread_scratch[t] = (int8_t *)context->GetScratchBuffer(
        context, op_data->threads[t].stack_scratch_index);
  }

  switch (op_data->kt) {
  case Conv2DValidDirect_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulDirectFn *aggr = (nn::MatMulDirectFn *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
    ot->setMultipliersAndBiases(multipliers_and_biases);
  } break;
  case Conv2DValidIndirect_t:
  case Conv2DPaddedIndirect_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulInt8 *aggr = (nn::MatMulInt8 *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
    ot->setMultipliersAndBiases(multipliers_and_biases);
  } break;
  case DepthwiseConv2DPaddedIndirect_t:
  case DepthwiseConv2DValidDirect_t: {
    nn::Filter2D_DW *f = (nn::Filter2D_DW *)op_data->filter2D;
    nn::MatMulDirectFn_DW *aggr =
        (nn::MatMulDirectFn_DW *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
    ot->setMultipliersAndBiases(multipliers_and_biases);
  } break;
  case BNNConv2DValidDirectBinary_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulBinaryDirectFn *aggr =
        (nn::MatMulBinaryDirectFn *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_binary *ot = (nn::OT_binary *)(f->ot_handler);
    ot->setThresholds(multipliers_and_biases);
  } break;
  case BNNConv2DValidIndirectBinary_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulBinary *aggr = (nn::MatMulBinary *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_binary *ot = (nn::OT_binary *)(f->ot_handler);
    ot->setThresholds(multipliers_and_biases);
  } break;
  case BNNConv2DValidDirectInt8_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulBinaryDirectFn *aggr =
        (nn::MatMulBinaryDirectFn *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_int8_clamped *ot = (nn::OT_int8_clamped *)(f->ot_handler);
    ot->setOffsetsMultipliersAndBiases(multipliers_and_biases);
  } break;
  case BNNConv2DValidIndirectInt8_t: {
    nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
    nn::MatMulBinary *aggr = (nn::MatMulBinary *)(f->aggregate_handler);
    aggr->setWeights(weights);
    nn::OT_int8_clamped *ot = (nn::OT_int8_clamped *)(f->ot_handler);
    ot->setOffsetsMultipliersAndBiases(multipliers_and_biases);
  } break;
  }
  // todo - this second for-loop is unpleasant
  // for (int t = 0; t < n_threads-1; ++t) {
  //   thread_variable_setup(thread_scratch[t], op_data->threads[t].kparams, xint->thread_info.thread_ids.id[t]);
  // }
  // // Now set up shared data, shared function pointer, and data for final thread.
  // thread_call((void *)&shared_data, thread_scratch[n_threads-1], op_data->threads[n_threads-1].kparams,
  //             (thread_function_pointer_t)conv2d_v2_thread_worker, &xint->thread_info);
  rtos_conv2d_v2_thread_worker((void *)&shared_data, thread_scratch[n_threads-1], op_data->threads[n_threads-1].kparams);

  return kTfLiteOk;
}

} // namespace conv_v2

TfLiteRegistration *Register_Conv2D_V2() {
  static TfLiteRegistration r = {conv_v2::Init, nullptr, conv_v2::Prepare,
                                 conv_v2::Eval};
  return &r;
}

} // namespace rtos
} // namespace xcore
} // namespace micro
} // namespace ops
} // namespace tflite
