/* ========================================
 *
 * Sweep Source Trigger - PSoC 5LP firmware
 * Protocol v1.0 (SCPI-style, CRLF line endings)
 *
 * ========================================
 */
#include "project.h"
#include <stdio.h>
#include <string.h>

/* ---- Firmware identity (bump when releasing new FW) ---- */
#define FW_VERSION "1.1.13"
#define FW_DATE    "2026-07-23"
#define FW_IDN     "PSOC 5LP Trigger; FW:" FW_VERSION "; DATE:" FW_DATE

#define LINE_MAX 64

/* Error codes: ERROR: <n> */
#define ERR_UNSUPPORTED 0
#define ERR_SWEEPING    1

// MUX CHANNEL
#define MUX_CH_RAW 0x00u
#define MUX_CH_DIV 0x01u

// SRAM (DMA):
#define TS_MAX 2048 // Buffer size for time stamp (MAX FRAME TRIGGERED)
static uint32 ts_buf[TS_MAX]; // time stamp buffer 
static uint32 ts_count = 0u; // This is redundandt, it should be like trigger_events_count
static uint8 ts_overflow = 0u; // This is a flag to notify if we overflowed the buffer and notify the user

static uint8 dmaChan; // Reference of the DMA channel
static uint8 dmaTd; // Referece of the DMA transfer descriptor (the plan)

static char line_buf[LINE_MAX];
static uint8 line_len = 0;

static unsigned trigger_divider = 2u;   /* SIG:TRIG:DIV */

static int laser_sweep_eTime = 0;      /* last sweep duration [us] */
static float trigger_frequency = 0.0f;  /* last trigger frequency [Hz] */
static unsigned trigger_events_count = 0u; /* last sweep trigger count */

static unsigned laser_sweep_count = 0u;

/* SYS:TRIG:NOT: 0=OFF 1=TIME 2=COUNT 3=FREQ 4=ALL */
static int notify_mode = 0;

static char tx[80];

// DMA configuration:
// This init the DMA --> it will connect the Timer_CAPTURE_LSB_PTR to the ts_buf
static void TimestampDma_Init(void)
{
    dmaChan = DMA_DmaInitialize(
        4u, 1u,                                    /* 4-byte burst, 1 request per burst */
        HI16((uint32)Timer_CAPTURE_LSB_PTR),
        HI16((uint32)ts_buf));

    dmaTd = CyDmaTdAllocate();

    CyDmaTdSetConfiguration(dmaTd, (uint16)(TS_MAX * 4u),
                            DMA_DISABLE_TD, TD_INC_DST_ADR);
    CyDmaTdSetAddress(dmaTd,
        LO16((uint32)Timer_CAPTURE_LSB_PTR),
        LO16((uint32)ts_buf));

    CyDmaChSetInitialTd(dmaChan, dmaTd);
}

// DMA configuration:
// This arm the DMA --> it will start the DMA transfer
// This is used to reset the DMA and start the transfer
static void TimestampDma_Arm(void)
{
    CyDmaChDisable(dmaChan);
    Timer_ClearFIFO();

    CyDmaTdSetConfiguration(dmaTd, (uint16)(TS_MAX * 4u),
                            DMA_DISABLE_TD, TD_INC_DST_ADR);
    CyDmaTdSetAddress(dmaTd,
        LO16((uint32)Timer_CAPTURE_LSB_PTR),
        LO16((uint32)ts_buf));
    CyDmaChSetInitialTd(dmaChan, dmaTd);
    CyDmaClearPendingDrq(dmaChan);
    CyDmaChEnable(dmaChan, 1u);

    ts_count = 0u;
    ts_overflow = 0u;
}

// UART PROTOCOL STUFF:
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
    /* n==1: bypass Counter, route comparator straight to output via mux.
     * n>=2: divide with Counter (period=n, ~50% duty via compare). */
    if (n <= 1u) {
        muxSel_Write(MUX_CH_RAW);
    } else {
        muxSel_Write(MUX_CH_DIV);
        Counter_WritePeriod((uint16)n);
        Counter_WriteCompare((uint16)((n / 2u) - 1u)); /* ~50% duty */
    }
}


// Debugging:
static void TimestampDma_SendRaw(void)
{
    uint8 *p;
    uint32 nbytes;
    uint32 left;

    sprintf(tx, "TS %lu %u\r\n",
            (unsigned long)ts_count,
            (unsigned)ts_overflow);
    UART_PutString(tx); // Send the number of timestamps and if it overflowed

    p = (uint8 *)ts_buf;
    nbytes = ts_count * 4u;
    left = nbytes;
    while (left > 0u) {
        uint8 chunk = (left > 255u) ? 255u : (uint8)left;
        UART_PutArray(p, chunk);
        p += chunk;
        left -= chunk;
    }
}

