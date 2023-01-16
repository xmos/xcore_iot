// Copyright 2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "xscope_endpoint.h"

#define VERSION "1.0.2"

// Abstraction for sleep portability
#if defined(__GNUC__) || defined(__MINGW32__)
#include <unistd.h>
#define SLEEP_MS(x)             usleep((x) * 1000)
#else
#include <windows.h>
#define SLEEP_MS(x)             Sleep(x)
#endif

#define XSTR(s)                 STR(s)
#define STR(x)                  #x
#define NUM_ELEMS(x)            (sizeof(x) / sizeof(x[0]))

/*
 * The xscope probe to process on.
 * Tracealyzer's trcStreamPort.c is currently expected to call xscope_bytes()
 * for probe ID 0.
 */
#define XSCOPE_PROBE_ID         0

#define MAX_LINE_BUFFER_BYTES   4096

/*
 * Enables additional informational logging while processing the PSF data.
 * This is mainly for development purposes.
 */
#define PRINT_PSF_EVENTS        0

/*
 * Enables printing records for probes other than XSCOPE_PROBE_ID when
 * the `--in-port` option is specified. This is mainly for development
 * purposes.
 */
#define PRINT_OTHER_RECORDS     0

typedef enum error_code {
    ERROR_NONE,
    ERROR_INTERNAL,
    ERROR_MUTUALLY_EXCLUSIVE_ARGS,
    ERROR_MISSING_ARG,
    ERROR_UNKOWN_ARG,
    ERROR_ARG_VALUE_MISSING,
    ERROR_ARG_VALUE_PARSING_FAILURE,
    ERROR_NOT_OPT_OR_FLAG,
    ERROR_INCOMPATIBLE_VCD,
    ERROR_DATA_TOO_SHORT,
    ERROR_FILE_SYSTEM,
    ERROR_OUT_OF_RESOURCES
} error_code_t;

typedef enum log_level {
    LOG_INF = 0x1,
    LOG_WRN = 0x2,
    LOG_ERR = 0x4
} log_level_t;

typedef enum process_psf_state {
    PROCESS_PSF_HEADER,
    PROCESS_PSF_TIMESTAMP,
    PROCESS_PSF_EVENT_TABLE_HEADER,
    PROCESS_PSF_EVENT_TABLE_ENTRY,
    PROCESS_PSF_EVENT
} process_psf_state_t;

typedef enum parsing_vcd_state {
    PARSING_VCD_HEADER,
    PARSING_VCD_RECORDS
} parsing_vcd_state_t;

// Type taken from from Tracealyzer sources.
typedef struct TraceHeader {
    uint32_t uiPSF;
    uint16_t uiVersion;
    uint16_t uiPlatform;
    uint32_t uiOptions;
    uint32_t uiNumCores;
    uint32_t isrTailchainingThreshold;
    char platformCfg[8];
    uint16_t uiPlatformCfgPatch;
    uint8_t uiPlatformCfgMinor;
    uint8_t uiPlatformCfgMajor;
} TraceHeader_t;

// Type taken from from Tracealyzer sources.
typedef struct TraceTimestamp
{
    uint32_t type;
    uint32_t frequency;
    uint32_t period;
    uint32_t wraparounds;
    uint32_t osTickHz;
    uint32_t latestTimestamp;
    uint32_t osTickCount;
} TraceTimestamp_t;

// A derivative of TraceEntryTable_t defined in trcEntryTable.c
typedef struct TraceEntryTableHeader
{
    uint32_t uiSlots;
    uint32_t uiEntrySymbolLength;
    uint32_t uiEntryStateCount;
} TraceEntryTableHeader_t;

/*
 * The available command line argument flags/options.
 */
