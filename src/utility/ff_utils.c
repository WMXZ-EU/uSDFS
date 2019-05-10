
#include <time.h>

#if defined(__IMXRT1052__) || (__IMXRT1062__)
    #include "imxrt.h"
#else
    #include "kinetis.h"
#endif

#include "../ff.h"

/*  
  int  tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
*/
// following routines are based on http://howardhinnant.github.io/date_algorithms.html
struct tm seconds2tm(uint32_t tt)
{ struct tm tx;
  tx.tm_sec   = tt % 60;    tt /= 60; // now it is minutes
  tx.tm_min   = tt % 60;    tt /= 60; // now it is hours
  tx.tm_hour  = tt % 24;    tt /= 24; // now it is days
  tx.tm_wday  = ((tt + 4) % 7) ;      // Sunday is day 0
  //
  tt += 719468;
  uint32_t era = (tt >= 0 ? tt : tt - 146096) / 146097;
  uint32_t doe = (tt - era * 146097);                             // [0, 146096]
  uint32_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365; // [0, 399]
  uint32_t yy = (yoe) + era * 400;
  uint32_t doy = doe - (365*yoe + yoe/4 - yoe/100);            // [0, 365]
  uint32_t mp = (5*doy + 2)/153;                               // [0, 11]
  uint32_t dd = doy - (153*mp+2)/5 + 1;                        // [1, 31]
  uint32_t mm = mp + (mp < 10 ? 3 : -9);                       // [1, 12]
//
  tx.tm_year=yy + (mm <= 2);
  tx.tm_mday=dd;
  tx.tm_mon=mm;
  return tx;
}

uint32_t tm2seconds(struct tm tx)
{
  uint32_t yy = tx.tm_year;
  uint32_t mm = tx.tm_mon;
  uint32_t dd = tx.tm_mday;
  
  yy -= mm <= 2;
  uint32_t era = (yy >= 0 ? yy : yy-399) / 400;
  uint32_t yoe = (yy - era * 400);      // [0, 399]
  uint32_t doy = (153*(mm + (mm > 2 ? -3 : 9)) + 2)/5 + dd-1;  // [0, 365]
  uint32_t doe = yoe * 365 + yoe/4 - yoe/100 + doy;         // [0, 146096]

  uint32_t tt = era * 146097 + (doe) - 719468;
  tt *= (24*3600);
  tt += (tx.tm_hour*60+tx.tm_min)*60+tx.tm_sec;

  return tt;
}
#if defined(__IMXRT1052__) || defined(__IMXRT1062__)

