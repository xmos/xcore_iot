# This file provides overrides to the Doxyfile configuration

PROJECT_NAME = XCORE-IOT
PROJECT_BRIEF = "XCORE-IOT"

EXTRACT_STATIC = YES

PREDEFINED = __DOXYGEN__=1 
PREDEFINED += DWORD_ALIGNED=
PREDEFINED += __attribute__((weak))=
PREDEFINED += C_API= MA_C_API= C_API_START= C_API_END= EXTERN_C=

# Core library APIs
INPUT += ../modules/core/modules/xcore_math/lib_xcore_math/lib_xcore_math/api ../modules/core/modules/xcore_math/lib_xcore_math/doc/programming_guide

# IO library APIs
INPUT += ../modules/io/modules/i2c/api
INPUT += ../modules/io/modules/i2s/api
INPUT += ../modules/io/modules/qspi_io/api
INPUT += ../modules/io/modules/spi/api
INPUT += ../modules/io/modules/uart/api
INPUT += ../modules/io/modules/mic_array/lib_mic_array/api ../modules/io/modules/mic_array/etc/vanilla ../modules/io/modules/mic_array/doc/programming_guide/src

# RTOS driver APIs
INPUT += ../modules/rtos/modules/drivers 

# RTOS SW Services
INPUT += ../modules/rtos/modules/sw_services/device_control/host ../modules/rtos/modules/sw_services/device_control/api 

USE_MATHJAX = YES
MATHJAX_FORMAT = HTML-CSS
MATHJAX_RELPATH = https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/

# Make short MATHJAX sequences prettier
ALIASES += math{1}="@f$\1@f$"
ALIASES += vector{1}="@f$\bar \1@f$"
ALIASES += operation{1}="@par Operation Performed:^^@parblock^^@f{flalign*}{ \1 && @f}^^@endparblock"

ALIASES += "beginrst=^^\verbatim embed:rst^^"
ALIASES += "endrst=\endverbatim"
