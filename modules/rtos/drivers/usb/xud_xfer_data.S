// Copyright 2021 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include "XUD_USB_Defines.h"

#define XUD_RES_RST  -1
#define XUD_RES_OKAY  0
#define XUD_RES_CTL   1
#define XUD_RES_ERR   2

//XUD_Result_t xud_setup_data_get_finish(XUD_ep ep);
//                                       r0
.globl xud_setup_data_get_finish
.type xud_setup_data_get_finish, @function
.cc_top xud_setup_data_get_finish.func
.align 4
xud_setup_data_get_finish:
#if defined(__XS2A__) || defined(__XS3A__)
.issue_mode single
  ENTSP_lu6 0
#endif
                                                // We must reset PID toggling on SETUP (both IN AND OUT)
#ifdef __XS3A__
    ldc         r11, USB_PIDn_DATA1
#else
    ldc         r11, USB_PID_DATA1
#endif
    stw         r11, r0[4]                      // Reset OUT toggle

    ldc         r11, 640                        // Assuming MAX_NUM_EPS is 16 and struct size is 40 here!
    add         r11, r0, r11

    ldc         r0, USB_PIDn_DATA1
    stw         r0, r11[4]                      // Reset IN toggle

    ldc         r0, XUD_RES_OKAY                // Return no error XUD_RES_OKAY

    retsp       0


.size xud_setup_data_get_finish, .-xud_setup_data_get_finish
.cc_bottom xud_setup_data_get_finish.func
.globl xud_setup_data_get_finish.nstackwords
.globl xud_setup_data_get_finish.maxchanends
.globl xud_setup_data_get_finish.maxtimers
.globl xud_setup_data_get_finish.maxcores
.set xud_setup_data_get_finish.nstackwords, 0
.set xud_setup_data_get_finish.maxchanends, 0
.set xud_setup_data_get_finish.maxtimers, 0
.set xud_setup_data_get_finish.maxcores, 1
.globl xud_setup_data_get_finish.locnoside
.globl xud_setup_data_get_finish.locnochandec
.globl xud_setup_data_get_finish.locnoglobalaccess
.globl xud_setup_data_get_finish.locnointerfaceaccess
.globl xud_setup_data_get_finish.locnonotificationselect
.set xud_setup_data_get_finish.locnoside, 1
.set xud_setup_data_get_finish.locnochandec, 1
.set xud_setup_data_get_finish.locnoglobalaccess, 1
.set xud_setup_data_get_finish.locnointerfaceaccess, 1
.set xud_setup_data_get_finish.locnonotificationselect, 1



//XUD_Result_t xud_data_get_finish(XUD_ep ep);
//                                 r0
.globl xud_data_get_finish
.type xud_data_get_finish, @function
.cc_top xud_data_get_finish.func
.align 4
xud_data_get_finish:
#if defined(__XS2A__) || defined(__XS3A__)
.issue_mode single
  ENTSP_lu6 0
#endif

    ldw         r11, r0[5]                      // Load EP type
    bf          r11, xud_data_get_finish_return // If this is an ISO EP, skip the PID toggle and check

                                                // Check PID
    ldw         r11, r0[6]                      // Load received PID from EP structure
    shr         r11, r11, 24                    // Shift off junk

    ldw         r1, r0[4]                       // Load expected PID
    xor         r11, r1, r11                    // Do the comparison
    bt          r11, xud_data_get_finish_badpkt

                                                // Toggle PID
#ifdef __XS3A__
    ldc         r2, 0x88
#else
    ldc         r2, 8
#endif
    xor         r1, r1, r2
    stw         r1, r0[4]

xud_data_get_finish_return:
    ldc         r0, XUD_RES_OKAY                // Return XUD_RES_OKAY
    retsp       0

xud_data_get_finish_badpkt:
    ldc         r0, XUD_RES_ERR                 // Return XUD_RES_ERR
    retsp       0

.size xud_data_get_finish, .-xud_data_get_finish
.cc_bottom xud_data_get_finish.func
.globl xud_data_get_finish.nstackwords
.globl xud_data_get_finish.maxchanends
.globl xud_data_get_finish.maxtimers
.globl xud_data_get_finish.maxcores
.set xud_data_get_finish.nstackwords, 0
.set xud_data_get_finish.maxchanends, 0
.set xud_data_get_finish.maxtimers, 0
.set xud_data_get_finish.maxcores, 1
.globl xud_data_get_finish.locnoside
.globl xud_data_get_finish.locnochandec
.globl xud_data_get_finish.locnoglobalaccess
.globl xud_data_get_finish.locnointerfaceaccess
.globl xud_data_get_finish.locnonotificationselect
.set xud_data_get_finish.locnoside, 1
.set xud_data_get_finish.locnochandec, 1
.set xud_data_get_finish.locnoglobalaccess, 1
.set xud_data_get_finish.locnointerfaceaccess, 1
.set xud_data_get_finish.locnonotificationselect, 1


/*
 * Note: This is essentially XUD_SetData_Select() from XUD_EpFuncs.S.
 * It returns the status, rather than passing it through a reference
 * parameter.
 */
//XUD_Result_t xud_data_set_finish(chanend_t c, XUD_ep ep);
//                                 r0,          r1
.globl xud_data_set_finish
.type xud_data_set_finish, @function
.cc_top xud_data_set_finish.func
.align 4
xud_data_set_finish:
#if defined(__XS2A__) || defined(__XS3A__)
.issue_mode single
  ENTSP_lu6 0
#endif
    testct      r3, res[r0]
    bt          r3, xud_data_set_finish_reset

    in          r11, res[r0]                     // Data sent okay
    ldc         r0, XUD_RES_OKAY                 // Return XUD_RES_OKAY for success.

    ldw         r11, r1[5]
    bf          r11, xud_data_set_finish_donepid // Don't do any toggling for ISO

                                                 // Toggle PID
    ldw         r11, r1[4]                       // Load EP PID from structure
    ldc         r3, 0x88
    xor         r11, r11, r3
    stw         r11, r1[4]                       // Store back PID

xud_data_set_finish_donepid:
    retsp       0

xud_data_set_finish_reset:
    mkmsk       r0, 32                           // Return -1 (XUD_RES_RST) as result
  //ldw         r2, r1[0] //mystery instruction  // Load mem address of EP in XUD ep table
    retsp       0

.size xud_data_set_finish, .-xud_data_set_finish
.cc_bottom xud_data_set_finish.func
.globl xud_data_set_finish.nstackwords
.globl xud_data_set_finish.maxchanends
.globl xud_data_set_finish.maxtimers
.globl xud_data_set_finish.maxcores
.set xud_data_set_finish.nstackwords, 0
.set xud_data_set_finish.maxchanends, 0
.set xud_data_set_finish.maxtimers, 0
.set xud_data_set_finish.maxcores, 1
.globl xud_data_set_finish.locnoside
.globl xud_data_set_finish.locnochandec
.globl xud_data_set_finish.locnoglobalaccess
.globl xud_data_set_finish.locnointerfaceaccess
.globl xud_data_set_finish.locnonotificationselect
.set xud_data_set_finish.locnoside, 1
.set xud_data_set_finish.locnochandec, 1
.set xud_data_set_finish.locnoglobalaccess, 1
.set xud_data_set_finish.locnointerfaceaccess, 1
.set xud_data_set_finish.locnonotificationselect, 1

