// Copyright 2023 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/port.h>
#include <stdio.h>

/* Store the version of the image in memory that will potentially be booted. */
int candidateImageVersion = -1;
/* Store the address of the image in memory that will potentially be booted. */
unsigned candidateImageAddress = 0;

/* Store the state of the buttons at init for image selection */
unsigned btn_val = 0;

enum interest
{
    NOT_INTERESTED = 0,
    INTERESTED = 1
};

#define BTN_A_IS_PRESSED    (((btn_val >> 0) & 0x01) == 0)
#define BTN_B_IS_PRESSED    (((btn_val >> 1) & 0x01) == 0)

void init(void)
{
    printf("loader init called\n");

    port_t p_btns = XS1_PORT_4D;
    port_enable(p_btns);
    btn_val = port_in(p_btns);
    port_disable(p_btns);
}

int checkCandidateImageVersion(int imageVersion)
{
    printf("loader checkCandidateImageVersion called\n");

    /* Reject all images if btn A is pressed to force load factory image */
    if (BTN_A_IS_PRESSED)
    {
        return NOT_INTERESTED;
    }

    return INTERESTED;
}

void recordCandidateImage(int imageVersion, unsigned imageAddress)
{
    candidateImageVersion = imageVersion;
    candidateImageAddress = imageAddress;
    printf("loader recordCandidateImage called\n");
}

unsigned reportSelectedImage(void)
{
    printf("loader reportSelectedImage called\n");
    return candidateImageAddress;
}