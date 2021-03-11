# Copyright 2020 XMOS LIMITED.
# This Software is subject to the terms of the XMOS Public Licence: Version 1.

import re

from Common.trace_processor import trace_processor
from Common.trace_record import record
from Common.decoded_record import decoded_record
from Common.arg_decoder import decode_args

regex_payload = "(\d+)(?::{1})(\d+)(?::{1})(\d+)(?:(?:,{1})(.*))*"


class trace_processor(trace_processor):
    def __init__(self, record_list, filter_args):
        super(trace_processor, self).__init__(record_list)
        self.filter_args = filter_args

    def print_records(self):
        for rec in self.record_list:
            print(rec)

    def print_decoded(self):
        for rec in self.decoded_list:
            print(rec)

    def decode(self, verbose=False):
        for each in self.record_list:
            p = re.search(regex_payload, each.Payload)
            if p:
                core = p.group(1)
                hwtick = p.group(2)
                trace = p.group(3)
                args = p.group(4)

                d_core = core
                d_trace = trace_map[int(trace)]
                d_hwtick = hwtick
                d_args = decode_args(int(trace), args)

                # print("Core:{0}    Trace:{1}    HWTick:{2}    Args:{3}".format(d_core, d_trace, d_hwtick, d_args))
                if self.filter_args[d_trace]:
                    self.decoded_list.append(
                        decoded_record(d_core, d_trace, d_hwtick, d_args)
                    )

    def process(self, verbose=False):
        for each in self.record_list:
            decoded = bytearray.fromhex(each.Payload).decode(encoding="utf-8")
            each.Payload = decoded
        return

    def sort_by_tick(self, verbose=False):
        self.decoded_list = sorted(self.decoded_list, key=lambda i: int(i.hwtick))


trace_map = [
    "USER_MSG",
    "START",
    "END",
    "MOVED_TASK_TO_READY_STATE",
    "POST_MOVED_TASK_TO_READY_STATE",
    "CREATE_MUTEX",
    "CREATE_MUTEX_FAILED",
    "GIVE_MUTEX_RECURSIVE",
    "GIVE_MUTEX_RECURSIVE_FAILED",
    "TAKE_MUTEX_RECURSIVE",
    "TAKE_MUTEX_RECURSIVE_FAILED",
    "CREATE_COUNTING_SEMAPHORE",
    "CREATE_COUNTING_SEMAPHORE_FAILED",
    "QUEUE_CREATE",
    "QUEUE_CREATE_FAILED",
    "QUEUE_SEND",
    "QUEUE_SEND_FAILED",
    "QUEUE_RECEIVE",
    "QUEUE_PEEK",
    "QUEUE_PEEK_FAILED",
    "QUEUE_PEEK_FROM_ISR",
    "QUEUE_RECEIVE_FAILED",
    "QUEUE_SEND_FROM_ISR",
    "QUEUE_SEND_FROM_ISR_FAILED",
    "QUEUE_RECEIVE_FROM_ISR",
    "QUEUE_RECEIVE_FROM_ISR_FAILED",
    "QUEUE_PEEK_FROM_ISR_FAILED",
    "QUEUE_DELETE",
    "QUEUE_REGISTRY_ADD",
    "BLOCKING_ON_QUEUE_SEND",
    "BLOCKING_ON_QUEUE_PEEK",
    "BLOCKING_ON_QUEUE_RECEIVE",
    "TASK_CREATE",
    "TASK_CREATE_FAILED",
    "TASK_DELETE",
    "TASK_DELAY_UNTIL",
    "TASK_DELAY",
    "TASK_PRIORITY_SET",
    "TASK_SUSPEND",
    "TASK_RESUME",
    "TASK_RESUME_FROM_ISR",
    "TASK_INCREMENT_TICK",
    "TASK_NOTIFY_TAKE_BLOCK",
    "TASK_NOTIFY_TAKE",
    "TASK_NOTIFY_WAIT_BLOCK",
    "TASK_NOTIFY_WAIT",
    "TASK_NOTIFY",
    "TASK_NOTIFY_FROM_ISR",
    "TASK_NOTIFY_GIVE_FROM_ISR",
    "TASK_PRIORITY_DISINHERIT",
    "TASK_PRIORITY_INHERIT",
    "TASK_SWITCHED_OUT",
    "TASK_SWITCHED_IN",
    "LOW_POWER_IDLE_BEGIN",
    "LOW_POWER_IDLE_END",
    "TIMER_CREATE",
    "TIMER_CREATE_FAILED",
    "TIMER_COMMAND_SEND",
    "TIMER_EXPIRED",
    "TIMER_COMMAND_RECEIVED",
    "MALLOC",
    "FREE",
    "EVENT_GROUP_CREATE",
    "EVENT_GROUP_CREATE_FAILED",
    "EVENT_GROUP_SYNC_BLOCK",
    "EVENT_GROUP_SYNC_END",
    "EVENT_GROUP_WAIT_BITS_BLOCK",
    "EVENT_GROUP_WAIT_BITS_END",
    "EVENT_GROUP_CLEAR_BITS",
    "EVENT_GROUP_CLEAR_BITS_FROM_ISR",
    "EVENT_GROUP_SET_BITS",
    "EVENT_GROUP_SET_BITS_FROM_ISR",
    "EVENT_GROUP_DELETE",
    "PEND_FUNC_CALL",
    "PEND_FUNC_CALL_FROM_ISR",
    "STREAM_BUFFER_CREATE_FAILED",
    "STREAM_BUFFER_CREATE_STATIC_FAILED",
    "STREAM_BUFFER_CREATE",
    "STREAM_BUFFER_DELETE",
    "STREAM_BUFFER_RESET",
    "BLOCKING_ON_STREAM_BUFFER_SEND",
    "STREAM_BUFFER_SEND",
    "STREAM_BUFFER_SEND_FAILED",
    "STREAM_BUFFER_SEND_FROM_ISR",
    "BLOCKING_ON_STREAM_BUFFER_RECEIVE",
    "STREAM_BUFFER_RECEIVE",
    "STREAM_BUFFER_RECEIVE_FAILED",
    "STREAM_BUFFER_RECEIVE_FROM_ISR",
    "INCREASE_TICK_COUNT",
    "TOTAL_TRACE_COUNT",
]
