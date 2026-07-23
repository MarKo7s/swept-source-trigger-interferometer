/* ========================================
 *
 * Sweep Source Trigger - PSoC 5LP firmware
 * Protocol v1.0 (SCPI-style, CRLF line endings)
 *
 * Timestamps: Timer.capture_out -> isr_timeStamp -> ReadCapture() into RAM,
 * then ConvertUs() → uint32 us; UART TSU may be absolute or delta-encoded.
 *
 * Sweep gate (this variant): active-low LEVEL on sw (not falling-edge).
 *
 * Creator wiring:
 *   count_out -> Timer.capture
 *   Timer.capture_out -> isr_timeStamp
 *
 * ========================================
 */
#include "project.h"
#include <stdio.h>
#include <string.h>

/* ---- Firmware identity (bump when releasing new FW) ---- */
#define FW_VERSION "1.3.1"
#define FW_DATE    "2026-07-23"
#define FW_IDN     "PSOC 5LP Trigger; FW:" FW_VERSION "; DATE:" FW_DATE

#define LINE_MAX 64

#define ERR_UNSUPPORTED 0
#define ERR_SWEEPING    1

#define MUX_CH_RAW 0x00u
#define MUX_CH_DIV 0x01u

/* SYS:TRIG:NOT modes */
#define NOTIFY_OFF       0
#define NOTIFY_TIME      1
#define NOTIFY_COUNT     2
#define NOTIFY_FREQ      3
#define NOTIFY_ALL       4
#define NOTIFY_TIMESTAMP 5

/* SYS:TIMESTAMP:DELTAENC */
#define DELTAENC_OFF     0
#define DELTAENC_UINT8   1
#define DELTAENC_UINT16  2

/* Max timestamps per sweep (ISR fill). Typical OCT A-line counts fit here. */
#define TS_MAX 4000u
/* Timer bus clock is 24 MHz → 24 ticks per microsecond. */
#define TIMER_TICKS_PER_US 24u

static uint32 ts_buf[TS_MAX];          /* raw Timer captures (ISR fill) */
static uint32 ts_us[TS_MAX];           /* us from sweep start (post-convert) */
static volatile uint32 ts_count = 0u;  /* filling this sweep */
static volatile uint8 ts_overflow = 0u;
static volatile uint8 ts_armed = 0u;
/* Frozen after each sweep for SIG:TRIG:TIMESTAMP? / NOTIFY_TIMESTAMP. */
static uint32 ts_count_last = 0u;
static uint8 ts_overflow_last = 0u;

static char line_buf[LINE_MAX];
static uint8 line_len = 0;

static unsigned trigger_divider = 2u;
static int laser_sweep_eTime = 0;
static float trigger_frequency = 0.0f;
static unsigned trigger_events_count = 0u;
static unsigned laser_sweep_count = 0u;

static int notify_mode = NOTIFY_OFF;
static int delta_enc = DELTAENC_OFF;

static char tx[96];

static uint32 Timer_ReadCounterLive(void)
{
    (void)CY_GET_REG8(Timer_COUNTER_LSB_PTR_8BIT);
    return CY_GET_REG32(Timer_COUNTER_LSB_PTR);
}

static void Timestamp_Arm(void)
{
    uint8 s;

    ts_armed = 0u;
    Timer_ClearFIFO();
    ts_count = 0u;
    ts_overflow = 0u;
    s = Timer_ReadStatusRegister();
    (void)s;
    isr_timeStamp_ClearPending();
    ts_armed = 1u;
}

static uint32 Timestamp_Finalize(void)
{
    ts_armed = 0u;

    while (0u != (Timer_ReadStatusRegister() & Timer_STATUS_FIFONEMP)) {
        if (ts_count >= TS_MAX) {
            ts_overflow = 1u;
            (void)Timer_ReadCapture();
        } else {
            ts_buf[ts_count++] = Timer_ReadCapture();
        }
    }
    return ts_count;
}

