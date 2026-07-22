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
#define FW_VERSION "1.1.0"
#define FW_DATE    "2026-07-22"
#define FW_IDN     "PSOC 5LP Trigger; FW:" FW_VERSION "; DATE:" FW_DATE

#define LINE_MAX 64

/* Error codes: ERROR: <n> */
#define ERR_UNSUPPORTED 0
#define ERR_SWEEPING    1

// MUX CHANNEL
#define MUX_CH_RAW 0x00u
#define MUX_CH_DIV 0x01u

static char line_buf[LINE_MAX];
static uint8 line_len = 0;

static uint count = 2;                 /* SIG:TRIG:DIV default */
static int time_count = 0;
static int time = 0;                   /* last sweep duration [us] */
static float frequency = 0.0f;         /* last trigger frequency [Hz] */
static uint total_count = 0;           /* last sweep trigger count */

/* SYS:TRIG:NOT: 0=OFF 1=TIME 2=COUNT 3=FREQ 4=ALL */
static int notify_mode = 0;

static char tx[80];

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

static void apply_divider(unsigned n)
{   
    // Attemping to divice by 1 or 0, use direct ouput from comparator
    if (n == 1u || n == 0u) {
        muxSel_Write(MUX_CH_RAW);
    }
    else {
        muxSel_Write(MUX_CH_DIV);
        count = n;
        Counter_WritePeriod(count);
        Counter_WriteCompare((int)(n / 2u) - 1); //Duty Cycle 50%
    }

}

/* End of laser sweep */
CY_ISR(INT_SW)
{
    int last_time_count = time_count;
    int time_current_counter = Timer_ReadCounter();

    time = last_time_count + (time_current_counter / 24);
    total_count = freqcounter_ReadCounter();

    if (time > 0) {
        frequency = (float)total_count / ((float)time * 1e-6f);
    } else {
        frequency = 0.0f;
    }

    if (notify_mode != 0) {
        switch (notify_mode) {
            case 1:
                sprintf(tx, "%d\r\n", time);
                break;
            case 2:
                sprintf(tx, "%u\r\n", total_count);
                break;
            case 3:
                sprintf(tx, "%f\r\n", frequency);
                break;
            case 4:
                sprintf(tx, "%d %u %f\r\n", time, total_count, frequency);
                break;
            default:
                tx[0] = '\0';
                break;
        }
        if (tx[0] != '\0') {
            UART_PutString(tx);
        }
    }

    freqcounter_WriteCounter(0); // Once it is read we can reset (This is why we do not reset by hardware)
    time_count = 0;
}

CY_ISR(INT_TIMER)
{
    time_count = time_count + 1;
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
        sprintf(tx, "%u\r\n", count);
        UART_PutString(tx);
        return;
    }

    if (sscanf(arg, "%u", &n) != 1 || n == 0u || n > 9999999u) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    apply_divider(n);
    reply_ok();
}

static void cmd_sig_trig_events_count(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%u\r\n", total_count);
    UART_PutString(tx);
}

static void cmd_sig_trig_events_freq(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%f\r\n", frequency);
    UART_PutString(tx);
}

static void cmd_laser_swe_time(int is_query, const char *arg)
{
    (void)arg;
    if (!is_query) {
        reply_err(ERR_UNSUPPORTED);
        return;
    }
    sprintf(tx, "%d\r\n", time);
    UART_PutString(tx);
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
    freqcounter_Start();

    //Currently 2 interrupts:
    isr_timer_Start();
    isr_timer_StartEx(INT_TIMER);

    isr_sw_Start();
    isr_sw_StartEx(INT_SW);
    
    UART_Start();
    UART_PutString("Trigger ready\r\n");

    apply_divider(count);

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