static const char *help_arg[] = {"-h", "--help"};
static const char *version_arg[] = {"--version"};
static const char *verbose_arg[] = {"-v", "--verbose"};
static const char *stream_arg[] = {"-s", "--stream"};
static const char *print_endpoint_arg[] = {"-p", "--print-endpoint"};
static const char *delay_arg[] = {"-d", "--delay"};
static const char *input_file_arg[] = {"-i", "--in-file"};
static const char *input_port_arg[] = {"-I", "--in-port"};
static const char *output_file_arg[] = {"-o", "--out-file"};

static bool running = true;
static int event_count = 0;
static long long line_count = 0;
static process_psf_state_t psf_state = PROCESS_PSF_HEADER;
static TraceEntryTableHeader_t psf_evt_table;
static uint32_t psf_evt_entry = 0;
static uint16_t *event_cnts = NULL;
static uint16_t num_cores;

/*
 * Variables set by command line arguments.
 */
static log_level_t log_level = LOG_WRN;
static bool show_help = false;
static bool show_version = false;
static bool stream_mode = false;
static bool print_endpoint = false;
static int sleep_ms = 1000;
static char *input_host = NULL;
static char *input_port = NULL;
static char *input_filename = NULL;
static char *output_filename = NULL;
static FILE *out_file = NULL;

static void print_help(char *arg0)
{
    printf("Usage:\n");
    printf("    %s [-h] [--version]\n\n", arg0);
    printf("    %s [-v] [-s] [-d <DELAY_MS>] -i <IN_FILE> -o <OUT_FILE>\n\n",
           arg0);
    printf("    %s [-v] [-p] -I <HOST>:<PORT> -o <OUT_FILE>\n\n", arg0);
    printf("Generate a Percepio Streaming Format (PSF) file based on Tracealyzer data received\n"
           "via an xscope Value Change Dump (VCD) file or an xscope endpoint socket connection.\n\n");
    printf("Options:\n");
    printf("    -h, --help                  This help menu.\n");
    printf("        --version               Print the version of this tool.\n");
    printf("    -v, --verbose               Print verbose output.\n");
    printf("    -s, --stream                Once the end of a file has been reached,\n"
           "                                continue to wait for more data. Terminate\n"
           "                                execution via Ctrl+C or other means.\n");
    printf("    -p, --print-endpoint        When using -in-port, this option will enable\n"
           "                                reception of printf data on this xscope endpoint.\n");
    printf("    -d, --delay <DELAY_MS>      The time in milliseconds to sleep when waiting for more\n"
           "                                data on the input file stream. This option only applies\n"
           "                                for --stream. Default = 1000.\n");
    printf("    -i, --in-file <IN_FILE>     The VCD file to process. In stream mode, the\n"
           "                                application will wait for such a file to exist.\n");
    printf("    -I, --in-port <HOST>:<PORT> The host and port (separated by ':') on which\n"
           "                                xgdb's --xscope-port is serving on.\n"
           "                                Note: --stream is implied when using this mode.\n");
    printf("    -o, --out-file <OUT_FILE>   The PSF file to generate.\n");
}

static void write_log(log_level_t level, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    if (level >= log_level) {
        switch (level) {
        case LOG_WRN:
            printf("WARNING: ");
            break;
        case LOG_ERR:
            printf("ERROR: ");
            break;
        default:
            break;
        }
        vprintf(format, args);
    }
    va_end(args);
}

static void print_stream_status(void)
{
    write_log(LOG_INF, "[STREAM STATUS]\n");

    if (input_filename)
        write_log(LOG_INF, "- Read %lld lines\n", line_count);

    write_log(LOG_INF, "- Processed %d events\n", event_count + 1);
}

