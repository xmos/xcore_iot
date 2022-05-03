// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "xcore_conv2d_v2.cc"

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
namespace conv_v2 {
namespace rtos {

#define XCORE_MAX_NUM_THREADS (5)

extern "C" {
// TODO
#pragma stackfunction 1000
void rtos_conv2d_v2_thread_worker(void *shard, void *scrtch, void *kp) {
  nn::AbstractKernel::Params *kparams = (nn::AbstractKernel::Params *)kp;
  auto scratch = static_cast<int8_t *>(scrtch);
  auto shared = static_cast<conv_v2::Conv2DShared *>(shard);
  execute(shared->Y, shared->X, shared->f, kparams, scratch);
}
}

// TfLiteStatus Eval(TfLiteContext *context, TfLiteNode *node) {
//   const TfLiteEvalTensor *input = tflite::micro::GetEvalInput(context, node,
//   0); TfLiteEvalTensor *output = tflite::micro::GetEvalOutput(context, node,
//   0); const TfLiteEvalTensor *weights_tensor =
//       tflite::micro::GetEvalInput(context, node, 1);
//   const TfLiteEvalTensor *multipliers_and_biases_tensor =
//       tflite::micro::GetEvalInput(context, node, 2);
//   tflite::micro::xcore::XCoreInterpreter *xint =
//       reinterpret_cast<tflite::micro::xcore::XCoreInterpreter *>(
//           context->impl_);

//   auto *op_data = reinterpret_cast<Conv2DOpData *>(node->user_data);
//   int n_threads = op_data->thread_count;

//   int8_t *weights =
//       (int8_t *)tflite::micro::GetTensorData<int8_t>(weights_tensor);
//   int16_t *multipliers_and_biases =
//       (int16_t *)tflite::micro::GetTensorData<int16_t>(
//           multipliers_and_biases_tensor);

//   int8_t *thread_scratch[n_threads];
//   Conv2DShared shared_data;
//   shared_data.X = (int8_t *)tflite::micro::GetTensorData<int8_t>(input);
//   shared_data.Y = (int8_t *)tflite::micro::GetTensorData<int8_t>(output);
//   shared_data.f = op_data->filter2D;
//   for (int t = 0; t < n_threads; ++t) {
//     thread_scratch[t] = (int8_t *)context->GetScratchBuffer(
//         context, op_data->threads[t].stack_scratch_index);
//   }

//   switch (op_data->kt) {
//   case Conv2DValidDirect_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulDirectFn *aggr = (nn::MatMulDirectFn *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
//     ot->setMultipliersAndBiases(multipliers_and_biases);
//   } break;
//   case Conv2DValidIndirect_t:
//   case Conv2DPaddedIndirect_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulInt8 *aggr = (nn::MatMulInt8 *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
//     ot->setMultipliersAndBiases(multipliers_and_biases);
//   } break;
//   case DepthwiseConv2DPaddedIndirect_t:
//   case DepthwiseConv2DValidDirect_t: {
//     nn::Filter2D_DW *f = (nn::Filter2D_DW *)op_data->filter2D;
//     nn::MatMulDirectFn_DW *aggr =
//         (nn::MatMulDirectFn_DW *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_int8 *ot = (nn::OT_int8 *)(f->ot_handler);
//     ot->setMultipliersAndBiases(multipliers_and_biases);
//   } break;
//   case BNNConv2DValidDirectBinary_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulBinaryDirectFn *aggr =
//         (nn::MatMulBinaryDirectFn *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_binary *ot = (nn::OT_binary *)(f->ot_handler);
//     ot->setThresholds(multipliers_and_biases);
//   } break;
//   case BNNConv2DValidIndirectBinary_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulBinary *aggr = (nn::MatMulBinary *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_binary *ot = (nn::OT_binary *)(f->ot_handler);
//     ot->setThresholds(multipliers_and_biases);
//   } break;
//   case BNNConv2DValidDirectInt8_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulBinaryDirectFn *aggr =
//         (nn::MatMulBinaryDirectFn *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_int8_clamped *ot = (nn::OT_int8_clamped *)(f->ot_handler);
//     ot->setOffsetsMultipliersAndBiases(multipliers_and_biases);
//   } break;
//   case BNNConv2DValidIndirectInt8_t: {
//     nn::Filter2D *f = (nn::Filter2D *)op_data->filter2D;
//     nn::MatMulBinary *aggr = (nn::MatMulBinary *)(f->aggregate_handler);
//     aggr->setWeights(weights);
//     nn::OT_int8_clamped *ot = (nn::OT_int8_clamped *)(f->ot_handler);
//     ot->setOffsetsMultipliersAndBiases(multipliers_and_biases);
//   } break;
//   }
//   // todo - this second for-loop is unpleasant
//   // for (int t = 0; t < n_threads-1; ++t) {
//   //   thread_variable_setup(thread_scratch[t], op_data->threads[t].kparams,
//   //   xint->thread_info.thread_ids.id[t]);
//   // }
//   // // Now set up shared data, shared function pointer, and data for final
//   // thread. thread_call((void *)&shared_data, thread_scratch[n_threads-1],
//   // op_data->threads[n_threads-1].kparams,
//   //             (thread_function_pointer_t)conv2d_v2_thread_worker,
//   //             &xint->thread_info);
//   rtos_conv2d_v2_thread_worker((void *)&shared_data,
//                                thread_scratch[n_threads - 1],
//                                op_data->threads[n_threads - 1].kparams);

//   return kTfLiteOk;
// }

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

  int8_t *thread_scratch[XCORE_MAX_NUM_THREADS];
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
  //   thread_variable_setup(thread_scratch[t], op_data->threads[t].kparams,
  //   xint->thread_info.thread_ids.id[t]);
  // }
  // // Now set up shared data, shared function pointer, and data for final
  // thread. thread_call((void *)&shared_data, thread_scratch[n_threads-1],
  // op_data->threads[n_threads-1].kparams,
  //             (thread_function_pointer_t)conv2d_v2_thread_worker,
  //             &xint->thread_info);
  rtos_conv2d_v2_thread_worker((void *)&shared_data,
                               thread_scratch[n_threads - 1],
                               op_data->threads[n_threads - 1].kparams);
  return kTfLiteOk;
}

} // namespace rtos
} // namespace conv_v2

namespace rtos {

TfLiteRegistration *Register_Conv2D_V2() {
  static TfLiteRegistration r = {conv_v2::Init, nullptr, conv_v2::Prepare,
                                 conv_v2::rtos::Eval};
  return &r;
}

} // namespace rtos
} // namespace xcore
} // namespace micro
} // namespace ops
} // namespace tflite
