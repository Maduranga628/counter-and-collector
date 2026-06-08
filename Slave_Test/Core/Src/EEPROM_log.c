/*
 * EEPROM_log.c
 *
 *  Created on: Apr 9, 2026
 *      Author: Maduranga
 */
#include "eeprom_log.h"
#include "string.h"

/* ── Write magic number to page 0 ───────────────────────────────────────*/
static void write_magic(void)
{
    uint8_t buf[2];
    buf[0] = EEPROM_MAGIC_HI;
    buf[1] = EEPROM_MAGIC_LO;
    EEPROM_Write(EEPROM_MAGIC_PAGE, 0, buf, 2);
}

/* ── Check if magic is valid ─────────────────────────────────────────────*/
static uint8_t check_magic(void)
{
    uint8_t buf[2];
    EEPROM_Read(EEPROM_MAGIC_PAGE, 0, buf, 2);
    return (buf[0] == EEPROM_MAGIC_HI && buf[1] == EEPROM_MAGIC_LO);
}

/* ── Get page number for a given session ─────────────────────────────────*/
static uint8_t session_page(uint8_t session_no)
{
    return EEPROM_SESSION_START_PAGE + session_no;
}

/* ── Init ────────────────────────────────────────────────────────────────*/
void EEPROM_LOG_Init(uint8_t *session_no_out, uint16_t *counter_out)
{
    if (!check_magic())
    {
        /* Fresh EEPROM — write magic and start from zero */
        write_magic();
        *session_no_out = 0;
        *counter_out    = 0;

        /* Write session 0 page with zeroed data so it exists */
        EEPROM_LOG_SaveCounter(0, 0);
        return;
    }

    /* Magic valid — scan forward to find last written session page */
    uint8_t last_valid_sn = 0;
    uint16_t last_valid_cnt = 0;

    for (uint8_t sn = 0; sn < EEPROM_MAX_SESSIONS; sn++)
    {
        uint8_t buf[6];
        EEPROM_Read(session_page(sn), EEPROM_SESSION_NO_OFFSET, buf, 6);

        /* Empty page — 0xFF means never written */
        if (buf[0] == 0xFF && buf[1] == 0xFF) break;

        last_valid_sn  = (uint8_t)buf[0] | ((uint8_t)buf[1] << 8);
        last_valid_cnt = (uint16_t)buf[2]
                       | ((uint16_t)buf[3] << 8)
                       | ((uint16_t)buf[4] << 16)
                       | ((uint16_t)buf[5] << 24);
    }

    *session_no_out = last_valid_sn;
    *counter_out    = last_valid_cnt;
}

/* ── Save counter to current session page (called on every increment) ───*/
void EEPROM_LOG_SaveCounter(uint8_t session_no, uint16_t counter)
{
    uint8_t buf[6];
    buf[0] = (uint8_t)(session_no & 0xFF);
    buf[1] = (uint8_t)(session_no >> 8);
    buf[2] = (uint8_t)(counter & 0xFF);
    buf[3] = (uint8_t)((counter >> 8)  & 0xFF);
    buf[4] = (uint8_t)((counter >> 16) & 0xFF);
    buf[5] = (uint8_t)((counter >> 24) & 0xFF);

    EEPROM_Write(session_page(session_no), EEPROM_SESSION_NO_OFFSET, buf, 6);
}

/* ── Commit session on reset button press ───────────────────────────────*/
/*    Old session is already saved. Just initialise the new session page. */
void EEPROM_LOG_CommitSession(uint8_t new_session_no)
{
    /* Write new session page with counter = 0 */
    EEPROM_LOG_SaveCounter(new_session_no, 0);
}

/* ── Read one session record ─────────────────────────────────────────────*/
uint8_t EEPROM_LOG_ReadSession(uint8_t session_no, SessionRecord_t *record_out)
{
    uint8_t buf[6];
    EEPROM_Read(session_page(session_no), EEPROM_SESSION_NO_OFFSET, buf, 6);

    if (buf[0] == 0xFF && buf[1] == 0xFF) return 0; /* empty */

    record_out->session_no = (uint8_t)buf[0] | ((uint8_t)buf[1] << 8);
    record_out->counter    = (uint16_t)buf[2]
                           | ((uint16_t)buf[3] << 8)
                           | ((uint16_t)buf[4] << 16)
                           | ((uint16_t)buf[5] << 24);
    return 1;
}

/* ── Get total session count ─────────────────────────────────────────────*/
uint8_t EEPROM_LOG_GetSessionCount(uint8_t current_session_no)
{
    /* Sessions are 0-indexed, so total = current + 1 */
    return current_session_no + 1;
}