static void print_psf_header(TraceHeader_t *header)
{
    write_log(LOG_INF, "[PSF Header]\n");
    write_log(LOG_INF, "- Format Version: 0x%04X\n", header->uiVersion);
    write_log(LOG_INF, "- Options: 0x%08X\n", header->uiOptions);
    write_log(LOG_INF, "- Number of Cores: %d\n", header->uiNumCores);
    write_log(LOG_INF, "- Platform: %.8s\n", header->platformCfg);
    write_log(LOG_INF, "- Platform ID: 0x%04X\n", header->uiPlatform);
    write_log(LOG_INF, "- Platform Config: %d.%d Patch %d\n",
              header->uiPlatformCfgMajor, header->uiPlatformCfgMinor,
              header->uiPlatformCfgPatch);
    write_log(LOG_INF, "- ISR Tail-Chaining Threshold: %d\n",
              header->isrTailchainingThreshold);
}

static void print_psf_timestamp(TraceTimestamp_t *timestamp)
{
    write_log(LOG_INF, "[PSF Timestamp]\n");
    write_log(LOG_INF, "- Type: %d\n", timestamp->type);
    write_log(LOG_INF, "- Frequency: %d\n", timestamp->frequency);
    write_log(LOG_INF, "- Period: %d\n", timestamp->period);
    write_log(LOG_INF, "- Wraparounds: %d\n", timestamp->wraparounds);
    write_log(LOG_INF, "- OS Tick Hz: %d\n", timestamp->osTickHz);
    write_log(LOG_INF, "- Latest Timestamp: %d\n", timestamp->latestTimestamp);
    write_log(LOG_INF, "- OS Tick Count: %d\n", timestamp->osTickCount);
}

#if (PRINT_PSF_EVENTS == 1)
static void print_psf_event(unsigned char trace_bytes[], int num_trace_bytes)
{
    printf("[PSF EVENT] TS: 0x%02X%02X%02X%02X, Core: %d, Cnt: 0x%01X%02X, ID = 0x%02X%02X, Data Len: %3d, Data:",
           trace_bytes[7], trace_bytes[6], trace_bytes[5], trace_bytes[4],
           trace_bytes[3] >> 4,
           trace_bytes[3] & 0xF, trace_bytes[2],
           trace_bytes[1], trace_bytes[0],
           num_trace_bytes - 8);

    for (int i = 0; i < num_trace_bytes - 8; i++)
        printf(" %02X", trace_bytes[i + 8]);

    printf("\n");
}

static void print_psf_event_table_header(unsigned char trace_bytes[],
                                         int trace_length)
{
    write_log(LOG_INF, "[PSF Event Table]\n");
    write_log(LOG_INF, "- Slots: %d\n", psf_evt_table.uiSlots);
    write_log(LOG_INF, "- Entry Symbol Length: %d\n",
              psf_evt_table.uiEntrySymbolLength);
    write_log(LOG_INF, "- Entry State Count: %d\n",
              psf_evt_table.uiEntryStateCount);
}

static void print_psf_event_table_entry(unsigned char trace_bytes[],
                                        int trace_length)
{
    uint32_t states_offset = sizeof(uint32_t);
    uint32_t options_offset =
            (psf_evt_table.uiEntryStateCount + 1) * sizeof(uint32_t);
    uint32_t symbol_offset = options_offset + sizeof(uint32_t);

    write_log(LOG_INF, "[PSF Event Entry]\n");
    write_log(LOG_INF, "- Address: 0x%02X%02X%02X%02X\n", trace_bytes[3],
              trace_bytes[2], trace_bytes[1], trace_bytes[0]);
    write_log(LOG_INF, "- States:");
    for (uint32_t i = 0; i < psf_evt_table.uiEntryStateCount; i++) {
        write_log(LOG_INF, " 0x%02X%02X%02X%02X",
                  trace_bytes[(i * sizeof(uint32_t)) + states_offset + 3],
                  trace_bytes[(i * sizeof(uint32_t)) + states_offset + 2],
                  trace_bytes[(i * sizeof(uint32_t)) + states_offset + 1],
                  trace_bytes[(i * sizeof(uint32_t)) + states_offset]);
    }
    write_log(LOG_INF, "\n");
    write_log(LOG_INF, "- Options: 0x%02X%02X%02X%02X\n",
              trace_bytes[options_offset + 3], trace_bytes[options_offset + 2],
              trace_bytes[options_offset + 1], trace_bytes[options_offset]);
    write_log(LOG_INF, "- Symbol: %.*s\n", psf_evt_table.uiEntrySymbolLength,
              &trace_bytes[symbol_offset]);
}
#endif /* (PRINT_PSF_EVENTS == 1) */

