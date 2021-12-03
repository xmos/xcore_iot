# Introduction

This code is a copy of the tflite-micro sources in the XCore SDK, at commit 4c5c556. It has been modified to run from flash by
adding:  

    XCORE_CODE_SECTION_ATTRIBUTE

To the following code symbols:

    Too many to enumerate

And adding:

    #include "swmem_macros.h"

to the top of every file containing a linked symbol.
