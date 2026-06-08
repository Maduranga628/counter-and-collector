/*
 * EEPROM_log.h
 *
 *  Created on: Apr 9, 2026
 *      Author: Maduranga
 */

#ifndef INC_EEPROM_LOG_H_
#define INC_EEPROM_LOG_H_

#include "stdint.h"
#include "EEPROM.h"

/* ── EEPROM layout ─────────────────────────────────────────────────────────
 *  Page 0  : [magic_hi (1B)][magic_lo (1B)][padding (62B)]
 *  Page 1  : Session 0 → [session_no (2B)][counter (4B)][padding (58B)]
 *  Page 2  : Session 1 → [session_no (2B)][counter (4B)][padding (58B)]
 *  Page N+1: Session N → [session_no (2B)][counter (4B)][padding (58B)]
 *
 *  Session page address = 1 + session_no
 * ─────────────────────────────────────────────────────────────────────────*/

#define EEPROM_MAGIC_PAGE       0
#define EEPROM_MAGIC_HI         0xBE
#define EEPROM_MAGIC_LO         0xEF

#define EEPROM_SESSION_START_PAGE   1       // page 1 = session 0
#define EEPROM_PAGE_SIZE            64
#define EEPROM_MAX_SESSIONS         510     // pages 1–510 (page 0 = magic)

#define EEPROM_SESSION_NO_OFFSET    0       // uint16_t
#define EEPROM_COUNTER_OFFSET       2       // uint32_t

typedef struct {
    uint8_t session_no;
    uint16_t counter;
} SessionRecord_t;

/* Call once at startup — restores session_no and counter from EEPROM */
void EEPROM_LOG_Init(uint8_t *session_no_out, uint16_t *counter_out);

/* Call on every increment button press — writes counter to current session page */
void EEPROM_LOG_SaveCounter(uint8_t session_no, uint16_t counter);

/* Call on reset button press — increments session, resets counter, writes new page */
void EEPROM_LOG_CommitSession(uint8_t new_session_no);

/* Read one session record by session number — returns 1 if valid, 0 if empty */
uint8_t EEPROM_LOG_ReadSession(uint8_t session_no, SessionRecord_t *record_out);

/* Get total number of sessions (committed + current active) */
uint8_t EEPROM_LOG_GetSessionCount(uint8_t current_session_no);


#endif /* INC_EEPROM_LOG_H_ */