static void print_record(unsigned int id, unsigned long long timestamp,
                         unsigned int length, unsigned long long data_val,
                         unsigned char *data_bytes)
{
    write_log(LOG_INF, "[PROBE %d] 0x%016llX ==> %lld", id, timestamp,
              data_val);
    for (unsigned int i = 0; i < length; i++) {
        if ((i == 0) || (i % 16) == 0)
            write_log(LOG_INF, "\n\t");
        else if ((i % 8) == 0)
            write_log(LOG_INF, " ");

        write_log(LOG_INF, "%02X ", data_bytes[i]);
    }

    write_log(LOG_INF, "\n");
}

static error_code_t process_psf_header(unsigned char trace_bytes[],
                                       int trace_length)
{
    const uint32_t expected_bom = 0x50534600;
    TraceHeader_t header;

    /* The xscope probe's first record should be the PSF header in
     * its entirety. */
    if (trace_length != sizeof(TraceHeader_t)) {
        write_log(LOG_ERR, "Incompatible PSF header length detected.\n");
        return ERROR_INCOMPATIBLE_VCD;
    }

    memcpy(&header, trace_bytes, sizeof(TraceHeader_t));

    // The magic cookie/BOM should always be "\0FSP" on xcore
    if (header.uiPSF != expected_bom) {
        write_log(LOG_ERR, "Incompatible PSF BOM detected.\n");
        return ERROR_INCOMPATIBLE_VCD;
    }

    print_psf_header(&header);

    if (header.uiNumCores > 0) {
        uint16_t data_size = header.uiNumCores * sizeof(uint16_t);
        event_cnts = malloc(data_size);

        if (event_cnts == NULL)
            return ERROR_OUT_OF_RESOURCES;

        num_cores = header.uiNumCores;

        /* Set each event count to 0xFFFF which is an invalid value
         * for the 12-bit counter. */
        memset(event_cnts, 0xFF, data_size);
    }

    return ERROR_NONE;
}

static error_code_t process_psf_timestamp(unsigned char trace_bytes[],
                                          int trace_length)
{
    TraceTimestamp_t timestamp;

    if (trace_length != sizeof(TraceTimestamp_t)) {
        write_log(LOG_ERR, "Incompatible PSF timestamp length detected.\n");
        return ERROR_INCOMPATIBLE_VCD;
    }

    memcpy(&timestamp, trace_bytes, sizeof(TraceTimestamp_t));
    print_psf_timestamp(&timestamp);
    return ERROR_NONE;
}

static error_code_t process_psf_event_table_header(unsigned char trace_bytes[],
                                                   int trace_length)
{
    if (trace_length != sizeof(TraceEntryTableHeader_t)) {
        write_log(LOG_ERR,
                  "Incompatible PSF event table header length detected.\n");
        return ERROR_INCOMPATIBLE_VCD;
    }

    memcpy(&psf_evt_table, trace_bytes, sizeof(TraceEntryTableHeader_t));

#if (PRINT_PSF_EVENTS == 1)
    print_psf_event_table_header(trace_bytes, trace_length);
#endif

    return ERROR_NONE;
}

static error_code_t process_psf_event_table_entry(unsigned char trace_bytes[],
                                                  int trace_length)
{
    uint32_t expected_len =
            (psf_evt_table.uiEntryStateCount + 2) * sizeof(uint32_t) +
            psf_evt_table.uiEntrySymbolLength;
    if (trace_length != expected_len) {
        write_log(LOG_ERR,
                  "Incompatible PSF event table header length detected.\n");
        return ERROR_INCOMPATIBLE_VCD;
    }

#if (PRINT_PSF_EVENTS == 1)
    print_psf_event_table_entry(trace_bytes, trace_length);
#endif

    return ERROR_NONE;
}

