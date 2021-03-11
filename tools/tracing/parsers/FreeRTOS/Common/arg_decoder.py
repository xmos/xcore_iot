# Copyright 2020 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.


def decode_args(traceID_num, traceArgs):
    retstr = ""
    if traceArgs != None:
        traceArgList = traceArgs.split(",")
        retstr = switch_dict[traceID_num](traceArgList)
    return retstr


# Define any custom user message handling in here
def USER_MSG(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def START(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def END(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def MOVED_TASK_TO_READY_STATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def POST_MOVED_TASK_TO_READY_STATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def CREATE_MUTEX(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def CREATE_MUTEX_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def GIVE_MUTEX_RECURSIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def GIVE_MUTEX_RECURSIVE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def TAKE_MUTEX_RECURSIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def TAKE_MUTEX_RECURSIVE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def CREATE_COUNTING_SEMAPHORE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def CREATE_COUNTING_SEMAPHORE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_CREATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def QUEUE_CREATE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_SEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_SEND_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_RECEIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_PEEK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_PEEK_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_PEEK_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_RECEIVE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_SEND_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_SEND_FROM_ISR_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_RECEIVE_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_RECEIVE_FROM_ISR_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_PEEK_FROM_ISR_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def QUEUE_DELETE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, string
def QUEUE_REGISTRY_ADD(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def BLOCKING_ON_QUEUE_SEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def BLOCKING_ON_QUEUE_PEEK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def BLOCKING_ON_QUEUE_RECEIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, string
def TASK_CREATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_CREATE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_DELETE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def TASK_DELAY_UNTIL(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_DELAY(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 3 arg(s) string, unsigned, unsigned
def TASK_PRIORITY_SET(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_SUSPEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_RESUME(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_RESUME_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def TASK_INCREMENT_TICK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_TAKE_BLOCK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_TAKE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_WAIT_BLOCK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_WAIT(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TASK_NOTIFY_GIVE_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) string, unsigned
def TASK_PRIORITY_DISINHERIT(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) string, unsigned
def TASK_PRIORITY_INHERIT(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_SWITCHED_OUT(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TASK_SWITCHED_IN(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def LOW_POWER_IDLE_BEGIN(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def LOW_POWER_IDLE_END(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TIMER_CREATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def TIMER_CREATE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 4 arg(s) string, int, int, int
def TIMER_COMMAND_SEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) string
def TIMER_EXPIRED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 3 arg(s) string, int, int
def TIMER_COMMAND_RECEIVED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) pointer, unsigned
def MALLOC(traceArgs):
    retstr = "Malloc {1} bytes at 0x{0}.".format(traceArgs[0], traceArgs[1])
    return retstr


# 2 arg(s) pointer, unsigned
def FREE(traceArgs):
    retstr = "Free {1} bytes at 0x{0}.".format(traceArgs[0], traceArgs[1])
    return retstr


# 1 arg(s) int
def EVENT_GROUP_CREATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 0 arg(s)
def EVENT_GROUP_CREATE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 3 arg(s) unsigned, unsigned, unsigned
def EVENT_GROUP_SYNC_BLOCK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 4 arg(s) unsigned, unsigned, unsigned, int
def EVENT_GROUP_SYNC_END(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, unsigned
def EVENT_GROUP_WAIT_BITS_BLOCK(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 3 arg(s) int, unsigned, int
def EVENT_GROUP_WAIT_BITS_END(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, unsigned
def EVENT_GROUP_CLEAR_BITS(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, unsigned
def EVENT_GROUP_CLEAR_BITS_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, unsigned
def EVENT_GROUP_SET_BITS(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, unsigned
def EVENT_GROUP_SET_BITS_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def EVENT_GROUP_DELETE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 4 arg(s) pointer, pointer, unsigned, int
def PEND_FUNC_CALL(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 4 arg(s) pointer, pointer, unsigned, int
def PEND_FUNC_CALL_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, int
def STREAM_BUFFER_CREATE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) int, int
def STREAM_BUFFER_CREATE_STATIC_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) pointer, int
def STREAM_BUFFER_CREATE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def STREAM_BUFFER_DELETE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def STREAM_BUFFER_RESET(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def BLOCKING_ON_STREAM_BUFFER_SEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) unsigned, int
def STREAM_BUFFER_SEND(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def STREAM_BUFFER_SEND_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) unsigned, int
def STREAM_BUFFER_SEND_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def BLOCKING_ON_STREAM_BUFFER_RECEIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) unsigned, int
def STREAM_BUFFER_RECEIVE(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) unsigned
def STREAM_BUFFER_RECEIVE_FAILED(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 2 arg(s) unsigned, int
def STREAM_BUFFER_RECEIVE_FROM_ISR(traceArgs):
    retstr = ""
    for each in traceArgs:
        retstr += each + " "
    return retstr


# 1 arg(s) int
def INCREASE_TICK_COUNT(traceArgs):
    retstr = "Add {0} to tick count.".format(traceArgs[0])
    return retstr


# not a trace
def TOTAL_TRACE_COUNT(traceArgs):
    retstr = "I should not have been called!"
    return retstr


switch_dict = {
    0: USER_MSG,
    1: START,
    2: END,
    3: MOVED_TASK_TO_READY_STATE,
    4: POST_MOVED_TASK_TO_READY_STATE,
    5: CREATE_MUTEX,
    6: CREATE_MUTEX_FAILED,
    7: GIVE_MUTEX_RECURSIVE,
    8: GIVE_MUTEX_RECURSIVE_FAILED,
    9: TAKE_MUTEX_RECURSIVE,
    10: TAKE_MUTEX_RECURSIVE_FAILED,
    11: CREATE_COUNTING_SEMAPHORE,
    12: CREATE_COUNTING_SEMAPHORE_FAILED,
    13: QUEUE_CREATE,
    14: QUEUE_CREATE_FAILED,
    15: QUEUE_SEND,
    16: QUEUE_SEND_FAILED,
    17: QUEUE_RECEIVE,
    18: QUEUE_PEEK,
    19: QUEUE_PEEK_FAILED,
    20: QUEUE_PEEK_FROM_ISR,
    21: QUEUE_RECEIVE_FAILED,
    22: QUEUE_SEND_FROM_ISR,
    23: QUEUE_SEND_FROM_ISR_FAILED,
    24: QUEUE_RECEIVE_FROM_ISR,
    25: QUEUE_RECEIVE_FROM_ISR_FAILED,
    26: QUEUE_PEEK_FROM_ISR_FAILED,
    27: QUEUE_DELETE,
    28: QUEUE_REGISTRY_ADD,
    29: BLOCKING_ON_QUEUE_SEND,
    30: BLOCKING_ON_QUEUE_PEEK,
    31: BLOCKING_ON_QUEUE_RECEIVE,
    32: TASK_CREATE,
    33: TASK_CREATE_FAILED,
    34: TASK_DELETE,
    35: TASK_DELAY_UNTIL,
    36: TASK_DELAY,
    37: TASK_PRIORITY_SET,
    38: TASK_SUSPEND,
    39: TASK_RESUME,
    40: TASK_RESUME_FROM_ISR,
    41: TASK_INCREMENT_TICK,
    42: TASK_NOTIFY_TAKE_BLOCK,
    43: TASK_NOTIFY_TAKE,
    44: TASK_NOTIFY_WAIT_BLOCK,
    45: TASK_NOTIFY_WAIT,
    46: TASK_NOTIFY,
    47: TASK_NOTIFY_FROM_ISR,
    48: TASK_NOTIFY_GIVE_FROM_ISR,
    49: TASK_PRIORITY_DISINHERIT,
    50: TASK_PRIORITY_INHERIT,
    51: TASK_SWITCHED_OUT,
    52: TASK_SWITCHED_IN,
    53: LOW_POWER_IDLE_BEGIN,
    54: LOW_POWER_IDLE_END,
    55: TIMER_CREATE,
    56: TIMER_CREATE_FAILED,
    57: TIMER_COMMAND_SEND,
    58: TIMER_EXPIRED,
    59: TIMER_COMMAND_RECEIVED,
    60: MALLOC,
    61: FREE,
    62: EVENT_GROUP_CREATE,
    63: EVENT_GROUP_CREATE_FAILED,
    64: EVENT_GROUP_SYNC_BLOCK,
    65: EVENT_GROUP_SYNC_END,
    66: EVENT_GROUP_WAIT_BITS_BLOCK,
    67: EVENT_GROUP_WAIT_BITS_END,
    68: EVENT_GROUP_CLEAR_BITS,
    69: EVENT_GROUP_CLEAR_BITS_FROM_ISR,
    70: EVENT_GROUP_SET_BITS,
    71: EVENT_GROUP_SET_BITS_FROM_ISR,
    72: EVENT_GROUP_DELETE,
    73: PEND_FUNC_CALL,
    74: PEND_FUNC_CALL_FROM_ISR,
    75: STREAM_BUFFER_CREATE_FAILED,
    76: STREAM_BUFFER_CREATE_STATIC_FAILED,
    77: STREAM_BUFFER_CREATE,
    78: STREAM_BUFFER_DELETE,
    79: STREAM_BUFFER_RESET,
    80: BLOCKING_ON_STREAM_BUFFER_SEND,
    81: STREAM_BUFFER_SEND,
    82: STREAM_BUFFER_SEND_FAILED,
    83: STREAM_BUFFER_SEND_FROM_ISR,
    84: BLOCKING_ON_STREAM_BUFFER_RECEIVE,
    85: STREAM_BUFFER_RECEIVE,
    86: STREAM_BUFFER_RECEIVE_FAILED,
    87: STREAM_BUFFER_RECEIVE_FROM_ISR,
    88: INCREASE_TICK_COUNT,
    89: TOTAL_TRACE_COUNT,
}