#ifdef XXXXXXXXXXXXXX
    /*********************************************************************************/
    // see also https://github.com/manitou48/teensy4/blob/master/rtc.ino
    #define SNVS_DEFAULT_PGD_VALUE (0x41736166U)
    #define SNVS_LPSR_PGD_MASK                       (0x8U)
    #define SNVS_LPSRTCMR      (IMXRT_SNVS.offset050)
    #define SNVS_LPSRTCLR      (IMXRT_SNVS.offset054)
    //----------------------------------------------------
    #define SNVS_LPTAR        (IMXRT_SNVS.offset058)

    #define SNVS_LPCR_SRTC_ENV_MASK         (0x1U)
    #define SNVS_LPCR_SRTC_ENV(x)           (((uint32_t)(((uint32_t)(x)) << 0U)) & SNVS_LPCR_SRTC_ENV_MASK)

    #define SNVS_LPCR_LPTA_EN_MASK          (0x2U)
    #define SNVS_LPCR_LPTA_EN(x)            (((uint32_t)(((uint32_t)(x)) << 1U)) & SNVS_LPCR_LPTA_EN_MASK)

    #define SNVS_LPCR_MC_ENV_MASK           (0x4U)
    #define SNVS_LPCR_MC_ENV(x)             (((uint32_t)(((uint32_t)(x)) << 2U)) & SNVS_LPCR_MC_ENV_MASK)

    #define SNVS_LPCR_LPWUI_EN_MASK         (0x8U)
    #define SNVS_LPCR_LPWUI_EN(x)           (((uint32_t)(((uint32_t)(x)) << 3U)) & SNVS_LPCR_LPWUI_EN_MASK)

    #define SNVS_LPCR_SRTC_INV_EN_MASK      (0x10U)
    #define SNVS_LPCR_SRTC_INV_EN(x)        (((uint32_t)(((uint32_t)(x)) << 4U)) & SNVS_LPCR_SRTC_INV_EN_MASK)

    #define SNVS_LPCR_DP_EN_MASK            (0x20U)
    #define SNVS_LPCR_DP_EN(x)              (((uint32_t)(((uint32_t)(x)) << 5U)) & SNVS_LPCR_DP_EN_MASK)

    #define SNVS_LPCR_TOP_MASK              (0x40U)
    #define SNVS_LPCR_TOP(x)                (((uint32_t)(((uint32_t)(x)) << 6U)) & SNVS_LPCR_TOP_MASK)

    #define SNVS_LPCR_PWR_GLITCH_EN_MASK    (0x80U)
    #define SNVS_LPCR_PWR_GLITCH_EN(x)      (((uint32_t)(((uint32_t)(x)) << 7U)) & SNVS_LPCR_PWR_GLITCH_EN_MASK)

    #define SNVS_LPCR_LPCALB_EN_MASK        (0x100U)
    #define SNVS_LPCR_LPCALB_EN(x)          (((uint32_t)(((uint32_t)(x)) << 8U)) & SNVS_LPCR_LPCALB_EN_MASK)

    #define SNVS_LPCR_LPCALB_VAL_MASK       (0x7C00U)
    #define SNVS_LPCR_LPCALB_VAL(x)         (((uint32_t)(((uint32_t)(x)) << 10U)) & SNVS_LPCR_LPCALB_VAL_MASK)

    #define SNVS_LPCR_BTN_PRESS_TIME_MASK   (0x30000U)
    #define SNVS_LPCR_BTN_PRESS_TIME(x)     (((uint32_t)(((uint32_t)(x)) << 16U)) & SNVS_LPCR_BTN_PRESS_TIME_MASK)

    #define SNVS_LPCR_DEBOUNCE_MASK         (0xC0000U)
    #define SNVS_LPCR_DEBOUNCE(x)           (((uint32_t)(((uint32_t)(x)) << 18U)) & SNVS_LPCR_DEBOUNCE_MASK)

    #define SNVS_LPCR_ON_TIME_MASK          (0x300000U)
    #define SNVS_LPCR_ON_TIME(x)            (((uint32_t)(((uint32_t)(x)) << 20U)) & SNVS_LPCR_ON_TIME_MASK)

    #define SNVS_LPCR_PK_EN_MASK            (0x400000U)
    #define SNVS_LPCR_PK_EN(x)              (((uint32_t)(((uint32_t)(x)) << 22U)) & SNVS_LPCR_PK_EN_MASK)

    #define SNVS_LPCR_PK_OVERRIDE_MASK      (0x800000U)
    #define SNVS_LPCR_PK_OVERRIDE(x)        (((uint32_t)(((uint32_t)(x)) << 23U)) & SNVS_LPCR_PK_OVERRIDE_MASK)

    #define SNVS_LPCR_GPR_Z_DIS_MASK        (0x1000000U)
    #define SNVS_LPCR_GPR_Z_DIS(x)          (((uint32_t)(((uint32_t)(x)) << 24U)) & SNVS_LPCR_GPR_Z_DIS_MASK)

    #define SNVS_LPSR_LPTA                  (0x1U)
#endif

    void rtc_init() 
    { CCM_CCGR2 |= CCM_CCGR2_IOMUXC_SNVS(CCM_CCGR_ON);
    SNVS_LPGPR = SNVS_DEFAULT_PGD_VALUE;
    SNVS_LPSR = SNVS_LPSR_PGD_MASK;
    // ? calibration
    // ? tamper pins
    
    SNVS_LPCR &= ~SNVS_LPCR_LPTA_EN_MASK; // clear alarm
    while (SNVS_LPCR & SNVS_LPCR_LPTA_EN_MASK); 
    SNVS_LPTAR=0;

    SNVS_LPCR |= 1;             // start RTC
    while (!(SNVS_LPCR & 1));
    }

    extern void *__rtc_localtime; 
    void rtc_set_time(uint32_t secs) 
    { //uint32_t secs = 1547051415;
    SNVS_LPCR &= ~1;   // stop RTC
    while (SNVS_LPCR & 1);
    SNVS_LPSRTCMR = (uint32_t)(secs >> 17U);
    SNVS_LPSRTCLR = (uint32_t)(secs << 15U);
    SNVS_LPCR |= 1;             // start RTC
    while (!(SNVS_LPCR & 1));
    }

    uint32_t rtc_secs() {
    uint32_t seconds = 0;
    uint32_t tmp = 0;

    /* Do consecutive reads until value is correct */
    do
    { seconds = tmp;
        tmp = (SNVS_LPSRTCMR << 17U) | (SNVS_LPSRTCLR >> 15U);
    } while (tmp != seconds);

    return seconds;
    }
#else
    uint32_t rtc_secs() { return RTC_TSR;}
#endif

DWORD get_fattime (void)
{
    struct tm tx=seconds2tm(rtc_secs());

    /* Pack date and time into a DWORD variable */
    return   (((DWORD)tx.tm_year-10) << 25)
            | ((DWORD)tx.tm_mon << 21)
            | ((DWORD)tx.tm_mday << 16)
            | ((DWORD)tx.tm_hour << 11)
            | ((DWORD)tx.tm_min << 5)
            | ((DWORD)tx.tm_sec >> 1);
}