static void detect_missing_events(unsigned char trace_bytes[],
                                  int num_trace_bytes)
{
    const int core_id_offset = 3;
    const int evt_cnt_offset_lo = 2;
    const int evt_cnt_offset_hi = 3;
    const int hi_evt_cnt_mask = 0x0F;
    const uint16_t invalid_evt_cnt = 0xFFFF;

    uint16_t core_id = trace_bytes[core_id_offset] >> 4;

    if (event_cnts == NULL || core_id >= num_cores)
        return;

    uint16_t event_cnt =
        ((trace_bytes[evt_cnt_offset_hi] & hi_evt_cnt_mask) << 8) |
        trace_bytes[evt_cnt_offset_lo];

    if (event_cnts[core_id] != invalid_evt_cnt)
    {
        uint16_t event_cnt_delta =
            (event_cnt > event_cnts[core_id]) ?
            (event_cnt - event_cnts[core_id]) :
            ((0x1000 - event_cnts[core_id]) + event_cnt);

        if (event_cnt_delta > 1)
            write_log(LOG_WRN,
                "Detected %d missing events (Core %d @ Current %d).\n",
                event_cnt_delta - 1, core_id, event_cnt);
    }

    event_cnts[core_id] = event_cnt;
}

static void modify_trace_event_count(unsigned char trace_bytes[],
                                     int num_trace_bytes)
{
    const int core_id_offset = 3;
    const int evt_cnt_offset_lo = 2;
    const int evt_cnt_offset_hi = 3;
    const int core_id_mask = 0xF0;
    const int hi_evt_cnt_mask = 0x0F;
    char *evt_cnt = (char *)&event_count;

    /*
     * Modify the event counter while retaining core id. The trace's event
     * count is only 12-bit; however, a larger datatype is used by the
     * application in order to provide a status report for indicating the
     * total number of events processed.
     * NOTE: This data is part of TraceBaseEvent_t and is assembled via
     * TRC_EVENT_SET_EVENT_COUNT in the Tracealyzer unit.
     */

    trace_bytes[evt_cnt_offset_lo] = evt_cnt[0];
    trace_bytes[evt_cnt_offset_hi] =
            (trace_bytes[core_id_offset] & core_id_mask) |
            (evt_cnt[1] & hi_evt_cnt_mask);
    event_count++;
}

static error_code_t process_trace_event(unsigned char trace_bytes[],
                                        int num_trace_bytes)
{
    /*
     * Tracealyzer's FreeRTOS unit tracks event data on a per-core basis;
     * however, when viewing all cores simultaneously in Tracealyzer, the event
     * counter needs to be to be monotonically increasingly.
     */

    // Protect against accessing stale/garbage data
    if (num_trace_bytes < 4) {
        // Provide the line number in the file when processing a VCD file.
        if (input_filename) {
            write_log(LOG_WRN,
                    "Trace event data length too small (line %lld).\n",
                    line_count);
        } else {
            write_log(LOG_WRN, "Trace event data length too small.\n");
        }

        return ERROR_DATA_TOO_SHORT;
    }

#if (PRINT_PSF_EVENTS == 1)
    print_psf_event(trace_bytes, num_trace_bytes);
#endif

    detect_missing_events(trace_bytes, num_trace_bytes);
    modify_trace_event_count(trace_bytes, num_trace_bytes);

    return ERROR_NONE;
}