CY_ISR(INT_TIMESTAMP)
{
    while (0u != (Timer_ReadStatusRegister() & Timer_STATUS_FIFONEMP)) {
        if (!ts_armed) {
            (void)Timer_ReadCapture();
        } else if (ts_count >= TS_MAX) {
            ts_overflow = 1u;
            (void)Timer_ReadCapture();
        } else {
            ts_buf[ts_count++] = Timer_ReadCapture();
        }
    }
}

static void reply_ok(void)
{
    UART_PutString("OK\r\n");
}

static void reply_err(int code)
{
    sprintf(tx, "ERROR: %d\r\n", code);
    UART_PutString(tx);
}

static void str_toupper(char *s)
{
    for (; *s; ++s) {
        if (*s >= 'a' && *s <= 'z') {
            *s = (char)(*s - 32);
        }
    }
}

static void apply_divider(uint16_t n)
{
    if (n <= 1u) {
        muxSel_Write(MUX_CH_RAW);
    } else {
        muxSel_Write(MUX_CH_DIV);
        Counter_WritePeriod((uint16)n);
        Counter_WriteCompare((uint16)((n / 2u) - 1u));
    }
}

/* Mean trigger freq [Hz] from first/last raw captures over (n-1) intervals. */
static float calculate_trigger_frequency_us(uint32 n0, uint32 n1, uint32 n,
                                            uint32 period)
{
    uint32 t0_us;
    uint32 t1_us;
    uint32 dt_us;

    if (n < 2u) {
        return 0.0f;
    }

    t0_us = (period - n0) / TIMER_TICKS_PER_US;
    t1_us = (period - n1) / TIMER_TICKS_PER_US;

    if (t1_us <= t0_us) {
        return 0.0f;
    }
    dt_us = t1_us - t0_us;

    return (float)(n - 1u) / ((float)dt_us * 1e-6f);
}

/* (period - capture) / 24 → us from sweep start (uint32; ~4294 s max). */
static void Timestamp_ConvertUs(uint32 period)
{
    uint32 i;
    uint32 n = ts_count_last;

    for (i = 0u; i < n; i++) {
        ts_us[i] = (period - ts_buf[i]) / TIMER_TICKS_PER_US;
    }
}

static void uart_put_nbytes(const uint8 *p, uint32 nbytes)
{
    uint32 left = nbytes;

    while (left > 0u) {
        uint8 chunk = (left > 255u) ? 255u : (uint8)left;
        UART_PutArray((uint8 *)p, chunk);
        p += chunk;
        left -= chunk;
    }
}

/* Header always: TSU <n> <ov> <fc> <t0> <enc>\r\n */
static void Timestamp_SendUs(void)
{
    uint32 n = ts_count_last;
    uint32 t0 = (n > 0u) ? ts_us[0] : 0u;

    sprintf(tx, "TSU %lu %u %u %lu %u\r\n",
            (unsigned long)n,
            (unsigned)ts_overflow_last,
            trigger_events_count,
            (unsigned long)t0,
            (unsigned)DELTAENC_OFF);
    UART_PutString(tx);
    uart_put_nbytes((const uint8 *)ts_us, n * 4u);
}

/* enc=1: (n-1) * uint8 deltas (clamp→ov). */
static void Timestamp_SendDeltaU8(void)
{
    uint32 n = ts_count_last;
    uint32 i;
    uint8 ov = ts_overflow_last;
    uint32 t0 = (n > 0u) ? ts_us[0] : 0u;

    for (i = 1u; i < n; i++) {
        if ((ts_us[i] - ts_us[i - 1u]) > 255u) {
            ov = 1u;
            break;
        }
    }

    sprintf(tx, "TSU %lu %u %u %lu %u\r\n",
            (unsigned long)n,
            (unsigned)ov,
            trigger_events_count,
            (unsigned long)t0,
            (unsigned)DELTAENC_UINT8);
    UART_PutString(tx);

    for (i = 1u; i < n; i++) {
        uint32 d = ts_us[i] - ts_us[i - 1u];
        UART_PutChar((d > 255u) ? 255u : (uint8)d);
    }
}

