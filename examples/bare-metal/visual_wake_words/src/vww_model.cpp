// This file is generated. Do not edit.
// Generated on: 07.06.2023 12:31:17


#include "../../api/xcore_config.h"
#include "lib_nn/api/version.h"
#include "lib_tflite_micro/api/version.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/conv.h"
#include "tensorflow/lite/micro/kernels/fully_connected.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/kernels/reduce.h"
#include "tensorflow/lite/micro/kernels/softmax.h"
#include "tensorflow/lite/micro/micro_context.h"

#if defined __GNUC__
#define ALIGN(X) __attribute__((aligned(X)))
#elif defined _MSC_VER
#define ALIGN(X) __declspec(align(X))
#elif defined __TASKING__
#define ALIGN(X) __align(X)
#endif

// Check lib_nn and lib_tflite_micro versions
// NOTE: xformer version is saved for debugging purposes
// If lib_nn and lib_tflite_micro versions are as expected,
// then the xformer version doesn't matter as the model should execute
// If major version is zero, then minor versions must match
// Otherwise, major versions must match and binary minor version
// must be less or equal to runtime minor version
// Check if runtime lib_tflite_micro version matches with compiled version
static_assert((0 == 0 && lib_tflite_micro::major_version == 0 && 5 == lib_tflite_micro::minor_version) ||
              (0 == lib_tflite_micro::major_version) ||
              (5  < lib_tflite_micro::minor_version),
             "Model has been compiled with lib_tflite_micro version incompatible with runtime lib_tflite_micro version!");

// Check if runtime lib_nn version matches with compiled version
static_assert((0 == 0 && lib_nn::major_version == 0 && 2 == lib_nn::minor_version) ||
              (0 == lib_nn::major_version) ||
              (2  < lib_nn::minor_version),
             "Model has been compiled with lib_nn version incompatible with runtime lib_nn version!");

namespace tflite {
namespace ops {
namespace micro {
namespace xcore {
extern TfLiteRegistration *Register_XC_pad_3_to_4(void);
extern TfLiteRegistration *Register_XC_pad(void);
extern TfLiteRegistration *Register_XC_ld_flash(void);
extern TfLiteRegistration *Register_XC_conv2d_v2(void);
} // namespace xcore
}  // namespace micro
}  // namespace ops
}  // namespace tflite