static error_code_t process_psf_data(unsigned char trace_bytes[],
                                     int trace_length)
{
    error_code_t res = ERROR_NONE;

    /*
     * Tracealyzer's prvSetRecorderEnabled() calls a sequence of functions that
     * write various metadata to the PSF file before events are written.
     * The state machine below handles this logic.
     */

    switch (psf_state) {
    case PROCESS_PSF_HEADER:
        res = process_psf_header(trace_bytes, trace_length);
        psf_state = PROCESS_PSF_TIMESTAMP;
        break;
    case PROCESS_PSF_TIMESTAMP:
        res = process_psf_timestamp(trace_bytes, trace_length);
        psf_state = PROCESS_PSF_EVENT_TABLE_HEADER;
        break;
    case PROCESS_PSF_EVENT_TABLE_HEADER:
        res = process_psf_event_table_header(trace_bytes, trace_length);
        psf_state = PROCESS_PSF_EVENT_TABLE_ENTRY;
        break;
    case PROCESS_PSF_EVENT_TABLE_ENTRY:
        psf_evt_entry++;
        res = process_psf_event_table_entry(trace_bytes, trace_length);

        if (psf_evt_entry >= psf_evt_table.uiSlots) {
            psf_state = PROCESS_PSF_EVENT;
        }
        break;
    case PROCESS_PSF_EVENT:
        res = process_trace_event(trace_bytes, trace_length);
        break;
    default:
        res = ERROR_INTERNAL;
        break;
    }

    return res;
}

static error_code_t process_vcd_file(FILE *input_file, FILE *output_file)
{
    int last_event_count = 0;
    parsing_vcd_state_t parsing_state = PARSING_VCD_HEADER;
    char line[MAX_LINE_BUFFER_BYTES];

    while (1) {
        const char delim[] = " \n\r";
        char *line_ptr = fgets(line, sizeof(line), input_file);

        if (line_ptr == NULL) {
            if (!stream_mode)
                break;

            if (last_event_count != event_count) {
                print_stream_status();
                last_event_count = event_count;
            }

            SLEEP_MS(sleep_ms);
            continue;
        }

        line_count++;

        /* Filter lines related to VCD header; afterwards, only process lines
         * that begin with 'l' which are expected to be run-length encoded
         * hex-strings representing the Tracealyzer PSF data. */
        if (parsing_state == PARSING_VCD_HEADER) {
            char *token = strtok(line, delim);
            const char *end_of_header = "$enddefinitions";

            if (strcmp(token, end_of_header) == 0)
                parsing_state = PARSING_VCD_RECORDS;

            continue;
        } else {
            bool is_trace_data = (line[0] == 'l');
            if (!is_trace_data)
                continue;
        }

        // The first token indicates the length of the data that follows
        char *len_field = strtok(line, delim);

        // The second token has the trace data that needs to be converted
        char *trace_data = strtok(NULL, delim);

        /* The third token indicates the probe ID. Skip lines not targeting the
         * expected probe ID. */
        char *scope_probe = strtok(NULL, delim);

        if (len_field == NULL || trace_data == NULL || scope_probe == NULL) {
            write_log(LOG_WRN, "Unexpected encoding (line %lld).\n",
                      line_count);
            continue;
        }

        if (strcmp(scope_probe, XSTR(XSCOPE_PROBE_ID)) != 0)
            continue;

        int trace_data_chars = strlen(trace_data);
        int decoded_trace_len;

        if ((sscanf(len_field, "l%d", &decoded_trace_len) != 1) ||
            (trace_data_chars != (decoded_trace_len << 1))) {
            write_log(LOG_WRN, "Unexpected encoding (line %lld).\n",
                      line_count);
            continue;
        }

        unsigned char trace_bytes[MAX_LINE_BUFFER_BYTES >> 1];

        // Convert the trace_data from a hex-string to an array of bytes
        for (int i = 0; i < decoded_trace_len; i++)
            sscanf(&trace_data[i << 1], "%02hhx", &trace_bytes[i]);

        error_code_t res = process_psf_data(trace_bytes, decoded_trace_len);
        if (res != ERROR_NONE && res != ERROR_DATA_TOO_SHORT)
            return res;

        if (fwrite(trace_bytes, sizeof(trace_bytes[0]), decoded_trace_len,
                   output_file) != decoded_trace_len)
            write_log(LOG_ERR, "Data lost while writing to file system.\n");
    }

    if (feof(input_file) && !stream_mode) {
        write_log(LOG_INF, "End of file reached.\n");
        write_log(LOG_INF, "Read %lld lines.\n", line_count);
        write_log(LOG_INF, "Processed %d events.\n", event_count + 1);
    }

    return ERROR_NONE;
}