/* enc=2: (n-1) * uint16 LE deltas (clamp→ov). */
static void Timestamp_SendDeltaU16(void)
{
    uint32 n = ts_count_last;
    uint32 i;
    uint8 ov = ts_overflow_last;
    uint32 t0 = (n > 0u) ? ts_us[0] : 0u;
    uint8 pair[2];

    for (i = 1u; i < n; i++) {
        if ((ts_us[i] - ts_us[i - 1u]) > 65535u) {
            ov = 1u;
            break;
        }
    }

    sprintf(tx, "TSU %lu %u %u %lu %u\r\n",
            (unsigned long)n,
            (unsigned)ov,
            trigger_events_count,
            (unsigned long)t0,
            (unsigned)DELTAENC_UINT16);
    UART_PutString(tx);

    for (i = 1u; i < n; i++) {
        uint32 d = ts_us[i] - ts_us[i - 1u];
        uint16 v = (d > 65535u) ? 65535u : (uint16)d;
        pair[0] = (uint8)(v & 0xFFu);
        pair[1] = (uint8)((v >> 8) & 0xFFu);
        UART_PutArray(pair, 2u);
    }
}

static void Timestamp_Send(void)
{
    switch (delta_enc) {
        case DELTAENC_UINT8:
            Timestamp_SendDeltaU8();
            break;
        case DELTAENC_UINT16:
            Timestamp_SendDeltaU16();
            break;
        default:
            Timestamp_SendUs();
            break;
    }
}

/* End-of-sweep UART notify (same call site for all modes). */
static void notify_sweep(void)
{
    switch (notify_mode) {
        case NOTIFY_TIME:
            sprintf(tx, "%d\r\n", laser_sweep_eTime);
            UART_PutString(tx);
            break;
        case NOTIFY_COUNT:
            sprintf(tx, "%u\r\n", trigger_events_count);
            UART_PutString(tx);
            break;
        case NOTIFY_FREQ:
            sprintf(tx, "%f\r\n", trigger_frequency);
            UART_PutString(tx);
            break;
        case NOTIFY_ALL:
            sprintf(tx, "%d %u %f\r\n",
                    laser_sweep_eTime, trigger_events_count, trigger_frequency);
            UART_PutString(tx);
            break;
        case NOTIFY_TIMESTAMP:
            Timestamp_Send();
            break;
        default: /* OFF or unknown */
            break;
    }
}

CY_ISR(INT_SW)
{
    uint32 n;
    uint32 period;

    n = Timestamp_Finalize();
    trigger_events_count = freqcounter_ReadCounter();
    laser_sweep_count++;

    if (n >= TS_MAX) {
        n = TS_MAX;
        ts_overflow = 1u;
    }
    if (trigger_events_count > TS_MAX) {
        ts_overflow = 1u;
    }

    /* Snapshot for queries before Arm() resets the live fill counter. */
    ts_count_last = n;
    ts_overflow_last = ts_overflow;
    ts_count = n;

    period = Timer_ReadPeriod();
    laser_sweep_eTime = (int)((period - Timer_ReadCounterLive()) / TIMER_TICKS_PER_US);

    if (n >= 2u) {
        trigger_frequency = calculate_trigger_frequency_us(
            ts_buf[0], ts_buf[n - 1u], n, period);
    } else {
        trigger_frequency = 0.0f;
    }

    Timestamp_ConvertUs(period);
    notify_sweep();

    Timestamp_Arm();
    freqcounter_WriteCounter(0);
    Timer_WriteCounter(period);
}

/* ---- Command handlers ---- */

static void cmd_idn(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    UART_PutString(FW_IDN "\r\n");
}

static void cmd_sig_trig_div(int is_query, const char *arg)
{
    unsigned n = 0;

    if (is_query) {
        sprintf(tx, "%u\r\n", trigger_divider);
        UART_PutString(tx);
        return;
    }

    if (sscanf(arg, "%u", &n) != 1 || n == 0u || n > 65535u) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    apply_divider(n);
    trigger_divider = n;
    reply_ok();
}

static void cmd_sig_trig_events_count(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%u\r\n", trigger_events_count);
    UART_PutString(tx);
}