namespace {

constexpr int kTensorArenaSize = 102784;
uint8_t tensor_arena[kTensorArenaSize] ALIGN(8);
template <int SZ, class T> struct TfArray {
  int sz; T elem[SZ];
};
enum used_operators_e {
  OP_XC_pad_3_to_4, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_CONV_2D, OP_RESHAPE, OP_SOFTMAX,  OP_LAST
};

#if defined(TFLMC_XCORE_PROFILE) || defined(TFLMC_PRINT_TENSORS)
const char *op_strs[] = {
"OP_XC_pad_3_to_4", "OP_XC_pad", "OP_XC_ld_flash", "OP_XC_conv2d_v2", "OP_CONV_2D", "OP_RESHAPE", "OP_SOFTMAX", };
#endif

#ifdef TFLMC_XCORE_PROFILE
int op_times[OP_LAST];
int op_counts[OP_LAST];
int64_t op_times_summed;
int time_t0, time_t1;
#endif

TfLiteContext ctx{};

TfLiteRegistration registrations[OP_LAST];
const TfArray<4, int> tensor_dimension0 = { 4, { 1,96,96,3 } };
const TfArray<1, float> quant0_scale = { 1, { 0.0039215688593685627, } };
const TfArray<1, int> quant0_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant0 = { (TfLiteFloatArray*)&quant0_scale, (TfLiteIntArray*)&quant0_zero, 0 };
const ALIGN(8) int64_t tensor_data1[2] = { 
    -1, 2, 
};
const TfArray<1, int> tensor_dimension1 = { 1, { 2 } };
const ALIGN(8) int16_t tensor_data2[32] = { 
    3642, 4338, 7483, 9397, 17574, 14495, 5036, 6458, 11447, 8298, 
    21996, 9662, 3982, 8704, 4160, 5157, 384, -5000, -5500, 1941, 
    -6570, -2999, 3285, -7683, -1068, 5048, -1634, -8362, 2228, 6166, 
    -7106, -4345, 
};
const TfArray<1, int> tensor_dimension2 = { 1, { 32 } };
const ALIGN(8) int16_t tensor_data3[32] = { 
    24727, 22009, 14548, 7344, 5582, 9522, 20233, 25989, 7362, 8652, 
    10628, 10160, 25045, 9039, 19940, 23635, 6228, -7110, 1172, -1872, 
    -1965, 34, 2265, -7773, -6229, 1189, -7764, -6387, 4688, 1414, 
    -17498, -3345, 
};
const TfArray<1, int> tensor_dimension3 = { 1, { 32 } };
const ALIGN(8) int16_t tensor_data4[24] = { 
    4034, 5651, 2832, 7483, 5708, 21644, 7744, 6939, -2418, -2597, 
    -2613, -2325, -2487, -5955, -4532, -6019, 0, 0, 0, 0, 
    0, 0, 0, 0, 
};
const TfArray<1, int> tensor_dimension4 = { 1, { 24 } };
const ALIGN(8) int16_t tensor_data5[24] = { 
    32114, 22793, 22184, 26762, 13632, 5085, 18290, 7417, -6128, -5964, 
    -7936, -6438, -9962, -5103, -6766, -5602, 0, 0, 0, 0, 
    0, 0, 0, 0, 
};
const TfArray<1, int> tensor_dimension5 = { 1, { 24 } };
const TfArray<4, int> tensor_dimension6 = { 4, { 1,96,96,4 } };
const TfArray<1, float> quant6_scale = { 1, { 0.0039215688593685627, } };
const TfArray<1, int> quant6_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant6 = { (TfLiteFloatArray*)&quant6_scale, (TfLiteIntArray*)&quant6_zero, 0 };
const TfArray<4, int> tensor_dimension7 = { 4, { 1,97,97,4 } };
const TfArray<1, float> quant7_scale = { 1, { 0.0039215688593685627, } };
const TfArray<1, int> quant7_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant7 = { (TfLiteFloatArray*)&quant7_scale, (TfLiteIntArray*)&quant7_zero, 0 };
const TfArray<1, int> tensor_dimension8 = { 1, { 768 } };
const TfArray<4, int> tensor_dimension9 = { 4, { 1,48,48,8 } };
const TfArray<1, float> quant9_scale = { 1, { 0.016774974763393402, } };
const TfArray<1, int> quant9_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant9 = { (TfLiteFloatArray*)&quant9_scale, (TfLiteIntArray*)&quant9_zero, 0 };
const TfArray<4, int> tensor_dimension10 = { 4, { 1,50,50,8 } };
const TfArray<1, float> quant10_scale = { 1, { 0.016774974763393402, } };
const TfArray<1, int> quant10_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant10 = { (TfLiteFloatArray*)&quant10_scale, (TfLiteIntArray*)&quant10_zero, 0 };
const TfArray<1, int> tensor_dimension11 = { 1, { 160 } };
const TfArray<4, int> tensor_dimension12 = { 4, { 1,48,48,8 } };
const TfArray<1, float> quant12_scale = { 1, { 0.045506704598665237, } };
const TfArray<1, int> quant12_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant12 = { (TfLiteFloatArray*)&quant12_scale, (TfLiteIntArray*)&quant12_zero, 0 };
const TfArray<1, int> tensor_dimension13 = { 1, { 512 } };
const TfArray<4, int> tensor_dimension14 = { 4, { 1,48,48,16 } };
const TfArray<1, float> quant14_scale = { 1, { 0.034476276487112045, } };
const TfArray<1, int> quant14_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant14 = { (TfLiteFloatArray*)&quant14_scale, (TfLiteIntArray*)&quant14_zero, 0 };
const TfArray<4, int> tensor_dimension15 = { 4, { 1,49,49,16 } };
const TfArray<1, float> quant15_scale = { 1, { 0.034476276487112045, } };
const TfArray<1, int> quant15_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant15 = { (TfLiteFloatArray*)&quant15_scale, (TfLiteIntArray*)&quant15_zero, 0 };
const TfArray<1, int> tensor_dimension16 = { 1, { 160 } };
const TfArray<4, int> tensor_dimension17 = { 4, { 1,24,24,16 } };
const TfArray<1, float> quant17_scale = { 1, { 0.027565766125917435, } };
const TfArray<1, int> quant17_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant17 = { (TfLiteFloatArray*)&quant17_scale, (TfLiteIntArray*)&quant17_zero, 0 };
const TfArray<1, int> tensor_dimension18 = { 1, { 768 } };
const TfArray<1, int> tensor_dimension19 = { 1, { 64 } };
const TfArray<4, int> tensor_dimension20 = { 4, { 1,24,24,32 } };
const TfArray<1, float> quant20_scale = { 1, { 0.033461596816778183, } };
const TfArray<1, int> quant20_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant20 = { (TfLiteFloatArray*)&quant20_scale, (TfLiteIntArray*)&quant20_zero, 0 };
const TfArray<4, int> tensor_dimension21 = { 4, { 1,26,26,32 } };
const TfArray<1, float> quant21_scale = { 1, { 0.033461596816778183, } };
const TfArray<1, int> quant21_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant21 = { (TfLiteFloatArray*)&quant21_scale, (TfLiteIntArray*)&quant21_zero, 0 };
const TfArray<1, int> tensor_dimension22 = { 1, { 304 } };
const TfArray<1, int> tensor_dimension23 = { 1, { 64 } };
const TfArray<4, int> tensor_dimension24 = { 4, { 1,24,24,32 } };
const TfArray<1, float> quant24_scale = { 1, { 0.041313942521810532, } };
const TfArray<1, int> quant24_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant24 = { (TfLiteFloatArray*)&quant24_scale, (TfLiteIntArray*)&quant24_zero, 0 };
const TfArray<1, int> tensor_dimension25 = { 1, { 1024 } };
const TfArray<1, int> tensor_dimension26 = { 1, { 64 } };
const TfArray<4, int> tensor_dimension27 = { 4, { 1,24,24,32 } };
const TfArray<1, float> quant27_scale = { 1, { 0.040529187768697739, } };
const TfArray<1, int> quant27_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant27 = { (TfLiteFloatArray*)&quant27_scale, (TfLiteIntArray*)&quant27_zero, 0 };
const TfArray<4, int> tensor_dimension28 = { 4, { 1,25,25,32 } };
const TfArray<1, float> quant28_scale = { 1, { 0.040529187768697739, } };
const TfArray<1, int> quant28_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant28 = { (TfLiteFloatArray*)&quant28_scale, (TfLiteIntArray*)&quant28_zero, 0 };
const TfArray<1, int> tensor_dimension29 = { 1, { 304 } };
const TfArray<1, int> tensor_dimension30 = { 1, { 64 } };
const TfArray<4, int> tensor_dimension31 = { 4, { 1,12,12,32 } };
const TfArray<1, float> quant31_scale = { 1, { 0.024886887520551682, } };
const TfArray<1, int> quant31_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant31 = { (TfLiteFloatArray*)&quant31_scale, (TfLiteIntArray*)&quant31_zero, 0 };
const TfArray<1, int> tensor_dimension32 = { 1, { 2048 } };
const TfArray<1, int> tensor_dimension33 = { 1, { 128 } };
const TfArray<4, int> tensor_dimension34 = { 4, { 1,12,12,64 } };
const TfArray<1, float> quant34_scale = { 1, { 0.029840102419257164, } };
const TfArray<1, int> quant34_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant34 = { (TfLiteFloatArray*)&quant34_scale, (TfLiteIntArray*)&quant34_zero, 0 };
const TfArray<4, int> tensor_dimension35 = { 4, { 1,14,14,64 } };
const TfArray<1, float> quant35_scale = { 1, { 0.029840102419257164, } };
const TfArray<1, int> quant35_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant35 = { (TfLiteFloatArray*)&quant35_scale, (TfLiteIntArray*)&quant35_zero, 0 };
const TfArray<1, int> tensor_dimension36 = { 1, { 592 } };
const TfArray<1, int> tensor_dimension37 = { 1, { 128 } };
const TfArray<4, int> tensor_dimension38 = { 4, { 1,12,12,64 } };
const TfArray<1, float> quant38_scale = { 1, { 0.026971876621246338, } };
const TfArray<1, int> quant38_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant38 = { (TfLiteFloatArray*)&quant38_scale, (TfLiteIntArray*)&quant38_zero, 0 };
const TfArray<1, int> tensor_dimension39 = { 1, { 4096 } };
const TfArray<1, int> tensor_dimension40 = { 1, { 128 } };
const TfArray<4, int> tensor_dimension41 = { 4, { 1,12,12,64 } };
const TfArray<1, float> quant41_scale = { 1, { 0.037047129124403, } };
const TfArray<1, int> quant41_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant41 = { (TfLiteFloatArray*)&quant41_scale, (TfLiteIntArray*)&quant41_zero, 0 };
const TfArray<4, int> tensor_dimension42 = { 4, { 1,13,13,64 } };
const TfArray<1, float> quant42_scale = { 1, { 0.037047129124403, } };
const TfArray<1, int> quant42_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant42 = { (TfLiteFloatArray*)&quant42_scale, (TfLiteIntArray*)&quant42_zero, 0 };
const TfArray<1, int> tensor_dimension43 = { 1, { 592 } };
const TfArray<1, int> tensor_dimension44 = { 1, { 192 } };
const TfArray<4, int> tensor_dimension45 = { 4, { 1,6,6,64 } };
const TfArray<1, float> quant45_scale = { 1, { 0.025276221334934235, } };
const TfArray<1, int> quant45_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant45 = { (TfLiteFloatArray*)&quant45_scale, (TfLiteIntArray*)&quant45_zero, 0 };
const TfArray<1, int> tensor_dimension46 = { 1, { 8192 } };
const TfArray<1, int> tensor_dimension47 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension48 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant48_scale = { 1, { 0.026093753054738045, } };
const TfArray<1, int> quant48_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant48 = { (TfLiteFloatArray*)&quant48_scale, (TfLiteIntArray*)&quant48_zero, 0 };
const TfArray<4, int> tensor_dimension49 = { 4, { 1,8,8,128 } };
const TfArray<1, float> quant49_scale = { 1, { 0.026093753054738045, } };
const TfArray<1, int> quant49_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant49 = { (TfLiteFloatArray*)&quant49_scale, (TfLiteIntArray*)&quant49_zero, 0 };
const TfArray<1, int> tensor_dimension50 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension51 = { 1, { 384 } };
const TfArray<4, int> tensor_dimension52 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant52_scale = { 1, { 0.030003571882843971, } };
const TfArray<1, int> quant52_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant52 = { (TfLiteFloatArray*)&quant52_scale, (TfLiteIntArray*)&quant52_zero, 0 };
const TfArray<1, int> tensor_dimension53 = { 1, { 16384 } };
const TfArray<1, int> tensor_dimension54 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension55 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant55_scale = { 1, { 0.029042452573776245, } };
const TfArray<1, int> quant55_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant55 = { (TfLiteFloatArray*)&quant55_scale, (TfLiteIntArray*)&quant55_zero, 0 };
const TfArray<4, int> tensor_dimension56 = { 4, { 1,8,8,128 } };
const TfArray<1, float> quant56_scale = { 1, { 0.029042452573776245, } };
const TfArray<1, int> quant56_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant56 = { (TfLiteFloatArray*)&quant56_scale, (TfLiteIntArray*)&quant56_zero, 0 };
const TfArray<1, int> tensor_dimension57 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension58 = { 1, { 384 } };
const TfArray<4, int> tensor_dimension59 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant59_scale = { 1, { 0.032251704484224319, } };
const TfArray<1, int> quant59_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant59 = { (TfLiteFloatArray*)&quant59_scale, (TfLiteIntArray*)&quant59_zero, 0 };
const TfArray<1, int> tensor_dimension60 = { 1, { 16384 } };
const TfArray<1, int> tensor_dimension61 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension62 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant62_scale = { 1, { 0.027397098019719124, } };
const TfArray<1, int> quant62_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant62 = { (TfLiteFloatArray*)&quant62_scale, (TfLiteIntArray*)&quant62_zero, 0 };
const TfArray<4, int> tensor_dimension63 = { 4, { 1,8,8,128 } };
const TfArray<1, float> quant63_scale = { 1, { 0.027397098019719124, } };
const TfArray<1, int> quant63_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant63 = { (TfLiteFloatArray*)&quant63_scale, (TfLiteIntArray*)&quant63_zero, 0 };
const TfArray<1, int> tensor_dimension64 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension65 = { 1, { 384 } };
const TfArray<4, int> tensor_dimension66 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant66_scale = { 1, { 0.032134708017110825, } };
const TfArray<1, int> quant66_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant66 = { (TfLiteFloatArray*)&quant66_scale, (TfLiteIntArray*)&quant66_zero, 0 };
const TfArray<1, int> tensor_dimension67 = { 1, { 16384 } };
const TfArray<1, int> tensor_dimension68 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension69 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant69_scale = { 1, { 0.023059310391545296, } };
const TfArray<1, int> quant69_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant69 = { (TfLiteFloatArray*)&quant69_scale, (TfLiteIntArray*)&quant69_zero, 0 };
const TfArray<4, int> tensor_dimension70 = { 4, { 1,8,8,128 } };
const TfArray<1, float> quant70_scale = { 1, { 0.023059310391545296, } };
const TfArray<1, int> quant70_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant70 = { (TfLiteFloatArray*)&quant70_scale, (TfLiteIntArray*)&quant70_zero, 0 };
const TfArray<1, int> tensor_dimension71 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension72 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension73 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant73_scale = { 1, { 0.038512997329235077, } };
const TfArray<1, int> quant73_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant73 = { (TfLiteFloatArray*)&quant73_scale, (TfLiteIntArray*)&quant73_zero, 0 };
const TfArray<1, int> tensor_dimension74 = { 1, { 16384 } };
const TfArray<1, int> tensor_dimension75 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension76 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant76_scale = { 1, { 0.021800652146339417, } };
const TfArray<1, int> quant76_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant76 = { (TfLiteFloatArray*)&quant76_scale, (TfLiteIntArray*)&quant76_zero, 0 };
const TfArray<4, int> tensor_dimension77 = { 4, { 1,8,8,128 } };
const TfArray<1, float> quant77_scale = { 1, { 0.021800652146339417, } };
const TfArray<1, int> quant77_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant77 = { (TfLiteFloatArray*)&quant77_scale, (TfLiteIntArray*)&quant77_zero, 0 };
const TfArray<1, int> tensor_dimension78 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension79 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension80 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant80_scale = { 1, { 0.039849907159805298, } };
const TfArray<1, int> quant80_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant80 = { (TfLiteFloatArray*)&quant80_scale, (TfLiteIntArray*)&quant80_zero, 0 };
const TfArray<1, int> tensor_dimension81 = { 1, { 16384 } };
const TfArray<1, int> tensor_dimension82 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension83 = { 4, { 1,6,6,128 } };
const TfArray<1, float> quant83_scale = { 1, { 0.018375413492321968, } };
const TfArray<1, int> quant83_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant83 = { (TfLiteFloatArray*)&quant83_scale, (TfLiteIntArray*)&quant83_zero, 0 };
const TfArray<4, int> tensor_dimension84 = { 4, { 1,7,7,128 } };
const TfArray<1, float> quant84_scale = { 1, { 0.018375413492321968, } };
const TfArray<1, int> quant84_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant84 = { (TfLiteFloatArray*)&quant84_scale, (TfLiteIntArray*)&quant84_zero, 0 };
const TfArray<1, int> tensor_dimension85 = { 1, { 1168 } };
const TfArray<1, int> tensor_dimension86 = { 1, { 256 } };
const TfArray<4, int> tensor_dimension87 = { 4, { 1,3,3,128 } };
const TfArray<1, float> quant87_scale = { 1, { 0.034316841512918472, } };
const TfArray<1, int> quant87_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant87 = { (TfLiteFloatArray*)&quant87_scale, (TfLiteIntArray*)&quant87_zero, 0 };
const TfArray<1, int> tensor_dimension88 = { 1, { 32768 } };
const TfArray<1, int> tensor_dimension89 = { 1, { 512 } };
const TfArray<4, int> tensor_dimension90 = { 4, { 1,3,3,256 } };
const TfArray<1, float> quant90_scale = { 1, { 0.031597897410392761, } };
const TfArray<1, int> quant90_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant90 = { (TfLiteFloatArray*)&quant90_scale, (TfLiteIntArray*)&quant90_zero, 0 };
const TfArray<4, int> tensor_dimension91 = { 4, { 1,5,5,256 } };
const TfArray<1, float> quant91_scale = { 1, { 0.031597897410392761, } };
const TfArray<1, int> quant91_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant91 = { (TfLiteFloatArray*)&quant91_scale, (TfLiteIntArray*)&quant91_zero, 0 };
const TfArray<1, int> tensor_dimension92 = { 1, { 2320 } };
const TfArray<1, int> tensor_dimension93 = { 1, { 768 } };
const TfArray<4, int> tensor_dimension94 = { 4, { 1,3,3,256 } };
const TfArray<1, float> quant94_scale = { 1, { 0.037584718316793442, } };
const TfArray<1, int> quant94_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant94 = { (TfLiteFloatArray*)&quant94_scale, (TfLiteIntArray*)&quant94_zero, 0 };
const TfArray<1, int> tensor_dimension95 = { 1, { 65536 } };
const TfArray<1, int> tensor_dimension96 = { 1, { 512 } };
const TfArray<4, int> tensor_dimension97 = { 4, { 1,3,3,256 } };
const TfArray<1, float> quant97_scale = { 1, { 0.020674895495176315, } };
const TfArray<1, int> quant97_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant97 = { (TfLiteFloatArray*)&quant97_scale, (TfLiteIntArray*)&quant97_zero, 0 };
const ALIGN(8) int32_t tensor_data98[2] = { 
    1108, -1108, 
};
const TfArray<1, int> tensor_dimension98 = { 1, { 2 } };
const TfArray<1, float> quant98_scale = { 1, { 9.7591488156467676e-05, } };
const TfArray<1, int> quant98_zero = { 1, { 0 } };
const TfLiteAffineQuantization quant98 = { (TfLiteFloatArray*)&quant98_scale, (TfLiteIntArray*)&quant98_zero, 0 };
const TfArray<1, int> tensor_dimension99 = { 1, { 2320 } };
const TfArray<1, int> tensor_dimension100 = { 1, { 512 } };
const TfArray<4, int> tensor_dimension101 = { 4, { 1,1,1,256 } };
const TfArray<1, float> quant101_scale = { 1, { 0.020674895495176315, } };
const TfArray<1, int> quant101_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant101 = { (TfLiteFloatArray*)&quant101_scale, (TfLiteIntArray*)&quant101_zero, 0 };
const TfArray<4, int> tensor_dimension102 = { 4, { 2,1,1,256 } };
const TfArray<1, float> quant102_scale = { 1, { 0.0047202892601490021, } };
const TfArray<1, int> quant102_zero = { 1, { 0 } };
const TfLiteAffineQuantization quant102 = { (TfLiteFloatArray*)&quant102_scale, (TfLiteIntArray*)&quant102_zero, 0 };
const TfArray<4, int> tensor_dimension103 = { 4, { 1,1,1,2 } };
const TfArray<1, float> quant103_scale = { 1, { 0.0235444325953722, } };
const TfArray<1, int> quant103_zero = { 1, { -6 } };
const TfLiteAffineQuantization quant103 = { (TfLiteFloatArray*)&quant103_scale, (TfLiteIntArray*)&quant103_zero, 0 };
const TfArray<2, int> tensor_dimension104 = { 2, { 1,2 } };
const TfArray<1, float> quant104_scale = { 1, { 0.0235444325953722, } };
const TfArray<1, int> quant104_zero = { 1, { -6 } };
const TfLiteAffineQuantization quant104 = { (TfLiteFloatArray*)&quant104_scale, (TfLiteIntArray*)&quant104_zero, 0 };
const TfArray<2, int> tensor_dimension105 = { 2, { 1,2 } };
const TfArray<1, float> quant105_scale = { 1, { 0.00390625, } };
const TfArray<1, int> quant105_zero = { 1, { -128 } };
const TfLiteAffineQuantization quant105 = { (TfLiteFloatArray*)&quant105_scale, (TfLiteIntArray*)&quant105_zero, 0 };
uint8_t ALIGN(4) opdata0[28] = { 112, 118, 0, 1, 4, 0, 0, 0, 4, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 128, 128, 128, 128, 6, 5, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs0 = { 1, { 0 } };
const TfArray<1, int> outputs0 = { 1, { 6 } };
uint8_t ALIGN(4) opdata1[61] = { 112, 112, 0, 24, 0, 0, 0, 0, 96, 0, 0, 0, 0, 0, 0, 0, 128, 1, 0, 0, 4, 0, 0, 0, 132, 1, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs1 = { 1, { 6 } };
const TfArray<1, int> outputs1 = { 1, { 7 } };
uint8_t ALIGN(4) opdata2[45] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 1, 0, 0, 3, 5, 2, 18, 14, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 64, 108, 3, 0, 22, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs2 = 0; /* empty TfLiteIntArray */
const TfArray<1, int> outputs2 = { 1, { 8 } };
uint8_t ALIGN(4) opdata3[158] = { 107, 116, 0, 109, 112, 0, 40, 8, 3, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 252, 255, 255, 255, 228, 255, 255, 255, 120, 1, 0, 0, 15, 0, 0, 0, 0, 0, 0, 0, 0, 97, 103, 103, 112, 0, 8, 8, 0, 0, 0, 36, 0, 0, 0, 0, 111, 116, 112, 0, 8, 8, 0, 0, 0, 3, 0, 254, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 1, 34, 20, 7, 83, 43, 133, 131, 72, 59, 56, 7, 1, 7, 87, 14, 1, 137, 77, 0, 96, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs3 = { 3, { 7,8,5 } };
const TfArray<1, int> outputs3 = { 1, { 9 } };
uint8_t ALIGN(4) opdata4[61] = { 112, 112, 0, 24, 144, 1, 0, 0, 48, 0, 0, 0, 8, 0, 0, 0, 128, 1, 0, 0, 8, 0, 0, 0, 144, 1, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs4 = { 1, { 9 } };
const TfArray<1, int> outputs4 = { 1, { 10 } };
uint8_t ALIGN(4) opdata5[45] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 1, 0, 160, 0, 5, 2, 18, 14, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 160, 107, 3, 0, 22, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs5 = 0; /* empty TfLiteIntArray */
const TfArray<1, int> outputs5 = { 1, { 11 } };
uint8_t ALIGN(4) opdata6[138] = { 107, 116, 0, 109, 112, 0, 8, 144, 1, 0, 0, 8, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 8, 0, 0, 0, 120, 1, 0, 0, 0, 111, 116, 112, 0, 8, 8, 0, 0, 0, 1, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs6 = { 3, { 10,11,4 } };
const TfArray<1, int> outputs6 = { 1, { 12 } };
uint8_t ALIGN(4) opdata7[45] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 1, 0, 0, 2, 5, 2, 18, 14, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 160, 105, 3, 0, 22, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs7 = 0; /* empty TfLiteIntArray */
const TfArray<1, int> outputs7 = { 1, { 13 } };
uint8_t ALIGN(4) opdata8[158] = { 107, 116, 0, 109, 112, 0, 40, 128, 1, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 248, 255, 255, 255, 232, 255, 255, 255, 120, 1, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 97, 103, 103, 112, 0, 8, 16, 0, 0, 0, 8, 0, 0, 0, 0, 111, 116, 112, 0, 8, 16, 0, 0, 0, 2, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 48, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 1, 34, 20, 7, 83, 43, 133, 131, 72, 59, 56, 7, 1, 7, 87, 14, 1, 137, 77, 0, 64, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs8 = { 3, { 12,13,3 } };
const TfArray<1, int> outputs8 = { 1, { 14 } };
uint8_t ALIGN(4) opdata9[61] = { 112, 112, 0, 24, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 16, 0, 0, 0, 16, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs9 = { 1, { 14 } };
const TfArray<1, int> outputs9 = { 1, { 15 } };
uint8_t ALIGN(4) opdata10[45] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 1, 0, 160, 0, 5, 2, 18, 14, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 105, 3, 0, 22, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs10 = 0; /* empty TfLiteIntArray */
const TfArray<1, int> outputs10 = { 1, { 16 } };
uint8_t ALIGN(4) opdata11[138] = { 107, 116, 0, 109, 112, 0, 8, 32, 6, 0, 0, 32, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 16, 0, 0, 0, 224, 2, 0, 0, 0, 111, 116, 112, 0, 8, 16, 0, 0, 0, 2, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs11 = { 3, { 15,16,2 } };
const TfArray<1, int> outputs11 = { 1, { 17 } };
uint8_t ALIGN(4) opdata12[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 3, 128, 0, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 128, 101, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs12 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs12 = { 2, { 18,19 } };
uint8_t ALIGN(4) opdata13[158] = { 107, 116, 0, 109, 112, 0, 40, 128, 1, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 240, 255, 255, 255, 240, 255, 255, 255, 112, 1, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 97, 103, 103, 112, 0, 8, 32, 0, 0, 0, 16, 0, 0, 0, 0, 111, 116, 112, 0, 8, 32, 0, 0, 0, 2, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 1, 34, 20, 7, 83, 43, 133, 131, 72, 59, 56, 7, 1, 7, 87, 14, 1, 137, 77, 0, 64, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs13 = { 3, { 17,18,19 } };
const TfArray<1, int> outputs13 = { 1, { 20 } };
uint8_t ALIGN(4) opdata14[61] = { 112, 112, 0, 24, 64, 3, 0, 0, 24, 0, 0, 0, 32, 0, 0, 0, 0, 3, 0, 0, 32, 0, 0, 0, 64, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs14 = { 1, { 20 } };
const TfArray<1, int> outputs14 = { 1, { 21 } };
uint8_t ALIGN(4) opdata15[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 48, 1, 128, 0, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 208, 99, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs15 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs15 = { 2, { 22,23 } };
uint8_t ALIGN(4) opdata16[138] = { 107, 116, 0, 109, 112, 0, 8, 64, 3, 0, 0, 32, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 32, 0, 0, 0, 224, 2, 0, 0, 0, 111, 116, 112, 0, 8, 32, 0, 0, 0, 1, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs16 = { 3, { 21,22,23 } };
const TfArray<1, int> outputs16 = { 1, { 24 } };
uint8_t ALIGN(4) opdata17[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 4, 128, 0, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 80, 95, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs17 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs17 = { 2, { 25,26 } };
uint8_t ALIGN(4) opdata18[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 32, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 224, 2, 0, 0, 0, 111, 116, 112, 0, 8, 32, 0, 0, 0, 2, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs18 = { 3, { 24,25,26 } };
const TfArray<1, int> outputs18 = { 1, { 27 } };
uint8_t ALIGN(4) opdata19[61] = { 112, 112, 0, 24, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 32, 0, 0, 0, 32, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs19 = { 1, { 27 } };
const TfArray<1, int> outputs19 = { 1, { 28 } };
uint8_t ALIGN(4) opdata20[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 48, 1, 128, 0, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 160, 93, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs20 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs20 = { 2, { 29,30 } };
uint8_t ALIGN(4) opdata21[138] = { 107, 116, 0, 109, 112, 0, 8, 64, 6, 0, 0, 64, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 32, 0, 0, 0, 192, 2, 0, 0, 0, 111, 116, 112, 0, 8, 32, 0, 0, 0, 2, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs21 = { 3, { 28,29,30 } };
const TfArray<1, int> outputs21 = { 1, { 31 } };
uint8_t ALIGN(4) opdata22[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 8, 0, 1, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 160, 84, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs22 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs22 = { 2, { 32,33 } };
uint8_t ALIGN(4) opdata23[142] = { 107, 116, 0, 109, 112, 0, 8, 128, 1, 0, 0, 32, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 96, 1, 0, 0, 0, 111, 116, 112, 0, 8, 64, 0, 0, 0, 3, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs23 = { 3, { 31,32,33 } };
const TfArray<1, int> outputs23 = { 1, { 34 } };
uint8_t ALIGN(4) opdata24[61] = { 112, 112, 0, 24, 128, 3, 0, 0, 12, 0, 0, 0, 64, 0, 0, 0, 0, 3, 0, 0, 64, 0, 0, 0, 128, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs24 = { 1, { 34 } };
const TfArray<1, int> outputs24 = { 1, { 35 } };
uint8_t ALIGN(4) opdata25[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 80, 2, 0, 1, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 80, 81, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs25 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs25 = { 2, { 36,37 } };
uint8_t ALIGN(4) opdata26[138] = { 107, 116, 0, 109, 112, 0, 8, 128, 3, 0, 0, 64, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 192, 2, 0, 0, 0, 111, 116, 112, 0, 8, 64, 0, 0, 0, 2, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs26 = { 3, { 35,36,37 } };
const TfArray<1, int> outputs26 = { 1, { 38 } };
uint8_t ALIGN(4) opdata27[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 16, 0, 1, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 80, 64, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs27 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs27 = { 2, { 39,40 } };
uint8_t ALIGN(4) opdata28[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 64, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 192, 2, 0, 0, 0, 111, 116, 112, 0, 8, 64, 0, 0, 0, 4, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs28 = { 3, { 38,39,40 } };
const TfArray<1, int> outputs28 = { 1, { 41 } };
uint8_t ALIGN(4) opdata29[61] = { 112, 112, 0, 24, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 64, 0, 0, 0, 64, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs29 = { 1, { 41 } };
const TfArray<1, int> outputs29 = { 1, { 42 } };
uint8_t ALIGN(4) opdata30[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 80, 2, 128, 1, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 128, 60, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs30 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs30 = { 2, { 43,44 } };
uint8_t ALIGN(4) opdata31[138] = { 107, 116, 0, 109, 112, 0, 8, 128, 6, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 64, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 64, 0, 0, 0, 250, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 1, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs31 = { 3, { 42,43,44 } };
const TfArray<1, int> outputs31 = { 1, { 45 } };
uint8_t ALIGN(4) opdata32[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 32, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 128, 26, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs32 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs32 = { 2, { 46,47 } };
uint8_t ALIGN(4) opdata33[142] = { 107, 116, 0, 109, 112, 0, 8, 128, 1, 0, 0, 64, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 64, 1, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 4, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs33 = { 3, { 45,46,47 } };
const TfArray<1, int> outputs33 = { 1, { 48 } };
uint8_t ALIGN(4) opdata34[61] = { 112, 112, 0, 24, 0, 4, 0, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 0, 4, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs34 = { 1, { 48 } };
const TfArray<1, int> outputs34 = { 1, { 49 } };
uint8_t ALIGN(4) opdata35[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 3, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 240, 18, 3, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs35 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs35 = { 2, { 50,51 } };
uint8_t ALIGN(4) opdata36[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 4, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 251, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 1, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs36 = { 3, { 49,50,51 } };
const TfArray<1, int> outputs36 = { 1, { 52 } };
uint8_t ALIGN(4) opdata37[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 64, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 240, 208, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs37 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs37 = { 2, { 53,54 } };
uint8_t ALIGN(4) opdata38[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 3, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs38 = { 3, { 52,53,54 } };
const TfArray<1, int> outputs38 = { 1, { 55 } };
uint8_t ALIGN(4) opdata39[61] = { 112, 112, 0, 24, 0, 4, 0, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 0, 4, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs39 = { 1, { 55 } };
const TfArray<1, int> outputs39 = { 1, { 56 } };
uint8_t ALIGN(4) opdata40[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 3, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 96, 201, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs40 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs40 = { 2, { 57,58 } };
uint8_t ALIGN(4) opdata41[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 4, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 251, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 1, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs41 = { 3, { 56,57,58 } };
const TfArray<1, int> outputs41 = { 1, { 59 } };
uint8_t ALIGN(4) opdata42[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 64, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 96, 135, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs42 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs42 = { 2, { 60,61 } };
uint8_t ALIGN(4) opdata43[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 3, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs43 = { 3, { 59,60,61 } };
const TfArray<1, int> outputs43 = { 1, { 62 } };
uint8_t ALIGN(4) opdata44[61] = { 112, 112, 0, 24, 0, 4, 0, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 0, 4, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs44 = { 1, { 62 } };
const TfArray<1, int> outputs44 = { 1, { 63 } };
uint8_t ALIGN(4) opdata45[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 3, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 208, 127, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs45 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs45 = { 2, { 64,65 } };
uint8_t ALIGN(4) opdata46[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 4, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 251, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 1, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs46 = { 3, { 63,64,65 } };
const TfArray<1, int> outputs46 = { 1, { 66 } };
uint8_t ALIGN(4) opdata47[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 64, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 208, 61, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs47 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs47 = { 2, { 67,68 } };
uint8_t ALIGN(4) opdata48[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 3, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs48 = { 3, { 66,67,68 } };
const TfArray<1, int> outputs48 = { 1, { 69 } };
uint8_t ALIGN(4) opdata49[61] = { 112, 112, 0, 24, 0, 4, 0, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 0, 4, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs49 = { 1, { 69 } };
const TfArray<1, int> outputs49 = { 1, { 70 } };
uint8_t ALIGN(4) opdata50[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 64, 55, 2, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs50 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs50 = { 2, { 71,72 } };
uint8_t ALIGN(4) opdata51[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 4, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 2, 0, 251, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs51 = { 3, { 70,71,72 } };
const TfArray<1, int> outputs51 = { 1, { 73 } };
uint8_t ALIGN(4) opdata52[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 64, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 64, 245, 1, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs52 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs52 = { 2, { 74,75 } };
uint8_t ALIGN(4) opdata53[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 2, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs53 = { 3, { 73,74,75 } };
const TfArray<1, int> outputs53 = { 1, { 76 } };
uint8_t ALIGN(4) opdata54[61] = { 112, 112, 0, 24, 0, 4, 0, 0, 6, 0, 0, 0, 128, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 0, 4, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs54 = { 1, { 76 } };
const TfArray<1, int> outputs54 = { 1, { 77 } };
uint8_t ALIGN(4) opdata55[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 176, 238, 1, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs55 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs55 = { 2, { 78,79 } };
uint8_t ALIGN(4) opdata56[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 4, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 2, 0, 251, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs56 = { 3, { 77,78,79 } };
const TfArray<1, int> outputs56 = { 1, { 80 } };
uint8_t ALIGN(4) opdata57[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 64, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 176, 172, 1, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs57 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs57 = { 2, { 81,82 } };
uint8_t ALIGN(4) opdata58[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 128, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 3, 0, 252, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs58 = { 3, { 80,81,82 } };
const TfArray<1, int> outputs58 = { 1, { 83 } };
uint8_t ALIGN(4) opdata59[61] = { 112, 112, 0, 24, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 128, 0, 0, 0, 128, 3, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs59 = { 1, { 83 } };
const TfArray<1, int> outputs59 = { 1, { 84 } };
uint8_t ALIGN(4) opdata60[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 144, 4, 0, 2, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 32, 166, 1, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs60 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs60 = { 2, { 85,86 } };
uint8_t ALIGN(4) opdata61[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 7, 0, 0, 0, 1, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 128, 0, 0, 0, 0, 2, 0, 0, 0, 111, 116, 112, 0, 8, 128, 0, 0, 0, 2, 0, 251, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 0, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs61 = { 3, { 84,85,86 } };
const TfArray<1, int> outputs61 = { 1, { 87 } };
uint8_t ALIGN(4) opdata62[57] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 0, 0, 128, 0, 0, 0, 4, 0, 0, 6, 6, 2, 27, 23, 0, 0, 0, 5, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 32, 34, 1, 0, 32, 0, 0, 0, 6, 42, 10, 38, 1,  }; /* custom_initial_data */
const int inputs62 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs62 = { 2, { 88,89 } };
uint8_t ALIGN(4) opdata63[142] = { 107, 116, 0, 109, 112, 0, 8, 128, 1, 0, 0, 128, 0, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 111, 116, 112, 0, 8, 0, 1, 0, 0, 3, 0, 253, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs63 = { 3, { 87,88,89 } };
const TfArray<1, int> outputs63 = { 1, { 90 } };
uint8_t ALIGN(4) opdata64[61] = { 112, 112, 0, 24, 0, 5, 0, 0, 3, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 1, 0, 0, 0, 5, 0, 0, 0, 112, 118, 0, 2, 33, 5, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 44, 0, 0, 0, 128, 128, 128, 128, 20, 6, 10, 38, 1,  }; /* custom_initial_data */
const TfArray<1, int> inputs64 = { 1, { 90 } };
const TfArray<1, int> outputs64 = { 1, { 91 } };
uint8_t ALIGN(4) opdata65[49] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 16, 9, 0, 6, 5, 5, 2, 21, 17, 0, 3, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 16, 19, 1, 0, 26, 0, 0, 0, 6, 41, 10, 38, 1,  }; /* custom_initial_data */
const int inputs65 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs65 = { 2, { 92,93 } };
uint8_t ALIGN(4) opdata66[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 5, 0, 0, 0, 1, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 111, 116, 112, 0, 8, 0, 1, 0, 0, 250, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 1, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs66 = { 3, { 91,92,93 } };
const TfArray<1, int> outputs66 = { 1, { 94 } };
uint8_t ALIGN(4) opdata67[45] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 0, 0, 0, 0, 1, 0, 0, 4, 0, 0, 6, 6, 2, 27, 23, 0, 3, 0, 1, 0, 2, 0, 16, 15, 22, 0, 5, 42, 6, 37, 1,  }; /* custom_initial_data */
const int inputs67 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs67 = { 2, { 95,96 } };
uint8_t ALIGN(4) opdata68[142] = { 107, 116, 0, 109, 112, 0, 8, 0, 3, 0, 0, 0, 1, 0, 0, 0, 97, 103, 103, 112, 0, 24, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 111, 116, 112, 0, 8, 0, 1, 0, 0, 3, 0, 251, 255, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 34, 20, 7, 99, 43, 117, 115, 72, 59, 56, 7, 1, 7, 103, 14, 0, 121, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs68 = { 3, { 94,95,96 } };
const TfArray<1, int> outputs68 = { 1, { 97 } };
uint8_t ALIGN(4) opdata69[39] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 2, 0, 16, 9, 0, 4, 5, 5, 2, 21, 17, 0, 3, 0, 1, 0, 2, 0, 0, 2, 18, 0, 5, 41, 6, 37, 1,  }; /* custom_initial_data */
const int inputs69 = 0; /* empty TfLiteIntArray */
const TfArray<2, int> outputs69 = { 2, { 99,100 } };
uint8_t ALIGN(4) opdata70[138] = { 107, 116, 0, 109, 112, 0, 8, 0, 9, 0, 0, 0, 3, 0, 0, 0, 97, 103, 103, 112, 0, 20, 144, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 111, 116, 112, 0, 8, 0, 1, 0, 0, 252, 255, 0, 0, 0, 111, 116, 116, 0, 115, 99, 114, 97, 116, 99, 104, 0, 97, 107, 112, 0, 32, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 34, 20, 7, 95, 43, 113, 111, 72, 59, 56, 7, 1, 7, 99, 14, 3, 117, 77, 0, 0, 20, 40, 4, 20, 20, 4, 4, 14, 36, 1,  }; /* custom_initial_data */
const TfArray<3, int> inputs70 = { 3, { 97,99,100 } };
const TfArray<1, int> outputs70 = { 1, { 101 } };
uint8_t ALIGN(4) opdata71[30] = { 97, 100, 100, 114, 0, 115, 105, 122, 101, 115, 0, 0, 1, 0, 0, 2, 5, 2, 18, 14, 2, 1, 2, 0, 10, 4, 41, 4, 36, 1,  }; /* custom_initial_data */
const int inputs71 = 0; /* empty TfLiteIntArray */
const TfArray<1, int> outputs71 = { 1, { 102 } };
const TfLiteConvParams opdata72 = { kTfLitePaddingValid, 1,1, kTfLiteActNone, 1,1 };
const TfArray<3, int> inputs72 = { 3, { 101,102,98 } };
const TfArray<1, int> outputs72 = { 1, { 103 } };
const TfLiteReshapeParams opdata73 = { { 0, 0, 0, 0, 0, 0, 0, 0, }, 0 };
const TfArray<2, int> inputs73 = { 2, { 103,1 } };
const TfArray<1, int> outputs73 = { 1, { 104 } };
const TfLiteSoftmaxParams opdata74 = { 1 };
const TfArray<1, int> inputs74 = { 1, { 104 } };
const TfArray<1, int> outputs74 = { 1, { 105 } };
TfLiteTensor tflTensors[] = {
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension0, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant0)) }, {quant0.scale->data[0], quant0.zero_point->data[0] },27648, kTfLiteArenaRw, false, },
  { {(int32_t*)tensor_data1},(TfLiteIntArray*)&tensor_dimension1, kTfLiteInt64, {kTfLiteNoQuantization, nullptr }, {0,0},16, kTfLiteMmapRo, false, },
  { {(int32_t*)tensor_data2},(TfLiteIntArray*)&tensor_dimension2, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},64, kTfLiteMmapRo, false, },
  { {(int32_t*)tensor_data3},(TfLiteIntArray*)&tensor_dimension3, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},64, kTfLiteMmapRo, false, },
  { {(int32_t*)tensor_data4},(TfLiteIntArray*)&tensor_dimension4, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},48, kTfLiteMmapRo, false, },
  { {(int32_t*)tensor_data5},(TfLiteIntArray*)&tensor_dimension5, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},48, kTfLiteMmapRo, false, },
  { {(int32_t*)(tensor_arena + 37648)},(TfLiteIntArray*)&tensor_dimension6, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant6)) }, {quant6.scale->data[0], quant6.zero_point->data[0] },36864, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension7, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant7)) }, {quant7.scale->data[0], quant7.zero_point->data[0] },37636, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 56080)},(TfLiteIntArray*)&tensor_dimension8, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 37648)},(TfLiteIntArray*)&tensor_dimension9, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant9)) }, {quant9.scale->data[0], quant9.zero_point->data[0] },18432, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension10, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant10)) }, {quant10.scale->data[0], quant10.zero_point->data[0] },20000, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20000)},(TfLiteIntArray*)&tensor_dimension11, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},160, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 75280)},(TfLiteIntArray*)&tensor_dimension12, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant12)) }, {quant12.scale->data[0], quant12.zero_point->data[0] },18432, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension13, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 38416)},(TfLiteIntArray*)&tensor_dimension14, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant14)) }, {quant14.scale->data[0], quant14.zero_point->data[0] },36864, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension15, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant15)) }, {quant15.scale->data[0], quant15.zero_point->data[0] },38416, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 38416)},(TfLiteIntArray*)&tensor_dimension16, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},160, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 40064)},(TfLiteIntArray*)&tensor_dimension17, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant17)) }, {quant17.scale->data[0], quant17.zero_point->data[0] },9216, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension18, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 768)},(TfLiteIntArray*)&tensor_dimension19, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},128, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 21632)},(TfLiteIntArray*)&tensor_dimension20, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant20)) }, {quant20.scale->data[0], quant20.zero_point->data[0] },18432, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension21, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant21)) }, {quant21.scale->data[0], quant21.zero_point->data[0] },21632, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 21632)},(TfLiteIntArray*)&tensor_dimension22, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},304, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 21936)},(TfLiteIntArray*)&tensor_dimension23, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},128, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 38432)},(TfLiteIntArray*)&tensor_dimension24, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant24)) }, {quant24.scale->data[0], quant24.zero_point->data[0] },18432, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension25, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1024, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 1024)},(TfLiteIntArray*)&tensor_dimension26, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},128, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20000)},(TfLiteIntArray*)&tensor_dimension27, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant27)) }, {quant27.scale->data[0], quant27.zero_point->data[0] },18432, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension28, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant28)) }, {quant28.scale->data[0], quant28.zero_point->data[0] },20000, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20000)},(TfLiteIntArray*)&tensor_dimension29, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},304, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20304)},(TfLiteIntArray*)&tensor_dimension30, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},128, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 21760)},(TfLiteIntArray*)&tensor_dimension31, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant31)) }, {quant31.scale->data[0], quant31.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension32, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},2048, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 2048)},(TfLiteIntArray*)&tensor_dimension33, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},256, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 12544)},(TfLiteIntArray*)&tensor_dimension34, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant34)) }, {quant34.scale->data[0], quant34.zero_point->data[0] },9216, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension35, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant35)) }, {quant35.scale->data[0], quant35.zero_point->data[0] },12544, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 12544)},(TfLiteIntArray*)&tensor_dimension36, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},592, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 13136)},(TfLiteIntArray*)&tensor_dimension37, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},256, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20032)},(TfLiteIntArray*)&tensor_dimension38, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant38)) }, {quant38.scale->data[0], quant38.zero_point->data[0] },9216, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension39, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},4096, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 4096)},(TfLiteIntArray*)&tensor_dimension40, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},256, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 10816)},(TfLiteIntArray*)&tensor_dimension41, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant41)) }, {quant41.scale->data[0], quant41.zero_point->data[0] },9216, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension42, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant42)) }, {quant42.scale->data[0], quant42.zero_point->data[0] },10816, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 10816)},(TfLiteIntArray*)&tensor_dimension43, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},592, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 11408)},(TfLiteIntArray*)&tensor_dimension44, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 12800)},(TfLiteIntArray*)&tensor_dimension45, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant45)) }, {quant45.scale->data[0], quant45.zero_point->data[0] },2304, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension46, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 15104)},(TfLiteIntArray*)&tensor_dimension47, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension48, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant48)) }, {quant48.scale->data[0], quant48.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension49, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant49)) }, {quant49.scale->data[0], quant49.zero_point->data[0] },8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension50, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 9360)},(TfLiteIntArray*)&tensor_dimension51, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20992)},(TfLiteIntArray*)&tensor_dimension52, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant52)) }, {quant52.scale->data[0], quant52.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension53, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},16384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 25600)},(TfLiteIntArray*)&tensor_dimension54, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16384)},(TfLiteIntArray*)&tensor_dimension55, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant55)) }, {quant55.scale->data[0], quant55.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension56, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant56)) }, {quant56.scale->data[0], quant56.zero_point->data[0] },8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension57, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 9360)},(TfLiteIntArray*)&tensor_dimension58, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20992)},(TfLiteIntArray*)&tensor_dimension59, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant59)) }, {quant59.scale->data[0], quant59.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension60, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},16384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 25600)},(TfLiteIntArray*)&tensor_dimension61, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16384)},(TfLiteIntArray*)&tensor_dimension62, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant62)) }, {quant62.scale->data[0], quant62.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension63, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant63)) }, {quant63.scale->data[0], quant63.zero_point->data[0] },8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension64, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 9360)},(TfLiteIntArray*)&tensor_dimension65, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20992)},(TfLiteIntArray*)&tensor_dimension66, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant66)) }, {quant66.scale->data[0], quant66.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension67, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},16384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 25600)},(TfLiteIntArray*)&tensor_dimension68, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16384)},(TfLiteIntArray*)&tensor_dimension69, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant69)) }, {quant69.scale->data[0], quant69.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension70, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant70)) }, {quant70.scale->data[0], quant70.zero_point->data[0] },8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension71, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 9360)},(TfLiteIntArray*)&tensor_dimension72, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20992)},(TfLiteIntArray*)&tensor_dimension73, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant73)) }, {quant73.scale->data[0], quant73.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension74, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},16384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 25600)},(TfLiteIntArray*)&tensor_dimension75, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16384)},(TfLiteIntArray*)&tensor_dimension76, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant76)) }, {quant76.scale->data[0], quant76.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension77, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant77)) }, {quant77.scale->data[0], quant77.zero_point->data[0] },8192, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8192)},(TfLiteIntArray*)&tensor_dimension78, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 9360)},(TfLiteIntArray*)&tensor_dimension79, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 20992)},(TfLiteIntArray*)&tensor_dimension80, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant80)) }, {quant80.scale->data[0], quant80.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension81, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},16384, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 25600)},(TfLiteIntArray*)&tensor_dimension82, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16384)},(TfLiteIntArray*)&tensor_dimension83, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant83)) }, {quant83.scale->data[0], quant83.zero_point->data[0] },4608, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension84, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant84)) }, {quant84.scale->data[0], quant84.zero_point->data[0] },6272, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 6272)},(TfLiteIntArray*)&tensor_dimension85, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},1168, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 7440)},(TfLiteIntArray*)&tensor_dimension86, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 35072)},(TfLiteIntArray*)&tensor_dimension87, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant87)) }, {quant87.scale->data[0], quant87.zero_point->data[0] },1152, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension88, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},32768, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 36224)},(TfLiteIntArray*)&tensor_dimension89, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},1024, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 32768)},(TfLiteIntArray*)&tensor_dimension90, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant90)) }, {quant90.scale->data[0], quant90.zero_point->data[0] },2304, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension91, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant91)) }, {quant91.scale->data[0], quant91.zero_point->data[0] },6400, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 6400)},(TfLiteIntArray*)&tensor_dimension92, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},2320, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 8720)},(TfLiteIntArray*)&tensor_dimension93, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},1536, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 67840)},(TfLiteIntArray*)&tensor_dimension94, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant94)) }, {quant94.scale->data[0], quant94.zero_point->data[0] },2304, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension95, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},65536, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 70144)},(TfLiteIntArray*)&tensor_dimension96, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},1024, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 65536)},(TfLiteIntArray*)&tensor_dimension97, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant97)) }, {quant97.scale->data[0], quant97.zero_point->data[0] },2304, kTfLiteArenaRw, false, },
  { {(int32_t*)tensor_data98},(TfLiteIntArray*)&tensor_dimension98, kTfLiteInt32, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant98)) }, {quant98.scale->data[0], quant98.zero_point->data[0] },8, kTfLiteMmapRo, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension99, kTfLiteInt8, {kTfLiteNoQuantization, nullptr }, {0,0},2320, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 2320)},(TfLiteIntArray*)&tensor_dimension100, kTfLiteInt16, {kTfLiteNoQuantization, nullptr }, {0,0},1024, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 3344)},(TfLiteIntArray*)&tensor_dimension101, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant101)) }, {quant101.scale->data[0], quant101.zero_point->data[0] },256, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension102, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant102)) }, {quant102.scale->data[0], quant102.zero_point->data[0] },512, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 512)},(TfLiteIntArray*)&tensor_dimension103, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant103)) }, {quant103.scale->data[0], quant103.zero_point->data[0] },2, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 16)},(TfLiteIntArray*)&tensor_dimension104, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant104)) }, {quant104.scale->data[0], quant104.zero_point->data[0] },2, kTfLiteArenaRw, false, },
  { {(int32_t*)(tensor_arena + 0)},(TfLiteIntArray*)&tensor_dimension105, kTfLiteInt8, {kTfLiteAffineQuantization, const_cast<void*>(static_cast<const void*>(&quant105)) }, {quant105.scale->data[0], quant105.zero_point->data[0] },2, kTfLiteArenaRw, false, },
};
TfLiteNode tflNodes[] = {
  { (TfLiteIntArray*)&inputs0, (TfLiteIntArray*)&outputs0, (TfLiteIntArray*)&inputs0, nullptr, const_cast<void*>(static_cast<const void*>(&opdata0)), nullptr, 28, },
  { (TfLiteIntArray*)&inputs1, (TfLiteIntArray*)&outputs1, (TfLiteIntArray*)&inputs1, nullptr, const_cast<void*>(static_cast<const void*>(&opdata1)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs2, (TfLiteIntArray*)&outputs2, (TfLiteIntArray*)&inputs2, nullptr, const_cast<void*>(static_cast<const void*>(&opdata2)), nullptr, 45, },
  { (TfLiteIntArray*)&inputs3, (TfLiteIntArray*)&outputs3, (TfLiteIntArray*)&inputs3, nullptr, const_cast<void*>(static_cast<const void*>(&opdata3)), nullptr, 158, },
  { (TfLiteIntArray*)&inputs4, (TfLiteIntArray*)&outputs4, (TfLiteIntArray*)&inputs4, nullptr, const_cast<void*>(static_cast<const void*>(&opdata4)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs5, (TfLiteIntArray*)&outputs5, (TfLiteIntArray*)&inputs5, nullptr, const_cast<void*>(static_cast<const void*>(&opdata5)), nullptr, 45, },
  { (TfLiteIntArray*)&inputs6, (TfLiteIntArray*)&outputs6, (TfLiteIntArray*)&inputs6, nullptr, const_cast<void*>(static_cast<const void*>(&opdata6)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs7, (TfLiteIntArray*)&outputs7, (TfLiteIntArray*)&inputs7, nullptr, const_cast<void*>(static_cast<const void*>(&opdata7)), nullptr, 45, },
  { (TfLiteIntArray*)&inputs8, (TfLiteIntArray*)&outputs8, (TfLiteIntArray*)&inputs8, nullptr, const_cast<void*>(static_cast<const void*>(&opdata8)), nullptr, 158, },
  { (TfLiteIntArray*)&inputs9, (TfLiteIntArray*)&outputs9, (TfLiteIntArray*)&inputs9, nullptr, const_cast<void*>(static_cast<const void*>(&opdata9)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs10, (TfLiteIntArray*)&outputs10, (TfLiteIntArray*)&inputs10, nullptr, const_cast<void*>(static_cast<const void*>(&opdata10)), nullptr, 45, },
  { (TfLiteIntArray*)&inputs11, (TfLiteIntArray*)&outputs11, (TfLiteIntArray*)&inputs11, nullptr, const_cast<void*>(static_cast<const void*>(&opdata11)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs12, (TfLiteIntArray*)&outputs12, (TfLiteIntArray*)&inputs12, nullptr, const_cast<void*>(static_cast<const void*>(&opdata12)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs13, (TfLiteIntArray*)&outputs13, (TfLiteIntArray*)&inputs13, nullptr, const_cast<void*>(static_cast<const void*>(&opdata13)), nullptr, 158, },
  { (TfLiteIntArray*)&inputs14, (TfLiteIntArray*)&outputs14, (TfLiteIntArray*)&inputs14, nullptr, const_cast<void*>(static_cast<const void*>(&opdata14)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs15, (TfLiteIntArray*)&outputs15, (TfLiteIntArray*)&inputs15, nullptr, const_cast<void*>(static_cast<const void*>(&opdata15)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs16, (TfLiteIntArray*)&outputs16, (TfLiteIntArray*)&inputs16, nullptr, const_cast<void*>(static_cast<const void*>(&opdata16)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs17, (TfLiteIntArray*)&outputs17, (TfLiteIntArray*)&inputs17, nullptr, const_cast<void*>(static_cast<const void*>(&opdata17)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs18, (TfLiteIntArray*)&outputs18, (TfLiteIntArray*)&inputs18, nullptr, const_cast<void*>(static_cast<const void*>(&opdata18)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs19, (TfLiteIntArray*)&outputs19, (TfLiteIntArray*)&inputs19, nullptr, const_cast<void*>(static_cast<const void*>(&opdata19)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs20, (TfLiteIntArray*)&outputs20, (TfLiteIntArray*)&inputs20, nullptr, const_cast<void*>(static_cast<const void*>(&opdata20)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs21, (TfLiteIntArray*)&outputs21, (TfLiteIntArray*)&inputs21, nullptr, const_cast<void*>(static_cast<const void*>(&opdata21)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs22, (TfLiteIntArray*)&outputs22, (TfLiteIntArray*)&inputs22, nullptr, const_cast<void*>(static_cast<const void*>(&opdata22)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs23, (TfLiteIntArray*)&outputs23, (TfLiteIntArray*)&inputs23, nullptr, const_cast<void*>(static_cast<const void*>(&opdata23)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs24, (TfLiteIntArray*)&outputs24, (TfLiteIntArray*)&inputs24, nullptr, const_cast<void*>(static_cast<const void*>(&opdata24)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs25, (TfLiteIntArray*)&outputs25, (TfLiteIntArray*)&inputs25, nullptr, const_cast<void*>(static_cast<const void*>(&opdata25)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs26, (TfLiteIntArray*)&outputs26, (TfLiteIntArray*)&inputs26, nullptr, const_cast<void*>(static_cast<const void*>(&opdata26)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs27, (TfLiteIntArray*)&outputs27, (TfLiteIntArray*)&inputs27, nullptr, const_cast<void*>(static_cast<const void*>(&opdata27)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs28, (TfLiteIntArray*)&outputs28, (TfLiteIntArray*)&inputs28, nullptr, const_cast<void*>(static_cast<const void*>(&opdata28)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs29, (TfLiteIntArray*)&outputs29, (TfLiteIntArray*)&inputs29, nullptr, const_cast<void*>(static_cast<const void*>(&opdata29)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs30, (TfLiteIntArray*)&outputs30, (TfLiteIntArray*)&inputs30, nullptr, const_cast<void*>(static_cast<const void*>(&opdata30)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs31, (TfLiteIntArray*)&outputs31, (TfLiteIntArray*)&inputs31, nullptr, const_cast<void*>(static_cast<const void*>(&opdata31)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs32, (TfLiteIntArray*)&outputs32, (TfLiteIntArray*)&inputs32, nullptr, const_cast<void*>(static_cast<const void*>(&opdata32)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs33, (TfLiteIntArray*)&outputs33, (TfLiteIntArray*)&inputs33, nullptr, const_cast<void*>(static_cast<const void*>(&opdata33)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs34, (TfLiteIntArray*)&outputs34, (TfLiteIntArray*)&inputs34, nullptr, const_cast<void*>(static_cast<const void*>(&opdata34)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs35, (TfLiteIntArray*)&outputs35, (TfLiteIntArray*)&inputs35, nullptr, const_cast<void*>(static_cast<const void*>(&opdata35)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs36, (TfLiteIntArray*)&outputs36, (TfLiteIntArray*)&inputs36, nullptr, const_cast<void*>(static_cast<const void*>(&opdata36)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs37, (TfLiteIntArray*)&outputs37, (TfLiteIntArray*)&inputs37, nullptr, const_cast<void*>(static_cast<const void*>(&opdata37)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs38, (TfLiteIntArray*)&outputs38, (TfLiteIntArray*)&inputs38, nullptr, const_cast<void*>(static_cast<const void*>(&opdata38)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs39, (TfLiteIntArray*)&outputs39, (TfLiteIntArray*)&inputs39, nullptr, const_cast<void*>(static_cast<const void*>(&opdata39)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs40, (TfLiteIntArray*)&outputs40, (TfLiteIntArray*)&inputs40, nullptr, const_cast<void*>(static_cast<const void*>(&opdata40)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs41, (TfLiteIntArray*)&outputs41, (TfLiteIntArray*)&inputs41, nullptr, const_cast<void*>(static_cast<const void*>(&opdata41)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs42, (TfLiteIntArray*)&outputs42, (TfLiteIntArray*)&inputs42, nullptr, const_cast<void*>(static_cast<const void*>(&opdata42)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs43, (TfLiteIntArray*)&outputs43, (TfLiteIntArray*)&inputs43, nullptr, const_cast<void*>(static_cast<const void*>(&opdata43)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs44, (TfLiteIntArray*)&outputs44, (TfLiteIntArray*)&inputs44, nullptr, const_cast<void*>(static_cast<const void*>(&opdata44)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs45, (TfLiteIntArray*)&outputs45, (TfLiteIntArray*)&inputs45, nullptr, const_cast<void*>(static_cast<const void*>(&opdata45)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs46, (TfLiteIntArray*)&outputs46, (TfLiteIntArray*)&inputs46, nullptr, const_cast<void*>(static_cast<const void*>(&opdata46)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs47, (TfLiteIntArray*)&outputs47, (TfLiteIntArray*)&inputs47, nullptr, const_cast<void*>(static_cast<const void*>(&opdata47)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs48, (TfLiteIntArray*)&outputs48, (TfLiteIntArray*)&inputs48, nullptr, const_cast<void*>(static_cast<const void*>(&opdata48)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs49, (TfLiteIntArray*)&outputs49, (TfLiteIntArray*)&inputs49, nullptr, const_cast<void*>(static_cast<const void*>(&opdata49)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs50, (TfLiteIntArray*)&outputs50, (TfLiteIntArray*)&inputs50, nullptr, const_cast<void*>(static_cast<const void*>(&opdata50)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs51, (TfLiteIntArray*)&outputs51, (TfLiteIntArray*)&inputs51, nullptr, const_cast<void*>(static_cast<const void*>(&opdata51)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs52, (TfLiteIntArray*)&outputs52, (TfLiteIntArray*)&inputs52, nullptr, const_cast<void*>(static_cast<const void*>(&opdata52)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs53, (TfLiteIntArray*)&outputs53, (TfLiteIntArray*)&inputs53, nullptr, const_cast<void*>(static_cast<const void*>(&opdata53)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs54, (TfLiteIntArray*)&outputs54, (TfLiteIntArray*)&inputs54, nullptr, const_cast<void*>(static_cast<const void*>(&opdata54)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs55, (TfLiteIntArray*)&outputs55, (TfLiteIntArray*)&inputs55, nullptr, const_cast<void*>(static_cast<const void*>(&opdata55)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs56, (TfLiteIntArray*)&outputs56, (TfLiteIntArray*)&inputs56, nullptr, const_cast<void*>(static_cast<const void*>(&opdata56)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs57, (TfLiteIntArray*)&outputs57, (TfLiteIntArray*)&inputs57, nullptr, const_cast<void*>(static_cast<const void*>(&opdata57)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs58, (TfLiteIntArray*)&outputs58, (TfLiteIntArray*)&inputs58, nullptr, const_cast<void*>(static_cast<const void*>(&opdata58)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs59, (TfLiteIntArray*)&outputs59, (TfLiteIntArray*)&inputs59, nullptr, const_cast<void*>(static_cast<const void*>(&opdata59)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs60, (TfLiteIntArray*)&outputs60, (TfLiteIntArray*)&inputs60, nullptr, const_cast<void*>(static_cast<const void*>(&opdata60)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs61, (TfLiteIntArray*)&outputs61, (TfLiteIntArray*)&inputs61, nullptr, const_cast<void*>(static_cast<const void*>(&opdata61)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs62, (TfLiteIntArray*)&outputs62, (TfLiteIntArray*)&inputs62, nullptr, const_cast<void*>(static_cast<const void*>(&opdata62)), nullptr, 57, },
  { (TfLiteIntArray*)&inputs63, (TfLiteIntArray*)&outputs63, (TfLiteIntArray*)&inputs63, nullptr, const_cast<void*>(static_cast<const void*>(&opdata63)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs64, (TfLiteIntArray*)&outputs64, (TfLiteIntArray*)&inputs64, nullptr, const_cast<void*>(static_cast<const void*>(&opdata64)), nullptr, 61, },
  { (TfLiteIntArray*)&inputs65, (TfLiteIntArray*)&outputs65, (TfLiteIntArray*)&inputs65, nullptr, const_cast<void*>(static_cast<const void*>(&opdata65)), nullptr, 49, },
  { (TfLiteIntArray*)&inputs66, (TfLiteIntArray*)&outputs66, (TfLiteIntArray*)&inputs66, nullptr, const_cast<void*>(static_cast<const void*>(&opdata66)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs67, (TfLiteIntArray*)&outputs67, (TfLiteIntArray*)&inputs67, nullptr, const_cast<void*>(static_cast<const void*>(&opdata67)), nullptr, 45, },
  { (TfLiteIntArray*)&inputs68, (TfLiteIntArray*)&outputs68, (TfLiteIntArray*)&inputs68, nullptr, const_cast<void*>(static_cast<const void*>(&opdata68)), nullptr, 142, },
  { (TfLiteIntArray*)&inputs69, (TfLiteIntArray*)&outputs69, (TfLiteIntArray*)&inputs69, nullptr, const_cast<void*>(static_cast<const void*>(&opdata69)), nullptr, 39, },
  { (TfLiteIntArray*)&inputs70, (TfLiteIntArray*)&outputs70, (TfLiteIntArray*)&inputs70, nullptr, const_cast<void*>(static_cast<const void*>(&opdata70)), nullptr, 138, },
  { (TfLiteIntArray*)&inputs71, (TfLiteIntArray*)&outputs71, (TfLiteIntArray*)&inputs71, nullptr, const_cast<void*>(static_cast<const void*>(&opdata71)), nullptr, 30, },
  { (TfLiteIntArray*)&inputs72, (TfLiteIntArray*)&outputs72, (TfLiteIntArray*)&inputs72, nullptr, const_cast<void*>(static_cast<const void*>(&opdata72)), nullptr, 0, },
  { (TfLiteIntArray*)&inputs73, (TfLiteIntArray*)&outputs73, (TfLiteIntArray*)&inputs73, nullptr, const_cast<void*>(static_cast<const void*>(&opdata73)), nullptr, 0, },
  { (TfLiteIntArray*)&inputs74, (TfLiteIntArray*)&outputs74, (TfLiteIntArray*)&inputs74, nullptr, const_cast<void*>(static_cast<const void*>(&opdata74)), nullptr, 0, },
};
used_operators_e used_ops[] = {
OP_XC_pad_3_to_4, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_pad, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_XC_conv2d_v2, OP_XC_ld_flash, OP_CONV_2D, OP_RESHAPE, OP_SOFTMAX, };


// Scratch buffer variables
int scratch_buffer_idx = 0;
const int scratch_buffer_offsets[28] = { 56848, 0, 512, 0, 896, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
tflite::MicroContext mc;

// Xcore context and thread variables
xc_context_config_t xc_config;
constexpr int kStackWordsPerThread = 256;
constexpr int threadsStackSizeInUint64 = 1 * kStackWordsPerThread/2;
// We use uint64_t for xcThreadsStack so that it is aligned to 8 bytes
uint64_t xcThreadsStack[threadsStackSizeInUint64];

// Functions to be used as function pointers for TfLiteContext and MicroContext 
static void* AllocatePersistentBuffer(struct TfLiteContext* ctx,
                                                 size_t bytes) {
  static uint8_t *AllocPtr = tensor_arena + sizeof(tensor_arena);

  AllocPtr -= bytes;
  return AllocPtr;
}

static TfLiteEvalTensor *GetEvalTensor(const struct TfLiteContext *context,
                                       int tensor_idx) {
  return (TfLiteEvalTensor*)&tflTensors[tensor_idx];
}

static TfLiteStatus RequestScratchBufferInArena(struct TfLiteContext *context, size_t bytes,
                                       int *buffer_idx) {
  *buffer_idx = scratch_buffer_idx++;
  return kTfLiteOk;
};

static void *GetScratchBuffer(struct TfLiteContext *context,
                                       int buffer_idx) {
  return tensor_arena + scratch_buffer_offsets[buffer_idx];
}

static TfLiteTensor* AllocateTempInputTensor(const TfLiteNode* node, int index) {
      return &ctx.tensors[node->inputs->data[index]];
}

static TfLiteTensor* AllocateTempOutputTensor(const TfLiteNode* node, int index) {
      return &ctx.tensors[node->outputs->data[index]];
}

static void DeallocateTempTfLiteTensor(TfLiteTensor* tensor) {
}

static void* external_context() {
  return &xc_config;
}

} // namespace

TfLiteStatus model_init(void *flash_data) {
  // Set flash data in xcore context config
  xc_config.flash_data = flash_data;

  // Setup microcontext functions
  mc.AllocateTempInputTensor = &AllocateTempInputTensor;
  mc.AllocateTempOutputTensor = &AllocateTempOutputTensor;
  mc.DeallocateTempTfLiteTensor = &DeallocateTempTfLiteTensor;
  mc.external_context = &external_context;

  // Setup tflitecontext functions
  ctx.AllocatePersistentBuffer = &AllocatePersistentBuffer;
  ctx.GetEvalTensor = &GetEvalTensor;
  ctx.RequestScratchBufferInArena = &RequestScratchBufferInArena;
  ctx.GetScratchBuffer = &GetScratchBuffer;
  
  // Set microcontext as the context ptr
  ctx.impl_ = (void*)&mc;
  ctx.tensors = tflTensors;
  ctx.tensors_size = 106;
  registrations[OP_XC_pad_3_to_4] = *(tflite::ops::micro::xcore::Register_XC_pad_3_to_4());
  registrations[OP_XC_pad] = *(tflite::ops::micro::xcore::Register_XC_pad());
  registrations[OP_XC_ld_flash] = *(tflite::ops::micro::xcore::Register_XC_ld_flash());
  registrations[OP_XC_conv2d_v2] = *(tflite::ops::micro::xcore::Register_XC_conv2d_v2());
  registrations[OP_CONV_2D] = tflite::Register_CONV_2D();
  registrations[OP_RESHAPE] = tflite::ops::micro::Register_RESHAPE();
  registrations[OP_SOFTMAX] = tflite::Register_SOFTMAX();


#ifdef TFLMC_XCORE_PROFILE
  printf("\nProfiling init()...");
  memset(op_times, 0, sizeof(op_times));
#endif

  for(size_t i = 0; i < 75; ++i) {
    if (registrations[used_ops[i]].init) {

#ifdef TFLMC_XCORE_PROFILE
      asm volatile ("gettime %0" : "=r" (time_t0));
#endif

      tflNodes[i].user_data = registrations[used_ops[i]].init(&ctx, (const char*)tflNodes[i].builtin_data, tflNodes[i].custom_initial_data_size);

#ifdef TFLMC_XCORE_PROFILE
      asm volatile ("gettime %0" : "=r" (time_t1));
      op_times[used_ops[i]] += time_t1 - time_t0;
      printf("\nnode %-5d %-32s %-12d", i, op_strs[used_ops[i]], time_t1 - time_t0);
#endif

    }
  }

#ifdef TFLMC_XCORE_PROFILE
    printf("\n\nCumulative times for init()...");
    for(int i=0; i<OP_LAST; i++){
      printf("\n%-32s %-12d", op_strs[i], op_times[i]);
    }
  printf("\n");
  printf("\nProfiling prepare()...");
  memset(op_times, 0, sizeof(op_times));
#endif

  for(size_t i = 0; i < 75; ++i) {
    if (registrations[used_ops[i]].prepare) {

#ifdef TFLMC_XCORE_PROFILE
      asm volatile ("gettime %0" : "=r" (time_t0));
#endif

      TfLiteStatus status = registrations[used_ops[i]].prepare(&ctx, &tflNodes[i]);

#ifdef TFLMC_XCORE_PROFILE
      asm volatile ("gettime %0" : "=r" (time_t1));
      op_times[used_ops[i]] += time_t1 - time_t0;
      printf("\nnode %-5d %-32s %-12d", i, op_strs[used_ops[i]], time_t1 - time_t0);
#endif

      if (status != kTfLiteOk) {
        return status;
      }
    }
  }

#ifdef TFLMC_XCORE_PROFILE
    printf("\n\nCumulative times for prepare()...");
    for(int i=0; i<OP_LAST; i++){
      printf("\n%-32s %-12d", op_strs[i], op_times[i]);
    }
  printf("\n");
#endif

  return kTfLiteOk;
}

static const int inTensorIndices[] = {
  0, 
};
TfLiteTensor* model_input(int index) {
  return &ctx.tensors[inTensorIndices[index]];
}

static const int outTensorIndices[] = {
  105, 
};
TfLiteTensor* model_output(int index) {
  return &ctx.tensors[outTensorIndices[index]];
}

#ifdef TFLMC_PRINT_TENSORS
unsigned char checksum(char *data, unsigned int length)
{
  static char sum;
  static char * end;
  sum = 0;
  end = data + length;

  do
  {
      sum -= *data++;
  } while (data != end);
  return sum;
}
#endif

TfLiteStatus model_invoke() {
  xc_config.thread_info.nstackwords = kStackWordsPerThread;
  xc_config.thread_info.stacks = &xcThreadsStack[threadsStackSizeInUint64 - 1];
  thread_init_1(&xc_config.thread_info);

#ifdef TFLMC_XCORE_PROFILE
  printf("\nProfiling invoke()...");
  memset(op_times, 0, sizeof(op_times));
  memset(op_counts, 0, sizeof(op_counts));
  op_times_summed = 0;
#endif

#ifdef TFLMC_PRINT_TENSORS
printf("[\n");
#endif

  for(size_t i = 0; i < 75; ++i) {

#ifdef TFLMC_PRINT_INPUT_TENSORS
    // print every input tensor
    printf("\nnode in %d", i);
    for (int j=0; j<tflNodes[i].inputs->size; j++){
      printf("\ntensor %d, input %d, %d bytes, checksum %d\n", tflNodes[i].inputs->data[j], j, tflTensors[tflNodes[i].inputs->data[j]].bytes, checksum(tflTensors[tflNodes[i].inputs->data[j]].data.raw, tflTensors[tflNodes[i].inputs->data[j]].bytes));
      for(int k=0; k<tflTensors[tflNodes[i].inputs->data[j]].bytes; k++){
        printf("%d,", (int8_t)tflTensors[tflNodes[i].inputs->data[j]].data.raw[k]);
      }
    }
    printf("\n");
#endif

#ifdef TFLMC_XCORE_PROFILE
  asm volatile ("gettime %0" : "=r" (time_t0));
#endif

    TfLiteStatus status = registrations[used_ops[i]].invoke(&ctx, &tflNodes[i]);

#ifdef TFLMC_XCORE_PROFILE
  asm volatile ("gettime %0" : "=r" (time_t1));
  op_times[used_ops[i]] += time_t1 - time_t0;
  op_counts[used_ops[i]] += 1;
  printf("\nnode %-5d %-32s %-12d", i, op_strs[used_ops[i]], time_t1 - time_t0);
#endif

#ifdef TFLMC_PRINT_TENSORS
    // print every output tensor
    printf("\n{\"node\" : \"%d\", \"op\" : \"%s\", \"data\" : [", i, op_strs[used_ops[i]]);
    for (int j=0; j<tflNodes[i].outputs->size; j++){
      printf("\n{\"tensor\" : %d, \"output\" : %d, \"bytes\" : %d, \"checksum\" : %d,\n", tflNodes[i].outputs->data[j], j, tflTensors[tflNodes[i].outputs->data[j]].bytes, checksum(tflTensors[tflNodes[i].outputs->data[j]].data.raw, tflTensors[tflNodes[i].outputs->data[j]].bytes));
      printf("\"val\" : [");
      for(int k=0; k<tflTensors[tflNodes[i].outputs->data[j]].bytes; k++){
        printf("%d", (int8_t)tflTensors[tflNodes[i].outputs->data[j]].data.raw[k]);
        if (k < tflTensors[tflNodes[i].outputs->data[j]].bytes-1){
          printf(",");
        }
      }
      if(j<tflNodes[i].outputs->size-1){
        printf("]},\n");
      } else {
        printf("]}]\n");
      }
    }

    if(i<75-1){
      printf("},\n");
    } else {
      printf("}\n");
    }
#endif

    if (status != kTfLiteOk) {
      thread_destroy(&xc_config.thread_info);
      return status;
    }
  }
#ifdef TFLMC_PRINT_TENSORS
printf("\n]");
#endif

  thread_destroy(&xc_config.thread_info);

#ifdef TFLMC_XCORE_PROFILE
  struct convopdata{
    const char * name;
    size_t thread_count;
    int evalStartTime;
    int threadsStartTime;
    int threadsDoneTime;
  };
  int conv_times1 = 0, conv_times2 = 0;
  printf("\n\nConv()...");
  for(size_t i = 0; i < 75; ++i) {
    if(used_ops[i] == OP_XC_conv2d_v2) {
      auto *op_data = reinterpret_cast<convopdata *>(tflNodes[i].user_data);
      conv_times1 += op_data->threadsStartTime - op_data->evalStartTime;
      conv_times2 += op_data->threadsDoneTime - op_data->threadsStartTime;
      printf("\nnode %-5d %-25s %-25s %-6d %-6d %-12d", i, op_strs[used_ops[i]], op_data->name, op_data->thread_count, op_data->threadsStartTime - op_data->evalStartTime, op_data->threadsDoneTime - op_data->threadsStartTime);
    }
  }
  printf("\nSummed - %-10d %-10d", conv_times1, conv_times2);

  printf("\n\nCumulative times for invoke()...");
  for(int i=0; i<OP_LAST; i++){
    op_times_summed += op_times[i];
    printf("\n%-5d %-32s %-12d %dms", op_counts[i], op_strs[i], op_times[i], op_times[i]/100000);
  }
  printf("\n\nTotal time for invoke() - %-10lld %lldms\n\n", op_times_summed, op_times_summed/100000);
#endif

  return kTfLiteOk;
}