static void xscope_exit_cb(void)
{
    running = false;
}

static void xscope_register_cb(unsigned int id, unsigned int type,
                               unsigned int r, unsigned int g, unsigned int b,
                               unsigned char *name, unsigned char *unit,
                               unsigned int data_type, unsigned char *data_name)
{
    if (!running)
        return;

    write_log(LOG_INF, "[REGISTERED] Probe ID: %d, Name: '%s'\n", id, name);
}

static void xscope_print_cb(unsigned long long timestamp, unsigned int length,
                            unsigned char *data)
{
    if (!running || (length == 0))
        return;

    printf("[PRINT] ");

    for (unsigned i = 0; i < length; i++)
        printf("%c", data[i]);
}

static void xscope_record_cb(unsigned int id, unsigned long long timestamp,
                             unsigned int length, unsigned long long data_val,
                             unsigned char *data_bytes)
{
    if (!running)
        return;

    if (id == XSCOPE_PROBE_ID) {
        error_code_t res = process_psf_data(data_bytes, length);
        if (res != ERROR_NONE && res != ERROR_DATA_TOO_SHORT) {
            running = false;
            return;
        }

        if (fwrite(data_bytes, sizeof(data_bytes[0]), length, out_file) !=
            length) {
            write_log(LOG_ERR, "Data lost while writing to file system.\n");
        }
    }
#if (PRINT_OTHER_RECORDS == 1)
    else {
        print_record(id, timestamp, length, data_val, data_bytes);
    }
#endif
}

static bool is_matching_arg(char *arg, const char *arg_options[],
                            int num_options)
{
    for(int i = 0; i < num_options; i++)
    {
        if (0 == strcmp(arg, arg_options[i]))
            return true;
    }

    return false;
}

static error_code_t next_arg_value(int argc, char *argv[], int *argi)
{
    if ((++(*argi) >= argc) || argv[*argi][0] == '-') {
        write_log(LOG_ERR, "Missing argument value (%s).\n", argv[*argi - 1]);
        return ERROR_ARG_VALUE_MISSING;
    }

    return ERROR_NONE;
}