static void cmd_sig_trig_events_freq(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%f\r\n", trigger_frequency);
    UART_PutString(tx);
}

/* Pull last sweep timestamps (header + uint32 us payload). */
static void cmd_sig_trig_timestamp(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    if (sw_Read() == 0u) {
        reply_err(ERR_SWEEPING);
        return;
    }
    Timestamp_Send();
}

static void cmd_laser_swe_time(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%d\r\n", laser_sweep_eTime);
    UART_PutString(tx);
}

static void cmd_laser_swe_count(int is_query, const char *arg)
{
    if (is_query) {
        sprintf(tx, "%u\r\n", laser_sweep_count);
        UART_PutString(tx);
        return;
    }

    if (!strcmp(arg, "0") || !strcmp(arg, "RESET")) {
        laser_sweep_count = 0u;
        reply_ok();
        return;
    }
    reply_err(ERR_UNSUPPORTED);
}

/* 1 = sweeping (SW low), 0 = idle. Allowed while sweeping. */
static void cmd_laser_swe_status(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%u\r\n", (sw_Read() == 0u) ? 1u : 0u);
    UART_PutString(tx);
}

static void cmd_sys_timestamp_deltaenc(int is_query, const char *arg)
{
    if (is_query) {
        if (delta_enc == DELTAENC_UINT8) {
            UART_PutString(
                "UINT8 WARNING max_dt_us=255 min_freq_hz=3922\r\n");
        } else if (delta_enc == DELTAENC_UINT16) {
            UART_PutString(
                "UINT16 WARNING max_dt_us=65535 min_freq_hz=15\r\n");
        } else {
            UART_PutString("OFF\r\n");
        }
        return;
    }

    if (!strcmp(arg, "OFF") || !strcmp(arg, "0")) {
        delta_enc = DELTAENC_OFF;
        reply_ok();
    } else if (!strcmp(arg, "UINT8") || !strcmp(arg, "1") ||
               !strcmp(arg, "8")) {
        delta_enc = DELTAENC_UINT8;
        UART_PutString(
            "OK - WARNING: max_dt_us=255 min_freq_hz=3922\r\n");
    } else if (!strcmp(arg, "UINT16") || !strcmp(arg, "2") ||
               !strcmp(arg, "16")) {
        delta_enc = DELTAENC_UINT16;
        UART_PutString(
            "OK - WARNING: max_dt_us=65535 min_freq_hz=15\r\n");
    } else {
        reply_err(ERR_UNSUPPORTED);
    }
}

