# This file provides overrides to the Doxyfile configuration

PROJECT_NAME = XCore SDK
PROJECT_BRIEF = "XCore Software Development Kit"

PREDEFINED = __DOXYGEN__=1 
PREDEFINED += DWORD_ALIGNED=
PREDEFINED += __attribute__((weak))=
PREDEFINED + = C_API= MA_C_API= C_API_START= C_API_END= EXTERN_C=

# HIL library APIs
INPUT += ../modules/hil/lib_i2c/api
INPUT += ../modules/hil/lib_i2s/api
INPUT += ../modules/hil/lib_qspi_io/api
INPUT += ../modules/hil/lib_spi/api
INPUT += ../modules/hil/lib_mic_array/lib_mic_array/lib_mic_array/api ../modules/hil/lib_mic_array/lib_mic_array/etc/vanilla
#INPUT += ../modules/hil/lib_l2_cache/api
#INPUT += ../modules/hil/lib_xud/api ../modules/hil/lib_xud/examples/*/api

# Other library APIs
INPUT += ../modules/lib_xs3_math 

# RTOS driver APIs
INPUT += ../modules/rtos/drivers 

# RTOS SW Services
INPUT += ../modules/rtos/sw_services/device_control/host ../modules/rtos/sw_services/device_control/api 

# Avona library APIs
INPUT += ../modules/avona/modules/lib_aec/api ../modules/avona/examples/bare-metal/shared_src/aec/
INPUT += ../modules/avona/modules/lib_ns/api
INPUT += ../modules/avona/modules/lib_agc/api
INPUT += ../modules/avona/modules/lib_adec/api
INPUT += ../modules/avona/modules/lib_ic/api
INPUT += ../modules/avona/modules/lib_vad/api

USE_MATHJAX = YES
MATHJAX_FORMAT = HTML-CSS
MATHJAX_RELPATH = https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/

# Make short MATHJAX sequences prettier
ALIASES += math{1}="@f$\1@f$"
ALIASES += vector{1}="@f$\bar \1@f$"
ALIASES += operation{1}="@par Operation Performed:^^@f{align*}{ \1 @f}"
ALIASES = "beginrst=^^\verbatim embed:rst^^"
ALIASES += "endrst=\endverbatim"