static error_code_t process_args(int argc, char *argv[])
{
    bool in_port_present = false;
    bool in_file_present = false;
    bool out_file_present = false;

    for (int i = 1; i < argc; i++) {
        if (is_matching_arg(argv[i], help_arg, NUM_ELEMS(help_arg))) {
            show_help = true;
            return ERROR_NONE;
        } else if (is_matching_arg(argv[i], version_arg,
                                   NUM_ELEMS(version_arg))) {
            show_version = true;
            return ERROR_NONE;
        } else if (is_matching_arg(argv[i], input_file_arg,
                                   NUM_ELEMS(input_file_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            input_filename = argv[i];
            in_file_present = true;
        } else if (is_matching_arg(argv[i], input_port_arg,
                                   NUM_ELEMS(input_port_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            /* The argument follows similar format to --xscope-port, where
             * the value specified follows the form <host>:<port>. */
            const char delims[] = ":";
            input_host = strtok(argv[i], delims);
            input_port = strtok(NULL, delims);
            in_port_present = (input_host && input_port);
        } else if (is_matching_arg(argv[i], output_file_arg,
                                   NUM_ELEMS(output_file_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            output_filename = argv[i];
            out_file_present = true;
        } else if (is_matching_arg(argv[i], print_endpoint_arg,
                                   NUM_ELEMS(print_endpoint_arg))) {
            print_endpoint = true;
        } else if (is_matching_arg(argv[i], delay_arg, NUM_ELEMS(delay_arg))) {
            if (next_arg_value(argc, argv, &i) != ERROR_NONE)
                return ERROR_ARG_VALUE_MISSING;

            if (sscanf(argv[i], "%d", &sleep_ms) != 1) {
                write_log(LOG_ERR, "Argument value (%s) could not be parsed.\n",
                          argv[i]);
                return ERROR_ARG_VALUE_PARSING_FAILURE;
            }
        } else if (is_matching_arg(argv[i], stream_arg,
                                   NUM_ELEMS(stream_arg))) {
            stream_mode = true;
        } else if (is_matching_arg(argv[i], verbose_arg,
                                   NUM_ELEMS(verbose_arg))) {
            log_level = LOG_INF;
        } else {
            write_log(LOG_ERR, "Unkown argument (%s).\n", argv[i]);
            return ERROR_UNKOWN_ARG;
        }
    }

    if (in_port_present && in_file_present)
        return ERROR_MUTUALLY_EXCLUSIVE_ARGS;

    return ((in_port_present || in_file_present) && out_file_present) ?
                   ERROR_NONE :
                   ERROR_MISSING_ARG;
}

int main(int argc, char *argv[])
{
    int exit_code = process_args(argc, argv);
    FILE *in_file = NULL;

    if (show_help || exit_code) {
        print_help(argv[0]);
        return exit_code;
    } else if (show_version) {
        printf("version %s\n", VERSION);
        return exit_code;
    }

    // Setup the input data source based on the specified user arguments
    if (input_filename) {
        write_log(LOG_INF, "Opening input file ...\n");

        while (1) {
            in_file = fopen(input_filename, "r");

            if (in_file != NULL)
                break;

            if (stream_mode) {
                SLEEP_MS(1000);
            } else {
                write_log(LOG_INF, "File not found.\n");
                return ERROR_FILE_SYSTEM;
            }
        }
    } else {
        write_log(LOG_INF, "Configuring xscope callbacks ...\n");

        if (print_endpoint)
            xscope_ep_set_print_cb(xscope_print_cb);

        xscope_ep_set_register_cb(xscope_register_cb);
        xscope_ep_set_record_cb(xscope_record_cb);
        xscope_ep_set_exit_cb(xscope_exit_cb);
    }

    write_log(LOG_INF, "Opening output file ...\n");
    out_file = fopen(output_filename, "wb");

    if (out_file == NULL) {
        if (in_file != NULL)
            fclose(in_file);

        return ERROR_FILE_SYSTEM;
    }

    // Process the input data source based on the specified user arguments
    if (input_filename) {
        write_log(LOG_INF, "Processing file (Probe: %d) ...\n",
                  XSCOPE_PROBE_ID);
        exit_code = process_vcd_file(in_file, out_file);
    } else {
        write_log(LOG_INF,
                  "Connecting to xscope (Probe: %d, Host: %s, Port: %s) ...\n",
                  XSCOPE_PROBE_ID, input_host, input_port);
        int error = xscope_ep_connect(input_host, input_port);
        if (error) {
            running = false;
            write_log(LOG_ERR, "Failed to connect to xscope (%d).\n", error);
        }

        // While 'running' print out basic status info for user feedback.
        int last_event_count = 0;
        while (running) {
            if (last_event_count != event_count) {
                print_stream_status();
                last_event_count = event_count;
            }

            SLEEP_MS(1000);
        }

        write_log(LOG_INF, "Disconnecting from xscope ...\n");
        xscope_ep_disconnect();
    }

    write_log(LOG_INF, "Closing files ...\n");
    fclose(out_file);
    if (in_file != NULL)
        fclose(in_file);

    if (event_cnts != NULL)
        free(event_cnts);

    write_log(LOG_INF, "Done.\n");

    return exit_code;
}