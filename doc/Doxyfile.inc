# This file provides overrides to the Doxyfile configuration

PROJECT_NAME = XCore SDK
PROJECT_BRIEF = "XCore Software Development Kit"

INPUT += ../modules/rtos/drivers 
INPUT += ../modules/lib_xs3_math 
#TODO: someday just add all the HIL modules w/ 
#INPUT += ../modules/hil
#INPUT += ../modules/hil/lib_xud/api ../modules/hil/lib_xud/examples/*/api

PREDEFINED = C_API= EXTERN_C= __attribute__((weak))=

USE_MATHJAX = YES
MATHJAX_FORMAT = HTML-CSS
MATHJAX_RELPATH = https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.5/

# Make short MATHJAX sequences prettier
ALIASES += math{1}="@f$\1@f$"
ALIASES += vector{1}="@f$\bar \1@f$"
ALIASES += operation{1}="@par Operation Performed:^^@f{align*}{ \1 @f}"

