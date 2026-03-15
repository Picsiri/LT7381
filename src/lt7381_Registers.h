#pragma once


#ifdef __cplusplus
extern "C" {
#endif

// Data from
/* https://www.buydisplay.com/download/manual/ER-TFTMC070-4_Datasheet.pdf */

// SPI Command bytes (4-wire SPI)

// A0=0, RW#=0 - write register address
#define LT7381_CMD_WRITE_COMMAND        0x00
// A0=0, RW#=1 - read status register data
#define LT7381_CMD_READ_STATUS          0x40

// A0=1, RW#=0 - write data to address
#define LT7381_CMD_WRITE_DATA           0x80
// A0=1, RW#=1 - read data from address
#define LT7381_CMD_READ_DATA            0xC0

/* ------------------------------- */
/* - Register Fields Description - */
/* ------------------------------- */

#define BIT_TO_VAL(bit_pos)             (1u<<(uint8_t)(bit_pos))
#define GET_BIT(byte, bit_pos)          ((uint8_t)(byte) & BIT_TO_VAL(bit_pos))
#define BIT(n)                          (1U << (n))
#define SET_BIT(x, n)                   ((x) |= BIT(n))
#define CLEAR_BIT(x, n)                 ((x) &= ~BIT(n))

/* Status Register */
#define STAT_REG_WRITE_MEMORY_FULL_BIT  0x07
#define STAT_REG_WRITE_MEMORY_EMPTY_BIT 0x06
#define STAT_REG_READ_MEMORY_FULL_BIT   0x05
#define STAT_REG_READ_MEMORY_EMPTY_BIT  0x04
#define STAT_REG_CORE_TASK_BUSY_BIT     0x03
#define STAT_REG_DISPLAY_RAM_READY_BIT  0x02
#define STAT_REG_INHIBIT_OPERATION_BIT  0x01
#define STAT_REG_INTERRUPT_ACTIVE_PIN   0x00

/* Chip Configuration Register */
#define CCR_REG_PLL_READY_BIT           0x07
#define CCR_REG_MASK_WAIT_STATE_BIT     0x06
#define CCR_REG_KEYPAD_SCAN_ENABLE_BIT  0x05
#define CCR_REG_PANEL_IF_SETTING_LOW    0x04
#define CCR_REG_PANEL_IF_SETTING_HIGH   0x03
#define CCR_REG_I2C_MASTER_ENABLE_BIT   0x02
#define CCR_REG_SPI_INTERFACE_ENA_BIT   0x01
#define CCR_REG_BUS_WIDTH_SELECT_BIT    0x00

#define CCR_REG_TFT_24_BIT              0x00
#define CCR_REG_TFT_18_BIT              0x01
#define CCR_REG_TFT_16_BIT              0x10
#define CCR_REG_TFT_NO_BIT              0x11

#define CCR_REG_BUS_8_BIT               0x00
#define CCR_REG_BUS_16_BIT              0x01

/* ------------------------------ */
/* ---- Register Address Map ---- */
/* ------------------------------ */

#define LT7381_REGISTER_SRR             0x00 /**< Software Reset Register */
#define LT7381_REGISTER_CCR             0x01 /**< Chip Configuration Register  */
#define LT7381_REGISTER_MACR            0x02
#define LT7381_REGISTER_ICR             0x03
#define LT7381_REGISTER_MRWDP           0x04
#define LT7381_REGISTER_PPLLC1          0x05
#define LT7381_REGISTER_PPLLC2          0x06
#define LT7381_REGISTER_MPLLC1          0x07
#define LT7381_REGISTER_MPLLC2          0x08
#define LT7381_REGISTER_CPLLC1          0x09
#define LT7381_REGISTER_CPLLC2          0x0A
#define LT7381_REGISTER_INTEN           0x0B
#define LT7381_REGISTER_INTF            0x0C
#define LT7381_REGISTER_MINTFR          0x0D
#define LT7381_REGISTER_PUENR           0x0E
#define LT7381_REGISTER_PSFSR           0x0F

#define LT7381_REGISTER_MPWCTR          0x10
#define LT7381_REGISTER_PIPCDEP         0x11
#define LT7381_REGISTER_DPCR            0x12
#define LT7381_REGISTER_PCSR            0x13
#define LT7381_REGISTER_HDWR            0x14
#define LT7381_REGISTER_HDWFTR          0x15
#define LT7381_REGISTER_HNDR            0x16
#define LT7381_REGISTER_HNDFTR          0x17
#define LT7381_REGISTER_HSTR            0x18
#define LT7381_REGISTER_HPWR            0x19
#define LT7381_REGISTER_VDHR_LOW        0x1A
#define LT7381_REGISTER_VDHR_HIGH       0x1B
#define LT7381_REGISTER_VNDR_LOW        0x1C
#define LT7381_REGISTER_VNDR_HIGH       0x1D
#define LT7381_REGISTER_VSTR            0x1E
#define LT7381_REGISTER_VPWR            0x1F

#define LT7381_REGISTER_MISA_LOW        0x20
#define LT7381_REGISTER_MISA_DOWN       0x21
#define LT7381_REGISTER_MISA_UP         0x22
#define LT7381_REGISTER_MISA_HIGH       0x23
#define LT7381_REGISTER_MIW_LOW         0x24
#define LT7381_REGISTER_MIW_HIGH        0x25
#define LT7381_REGISTER_MWULX_LOW       0x26
#define LT7381_REGISTER_MWULX_HIGH      0x27
#define LT7381_REGISTER_MWULY_LOW       0x28
#define LT7381_REGISTER_MWULY_HIGH      0x29
#define LT7381_REGISTER_PWDULX_LOW      0x2A
#define LT7381_REGISTER_PWDULX_HIGH     0x2B
#define LT7381_REGISTER_PWDULY_LOW      0x2C
#define LT7381_REGISTER_PWDULY_HIGH     0x2D
#define LT7381_REGISTER_PISA_LOW        0x2E
#define LT7381_REGISTER_PISA_DOWN       0x2F

#define LT7381_REGISTER_PISA_UP         0x30
#define LT7381_REGISTER_PISA_HIGH       0x31
#define LT7381_REGISTER_PIW_LOW         0x32
#define LT7381_REGISTER_PIW_HIGH        0x33
#define LT7381_REGISTER_PWIULX_LOW      0x34
#define LT7381_REGISTER_PWIULX_HIGH     0x35
#define LT7381_REGISTER_PWIULY_LOW      0x36
#define LT7381_REGISTER_PWIULY_HIGH     0x37
#define LT7381_REGISTER_PWW_LOW         0x38
#define LT7381_REGISTER_PWW_HIGH        0x39
#define LT7381_REGISTER_PWH_LOW         0x3A
#define LT7381_REGISTER_PWH_HIGH        0x3B
#define LT7381_REGISTER_GTCCR           0x3C
#define LT7381_REGISTER_BTCR            0x3D
#define LT7381_REGISTER_CURHS           0x3E
#define LT7381_REGISTER_CURVS           0x3F

#define LT7381_REGISTER_GCHP_LOW        0x40
#define LT7381_REGISTER_GCHP_HIGH       0x41
#define LT7381_REGISTER_GCVP_LOW        0x42
#define LT7381_REGISTER_GCVP_HIGH       0x43
#define LT7381_REGISTER_GCC0            0x44
#define LT7381_REGISTER_GCC1            0x45
#define LT7381_REGISTER 0x46
#define LT7381_REGISTER 0x47
#define LT7381_REGISTER 0x48
#define LT7381_REGISTER 0x49
#define LT7381_REGISTER 0x4A
#define LT7381_REGISTER 0x4B
#define LT7381_REGISTER 0x4C
#define LT7381_REGISTER 0x4D
#define LT7381_REGISTER 0x4E
#define LT7381_REGISTER 0x4F

#define LT7381_REGISTER_CVSSA_LOW       0x50
#define LT7381_REGISTER_CVSSA_DOWN      0x51
#define LT7381_REGISTER_CVSSA_UP        0x52
#define LT7381_REGISTER_CVSSA_HIGH      0x53
#define LT7381_REGISTER_CVS_IMWTH_LOW   0x54
#define LT7381_REGISTER_CVS_IMWTH_HIGH  0x55
#define LT7381_REGISTER_AWUL_X_LOW      0x56
#define LT7381_REGISTER_AWUL_X_HIGH     0x57
#define LT7381_REGISTER_AWUL_Y_LOW      0x58
#define LT7381_REGISTER_AWUL_Y_HIGH     0x59
#define LT7381_REGISTER_AW_WTH_LOW      0x5A
#define LT7381_REGISTER_AW_WTH_HIGH     0x5B
#define LT7381_REGISTER_AW_HT_LOW       0x5C
#define LT7381_REGISTER_AW_HT_HIGH      0x5D
#define LT7381_REGISTER_AW_COLOR        0x5E
#define LT7381_REGISTER_CURH_LOW        0x5F

#define LT7381_REGISTER_CURH_HIGH       0x60
#define LT7381_REGISTER_CURV_LOW        0x61
#define LT7381_REGISTER_CURV_HIGH       0x62
#define LT7381_REGISTER_F_CURX_LOW      0x63
#define LT7381_REGISTER_F_CURX_HIGH     0x64
#define LT7381_REGISTER_F_CURY_LOW      0x65
#define LT7381_REGISTER_F_CURY_HIGH     0x66
#define LT7381_REGISTER_DCR0            0x67
#define LT7381_REGISTER_DLHSR_LOW       0x68
#define LT7381_REGISTER_DLHSR_HIGH      0x69
#define LT7381_REGISTER_DLVSR_LOW       0x6A
#define LT7381_REGISTER_DLVSR_HIGH      0x6B
#define LT7381_REGISTER_DLHER_LOW       0x6C
#define LT7381_REGISTER_DLHER_HIGH      0x6D
#define LT7381_REGISTER_DLVER_LOW       0x6E
#define LT7381_REGISTER_DLVER_HIGH      0x6F

#define LT7381_REGISTER_DTPH_LOW        0x70
#define LT7381_REGISTER_DTPH_HIGH       0x71
#define LT7381_REGISTER_DTPV_LOW        0x72
#define LT7381_REGISTER_DTPV_HIGH       0x73
#define LT7381_REGISTER 0x74
#define LT7381_REGISTER 0x75
#define LT7381_REGISTER_DCR1            0x76
#define LT7381_REGISTER_ELL_A_LOW       0x77
#define LT7381_REGISTER_ELL_A_HIGH      0x78
#define LT7381_REGISTER_ELL_B_LOW       0x79
#define LT7381_REGISTER_ELL_B_HIGH      0x7A
#define LT7381_REGISTER_DEHR_LOW        0x7B
#define LT7381_REGISTER_DEHR_HIGH       0x7C
#define LT7381_REGISTER_DEVR_LOW        0x7D
#define LT7381_REGISTER_DEVR_HIGH       0x7E
#define LT7381_REGISTER 0x7F

#define LT7381_REGISTER 0x80
#define LT7381_REGISTER 0x81
#define LT7381_REGISTER 0x82
#define LT7381_REGISTER 0x83
#define LT7381_REGISTER_PSCLR           0x84
#define LT7381_REGISTER_PMUXR           0x85 /**< PWM Clock Mux Register */
#define LT7381_REGISTER_PCFGR           0x86 /**< PWM Configuration Register */
#define LT7381_REGISTER_DZ_LENGTH       0x87
#define LT7381_REGISTER_TCMPB0_LOW      0x88 /**< Timer-0 Compare Buffer Register */
#define LT7381_REGISTER_TCMPB0_HIGH     0x89 /**< Timer-0 Compare Buffer Register */
#define LT7381_REGISTER_TCNTB0_LOW      0x8A /**< Timer-0 Count Buffer Register */
#define LT7381_REGISTER_TCNTB0_HIGH     0x8B /**< Timer-0 Count Buffer Register */
#define LT7381_REGISTER_TCMPB1_LOW      0x8C /**< Timer-1 Compare Buffer Register */
#define LT7381_REGISTER_TCMPB1_HIGH     0x8D /**< Timer-1 Compare Buffer Register */
#define LT7381_REGISTER_TCNTB1_LOW      0x8E /**< Timer-1 Count Buffer Register */
#define LT7381_REGISTER_TCNTB1_HIGH     0x8F /**< Timer-1 Count Buffer Register */

#define LT7381_REGISTER_BLT_CTRL0       0x90
#define LT7381_REGISTER_BLT_CTRL1       0x91
#define LT7381_REGISTER_BLT_COLR        0x92
#define LT7381_REGISTER_S0_STR_LOW      0x93
#define LT7381_REGISTER_S0_STR_DOWN     0x94
#define LT7381_REGISTER_S0_STR_UP       0x95
#define LT7381_REGISTER_S0_STR_HIGH     0x96
#define LT7381_REGISTER_S0_WTH_LOW      0x97
#define LT7381_REGISTER_S0_WTH_HIGH     0x98
#define LT7381_REGISTER_S0_X_LOW        0x99
#define LT7381_REGISTER_S0_X_HIGH       0x9A
#define LT7381_REGISTER_S0_Y_LOW        0x9B
#define LT7381_REGISTER_S0_Y_HIGH       0x9C
#define LT7381_REGISTER_S1_STR_LOW      0x9D
#define LT7381_REGISTER_S1_STR_DOWN     0x9E
#define LT7381_REGISTER_S1_STR_UP       0x9F

#define LT7381_REGISTER_S1_STR_HIGH     0xA0
#define LT7381_REGISTER_S1_WTH_LOW      0xA1
#define LT7381_REGISTER_S1_WTH_HIGH     0xA2
#define LT7381_REGISTER_S1_X_LOW        0xA3
#define LT7381_REGISTER_S1_X_HIGH       0xA4
#define LT7381_REGISTER_S1_Y_LOW        0xA5
#define LT7381_REGISTER_S1_Y_HIGH       0xA6
#define LT7381_REGISTER_DT_STR_LOW      0xA7
#define LT7381_REGISTER_DT_STR_DOWN     0xA8
#define LT7381_REGISTER_DT_STR_UP       0xA9
#define LT7381_REGISTER_DT_STR_HIGH     0xAA
#define LT7381_REGISTER_DT_WTH_LOW      0xAB
#define LT7381_REGISTER_DT_WTH_HIGH     0xAC
#define LT7381_REGISTER_DT_X_LOW        0xAD
#define LT7381_REGISTER_DT_X_HIGH       0xAE
#define LT7381_REGISTER_DT_Y_LOW        0xAF

#define LT7381_REGISTER_DT_Y_HIGH       0xB0
#define LT7381_REGISTER_BLT_WTH_LOW     0xB1
#define LT7381_REGISTER_BLT_WTH_HIGH    0xB2
#define LT7381_REGISTER_BLT_HIG_LOW     0xB3
#define LT7381_REGISTER_BLT_HIG_HIGH    0xB4
#define LT7381_REGISTER_APB_CTRL        0xB5
#define LT7381_REGISTER 0xB6
#define LT7381_REGISTER 0xB7
#define LT7381_REGISTER 0xB8
#define LT7381_REGISTER 0xB9
#define LT7381_REGISTER 0xBA
#define LT7381_REGISTER 0xBB
#define LT7381_REGISTER 0xBC
#define LT7381_REGISTER 0xBD
#define LT7381_REGISTER 0xBE
#define LT7381_REGISTER 0xBF

#define LT7381_REGISTER 0xC0
#define LT7381_REGISTER 0xC1
#define LT7381_REGISTER 0xC2
#define LT7381_REGISTER 0xC3
#define LT7381_REGISTER 0xC4
#define LT7381_REGISTER 0xC5
#define LT7381_REGISTER 0xC6
#define LT7381_REGISTER 0xC7
#define LT7381_REGISTER 0xC8
#define LT7381_REGISTER 0xC9
#define LT7381_REGISTER 0xCA
#define LT7381_REGISTER 0xCB
#define LT7381_REGISTER_CCR0            0xCC
#define LT7381_REGISTER_CCR1            0xCD
#define LT7381_REGISTER 0xCE
#define LT7381_REGISTER 0xCF

#define LT7381_REGISTER_FLDR            0xD0
#define LT7381_REGISTER_F2FSSR          0xD1
#define LT7381_REGISTER_FGCR            0xD2
#define LT7381_REGISTER_FGCG            0xD3
#define LT7381_REGISTER_FGCB            0xD4
#define LT7381_REGISTER_BGCR            0xD5
#define LT7381_REGISTER_BGCG            0xD6
#define LT7381_REGISTER_BGCB            0xD7
#define LT7381_REGISTER 0xD8
#define LT7381_REGISTER 0xD9
#define LT7381_REGISTER 0xDA
#define LT7381_REGISTER_CGRAM_STR0      0xDB
#define LT7381_REGISTER 0xDC
#define LT7381_REGISTER 0xDD
#define LT7381_REGISTER 0xDE
#define LT7381_REGISTER_PMU             0xDF

#define LT7381_REGISTER_SDRAR           0xE0 /**< SDRAM Attribute Register  */
#define LT7381_REGISTER_SDRMD           0xE1 /**< SDRAM Mode Register */
#define LT7381_REGISTER_SDR_REF_LOW     0xE2 /**< SDRAM Auto Refresh Interval L */
#define LT7381_REGISTER_SDR_REF_HIGH    0xE3 /**< SDRAM Auto Refresh Interval H */
#define LT7381_REGISTER_SDRCR           0xE4 /**< SDRAM Control Register */
#define LT7381_REGISTER_I2CMCK_LOW      0xE5
#define LT7381_REGISTER_I2CMCK_HIGH     0xE6

#define LT7381_REGISTER_I2CMTXR         0xE7
#define LT7381_REGISTER_I2CMRXR         0xE8
#define LT7381_REGISTER_I2CMCMD         0xE9
#define LT7381_REGISTER_I2CMST          0xEA
#define LT7381_REGISTER 0xEB
#define LT7381_REGISTER 0xEC
#define LT7381_REGISTER 0xED
#define LT7381_REGISTER 0xEE
#define LT7381_REGISTER 0xEF

#define LT7381_REGISTER_GPIOAD          0xF0
#define LT7381_REGISTER_GPIOA           0xF1
#define LT7381_REGISTER_GPIOB           0xF2
#define LT7381_REGISTER_GPIOCD          0xF3
#define LT7381_REGISTER_GPIOC           0xF4
#define LT7381_REGISTER_GPIODD          0xF5
#define LT7381_REGISTER_GPIOD           0xF6
#define LT7381_REGISTER 0xF7
#define LT7381_REGISTER 0xF8
#define LT7381_REGISTER 0xF9
#define LT7381_REGISTER 0xFA
#define LT7381_REGISTER_KSCR1           0xFB
#define LT7381_REGISTER_KSCR2           0xFC
#define LT7381_REGISTER_KSDR1           0xFD
#define LT7381_REGISTER_KSDR2           0xFE
#define LT7381_REGISTER_KSDR3           0xFF

#undef LT7381_REGISTER



#ifdef __cplusplus
}
#endif