/* End of laser sweep */
CY_ISR(INT_SW)
{
    CyDmaChDisable(dmaChan);
    /* Timer @ 24 MHz: 24 ticks = 1 us (enable/reset by sw) */
    uint32 cnt = Timer_ReadCounter();
    uint32 ticks = Timer_ReadPeriod() - cnt;
    laser_sweep_eTime = (int)(ticks / 24u); // In us
    trigger_events_count = freqcounter_ReadCounter();
    laser_sweep_count++;

    // Check if timestamps buffer overflowed
    ts_count = trigger_events_count;
    if (ts_count > TS_MAX) {
        ts_count = TS_MAX;
        ts_overflow = 1;
    }

    // Debug:
    /* Raw uint32 dump (no us conversion yet) */
    TimestampDma_SendRaw();

    // // Action:
    // if (laser_sweep_eTime > 0) {
    //     trigger_frequency = (float)trigger_events_count / ((float)laser_sweep_eTime * 1e-6f); // In Hz
    // } else {
    //     trigger_frequency = 0.0f;
    // }

    // if (notify_mode != 0) {
    //     switch (notify_mode) {
    //         case 1:
    //             sprintf(tx, "%d\r\n", laser_sweep_eTime);
    //             break;
    //         case 2:
    //             sprintf(tx, "%u\r\n", trigger_events_count);
    //             break;
    //         case 3:
    //             sprintf(tx, "%f\r\n", trigger_frequency);
    //             break;
    //         case 4:
    //             sprintf(tx, "%d %u %f\r\n", laser_sweep_eTime, trigger_events_count, trigger_frequency);
    //             break;
    //         default:
    //             tx[0] = '\0';
    //             break;
    //     }
    //     if (tx[0] != '\0') {
    //         UART_PutString(tx);
    //     }
    // }

    // Prepare for next sweep:
    TimestampDma_Arm(); // Rearm the DMA (basically point to the beginning of the buffer)
    freqcounter_WriteCounter(0); /* reset after read (no HW reset on this block) */
    Timer_WriteCounter(Timer_ReadPeriod());  /* Reset the timer and prepare next sweep */
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

    /* Reset only: LASER:SWE:COUNT 0 | RESET */
    if (!strcmp(arg, "0") || !strcmp(arg, "RESET")) {
        laser_sweep_count = 0u;
        reply_ok();
        return;
    }
    reply_err(ERR_UNSUPPORTED);
}

static void cmd_sys_trig_not(int is_query, const char *arg)
{
    if (is_query) {
        sprintf(tx, "%d\r\n", notify_mode);
        UART_PutString(tx);
        return;
    }

    if (!strcmp(arg, "OFF") || !strcmp(arg, "0")) {
        notify_mode = 0;
    } else if (!strcmp(arg, "TIME") || !strcmp(arg, "1")) {
        notify_mode = 1;
    } else if (!strcmp(arg, "COUNT") || !strcmp(arg, "2")) {
        notify_mode = 2;
    } else if (!strcmp(arg, "FREQ") || !strcmp(arg, "FREQUENCY") || !strcmp(arg, "3")) {
        notify_mode = 3;
    } else if (!strcmp(arg, "ALL") || !strcmp(arg, "4")) {
        notify_mode = 4;
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

/* Commands - Read/Write */
static const cmd_t cmds[] = {
    { "*IDN",                 cmd_idn,                    1, 0 },
    { "SIG:TRIG:DIV",         cmd_sig_trig_div,           1, 1 },
    { "SIG:TRIG:EVENTS:COUNT", cmd_sig_trig_events_count, 1, 0 },
    { "SIG:TRIG:EVENTS:FREQ", cmd_sig_trig_events_freq,   1, 0 },
    { "LASER:SWE:TIME",       cmd_laser_swe_time,         1, 0 },
    { "LASER:SWE:COUNT",      cmd_laser_swe_count,        1, 1 },
    { "SYS:TRIG:NOT",         cmd_sys_trig_not,           1, 1 },
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

int main(void)
{
    uint8 rx;

    CyGlobalIntEnable;

    Comp_Start();
    Counter_Start();
    Timer_Start();
    TimestampDma_Init();
    TimestampDma_Arm();
    freqcounter_Start();


    isr_sw_Start();
    isr_sw_StartEx(INT_SW);

    apply_divider(trigger_divider);
    
    UART_Start();
    UART_PutString("Trigger ready\r\n");

    for (;;) {
        if (UART_GetRxBufferSize() != 0u && sw_Read() == 1u) {
            rx = UART_GetChar();

            if (rx == '\r') {
                /* CRLF: ignore CR */
            } else if (rx == '\n') {
                line_buf[line_len] = '\0';
                dispatch_line(line_buf);
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

        /* During sweep: reject commands */
        if (UART_GetRxBufferSize() != 0u && sw_Read() == 0u) {
            rx = UART_GetChar();
            if (rx == '\n') {
                reply_err(ERR_SWEEPING);
            }
            UART_ClearRxBuffer();
            line_len = 0;
            line_buf[0] = '\0';
        }
    }
}

/* [] END OF FILE */