static void cmd_sys_trig_not(int is_query, const char *arg)
{
    if (is_query) {
        sprintf(tx, "%d\r\n", notify_mode);
        UART_PutString(tx);
        return;
    }

    if (!strcmp(arg, "OFF") || !strcmp(arg, "0")) {
        notify_mode = NOTIFY_OFF;
    } else if (!strcmp(arg, "TIME") || !strcmp(arg, "1")) {
        notify_mode = NOTIFY_TIME;
    } else if (!strcmp(arg, "COUNT") || !strcmp(arg, "2")) {
        notify_mode = NOTIFY_COUNT;
    } else if (!strcmp(arg, "FREQ") || !strcmp(arg, "FREQUENCY") || !strcmp(arg, "3")) {
        notify_mode = NOTIFY_FREQ;
    } else if (!strcmp(arg, "ALL") || !strcmp(arg, "4")) {
        notify_mode = NOTIFY_ALL;
    } else if (!strcmp(arg, "TIMESTAMP") || !strcmp(arg, "TS") || !strcmp(arg, "5")) {
        notify_mode = NOTIFY_TIMESTAMP;
    } else {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    reply_ok();
}

typedef void (*cmd_fn)(int is_query, const char *arg);

typedef struct {
    const char *name;
    cmd_fn fn;
    int allow_query;
    int allow_set;
} cmd_t;

static const cmd_t cmds[] = {
    { "*IDN",                  cmd_idn,                    1, 0 },
    { "SIG:TRIG:DIV",          cmd_sig_trig_div,           1, 1 },
    { "SIG:TRIG:EVENTS:COUNT", cmd_sig_trig_events_count,  1, 0 },
    { "SIG:TRIG:EVENTS:FREQ",  cmd_sig_trig_events_freq,   1, 0 },
    { "SIG:TRIG:TIMESTAMP",    cmd_sig_trig_timestamp,     1, 0 },
    { "LASER:SWE:TIME",        cmd_laser_swe_time,         1, 0 },
    { "LASER:SWE:COUNT",       cmd_laser_swe_count,        1, 1 },
    { "LASER:SWE:STATUS",      cmd_laser_swe_status,       1, 0 },
    { "SYS:TIMESTAMP:DELTAENC", cmd_sys_timestamp_deltaenc, 1, 1 },
    { "SYS:TRIG:NOT",          cmd_sys_trig_not,           1, 1 },
};

static void dispatch_line(char *line)
{
    int is_query = 0;
    char *arg = "";
    char *sp;
    char *q;
    unsigned i;

    while (*line == ' ' || *line == '\t') {
        line++;
    }
    if (*line == '\0') {
        return;
    }

    q = strchr(line, '?');
    if (q != NULL && q[1] == '\0') {
        *q = '\0';
        is_query = 1;
    }

    sp = strchr(line, ' ');
    if (sp != NULL) {
        *sp = '\0';
        arg = sp + 1;
        while (*arg == ' ') {
            arg++;
        }
    }

    str_toupper(line);
    if (!is_query && arg[0] != '\0') {
        str_toupper(arg);
    }

    for (i = 0; i < (unsigned)(sizeof(cmds) / sizeof(cmds[0])); i++) {
        if (strcmp(line, cmds[i].name) != 0) {
            continue;
        }
        if (is_query && !cmds[i].allow_query) {
            reply_err(ERR_UNSUPPORTED);
            return;
        }
        if (!is_query && !cmds[i].allow_set) {
            reply_err(ERR_UNSUPPORTED);
            return;
        }
        cmds[i].fn(is_query, arg);
        return;
    }

    reply_err(ERR_UNSUPPORTED);
}

/* True if line is LASER:SWE:STATUS? (uses a local copy). */
static int is_laser_swe_status_query(const char *line)
{
    char tmp[LINE_MAX];
    char *p;
    char *q;
    unsigned i;

    for (i = 0u; i < (LINE_MAX - 1u) && line[i] != '\0'; i++) {
        tmp[i] = line[i];
    }
    tmp[i] = '\0';

    p = tmp;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    q = strchr(p, '?');
    if (q == NULL || q[1] != '\0') {
        return 0;
    }
    *q = '\0';
    str_toupper(p);
    return (strcmp(p, "LASER:SWE:STATUS") == 0) ? 1 : 0;
}

int main(void)
{
    uint8 rx;

    CyGlobalIntEnable;

    Comp_Start();
    Counter_Start();
    Timer_Start();
    freqcounter_Start();

    isr_timeStamp_StartEx(INT_TIMESTAMP);
    Timestamp_Arm();

    isr_sw_StartEx(INT_SW);

    apply_divider(trigger_divider);

    UART_Start();
    UART_PutString("Trigger ready\r\n");

    for (;;) {
        if (UART_GetRxBufferSize() == 0u) {
            continue;
        }

        rx = UART_GetChar();

        if (rx == '\r') {
            /* ignore CR */
        } else if (rx == '\n') {
            line_buf[line_len] = '\0';
            if (sw_Read() == 0u) {
                /* Sweeping: only LASER:SWE:STATUS? is answered. */
                if (is_laser_swe_status_query(line_buf)) {
                    dispatch_line(line_buf);
                } else {
                    reply_err(ERR_SWEEPING);
                }
            } else {
                dispatch_line(line_buf);
            }
            line_len = 0;
            line_buf[0] = '\0';
        } else {
            if (line_len < (LINE_MAX - 1u)) {
                line_buf[line_len++] = (char)rx;
            } else {
                line_len = 0;
                line_buf[0] = '\0';
                reply_err(ERR_UNSUPPORTED);
            }
        }
    }
}

/* [] END OF FILE */
