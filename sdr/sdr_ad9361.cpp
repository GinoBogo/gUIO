/* ************************************************************************** */
/** SDR AD9361 driver (SPI based)

  @Company
    Airbus Italia S.p.A.

  @File Name
    sdr_ad9361.cpp

  @Summary
    SDR AD9361 driver (SPI based)

  @Description
    AD9361 driver library (SPI based).
                                                                              */
/* ************************************************************************** */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

// STD libraries
#include <cmath>
#include <cstdlib>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// project libraries
#include "GLogger.hpp"
#include "sdr_ad9361.hpp" // SDR AD9361 driver
#include "spi_if.hpp"     // SPI interface API
#include "stime.hpp"      // System Time Services

// clang-format off
#ifndef UNUSED
#define UNUSED(x) if (x) {}
#endif
// clang-format on

// Warning: diagnostic disabled
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

// *****************************************************************************
// *****************************************************************************
// Section: Macros and constants
// *****************************************************************************
// *****************************************************************************

#define ARRAY_SIZE(arr)                      (sizeof(arr) / sizeof((arr)[0]))
#define min(x, y)                            (((x) < (y)) ? (x) : (y))
#define min_t(type, x, y)                    (type) min((type)(x), (type)(y))
#define max(x, y)                            (((x) > (y)) ? (x) : (y))
#define max_t(type, x, y)                    (type) max((type)(x), (type)(y))
#define clamp(val, min_val, max_val)         (max(min((val), (max_val)), (min_val)))
#define clamp_t(type, val, min_val, max_val) (type) clamp((type)(val), (type)(min_val), (type)(max_val))
#define DIV_ROUND_UP(x, y)                   (((x) + (y)-1) / (y))
#define DIV_ROUND_CLOSEST(x, divisor)        (((x) + (divisor) / 2) / (divisor))
#define BIT(x)                               (1 << (x))
#define CLK_GET_RATE_NOCACHE                 BIT(6)

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* uint32_t int_sqrt(uint32_t x)

  Summary:
    Integer square-root function.

  Description:
    Integer square-root function.

  Remarks:
    None.
*/
uint32_t int_sqrt(uint32_t x) {

    uint32_t b, m, y = 0;

    if (x <= 1) {
        return x;
    }

    m = 1UL << ((sizeof(uint32_t) * 8) - 2);

    while (m != 0) {
        b = y + m;
        y >>= 1;

        if (x >= b) {
            x -= b;
            y += m;
        }

        m >>= 2;
    }

    return y;
}

// *****************************************************************************
/* int32_t int_log2(int32_t x)

  Summary:
    Integer log2 function.

  Description:
    Integer log2 function.

  Remarks:
    None.
*/
int32_t int_log2(int32_t x) {

    int32_t A      = !(!(x >> 16));
    int32_t count  = 0;
    int32_t x_copy = x;

    count = count + (A << 4);

    x_copy = (((~A + 1) & (x >> 16)) + (~(~A + 1) & x));

    A      = !(!(x_copy >> 8));
    count  = count + (A << 3);
    x_copy = (((~A + 1) & (x_copy >> 8)) + (~(~A + 1) & x_copy));

    A      = !(!(x_copy >> 4));
    count  = count + (A << 2);
    x_copy = (((~A + 1) & (x_copy >> 4)) + (~(~A + 1) & x_copy));

    A      = !(!(x_copy >> 2));
    count  = count + (A << 1);
    x_copy = (((~A + 1) & (x_copy >> 2)) + (~(~A + 1) & x_copy));

    A     = !(!(x_copy >> 1));
    count = count + A;

    return count;
}

// *****************************************************************************
/* uint64_t int_do_div(uint64_t* n, uint64_t base)

  Summary:
    Integer divide function.

  Description:
    Integer divide function.

  Remarks:
    None.
*/
uint64_t int_do_div(uint64_t* n, //
                    uint64_t  base) {

    uint64_t mod = 0;

    mod = *n % base;
    *n  = *n / base;

    return mod;
}

// *****************************************************************************
/* int32_t ad9361_find_opt_delay(uint8_t* field, uint32_t size, uint32_t* ret_start)

  Summary:
    Find optimal delay value.

  Description:
    The optimal delay in case of success, negative error code otherwise.

  Remarks:
    None.
*/
int32_t ad9361_find_opt_delay(uint8_t*  field, //
                              uint32_t  size,
                              uint32_t* ret_start) {

    int32_t i, cnt = 0, max_cnt = 0, start, max_start = 0;

    for (i = 0, start = -1; i < (int64_t)size; i++) {
        if (field[i] == 0) {
            if (start == -1) {
                start = i;
            }

            cnt++;
        }
        else {
            if (cnt > max_cnt) {
                max_cnt   = cnt;
                max_start = start;
            }

            start = -1;
            cnt   = 0;
        }
    }

    if (cnt > max_cnt) {
        max_cnt   = cnt;
        max_start = start;
    }

    *ret_start = max_start;

    return max_cnt;
}

// *****************************************************************************
/* int32_t ad9361_check_cal_done(ad9361_rf_phy_t* phy, uint32_t reg, uint32_t mask, bool done_state)

  Summary:
    Check the calibration done bit.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param reg The register address.
    @param mask The bit mask.
    @param done_state The done state [0,1].

  Remarks:
    None.
*/
int32_t ad9361_check_cal_done(ad9361_rf_phy_t* phy, //
                              uint32_t         reg,
                              uint32_t         mask,
                              bool             done_state) {

    uint32_t timeout = 5000; // RFDC_CAL can take long

    do {
        uint32_t state = SPI_SDR_ReadF(phy->id_no, reg, mask);

        if (state == done_state) {
            return 0;
        }

        if (reg == REG_CALIBRATION_CTRL) {
            STIME_uSleep(1200);
        }
        else {
            STIME_uSleep(120);
        }
    }
    while (timeout--);

    LOG_FORMAT(warning, "Calibration timeout [reg 0x%X, mask 0x%X] (%s)", reg, mask, __func__);

    return -ETIMEDOUT;
}

// *****************************************************************************
/* int32_t ad9361_run_calibration(ad9361_rf_phy_t* phy, uint32_t mask)

  Summary:
    Run an AD9361 calibration and check the calibration done bit.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param mask The calibration bit mask [ RX_BB_TUNE_CAL, TX_BB_TUNE_CAL, RX_QUAD_CAL, TX_QUAD_CAL, RX_GAIN_STEP_CAL, TXMON_CAL, RFDC_CAL, BBDC_CAL ].

  Remarks:
    None.
*/
int32_t ad9361_run_calibration(ad9361_rf_phy_t* phy, //
                               uint32_t         mask) {

    SPI_SDR_Write(phy->id_no, REG_CALIBRATION_CTRL, mask);

    LOG_FORMAT(debug, "Calibration mask 0x%" PRIx32 " (%s)", mask, __func__);

    return ad9361_check_cal_done(phy, REG_CALIBRATION_CTRL, mask, 0);
}

// *****************************************************************************
/* rx_gain_table_name_t ad9361_gt_tableindex(uint64_t freq)

  Summary:
    Choose the right RX gain table index for the selected frequency.

  Description:
    Returns the index to the RX gain table.

    @param freq The frequency value [Hz].

  Remarks:
    None.
*/
rx_gain_table_name_t ad9361_gt_tableindex(uint64_t freq) {

    rx_gain_table_name_t _val = TBL_4000_6000_MHZ;

    if (freq <= 1300000000ULL) {
        _val = TBL_200_1300_MHZ;
    }
    else if (freq <= 4000000000ULL) {
        _val = TBL_1300_4000_MHZ;
    }

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_load_gt(ad9361_rf_phy_t* phy, uint64_t freq, uint32_t dest)

  Summary:
    Load the gain table for the selected frequency range and receiver.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param freq The frequency value [Hz].
    @param dest The destination [ GT_RX1, GT_RX2 ].

  Remarks:
    None.
*/
int32_t ad9361_load_gt(ad9361_rf_phy_t* phy, //
                       uint64_t         freq,
                       uint32_t         dest) {

    const uint8_t(*tab)[3];
    rx_gain_table_name_t band;
    uint32_t             index_max;

    LOG_FORMAT(debug, "Set frequency %d (%s)", freq, __func__);

    band = ad9361_gt_tableindex(freq);

    LOG_FORMAT(debug, "Get frequency %d, band %d (%s)", freq, band, __func__);

    // check if table is present
    if (phy->current_table == band) {
        return 0;
    }

    SPI_SDR_WriteF(phy->id_no, REG_AGC_CONFIG_2, AGC_USE_FULL_GAIN_TABLE, !phy->pdata.split_gt);

    if (phy->pdata.split_gt) {
        tab       = &split_gain_table[band][0];
        index_max = SIZE_SPLIT_TABLE;
    }
    else {
        tab       = &full__gain_table[band][0];
        index_max = SIZE_FULL__TABLE;
    }

    SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_CONFIG, START_GAIN_TABLE_CLOCK | RECEIVER_SELECT(dest)); // Start Gain Table Clock

    for (decltype(index_max) i{0}; i < index_max; i++) {
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_ADDRESS, i);                                                                // Gain Table Index
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_WRITE_DATA1, tab[i][0]);                                                    // Ext LNA, Int LNA, & Mixer Gain Word
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_WRITE_DATA2, tab[i][1]);                                                    // TIA & LPF Word
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_WRITE_DATA3, tab[i][2]);                                                    // DC Cal bit & Dig Gain Word
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_CONFIG, START_GAIN_TABLE_CLOCK | WRITE_GAIN_TABLE | RECEIVER_SELECT(dest)); // Gain Table Index
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_READ_DATA1, 0);                                                             // Dummy Write to delay 3 ADCCLK/16 cycles
        SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_READ_DATA1, 0);                                                             // Dummy Write to delay ~1u
    }

    SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_CONFIG, START_GAIN_TABLE_CLOCK | RECEIVER_SELECT(dest)); // Clear Write Bit
    SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_READ_DATA1, 0);                                          // Dummy Write to delay ~1u
    SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_READ_DATA1, 0);                                          // Dummy Write to delay ~1u
    SPI_SDR_Write(phy->id_no, REG_GAIN_TABLE_CONFIG, 0);                                              // Stop Gain Table Clock

    phy->current_table = band;

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_setup_ext_lna(ad9361_rf_phy_t* phy, elna_control_t* ctrl)

  Summary:
    Setup the external low-noise amplifier (LNA).

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param ctrl Pointer to eLNA control structure.

  Remarks:
    None.
*/
int32_t ad9361_setup_ext_lna(ad9361_rf_phy_t* phy, //
                             elna_control_t*  ctrl) {

    SPI_SDR_WriteF(phy->id_no, REG_EXTERNAL_LNA_CTRL, EXTERNAL_LNA1_CTRL, ctrl->elna_1_control_en);

    SPI_SDR_WriteF(phy->id_no, REG_EXTERNAL_LNA_CTRL, EXTERNAL_LNA2_CTRL, ctrl->elna_2_control_en);

    SPI_SDR_Write(phy->id_no, REG_EXT_LNA_HIGH_GAIN, EXT_LNA_HIGH_GAIN(ctrl->gain_mdB / 500));

    SPI_SDR_Write(phy->id_no, REG_EXT_LNA_LOW_GAIN, EXT_LNA_LOW_GAIN(ctrl->bypass_loss_mdB / 500));

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_clkout_control(ad9361_rf_phy_t* phy, ad9361_clkout_t mode)

  Summary:
    Set the clock output mode.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param mode The clock output mode [].

  Remarks:
    None.
*/
int32_t ad9361_clkout_control(ad9361_rf_phy_t* phy, //
                              ad9361_clkout_t  mode) {

    if (mode == CLKOUT_DISABLE) {
        SPI_SDR_WriteF(phy->id_no, REG_BBPLL, CLKOUT_ENABLE, 0);
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_BBPLL, CLKOUT_ENABLE | CLKOUT_SELECT(~0), ((mode - 1) << 1) | 0x01);
    }

    return 0;
}

// *****************************************************************************
/* int32_t  ad9361_load_mixer_gm_subtable(ad9361_rf_phy_t* phy)

  Summary:
    Load the Gm Sub Table.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.

  Remarks:
    None.
*/
int32_t ad9361_load_mixer_gm_subtable(ad9361_rf_phy_t* phy) {

    int32_t i, addr;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_CONFIG, START_GM_SUB_TABLE_CLOCK); // Start Clock

    for (i = 0, addr = ARRAY_SIZE(gm_st_ctrl); i < (int64_t)ARRAY_SIZE(gm_st_ctrl); i++) {
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_ADDRESS, --addr);                                       // Gain Table Index
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_BIAS_WRITE, 0);                                         // Bias
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_GAIN_WRITE, gm_st_gain[i]);                             // Gain
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_CTRL_WRITE, gm_st_ctrl[i]);                             // Control
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_CONFIG, WRITE_GM_SUB_TABLE | START_GM_SUB_TABLE_CLOCK); // Write Words
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_GAIN_READ, 0);                                          // Dummy Delay
        SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_GAIN_READ, 0);                                          // Dummy Delay
    }

    SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_CONFIG, START_GM_SUB_TABLE_CLOCK); // Clear Write
    SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_GAIN_READ, 0);                     // Dummy Delay
    SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_GAIN_READ, 0);                     // Dummy Delay
    SPI_SDR_Write(phy->id_no, REG_GM_SUB_TABLE_CONFIG, 0);                        // Stop Clock

    return 0;
}

// *****************************************************************************
/* uint32_t ad9361_rfvco_tableindex(uint32_t freq)

  Summary:
    Choose the right RF VCO table index for the selected frequency.

  Description:
    Returns the index from the RF VCO table.

    @param freq The frequency value [Hz].

  Remarks:
    None.
*/
uint32_t ad9361_rfvco_tableindex(uint32_t freq) {

    uint32_t _val = LUT_FTDD_80;

    if (freq < 50000000UL) {
        _val = LUT_FTDD_40;
    }
    else if (freq <= 70000000UL) {
        _val = LUT_FTDD_60;
    }

    return _val;
}

// *****************************************************************************
/* int32_t  ad9361_rfpll_vco_init(ad9361_rf_phy_t* phy, bool tx, uint64_t vco_freq, uint32_t ref_clk)

  Summary:
    Initialize the RFPLL VCO.

  Description:
    Returns 0 in case of success.

    @param phy The AD9361 state structure.
    @param tx Set true for TX_RFPLL.
    @param vco_freq The VCO frequency [Hz].
    @param ref_clk The reference clock frequency [Hz].

  Remarks:
    None.
*/
int32_t ad9361_rfpll_vco_init(ad9361_rf_phy_t* phy, //
                              bool             tx,
                              uint64_t         vco_freq,
                              uint32_t         ref_clk) {

    const synth_lut_t* tab;

    uint32_t range;
    int32_t  i    = 0;
    uint32_t offs = 0;

    range = ad9361_rfvco_tableindex(ref_clk);

    LOG_FORMAT(debug,
               "vco_freq %" PRIu64 ", ref_clk %" PRIu32 ", range %" PRIu32 " (%s)", //
               vco_freq,
               ref_clk,
               range,
               __func__);

    int_do_div(&vco_freq, 1000000UL); // vco_freq in MHz

    if (phy->pdata.fdd || phy->pdata.tdd_use_fdd_tables) {
        tab = &synth_lut_fdd[range][0];
    }
    else {
        tab = &synth_lut_tdd[range][0];
    }

    if (tx) {
        offs = REG_TX_VCO_OUTPUT - REG_RX_VCO_OUTPUT;
    }

    while ((i < SYNTH_LUT_SIZE) && (tab[i].VCO_MHz > vco_freq)) {
        i++;
    }

    LOG_FORMAT(debug, "VCO freq %d MHz, TAB index %d (%s)", tab[i].VCO_MHz, i, __func__);

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_OUTPUT + offs, VCO_OUTPUT_LEVEL(tab[i].VCO_Output_Level) | PORB_VCO_LOGIC);

    SPI_SDR_WriteF(phy->id_no, REG_RX_ALC_VARACTOR + offs, VCO_VARACTOR(~0), tab[i].VCO_Varactor);

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_BIAS_1 + offs, VCO_BIAS_REF(tab[i].VCO_Bias_Ref) | VCO_BIAS_TCF(tab[i].VCO_Bias_Tcf));

    SPI_SDR_Write(phy->id_no, REG_RX_FORCE_VCO_TUNE_1 + offs, VCO_CAL_OFFSET(tab[i].VCO_Cal_Offset));

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_VARACTOR_CTRL_1 + offs, VCO_VARACTOR_REFERENCE(tab[i].VCO_Varactor_Reference));

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_CAL_REF + offs, VCO_CAL_REF_TCF(0));

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_VARACTOR_CTRL_0 + offs, VCO_VARACTOR_OFFSET(0) | VCO_VARACTOR_REFERENCE_TCF(7));

    SPI_SDR_WriteF(phy->id_no, REG_RX_CP_CURRENT + offs, CHARGE_PUMP_CURRENT(~0), tab[i].Charge_Pump_Current);

    SPI_SDR_Write(phy->id_no, REG_RX_LOOP_FILTER_1 + offs, LOOP_FILTER_C2(tab[i].LF_C2) | LOOP_FILTER_C1(tab[i].LF_C1));

    SPI_SDR_Write(phy->id_no, REG_RX_LOOP_FILTER_2 + offs, LOOP_FILTER_R1(tab[i].LF_R1) | LOOP_FILTER_C3(tab[i].LF_C3));

    SPI_SDR_Write(phy->id_no, REG_RX_LOOP_FILTER_3 + offs, LOOP_FILTER_R3(tab[i].LF_R3));

    return 0;
}

// *****************************************************************************
/* void ad9361_ensm_force_state(ad9361_rf_phy_t* phy, uint8_t ensm_state)

  Summary:
    Force Enable State Machine (ENSM) to the desired state (internally used only).

  Description:
    @param phy The AD9361 state structure.
    @param ensm_state The ENSM state [ ENSM_STATE_SLEEP_WAIT, ENSM_STATE_ALERT, ENSM_STATE_TX, ENSM_STATE_TX_FLUSH ,
  ENSM_STATE_RX, ENSM_STATE_RX_FLUSH, ENSM_STATE_FDD, ENSM_STATE_FDD_FLUSH ].

  Remarks:
    None.
*/
void ad9361_ensm_force_state(ad9361_rf_phy_t* phy, //
                             uint8_t          ensm_state) {

    uint8_t  dev_ensm_state;
    uint32_t val;

    dev_ensm_state = SPI_SDR_ReadF(phy->id_no, REG_STATE, ENSM_STATE(~0));

    phy->prev_ensm_state = dev_ensm_state;

    if (dev_ensm_state == ensm_state) {
        LOG_FORMAT(warning, "Nothing to do, device is already in 0x%02X state (%s)", ensm_state, __func__);
        goto out;
    }

    LOG_FORMAT(debug, "Device is in 0x%02X state, forcing to 0x%02X (%s)", dev_ensm_state, ensm_state, __func__);

    val = SPI_SDR_Read(phy->id_no, REG_ENSM_CONFIG_1);

    // Enable control through SPI writes, and take out from alert
    if (val & ENABLE_ENSM_PIN_CTRL) {
        val &= ~ENABLE_ENSM_PIN_CTRL;
        phy->ensm_pin_ctl_en = true;
    }
    else {
        phy->ensm_pin_ctl_en = false;
    }

    if (dev_ensm_state) {
        val &= ~(TO_ALERT);
    }

    switch (ensm_state) {

        case ENSM_STATE_TX:
        case ENSM_STATE_FDD:
            val |= FORCE_TX_ON;
            break;

        case ENSM_STATE_RX:
            val |= FORCE_RX_ON;
            break;

        case ENSM_STATE_ALERT:
            val &= ~(FORCE_TX_ON | FORCE_RX_ON);
            val |= TO_ALERT | FORCE_ALERT_STATE;
            break;

        default:
            LOG_FORMAT(debug, "No handling for forcing %d ensm state, (%s)", ensm_state, __func__);
            goto out;
    }

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, TO_ALERT | FORCE_ALERT_STATE);

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, val);

out:
    return;
}

// *****************************************************************************
/* void ad9361_ensm_restore_prev_state(ad9361_rf_phy_t* phy)

  Summary:
    Restore the previous Enable State Machine (ENSM) state.

  Description:
    @param phy The AD9361 state structure.

  Remarks:
    None.
*/
void ad9361_ensm_restore_prev_state(ad9361_rf_phy_t* phy) {

    uint32_t val = SPI_SDR_Read(phy->id_no, REG_ENSM_CONFIG_1);

    // We are restoring state only, so clear State bits first which might have set while forcing a particular state
    val &= ~(FORCE_TX_ON | FORCE_RX_ON | TO_ALERT | FORCE_ALERT_STATE);

    switch (phy->prev_ensm_state) {

        case ENSM_STATE_TX:
        case ENSM_STATE_FDD:
            val |= FORCE_TX_ON;
            break;

        case ENSM_STATE_RX:
            val |= FORCE_RX_ON;
            break;

        case ENSM_STATE_ALERT:
            val |= TO_ALERT;
            break;

        case ENSM_STATE_INVALID:
            LOG_FORMAT(debug, "No need to restore, ENSM state wasn't saved (%s)", __func__);
            goto out;

        default:
            LOG_FORMAT(warning, "Could not restore the %d ENSM state (%s)", phy->prev_ensm_state, __func__);
            goto out;
    }

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, TO_ALERT | FORCE_ALERT_STATE);

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, val);

    if (phy->ensm_pin_ctl_en) {
        val |= ENABLE_ENSM_PIN_CTRL;
        SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, val);
    }

out:
    return;
}

// *****************************************************************************
/* void ad9361_init_gain_info(rx_gain_info_t* rx_gain, rx_gain_table_t type, int32_t starting_gain, int32_t
  max_gain, int32_t gain_step, int32_t max_idx, int32_t idx_offset)

  Summary:
    Initialize the rx_gain_info structure.

  Description:
    @param rx_gain The rx_gain_info structure pointer.
    @param type Either Full or Split Table
    @param starting_gain The starting gain value.
    @param max_gain The maximum gain value.
    @param gain_step The gain step.
    @param max_idx The max table size.
    @param idx_offset Offset in the table where linear progression starts

  Remarks:
    None.
*/
void ad9361_init_gain_info(rx_gain_info_t* rx_gain, //
                           rx_gain_table_t type,
                           int32_t         starting_gain,
                           int32_t         max_gain,
                           int32_t         gain_step,
                           int32_t         max_idx,
                           int32_t         idx_offset) {

    rx_gain->tbl_type         = type;
    rx_gain->starting_gain_db = starting_gain;
    rx_gain->max_gain_db      = max_gain;
    rx_gain->gain_step_db     = gain_step;
    rx_gain->max_idx          = max_idx;
    rx_gain->idx_step_offset  = idx_offset;
}

// *****************************************************************************
/* int32_t ad9361_en_dis_tx(ad9361_rf_phy_t* phy, uint32_t tx_if, uint32_t enable)

  Summary:
    Enable/disable the desired TX channel.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param tx_if The desired channel number [1, 2].
    @param enable Enable/disable option.

  Remarks:
    None.
*/
int32_t ad9361_en_dis_tx(ad9361_rf_phy_t* phy, //
                         uint32_t         tx_if,
                         uint32_t         enable) {

    int32_t _val = 0;

    if ((tx_if == 2) && (!phy->pdata.rx2tx2 && enable)) {
        _val = -EINVAL;
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_CHANNEL_ENABLE(tx_if), enable);
    }

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_en_dis_rx(ad9361_rf_phy_t* phy, uint32_t rx_if, uint32_t enable)

  Summary:
    Enable/disable the desired RX channel.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param rx_if The desired channel number [1, 2].
    @param enable Enable/disable option.

  Remarks:
    None.
*/
int32_t ad9361_en_dis_rx(ad9361_rf_phy_t* phy, //
                         uint32_t         rx_if,
                         uint32_t         enable) {

    int32_t _val = 0;

    if ((rx_if == 2) && (!phy->pdata.rx2tx2 && enable)) {
        _val = -EINVAL;
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_CHANNEL_ENABLE(rx_if), enable);
    }

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_gc_update(ad9361_rf_phy_t* phy)

  Summary:
    Update the Gain Control.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param rx_if The desired channel number [1, 2].
    @param enable Enable/disable option.

  Remarks:
    None.
*/
int32_t ad9361_gc_update(ad9361_rf_phy_t* phy) {

    uint32_t clkrf;
    uint32_t reg, delay_lna, settling_delay, dec_pow_meas_dur;
    uint32_t fir_div;

    clkrf = clk_get_rate(phy, &(phy->ref_clk_scale[CLKRF_CLK]));

    delay_lna = phy->pdata.elna_ctrl.settling_delay_ns;

    // AGC Attack Delay (us)=ceiling((((0.2+Delay_LNA)*ClkRF+14))/(2*ClkRF))+1
    // ClkRF in MHz, delay in us

    reg = (200 * delay_lna) / 2 + (14000000UL / (clkrf / 500U));
    reg = DIV_ROUND_UP(reg, 1000UL) + phy->pdata.gain_ctrl.agc_attack_delay_extra_margin_us;
    reg = clamp_t(uint8_t, reg, 0U, 31U);

    SPI_SDR_WriteF(phy->id_no, REG_AGC_ATTACK_DELAY, AGC_ATTACK_DELAY(~0), reg);

    // Peak Overload Wait Time (ClkRF cycles)=ceiling((0.1+Delay_LNA) *clkRF+1)

    reg = (delay_lna + 100UL) * (clkrf / 1000UL);
    reg = DIV_ROUND_UP(reg, 1000000UL) + 1;
    reg = clamp_t(uint8_t, reg, 0U, 31U);

    SPI_SDR_WriteF(phy->id_no, REG_PEAK_WAIT_TIME, PEAK_OVERLOAD_WAIT_TIME(~0), reg);

    // Settling Delay in 0x111.  Applies to all gain control modes:
    // 0x111[D4:D0]= ceiling(((0.2+Delay_LNA)*clkRF do-debug = false;+14)/2)

    reg = (delay_lna + 200UL) * (clkrf / 2000UL);
    reg = DIV_ROUND_UP(reg, 1000000UL) + 7;
    reg = settling_delay = clamp_t(uint8_t, reg, 0U, 31U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, SETTLING_DELAY(~0), reg);

    // Gain Update Counter [15:0]= round((((time*ClkRF-0x111[D4:D0]*2)-2))/2)

    reg = phy->pdata.gain_ctrl.gain_update_interval_us * (clkrf / 1000UL) - settling_delay * 2000UL - 2000UL;
    reg = DIV_ROUND_CLOSEST(reg, 2000UL);
    reg = clamp_t(uint32_t, reg, 0U, 131071UL);

    if ((phy->agc_mode[0] == RF_GAIN_FASTATTACK_AGC) || (phy->agc_mode[1] == RF_GAIN_FASTATTACK_AGC)) {
        dec_pow_meas_dur = phy->pdata.gain_ctrl.f_agc_dec_pow_measuremnt_duration;
    }
    else {
        fir_div          = DIV_ROUND_CLOSEST(clkrf, clk_get_rate(phy, &(phy->ref_clk_scale[RX_SAMPL_CLK])));
        dec_pow_meas_dur = phy->pdata.gain_ctrl.dec_pow_measuremnt_duration;

        if (((reg * 2 / fir_div) / dec_pow_meas_dur) < 2) {
            dec_pow_meas_dur = reg / fir_div;
        }
    }

    // Power Measurement Duration
    SPI_SDR_WriteF(phy->id_no, REG_DEC_POWER_MEASURE_DURATION_0, DEC_POWER_MEASUREMENT_DURATION(~0), int_log2(dec_pow_meas_dur / 16));

    SPI_SDR_WriteF(phy->id_no, REG_DIGITAL_SAT_COUNTER, DOUBLE_GAIN_COUNTER, reg > 65535);

    if (reg > 65535) {
        reg /= 2;
    }

    SPI_SDR_Write(phy->id_no, REG_GAIN_UPDATE_COUNTER1, reg & 0xFF);
    SPI_SDR_Write(phy->id_no, REG_GAIN_UPDATE_COUNTER2, reg >> 8);

    // Fast AGC State Wait Time - Energy Detect Count

    reg = DIV_ROUND_CLOSEST(phy->pdata.gain_ctrl.f_agc_state_wait_time_ns * (clkrf / 1000UL), 1000000UL);
    reg = clamp_t(uint32_t, reg, 0U, 31U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_ENERGY_DETECT_COUNT, ENERGY_DETECT_COUNT(~0), reg);

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_rx_adc_setup(ad9361_rf_phy_t* phy, uint32_t bbpll_freq, uint32_t adc_sampl_freq_Hz)

  Summary:
    Setup the RX ADC.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param bbpll_freq The BBPLL frequency [Hz].
    @param adc_sampl_freq_Hz The ADC sampling frequency [Hz].

  Remarks:
    None.
*/
int32_t ad9361_rx_adc_setup(ad9361_rf_phy_t* phy, //
                            uint32_t         bbpll_freq,
                            uint32_t         adc_sampl_freq_Hz) {

    uint32_t scale_snr_1e3, maxsnr, sqrt_inv_rc_tconst_1e3, tmp_1e3, //
        scaled_adc_clk_1e6, inv_scaled_adc_clk_1e3, sqrt_term_1e3,   //
        min_sqrt_term_1e3, bb_bw_Hz;

    uint64_t tmp, invrc_tconst_1e6;
    uint8_t  data[40];
    uint32_t i;
    uint8_t  c3_msb = SPI_SDR_Read(phy->id_no, REG_RX_BBF_C3_MSB);
    uint8_t  c3_lsb = SPI_SDR_Read(phy->id_no, REG_RX_BBF_C3_LSB);
    uint8_t  r2346  = SPI_SDR_Read(phy->id_no, REG_RX_BBF_R2346);

    // BBBW = (BBPLL / RxTuneDiv) * ln(2) / (1.4 * 2PI)
    // We assume ad9361_rx_bb_analog_filter_calib() is always run prior

    tmp = bbpll_freq * 10000ULL;
    int_do_div(&tmp, 126906UL * phy->rxbbf_div);
    bb_bw_Hz = tmp;

    LOG_FORMAT(debug, "bb_bw_Hz %" PRIu32 ", adc_sampl_freq_Hz %" PRIu32 " (%s)", bb_bw_Hz, adc_sampl_freq_Hz, __func__);

    LOG_FORMAT(debug, "c3_msb 0x%X, c3_lsb 0x%X, r2346 0x%X (%s)", c3_msb, c3_lsb, r2346, __func__);

    bb_bw_Hz = clamp(bb_bw_Hz, 200000UL, 28000000UL);

    if (adc_sampl_freq_Hz < 80000000) {
        scale_snr_1e3 = 1000;
    }
    else {
        scale_snr_1e3 = 1585; // pow(10, scale_snr_dB / 10) ;
    }

    if (bb_bw_Hz >= 18000000) {
        invrc_tconst_1e6 = (160975ULL * r2346 * ((160 * c3_msb) + (10 * c3_lsb) + 140) * (bb_bw_Hz) * (1000 + (10 * (bb_bw_Hz - 18000000) / 1000000)));
        int_do_div(&invrc_tconst_1e6, 1000UL);
    }
    else {
        invrc_tconst_1e6 = (160975ULL * r2346 * ((160 * c3_msb) + (10 * c3_lsb) + 140) * (bb_bw_Hz));
    }

    int_do_div(&invrc_tconst_1e6, 1000000000UL);

    if (invrc_tconst_1e6 > ULONG_MAX) {
        LOG_FORMAT(error, "invrc_tconst_1e6 > ULONG_MAX (%s)", __func__);
    }

    sqrt_inv_rc_tconst_1e3 = int_sqrt((uint32_t)invrc_tconst_1e6);

    maxsnr = 640 / 160;

    scaled_adc_clk_1e6 = DIV_ROUND_CLOSEST(adc_sampl_freq_Hz, 640);

    inv_scaled_adc_clk_1e3 = DIV_ROUND_CLOSEST(640000000, DIV_ROUND_CLOSEST(adc_sampl_freq_Hz, 1000));

    tmp_1e3 = DIV_ROUND_CLOSEST(980000 + 20 * max_t(uint32_t, 1000U, DIV_ROUND_CLOSEST(inv_scaled_adc_clk_1e3, maxsnr)), 1000);

    sqrt_term_1e3 = int_sqrt(scaled_adc_clk_1e6);

    min_sqrt_term_1e3 = min_t(uint32_t, 1000U, int_sqrt(maxsnr * scaled_adc_clk_1e6));

    LOG_FORMAT(debug,
               "invrc_tconst_1e6 %" PRIu64 ", sqrt_inv_rc_tconst_1e3 %" PRIu32 " (%s)", //
               invrc_tconst_1e6,
               sqrt_inv_rc_tconst_1e3,
               __func__);
    LOG_FORMAT(debug,
               "scaled_adc_clk_1e6 %" PRIu32 ", inv_scaled_adc_clk_1e3 %" PRIu32 " (%s)", //
               scaled_adc_clk_1e6,
               inv_scaled_adc_clk_1e3,
               __func__);
    LOG_FORMAT(debug,
               "tmp_1e3 %" PRIu32 ", sqrt_term_1e3 %" PRIu32 ", min_sqrt_term_1e3 %" PRIu32 " (%s)", //
               tmp_1e3,
               sqrt_term_1e3,
               min_sqrt_term_1e3,
               __func__);

    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0x24;
    data[4] = 0x24;
    data[5] = 0;
    data[6] = 0;

    tmp = -50000000 + (8ULL * scale_snr_1e3 * sqrt_inv_rc_tconst_1e3 * min_sqrt_term_1e3);

    int_do_div(&tmp, 100000000UL);

    data[7] = min_t(uint64_t, 124U, tmp);

    tmp = (invrc_tconst_1e6 >> 1) + (20 * inv_scaled_adc_clk_1e3 * data[7] / 80 * 1000ULL);

    int_do_div(&tmp, invrc_tconst_1e6);

    data[8] = min_t(uint64_t, 255U, tmp);

    tmp = (-500000 + (77ULL * sqrt_inv_rc_tconst_1e3 * min_sqrt_term_1e3));

    int_do_div(&tmp, 1000000UL);

    data[10] = min_t(uint64_t, 127U, tmp);
    data[9]  = min_t(uint32_t, 127U, ((800 * data[10]) / 1000));

    tmp = ((invrc_tconst_1e6 >> 1) + (20 * inv_scaled_adc_clk_1e3 * data[10] * 1000ULL));

    int_do_div(&tmp, invrc_tconst_1e6 * 77);

    data[11] = min_t(uint64_t, 255U, tmp);
    data[12] = min_t(uint32_t, 127U, (-500000 + 80 * sqrt_inv_rc_tconst_1e3 * min_sqrt_term_1e3) / 1000000UL);

    tmp = -3 * (long)(invrc_tconst_1e6 >> 1) + inv_scaled_adc_clk_1e3 * data[12] * (1000ULL * 20 / 80);

    int_do_div(&tmp, invrc_tconst_1e6);

    data[13] = min_t(uint64_t, 255, tmp);
    data[14] = 21 * (inv_scaled_adc_clk_1e3 / 10000);
    data[15] = min_t(uint32_t, 127U, (500 + (1025 * data[7])) / 1000);
    data[16] = min_t(uint32_t, 127U, (data[15] * tmp_1e3) / 1000);
    data[17] = data[15];
    data[18] = min_t(uint32_t, 127U, (500 + (975 * data[10])) / 1000);
    data[19] = min_t(uint32_t, 127U, (data[18] * tmp_1e3) / 1000);
    data[20] = data[18];
    data[21] = min_t(uint32_t, 127U, (500 + (975 * data[12])) / 1000);
    data[22] = min_t(uint32_t, 127, (data[21] * tmp_1e3) / 1000);
    data[23] = data[21];
    data[24] = 0x2E;
    data[25] = (128 + min_t(uint32_t, 63000U, DIV_ROUND_CLOSEST(63 * scaled_adc_clk_1e6, 1000)) / 1000);
    data[26] = min_t(uint32_t, 63U, 63 * scaled_adc_clk_1e6 / 1000000 * (920 + (80 * inv_scaled_adc_clk_1e3 / 1000)) / 1000);
    data[27] = min_t(uint32_t, 63, (32 * sqrt_term_1e3) / 1000);
    data[28] = data[25];
    data[29] = data[26];
    data[30] = data[27];
    data[31] = data[25];
    data[32] = data[26];
    data[33] = min_t(uint32_t, 63U, 63 * sqrt_term_1e3 / 1000);
    data[34] = min_t(uint32_t, 127U, 64 * sqrt_term_1e3 / 1000);
    data[35] = 0x40;
    data[36] = 0x40;
    data[37] = 0x2C;
    data[38] = 0x00;
    data[39] = 0x00;

    for (i = 0; i < 40; i++) {
        SPI_SDR_Write(phy->id_no, 0x200 + i, data[i]);
    }

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_rx_tia_calib(ad9361_rf_phy_t* phy, uint32_t bb_bw_Hz)

  Summary:
    Perform a RX TIA calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param bb_bw_Hz The baseband bandwidth [Hz].

  Remarks:
    None.
*/
int32_t ad9361_rx_tia_calib(ad9361_rf_phy_t* phy, //
                            uint32_t         bb_bw_Hz) {

    uint32_t Cbbf, R2346;
    uint64_t CTIA_fF;
    uint8_t  reg1EB = SPI_SDR_Read(phy->id_no, REG_RX_BBF_C3_MSB);
    uint8_t  reg1EC = SPI_SDR_Read(phy->id_no, REG_RX_BBF_C3_LSB);
    uint8_t  reg1E6 = SPI_SDR_Read(phy->id_no, REG_RX_BBF_R2346);
    uint8_t  reg1DB, reg1DF, reg1DD, reg1DC, reg1DE, temp;

    LOG_FORMAT(debug, "bb_bw_Hz %" PRIu32 " (%s)", bb_bw_Hz, __func__);

    bb_bw_Hz = clamp(bb_bw_Hz, 200000UL, 20000000UL);

    Cbbf  = (reg1EB * 160) + (reg1EC * 10) + 140; // fF
    R2346 = 18300 * RX_BBF_R2346(reg1E6);

    CTIA_fF = Cbbf * R2346 * 560ULL;
    int_do_div(&CTIA_fF, 3500000UL);

    if (bb_bw_Hz <= 3000000UL) {
        reg1DB = 0xE0;
    }
    else if (bb_bw_Hz <= 10000000UL) {
        reg1DB = 0x60;
    }
    else {
        reg1DB = 0x20;
    }

    if (CTIA_fF > 2920ULL) {
        reg1DC = 0x40;
        reg1DE = 0x40;
        temp   = min(127U, DIV_ROUND_CLOSEST((uint32_t)CTIA_fF - 400, 320U));
        reg1DD = temp;
        reg1DF = temp;
    }
    else {
        temp   = DIV_ROUND_CLOSEST((uint32_t)CTIA_fF - 400, 40U) + 0x40;
        reg1DC = temp;
        reg1DE = temp;
        reg1DD = 0;
        reg1DF = 0;
    }

    SPI_SDR_Write(phy->id_no, REG_RX_TIA_CONFIG, reg1DB);
    SPI_SDR_Write(phy->id_no, REG_TIA1_C_LSB, reg1DC);
    SPI_SDR_Write(phy->id_no, REG_TIA1_C_MSB, reg1DD);
    SPI_SDR_Write(phy->id_no, REG_TIA2_C_LSB, reg1DE);
    SPI_SDR_Write(phy->id_no, REG_TIA2_C_MSB, reg1DF);

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_rx_bb_analog_filter_calib(ad9361_rf_phy_t* phy, uint32_t rx_bb_bw, uint32_t bbpll_freq)

  Summary:
    Perform a baseband RX analog filter calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param rx_bb_bw The baseband bandwidth [Hz].
    @param bbpll_freq The BBPLL frequency [Hz].

  Remarks:
    None.
*/
int32_t ad9361_rx_bb_analog_filter_calib(ad9361_rf_phy_t* phy, //
                                         uint32_t         rx_bb_bw,
                                         uint32_t         bbpll_freq) {

    uint32_t target;
    uint8_t  _tmp;
    int32_t  _val;

    LOG_FORMAT(debug, "rx_bb_bw %" PRIu32 ", bbpll_freq %" PRIu32 " (%s)", rx_bb_bw, bbpll_freq, __func__);

    rx_bb_bw = clamp(rx_bb_bw, 200000UL, 28000000UL);

    // 1.4 * BBBW * 2PI / ln(2)
    target         = 126906UL * (rx_bb_bw / 10000UL);
    phy->rxbbf_div = min_t(uint32_t, 511UL, DIV_ROUND_UP(bbpll_freq, target));

    // Set RX baseband filter divide value
    SPI_SDR_Write(phy->id_no, REG_RX_BBF_TUNE_DIVIDE, phy->rxbbf_div);
    SPI_SDR_WriteF(phy->id_no, REG_RX_BBF_TUNE_CONFIG, BIT(0), (phy->rxbbf_div >> 8));

    // Write the BBBW into registers 0x1FB and 0x1FC
    SPI_SDR_Write(phy->id_no, REG_RX_BBBW_MHZ, rx_bb_bw / 1000000UL);

    _tmp = DIV_ROUND_CLOSEST((rx_bb_bw % 1000000UL) * 128, 1000000UL);
    SPI_SDR_Write(phy->id_no, REG_RX_BBBW_KHZ, min_t(uint8_t, 127, _tmp));

    SPI_SDR_Write(phy->id_no, REG_RX_MIX_LO_CM, RX_MIX_LO_CM(0x3F)); // Set Rx Mix LO CM

    // Enable the RX BBF tune circuit by writing 0x1E2=0x02 and 0x1E3=0x02
    SPI_SDR_Write(phy->id_no, REG_RX1_TUNE_CTRL, RX1_TUNE_RESAMPLE);
    SPI_SDR_Write(phy->id_no, REG_RX2_TUNE_CTRL, RX2_TUNE_RESAMPLE);

    // Start the RX Baseband Filter calibration in register 0x016[7]
    // Calibration is complete when register 0x016[7] self clears
    _val = ad9361_run_calibration(phy, RX_BB_TUNE_CAL);

    // Disable the RX baseband filter tune circuit, write 0x1E2=3, 0x1E3=3
    SPI_SDR_Write(phy->id_no, REG_RX1_TUNE_CTRL, RX1_TUNE_RESAMPLE | RX1_PD_TUNE);
    SPI_SDR_Write(phy->id_no, REG_RX2_TUNE_CTRL, RX2_TUNE_RESAMPLE | RX2_PD_TUNE);

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_tx_bb_analog_filter_calib(ad9361_rf_phy_t* phy, uint32_t tx_bb_bw, uint32_t bbpll_freq)

  Summary:
    Perform a baseband TX analog filter calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param tx_bb_bw The baseband bandwidth [Hz].
    @param bbpll_freq The BBPLL frequency [Hz].

  Remarks:
    None.
*/
int32_t ad9361_tx_bb_analog_filter_calib(ad9361_rf_phy_t* phy, //
                                         uint32_t         tx_bb_bw,
                                         uint32_t         bbpll_freq) {

    uint32_t target, txbbf_div;
    int32_t  _val;

    LOG_FORMAT(debug, "tx_bb_bw %" PRIu32 ", bbpll_freq %" PRIu32 " (%s)", tx_bb_bw, bbpll_freq, __func__);

    tx_bb_bw = clamp(tx_bb_bw, 625000UL, 20000000UL);

    // 1.6 * BBBW * 2PI / ln(2)
    target    = 145036 * (tx_bb_bw / 10000UL);
    txbbf_div = min_t(uint32_t, 511UL, DIV_ROUND_UP(bbpll_freq, target));

    // Set TX baseband filter divide value
    SPI_SDR_Write(phy->id_no, REG_TX_BBF_TUNE_DIVIDER, txbbf_div);
    SPI_SDR_WriteF(phy->id_no, REG_TX_BBF_TUNE_MODE, TX_BBF_TUNE_DIVIDER, (txbbf_div >> 8));

    // Enable the TX baseband filter tune circuit by setting 0x0CA=0x22.
    SPI_SDR_Write(phy->id_no, REG_TX_TUNE_CTRL, TUNER_RESAMPLE | TUNE_CTRL(1));

    // Start the TX Baseband Filter calibration in register 0x016[6]
    // Calibration is complete when register 0x016[] self clears
    _val = ad9361_run_calibration(phy, TX_BB_TUNE_CAL);

    // Disable the TX baseband filter tune circuit by writing 0x0CA=0x26.
    SPI_SDR_Write(phy->id_no, REG_TX_TUNE_CTRL, TUNER_RESAMPLE | TUNE_CTRL(1) | PD_TUNE);

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_tx_bb_second_filter_calib(ad9361_rf_phy_t* phy, uint32_t tx_bb_bw)

  Summary:
    Perform a baseband TX secondary filter calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param tx_rf_bw The RF bandwidth [Hz].

  Remarks:
    None.
*/
int32_t ad9361_tx_bb_second_filter_calib(ad9361_rf_phy_t* phy, //
                                         uint32_t         tx_bb_bw) {

    uint64_t cap;
    uint32_t corner, res = 1, div;
    uint32_t reg_conf, reg_res;

    LOG_FORMAT(debug, "tx_bb_bw %" PRIu32 " (%s)", tx_bb_bw, __func__);

    tx_bb_bw = clamp(tx_bb_bw, 530000UL, 20000000UL);

    // BBBW * 5PI
    corner = 15708 * (tx_bb_bw / 10000UL);

    for (auto i = 0; i < 4; i++) {
        div = corner * res;

        cap = (500000000ULL) + (div >> 1);

        int_do_div(&cap, div);

        cap -= 12ULL;

        if (cap < 64ULL) {
            break;
        }

        res <<= 1;
    }

    if (cap > 63ULL) {
        cap = 63ULL;
    }

    if (tx_bb_bw <= 4500000UL) {
        reg_conf = 0x59;
    }
    else if (tx_bb_bw <= 12000000UL) {
        reg_conf = 0x56;
    }
    else {
        reg_conf = 0x57;
    }

    switch (res) {
        case 1:
            reg_res = 0x0C;
            break;

        case 2:
            reg_res = 0x04;
            break;

        case 4:
            reg_res = 0x03;
            break;

        case 8:
            reg_res = 0x01;
            break;

        default:
            reg_res = 0x01;
    }

    SPI_SDR_Write(phy->id_no, REG_CONFIG0, reg_conf);
    SPI_SDR_Write(phy->id_no, REG_RESISTOR, reg_res);
    SPI_SDR_Write(phy->id_no, REG_CAPACITOR, (uint8_t)cap);

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_txrx_synth_cp_calib(ad9361_rf_phy_t* phy, uint32_t ref_clk_hz, bool tx)

  Summary:
    Perform a RF synthesizer charge pump calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param ref_clk_hz The reference clock rate [Hz].
    @param tx The Synthesizer TX = 1, RX = 0.

  Remarks:
    None.
*/
int32_t ad9361_txrx_synth_cp_calib(ad9361_rf_phy_t* phy, //
                                   uint32_t         ref_clk_hz,
                                   bool             tx) {

    uint32_t offs = tx ? 0x40 : 0;
    uint32_t vco_cal_cnt;

    LOG_FORMAT(debug, "ref_clk_hz %" PRIu32 ", %s synthesizer (%s)", ref_clk_hz, tx ? "TX" : "RX", __func__);

    // REVIST:
    SPI_SDR_Write(phy->id_no, REG_RX_DSM_SETUP_1 + offs, 0x00);

    SPI_SDR_Write(phy->id_no, REG_RX_LO_GEN_POWER_MODE + offs, 0x00);
    SPI_SDR_Write(phy->id_no, REG_RX_VCO_LDO + offs, 0x0B);
    SPI_SDR_Write(phy->id_no, REG_RX_VCO_PD_OVERRIDES + offs, 0x02);
    SPI_SDR_Write(phy->id_no, REG_RX_CP_CURRENT + offs, 0x80);
    SPI_SDR_Write(phy->id_no, REG_RX_CP_CONFIG + offs, 0x00);

    // see Table 70 Example Calibration Times for RF VCO Cal
    if (phy->pdata.fdd || phy->pdata.tdd_use_fdd_tables) {
        vco_cal_cnt = VCO_CAL_EN | VCO_CAL_COUNT(3) | FB_CLOCK_ADV(2);
    }
    else {
        if (ref_clk_hz > 40000000UL) {
            vco_cal_cnt = VCO_CAL_EN | VCO_CAL_COUNT(1) | FB_CLOCK_ADV(2);
        }
        else {
            vco_cal_cnt = VCO_CAL_EN | VCO_CAL_COUNT(0) | FB_CLOCK_ADV(2);
        }
    }

    SPI_SDR_Write(phy->id_no, REG_RX_VCO_CAL + offs, vco_cal_cnt);

    // Enable FDD mode during calibrations

    if (!phy->pdata.fdd) {
        SPI_SDR_WriteF(phy->id_no, REG_PARALLEL_PORT_CONF_3, HALF_DUPLEX_MODE, 0);
    }

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_2, DUAL_SYNTH_MODE);
    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, FORCE_ALERT_STATE | TO_ALERT);
    SPI_SDR_Write(phy->id_no, REG_ENSM_MODE, FDD_MODE);

    SPI_SDR_Write(phy->id_no, REG_RX_CP_CONFIG + offs, CP_CAL_ENABLE);

    return ad9361_check_cal_done(phy, REG_RX_CAL_STATUS + offs, CP_CAL_VALID, 1);
}

// *****************************************************************************
/* int32_t ad9361_bb_dc_offset_calib(ad9361_rf_phy_t* phy)

  Summary:
    Perform a baseband DC offset calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.

  Remarks:
    None.
*/
int32_t ad9361_bb_dc_offset_calib(ad9361_rf_phy_t* phy) {

    LOG_FORMAT(debug, "Start (%s)", __func__);

    SPI_SDR_Write(phy->id_no, REG_BB_DC_OFFSET_COUNT, 0x3F);
    SPI_SDR_Write(phy->id_no, REG_BB_DC_OFFSET_SHIFT, BB_DC_M_SHIFT(0x0F));
    SPI_SDR_Write(phy->id_no, REG_BB_DC_OFFSET_ATTEN, BB_DC_OFFSET_ATTEN(1));

    return ad9361_run_calibration(phy, BBDC_CAL);
}

// *****************************************************************************
/* int32_t ad9361_rf_dc_offset_calib(ad9361_rf_phy_t* phy, uint64_t rx_freq)

  Summary:
    Perform a RF DC offset calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param ref_clk_hz The RX LO frequency [Hz].

  Remarks:
    None.
*/
int32_t ad9361_rf_dc_offset_calib(ad9361_rf_phy_t* phy, //
                                  uint64_t         rx_freq) {

    LOG_FORMAT(debug, "rx_freq %" PRIu64 " (%s)", rx_freq, __func__);

    SPI_SDR_Write(phy->id_no, REG_WAIT_COUNT, 0x20);

    if (rx_freq <= 4000000000ULL) {
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_COUNT, phy->pdata.rf_dc_offset_count_low);
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_CONFIG_1, RF_DC_CALIBRATION_COUNT(4) | DAC_FS(2));
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_ATTEN, RF_DC_OFFSET_ATTEN(phy->pdata.dc_offset_attenuation_low));
    }
    else {
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_COUNT, phy->pdata.rf_dc_offset_count_high);
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_CONFIG_1, RF_DC_CALIBRATION_COUNT(4) | DAC_FS(3));
        SPI_SDR_Write(phy->id_no, REG_RF_DC_OFFSET_ATTEN, RF_DC_OFFSET_ATTEN(phy->pdata.dc_offset_attenuation_high));
    }

    SPI_SDR_Write(phy->id_no, REG_DC_OFFSET_CONFIG2, USE_WAIT_COUNTER_FOR_RF_DC_INIT_CAL | DC_OFFSET_UPDATE(3));

    if (phy->pdata.rx1rx2_phase_inversion_en || (phy->pdata.port_ctrl.pp_conf[1] & INVERT_RX2)) {
        SPI_SDR_Write(phy->id_no, REG_INVERT_BITS, (1 << 4));
    }
    else {
        SPI_SDR_Write(phy->id_no, REG_INVERT_BITS, INVERT_RX1_RF_DC_CGOUT_WORD | INVERT_RX2_RF_DC_CGOUT_WORD);
    }

    return ad9361_run_calibration(phy, RFDC_CAL);
}

// *****************************************************************************
/* int32_t ad9361_update_rf_bandwidth(ad9361_rf_phy_t* phy, uint32_t rf_rx_bw, uint32_t rf_tx_bw)

  Summary:
    Update RF bandwidth.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param rf_rx_bw RF RX bandwidth [Hz].
    @param rf_tx_bw RF TX bandwidth [Hz].

  Remarks:
    None.
*/
int32_t ad9361_update_rf_bandwidth(ad9361_rf_phy_t* phy, //
                                   uint32_t         rf_rx_bw,
                                   uint32_t         rf_tx_bw) {

    uint32_t real_rx_bandwidth = rf_rx_bw / 2;
    uint32_t real_tx_bandwidth = rf_tx_bw / 2;
    uint32_t bbpll_freq;
    int32_t  _val;

    LOG_FORMAT(debug, "rf_rx_bw %" PRIu32 ", rf_tx_bw %" PRIu32 " (%s)", rf_rx_bw, rf_tx_bw, __func__);

    bbpll_freq = clk_get_rate(phy, &(phy->ref_clk_scale[BBPLL_CLK]));

    _val = ad9361_rx_bb_analog_filter_calib(phy, real_rx_bandwidth, bbpll_freq);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_tx_bb_analog_filter_calib(phy, real_tx_bandwidth, bbpll_freq);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rx_tia_calib(phy, real_rx_bandwidth);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_tx_bb_second_filter_calib(phy, real_tx_bandwidth);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rx_adc_setup(phy, bbpll_freq, clk_get_rate(phy, &(phy->ref_clk_scale[ADC_CLK])));
    if (_val < 0) {
        return _val;
    }

    return 0;
}

// *****************************************************************************
/* static int32_t __ad9361_tx_quad_calib(ad9361_rf_phy_t* phy, uint32_t phase, uint32_t rxnco_word, uint32_t decim, uint8_t* res)

  Summary:
    TX quad calib.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param phase phase
    @param rxnco_word Rx NCO word.
    @param decim decim
    @param res res

  Remarks:
    None.
*/
static int32_t __ad9361_tx_quad_calib(ad9361_rf_phy_t* phy, //
                                      uint32_t         phase,
                                      uint32_t         rxnco_word,
                                      uint32_t         decim,
                                      uint8_t*         res) {

    int32_t _val;

    SPI_SDR_Write(phy->id_no, REG_QUAD_CAL_NCO_FREQ_PHASE_OFFSET, RX_NCO_FREQ(rxnco_word) | RX_NCO_PHASE_OFFSET(phase));
    SPI_SDR_Write(phy->id_no, REG_QUAD_CAL_CTRL, SETTLE_MAIN_ENABLE | DC_OFFSET_ENABLE | QUAD_CAL_SOFT_RESET | GAIN_ENABLE | PHASE_ENABLE | M_DECIM(decim));
    SPI_SDR_Write(phy->id_no, REG_QUAD_CAL_CTRL, SETTLE_MAIN_ENABLE | DC_OFFSET_ENABLE | GAIN_ENABLE | PHASE_ENABLE | M_DECIM(decim));

    _val = ad9361_run_calibration(phy, TX_QUAD_CAL);
    if (_val < 0) {
        return _val;
    }

    if (res != nullptr) {
        *res = SPI_SDR_Read(phy->id_no, REG_QUAD_CAL_STATUS_TX1) & (TX1_LO_CONV | TX1_SSB_CONV);
    }

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_tx_quad_phase_search(ad9361_rf_phy_t* phy, uint32_t rxnco_word, uint8_t decim)

  Summary:
    Loop through all possible phase offsets in case the QUAD CAL doesn't converge.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param rxnco_word Rx NCO word.

  Remarks:
    None.
*/
int32_t ad9361_tx_quad_phase_search(ad9361_rf_phy_t* phy, //
                                    uint32_t         rxnco_word,
                                    uint8_t          decim) {

    int32_t  _val;
    uint8_t  field[64], val;
    uint32_t start;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    for (auto i = 0; i < (int64_t)(ARRAY_SIZE(field) / 2); i++) {
        _val = __ad9361_tx_quad_calib(phy, i, rxnco_word, decim, &val);

        if (_val < 0) {
            return _val;
        }

        // Handle 360/0 wrap around
        field[i] = field[i + 32] = !((val & TX1_LO_CONV) && (val & TX1_SSB_CONV));
    }

    _val = ad9361_find_opt_delay(field, ARRAY_SIZE(field), &start);

    phy->last_tx_quad_cal_phase = (start + _val / 2) & 0x1F;

    _val = __ad9361_tx_quad_calib(phy, phy->last_tx_quad_cal_phase, rxnco_word, decim, NULL);
    if (_val < 0) {
        return _val;
    }

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_tx_quad_calib(ad9361_rf_phy_t* phy, uint32_t bw_rx, uint32_t bw_tx, int32_t rx_phase)

  Summary:
    Perform a TX quadrature calibration.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param bw_rx The RX bandwidth [Hz].
    @param bw_tx The TX bandwidth [Hz].
    @param rx_phase The optional RX phase value overwrite (set to zero).

  Remarks:
    None.
*/
int32_t ad9361_tx_quad_calib(ad9361_rf_phy_t* phy, //
                             uint32_t         bw_rx,
                             uint32_t         bw_tx,
                             int32_t          rx_phase) {

    uint32_t clktf, clkrf;
    int32_t  txnco_word, rxnco_word, txnco_freq, _val;
    uint8_t  __rx_phase = 0, reg_inv_bits = 0, val = 0, decim;
    uint32_t index_max, i, lpf_tia_mask;

    const uint8_t(*tab)[3];

    // Find NCO frequency that matches this equation:
    // BW / 4 = Rx NCO freq = Tx NCO freq:
    // Rx NCO = ClkRF * (rxNCO <1:0> + 1) / 32
    // Tx NCO = ClkTF * (txNCO <1:0> + 1) / 32

    clkrf = clk_get_rate(phy, &(phy->ref_clk_scale[CLKRF_CLK]));
    clktf = clk_get_rate(phy, &(phy->ref_clk_scale[CLKTF_CLK]));

    LOG_FORMAT(debug,
               "bw_tx %" PRIu32 ", clkrf %" PRIu32 ", clktf %" PRIu32 " (%s)", //
               bw_tx,
               clkrf,
               clktf,
               __func__);

    txnco_word = DIV_ROUND_CLOSEST(bw_tx * 8, clktf) - 1;
    txnco_word = clamp_t(int, txnco_word, 0, 3);
    rxnco_word = txnco_word;

    LOG_FORMAT(debug,
               "TX NCO frequency: %" PRIu32 ", BW/4: %" PRIu32 ", txnco_word %" PRId32 " (%s)", //
               clktf * (txnco_word + 1) / 32,
               bw_tx / 4,
               txnco_word,
               __func__);

    if (clktf <= 4000000UL) {
        decim = 2;
    }
    else {
        decim = 3;
    }

    if (clkrf == (2 * clktf)) {
        __rx_phase = 0x0E;

        switch (txnco_word) {
            case 0:
                txnco_word++;
                break;

            case 1:
                rxnco_word--;
                break;

            case 2:
                rxnco_word -= 2;
                txnco_word--;
                break;

            case 3:
                rxnco_word -= 2; // REVISIT
                __rx_phase = 0x08;
                break;

            default:
                break;
        }
    }
    else if (clkrf == clktf) {
        switch (txnco_word) {
            case 0:
            case 3:
                __rx_phase = 0x15;
                break;

            case 2:
                __rx_phase = 0x1F;
                break;

            case 1:
                if (SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, 0x3F) == 0x22) {
                    __rx_phase = 0x15; // REVISIT
                }
                else {
                    __rx_phase = 0x1A;
                }
                break;

            default:
                break;
        }
    }
    else {
        LOG_FORMAT(warning,
                   "Un-handled case in line %d, clkrf %" PRIu32 ", clktf %" PRIu32 " (%s)", //
                   __LINE__,
                   clkrf,
                   clktf,
                   __func__);
    }

    if (rx_phase >= 0) {
        __rx_phase = rx_phase;
    }

    txnco_freq = clktf * (txnco_word + 1) / 32;

    if ((txnco_freq > (int64_t)(bw_rx / 4)) || (txnco_freq > (int64_t)(bw_tx / 4))) {
        // Make sure the BW during calibration is wide enough
        _val = ad9361_update_rf_bandwidth(phy, txnco_freq * 8, txnco_freq * 8);
        if (_val < 0) {
            return _val;
        }
    }

    if (phy->pdata.rx1rx2_phase_inversion_en || (phy->pdata.port_ctrl.pp_conf[1] & INVERT_RX2)) {
        SPI_SDR_WriteF(phy->id_no, REG_PARALLEL_PORT_CONF_2, INVERT_RX2, 0);

        reg_inv_bits = SPI_SDR_Read(phy->id_no, REG_INVERT_BITS);

        SPI_SDR_Write(phy->id_no, REG_INVERT_BITS, INVERT_RX1_RF_DC_CGOUT_WORD | INVERT_RX2_RF_DC_CGOUT_WORD);
    }

    SPI_SDR_WriteF(phy->id_no, REG_KEXP_2, TX_NCO_FREQ(~0), txnco_word);
    SPI_SDR_Write(phy->id_no, REG_QUAD_CAL_COUNT, 0xFF);
    SPI_SDR_Write(phy->id_no, REG_KEXP_1, KEXP_TX(1) | KEXP_TX_COMP(3) | KEXP_DC_I(3) | KEXP_DC_Q(3));
    SPI_SDR_Write(phy->id_no, REG_MAG_FTEST_THRESH, 0x01);
    SPI_SDR_Write(phy->id_no, REG_MAG_FTEST_THRESH_2, 0x01);

    if (phy->pdata.split_gt) {
        tab          = &split_gain_table[phy->current_table][0];
        index_max    = SIZE_SPLIT_TABLE;
        lpf_tia_mask = 0x20;
    }
    else {
        tab          = &full__gain_table[phy->current_table][0];
        index_max    = SIZE_FULL__TABLE;
        lpf_tia_mask = 0x3F;
    }

    for (i = 0; i < index_max; i++) {
        if ((tab[i][1] & lpf_tia_mask) == 0x20) {
            SPI_SDR_Write(phy->id_no, REG_TX_QUAD_FULL_LMT_GAIN, i);
            break;
        }
    }

    if (i >= index_max) {
        LOG_FORMAT(error, "Failed to find suitable LPF TIA value in gain table (%s)", __func__);
    }

    SPI_SDR_Write(phy->id_no, REG_QUAD_SETTLE_COUNT, 0xF0);
    SPI_SDR_Write(phy->id_no, REG_TX_QUAD_LPF_GAIN, 0x00);

    _val = __ad9361_tx_quad_calib(phy, __rx_phase, rxnco_word, decim, &val);

    LOG_FORMAT(debug,
               "LO leakage %d, Quadrature Calibration %d, rx_phase %d (%s)", //
               !!(val & TX1_LO_CONV),
               !!(val & TX1_SSB_CONV),
               __rx_phase,
               __func__);

    // Calibration failed -> try last phase offset
    if (val != (TX1_LO_CONV | TX1_SSB_CONV)) {
        if (phy->last_tx_quad_cal_phase < 31) {
            _val = __ad9361_tx_quad_calib(phy, phy->last_tx_quad_cal_phase, rxnco_word, decim, &val);
        }
    }
    else {
        phy->last_tx_quad_cal_phase = __rx_phase;
    }

    // Calibration failed -> loop through all 32 phase offsets
    if (val != (TX1_LO_CONV | TX1_SSB_CONV)) {
        _val = ad9361_tx_quad_phase_search(phy, rxnco_word, decim);
    }

    if (phy->pdata.rx1rx2_phase_inversion_en || (phy->pdata.port_ctrl.pp_conf[1] & INVERT_RX2)) {
        SPI_SDR_WriteF(phy->id_no, REG_PARALLEL_PORT_CONF_2, INVERT_RX2, 1);
        SPI_SDR_Write(phy->id_no, REG_INVERT_BITS, reg_inv_bits);
    }

    if (phy->pdata.rx1rx2_phase_inversion_en || (phy->pdata.port_ctrl.pp_conf[1] & INVERT_RX2)) {
        SPI_SDR_WriteF(phy->id_no, REG_PARALLEL_PORT_CONF_2, INVERT_RX2, 1);
        SPI_SDR_Write(phy->id_no, REG_INVERT_BITS, reg_inv_bits);
    }

    if ((txnco_freq > (int64_t)(bw_rx / 4)) || (txnco_freq > (int64_t)(bw_tx / 4))) {
        ad9361_update_rf_bandwidth(phy, phy->current_rx_bw_Hz, phy->current_tx_bw_Hz);
    }

    return _val;
}

// *****************************************************************************
/* int32_t ad9361_tracking_control(ad9361_rf_phy_t* phy, bool bbdc_track, bool rfdc_track, bool rxquad_track)

  Summary:
    Setup RX tracking calibrations.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.
    @param bbdc_track Set true, will enable the BBDC tracking.
    @param rfdc_track Set true, will enable the RFDC tracking.
    @param rxquad_track Set true, will enable the RXQUAD tracking.

  Remarks:
    None.
*/
int32_t ad9361_tracking_control(ad9361_rf_phy_t* phy, //
                                bool             bbdc_track,
                                bool             rfdc_track,
                                bool             rxquad_track) {

    uint32_t qtrack = 0;

    LOG_FORMAT(debug,
               "bbdc_track %d, rfdc_track %d, rxquad_track %d (%s)", //
               bbdc_track,
               rfdc_track,
               rxquad_track,
               __func__);

    SPI_SDR_Write(phy->id_no, REG_CALIBRATION_CONFIG_2, CALIBRATION_CONFIG2_DFLT | K_EXP_PHASE(0x15));

    SPI_SDR_Write(phy->id_no, REG_CALIBRATION_CONFIG_3, PREVENT_POS_LOOP_GAIN | K_EXP_AMPLITUDE(0x15));

    SPI_SDR_Write(phy->id_no, REG_DC_OFFSET_CONFIG2, USE_WAIT_COUNTER_FOR_RF_DC_INIT_CAL | DC_OFFSET_UPDATE(phy->pdata.dc_offset_update_events) | (bbdc_track ? ENABLE_BB_DC_OFFSET_TRACKING : 0) | (rfdc_track ? ENABLE_RF_OFFSET_TRACKING : 0));

    SPI_SDR_WriteF(phy->id_no, REG_RX_QUAD_GAIN2, CORRECTION_WORD_DECIMATION_M(~0), (phy->pdata.qec_tracking_slow_mode_en ? 4 : 0));

    if (rxquad_track) {
        qtrack = ENABLE_TRACKING_MODE_CH1 | (phy->pdata.rx2tx2 ? ENABLE_TRACKING_MODE_CH2 : 0);
    }

    SPI_SDR_Write(phy->id_no, REG_CALIBRATION_CONFIG_1, ENABLE_PHASE_CORR | ENABLE_GAIN_CORR | FREE_RUN_MODE | ENABLE_CORR_WORD_DECIMATION | qtrack);

    return 0;
}

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* uint32_t clk_get_rate(ad9361_rf_phy_t* phy, refclk_scale_t* clk_priv)

  Summary:
    Get clock rate.

  Description:
    Get clock rate.

  Remarks:
    None.
*/
uint32_t clk_get_rate(ad9361_rf_phy_t* phy, //
                      refclk_scale_t*  clk_priv) {

    uint32_t rate   = 0;
    uint32_t source = clk_priv->source;

    switch (source) {

        case TX_REFCLK:
        case RX_REFCLK:
        case BB_REFCLK:
            rate = ad9361_clk_factor_recalc_rate(phy, clk_priv, phy->clk_refin.rate);
            break;

        case TX_RFPLL:
        case RX_RFPLL:
            rate = ad9361_rfpll_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
            break;

        case BBPLL_CLK:
            rate = ad9361_bbpll_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
            break;

        case ADC_CLK:
        case R2_CLK:
        case R1_CLK:
        case CLKRF_CLK:
        case RX_SAMPL_CLK:
        case DAC_CLK:
        case T2_CLK:
        case T1_CLK:
        case CLKTF_CLK:
        case TX_SAMPL_CLK:
            rate = ad9361_clk_factor_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
            break;

        default:
            break;
    }

    return rate;
}

/**
 * Calculate the closest possible clock rate that can be set.
 * @param rate The clock rate.
 * @param prate The parent clock rate.
 * @return The closest possible clock rate that can be set.
 */
int32_t ad9361_bbpll_round_rate(uint32_t  rate, //
                                uint32_t* prate) {

    uint64_t _tmp, temp;
    uint32_t fract, integer;

    if (rate > MAX_BBPLL_FREQ) {
        return MAX_BBPLL_FREQ;
    }

    if (rate < MIN_BBPLL_FREQ) {
        return MIN_BBPLL_FREQ;
    }

    temp = rate;
    _tmp = int_do_div(&temp, *prate);
    rate = temp;
    _tmp = _tmp * BBPLL_MODULUS + (*prate >> 1);

    int_do_div(&_tmp, *prate);

    integer = rate;
    fract   = _tmp;
    _tmp    = *prate * (uint64_t)fract;

    int_do_div(&_tmp, BBPLL_MODULUS);

    _tmp += *prate * integer;

    return _tmp;
}

/**
 * Calculate the closest possible clock rate that can be set.
 * @param refclk_scale The refclk_scale structure.
 * @param rate The clock rate.
 * @param parent_rate The parent clock rate.
 * @return The closest possible clock rate that can be set.
 */
int32_t ad9361_rfpll_round_rate(uint32_t rate) {

    if (ad9361_from_clk(rate) > MAX_CARRIER_FREQ_HZ || ad9361_from_clk(rate) < MIN_CARRIER_FREQ_HZ) {
        return -EINVAL;
    }

    return rate;
}

// *****************************************************************************
/* int32_t clk_set_rate(ad9361_rf_phy_t* phy, refclk_scale_t* clk_priv, uint32_t rate)

  Summary:
    Set clock rate.

  Description:
    Set clock rate.

  Remarks:
    None.
*/
int32_t clk_set_rate(ad9361_rf_phy_t* phy, //
                     refclk_scale_t*  clk_priv,
                     uint32_t         rate) {

    uint32_t source = clk_priv->source;
    int32_t  i;
    uint32_t round_rate;

    if (phy->clks[source].rate != rate) {
        switch (source) {
            case TX_REFCLK:
            case RX_REFCLK:
            case BB_REFCLK:
                round_rate = ad9361_clk_factor_round_rate(phy, clk_priv, rate, &phy->clk_refin.rate);
                ad9361_clk_factor_set_rate(phy, clk_priv, round_rate, phy->clk_refin.rate);
                phy->clks[source].rate = ad9361_clk_factor_recalc_rate(phy, clk_priv, phy->clk_refin.rate);
                break;

            case TX_RFPLL:
            case RX_RFPLL:
                round_rate = ad9361_rfpll_round_rate(rate);
                ad9361_rfpll_set_rate(phy, clk_priv, round_rate, phy->clks[clk_priv->parent_source].rate);
                phy->clks[source].rate = ad9361_rfpll_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
                break;

            case BBPLL_CLK:
                round_rate = ad9361_bbpll_round_rate(rate, &phy->clks[clk_priv->parent_source].rate);
                ad9361_bbpll_set_rate(phy, clk_priv, round_rate, phy->clks[clk_priv->parent_source].rate);
                phy->clks[source].rate = ad9361_bbpll_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
                break;

            case ADC_CLK:
            case R2_CLK:
            case R1_CLK:
            case CLKRF_CLK:
            case RX_SAMPL_CLK:
            case DAC_CLK:
            case T2_CLK:
            case T1_CLK:
            case CLKTF_CLK:
            case TX_SAMPL_CLK:
                round_rate = ad9361_clk_factor_round_rate(phy, clk_priv, rate, &phy->clks[clk_priv->parent_source].rate);
                ad9361_clk_factor_set_rate(phy, clk_priv, round_rate, phy->clks[clk_priv->parent_source].rate);
                phy->clks[source].rate = ad9361_clk_factor_recalc_rate(phy, clk_priv, phy->clks[clk_priv->parent_source].rate);
                break;

            default:
                break;
        }

        for (i = BB_REFCLK; i < BBPLL_CLK; i++) {
            phy->clks[i].rate = ad9361_clk_factor_recalc_rate(phy, &(phy->ref_clk_scale[i]), phy->clk_refin.rate);
        }

        phy->clks[BBPLL_CLK].rate = ad9361_bbpll_recalc_rate(phy, &(phy->ref_clk_scale[BBPLL_CLK]), phy->clks[phy->ref_clk_scale[BBPLL_CLK].parent_source].rate);

        for (i = ADC_CLK; i < RX_RFPLL; i++) {
            phy->clks[i].rate = ad9361_clk_factor_recalc_rate(phy, &(phy->ref_clk_scale[i]), phy->clks[phy->ref_clk_scale[i].parent_source].rate);
        }

        for (i = RX_RFPLL; i < NUM_AD9361_CLKS; i++) {
            phy->clks[i].rate = ad9361_rfpll_recalc_rate(phy, &(phy->ref_clk_scale[i]), phy->clks[phy->ref_clk_scale[i].parent_source].rate);
        }
    }

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_bist_loopback(ad9361_rf_phy_t* phy, int32_t mode)

  Summary:
    BIST loopback mode.

  Description:
    Returns 0 in case of success, negative error code otherwise.

  Remarks:
    None.
*/
int32_t ad9361_set_bist_loopback(ad9361_rf_phy_t* phy, //
                                 int32_t          mode) {

    uint32_t sp_hd, reg;

    LOG_FORMAT(debug, "mode %" PRId32 " (%s)", mode, __func__);

    reg = SPI_SDR_Read(phy->id_no, REG_OBSERVE_CONFIG);

    phy->bist_loopback_mode = mode;

    switch (mode) {
        case 0:
            reg &= ~(DATA_PORT_SP_HD_LOOP_TEST_OE | DATA_PORT_LOOP_TEST_ENABLE);
            SPI_SDR_Write(phy->id_no, REG_OBSERVE_CONFIG, reg);
            return 0;

        case 1:
            // loopback (AD9361 internal) TX->RX
            sp_hd = SPI_SDR_Read(phy->id_no, REG_PARALLEL_PORT_CONF_3);
            if ((sp_hd & SINGLE_PORT_MODE) && (sp_hd & HALF_DUPLEX_MODE)) {
                reg |= DATA_PORT_SP_HD_LOOP_TEST_OE;
            }
            else {
                reg &= ~DATA_PORT_SP_HD_LOOP_TEST_OE;
            }
            reg |= DATA_PORT_LOOP_TEST_ENABLE;
            SPI_SDR_Write(phy->id_no, REG_OBSERVE_CONFIG, reg);
            return 0;

        case 2:
            // loopback (FPGA internal) RX->TX
            reg &= ~(DATA_PORT_SP_HD_LOOP_TEST_OE | DATA_PORT_LOOP_TEST_ENABLE);
            SPI_SDR_Write(phy->id_no, REG_OBSERVE_CONFIG, reg);
            return 0;

        default:
            return -EINVAL;
    }
}

// *****************************************************************************
/* int32_t ad9361_get_bist_loopback(ad9361_rf_phy_t* phy)

  Summary:
    BIST loopback mode.

  Description:
    Get BIST loopback mode. Returns 0 in case of success, negative error code otherwise.

  Remarks:
    None.
*/
int32_t ad9361_get_bist_loopback(ad9361_rf_phy_t* phy) {

    return phy->bist_loopback_mode;
}

// *****************************************************************************
/* int32_t  ad9361_bist_prbs(ad9361_rf_phy_t* phy, ad9361_bist_mode_t mode)

  Summary:
    Bist mode (pseudo random binary sequence).

  Description:
    Returns 0 in case of success, negative error code otherwise.

  Remarks:
    None.
*/
int32_t ad9361_bist_prbs(ad9361_rf_phy_t*   phy, //
                         ad9361_bist_mode_t mode) {

    uint32_t reg = 0;

    LOG_FORMAT(debug, "mode %d (%s)", mode, __func__);

    phy->bist_prbs_mode = mode;

    switch (mode) {

        case BIST_DISABLE:
            reg = 0;
            break;

        case BIST_INJ_TX:
            reg = BIST_CTRL_POINT(0) | BIST_ENABLE;
            break;

        case BIST_INJ_RX:
            reg = BIST_CTRL_POINT(2) | BIST_ENABLE;
            break;

        default:
            return -1;
    }

    SPI_SDR_Write(phy->id_no, REG_BIST_CONFIG, reg);

    return 0;
}

// *****************************************************************************
/* uint32_t ad9361_to_clk(uint64_t freq)

  Summary:
    Shift the real frequency value, so it fits type unsigned long.

  Description:
    Returns the shifted frequency value.

   @param freq The frequency value [Hz].

  Remarks:
    None.
*/
uint32_t ad9361_to_clk(uint64_t freq) {

    return (uint32_t)(freq >> 1);
}

// *****************************************************************************
/* uint32_t ad9361_to_clk(uint64_t freq)

  Summary:
    Shift back the frequency value, so it reflects the real value.

  Description:
    Returns the shifted frequency value.
    Note: PLL operates between 47 .. 6000 MHz which is > 2^32.

    @param freq The frequency value [Hz].

  Remarks:
    None.
*/
uint64_t ad9361_from_clk(uint32_t freq) {

    return ((uint64_t)freq << 1);
}

// *****************************************************************************
/* int32_t  ad9361_set_tx_atten(ad9361_rf_phy_t* phy, uint32_t atten_mdb, bool tx1, bool tx2, bool immed)

  Summary:
    Set the attenuation for the selected TX channels.

  Description:
    Returns the shifted frequency value.

    @param phy The AD9361 state structure.
    @param atten_mdb Attenuation value [mdB].
    @param tx1 Set true, the attenuation of the TX1 will be affected.
    @param tx2 Set true, the attenuation of the TX2 will be affected.
    @param immed Set true, an immediate update will take place.

  Remarks:
    None.
*/
int32_t ad9361_set_tx_atten(ad9361_rf_phy_t* phy, //
                            uint32_t         atten_mdb,
                            bool             tx1,
                            bool             tx2,
                            bool             immed) {

    uint8_t buf[2];

    LOG_FORMAT(debug, "atten_mdb %" PRIu32 ", tx1 %d, tx2 %d (%s)", atten_mdb, tx1, tx2, __func__);

    if (atten_mdb > 89750) { // 89.75 dB
        return -EINVAL;
    }

    atten_mdb /= 250; // Scale to 0.25dB / LSB

    buf[0] = atten_mdb >> 8;
    buf[1] = atten_mdb & 0xFF;

    SPI_SDR_WriteF(phy->id_no, REG_TX2_DIG_ATTEN, IMMEDIATELY_UPDATE_TPC_ATTEN, 0);

    if (tx1) {
        SPI_SDR_WriteM(phy->id_no, REG_TX1_ATTEN_1, buf, 2);
    }

    if (tx2) {
        SPI_SDR_WriteM(phy->id_no, REG_TX2_ATTEN_1, buf, 2);
    }

    if (immed) {
        SPI_SDR_WriteF(phy->id_no, REG_TX2_DIG_ATTEN, IMMEDIATELY_UPDATE_TPC_ATTEN, 1);
    }

    return 0;
}

// *****************************************************************************
/* int32_t ad9361_get_tx_atten(ad9361_rf_phy_t* phy, uint32_t tx_num)

  Summary:
    Get the attenuation for the selected TX channel.

  Description:
    Returns the attenuation value [mdB] or negative error code in case of failure.

    @param phy The AD9361 state structure.
    @param tx_num The selected channel [1, 2].

  Remarks:
    None.
*/
int32_t ad9361_get_tx_atten(ad9361_rf_phy_t* phy, //
                            uint32_t         tx_num) {

    uint8_t  buf[2] = {0x00, 0x00};
    uint32_t code;

    SPI_SDR_ReadM(phy->id_no, (tx_num == 1) ? REG_TX1_ATTEN_1 : REG_TX2_ATTEN_1, buf, 2);

    code = (buf[0] << 8) | buf[1];

    code *= 250;

    return code;
}

// *****************************************************************************
/* int32_t ad9361_init_gain_tables(ad9361_rf_phy_t* phy)

  Summary:
    Initialize the gain table information.

  Description:
    Returns 0 in case of success, negative error code otherwise.

    @param phy The AD9361 state structure.

  Remarks:
    None.
*/
int32_t ad9361_init_gain_tables(ad9361_rf_phy_t* phy) {

    rx_gain_info_t* rx_gain;

    // Initialize Meta data according to default gain tables of AD9631.
    // Changing/Writing of gain tables is not supported yet.
    rx_gain = &phy->rx_gain[TBL_200_1300_MHZ];
    ad9361_init_gain_info(rx_gain, RXGAIN_FULL_TBL, 1, 77, 1, SIZE_FULL__TABLE, 0);

    rx_gain = &phy->rx_gain[TBL_1300_4000_MHZ];
    ad9361_init_gain_info(rx_gain, RXGAIN_FULL_TBL, -4, 71, 1, SIZE_FULL__TABLE, 1);

    rx_gain = &phy->rx_gain[TBL_4000_6000_MHZ];
    ad9361_init_gain_info(rx_gain, RXGAIN_FULL_TBL, -10, 62, 1, SIZE_FULL__TABLE, 4);

    return 0;
}

/**
 * Enable/disable the VCO cal.
 * @param phy The AD9361 state structure.
 * @param rx Set true for rx.
 * @param enable Set true to enable.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_trx_vco_cal_control(ad9361_rf_phy_t* phy, //
                                   bool             tx,
                                   bool             enable) {

    LOG_FORMAT(debug, "state %d (%s)", enable, __func__);

    SPI_SDR_WriteF(phy->id_no, tx ? REG_TX_PFD_CONFIG : REG_RX_PFD_CONFIG, BYPASS_LD_SYNTH, !enable);

    return 0;
}

/**
 * Enable/disable the ext. LO.
 * @param phy The AD9361 state structure.
 * @param rx Set true for rx.
 * @param enable Set true to enable.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_trx_ext_lo_control(ad9361_rf_phy_t* phy, //
                                  bool             tx,
                                  bool             enable) {

    int32_t val = enable ? ~0 : 0;

    LOG_FORMAT(debug, "state %d (%s)", enable, __func__);

    if (tx) {
        SPI_SDR_WriteF(phy->id_no, REG_ENSM_CONFIG_2, POWER_DOWN_TX_SYNTH, enable);

        SPI_SDR_WriteF(phy->id_no, REG_RFPLL_DIVIDERS, TX_VCO_DIVIDER(~0), 0x07);

        SPI_SDR_Write(phy->id_no, REG_TX_SYNTH_POWER_DOWN_OVERRIDE, enable ? TX_SYNTH_VCO_ALC_POWER_DOWN | TX_SYNTH_PTAT_POWER_DOWN | TX_SYNTH_VCO_POWER_DOWN : 0x00);

        SPI_SDR_WriteF(phy->id_no, REG_ANALOG_POWER_DOWN_OVERRIDE, TX_EXT_VCO_BUFFER_POWER_DOWN, !enable);

        SPI_SDR_Write(phy->id_no, REG_TX_LO_GEN_POWER_MODE, TX_LO_GEN_POWER_MODE(val));
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_ENSM_CONFIG_2, POWER_DOWN_RX_SYNTH, enable);

        SPI_SDR_WriteF(phy->id_no, REG_RFPLL_DIVIDERS, RX_VCO_DIVIDER(~0), 0x07);

        SPI_SDR_Write(phy->id_no, REG_RX_SYNTH_POWER_DOWN_OVERRIDE, enable ? RX_SYNTH_VCO_ALC_POWER_DOWN | RX_SYNTH_PTAT_POWER_DOWN | RX_SYNTH_VCO_POWER_DOWN : 0x00);

        SPI_SDR_WriteF(phy->id_no, REG_ANALOG_POWER_DOWN_OVERRIDE, RX_EXT_VCO_BUFFER_POWER_DOWN, !enable);

        SPI_SDR_Write(phy->id_no, REG_RX_LO_GEN_POWER_MODE, RX_LO_GEN_POWER_MODE(val));
    }

    return 0;
}

/**
 * Setup the reference clock delay unit counter register.
 * @param phy The AD9361 state structure.
 * @param ref_clk_hz The reference clock frequency [Hz].
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_ref_clk_cycles(ad9361_rf_phy_t* phy, //
                                  uint32_t         ref_clk_hz) {

    LOG_FORMAT(debug, "ref_clk_hz %" PRIu32 " (%s)", ref_clk_hz, __func__);

    SPI_SDR_Write(phy->id_no, REG_REFERENCE_CLOCK_CYCLES, REFERENCE_CLOCK_CYCLES_PER_US((ref_clk_hz / 1000000UL) - 1));

    return 0;
}

/**
 * Setup the DCXO tune.
 * @param phy The AD9361 state structure.
 * @param coarse The DCXO tune coarse.
 * @param fine The DCXO tune fine.
 * @return 0 in case of success, negative error code otherwise.
 */
void ad9361_set_dcxo_tune(ad9361_rf_phy_t* phy, //
                          uint32_t         coarse,
                          uint32_t         fine) {

    LOG_FORMAT(debug, "coarse %" PRIu32 ", fine %" PRIu32 " (%s)", coarse, fine, __func__);

    SPI_SDR_Write(phy->id_no, REG_DCXO_COARSE_TUNE, DCXO_TUNE_COARSE(coarse));
    SPI_SDR_Write(phy->id_no, REG_DCXO_FINE_TUNE_LOW, DCXO_TUNE_FINE_LOW(fine));
    SPI_SDR_Write(phy->id_no, REG_DCXO_FINE_TUNE_HIGH, DCXO_TUNE_FINE_HIGH(fine));
}

/**
 * Setup TXMON.
 * @param phy The AD9361 state structure.
 * @param ctrl The TXMON settings.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_txmon_setup(ad9361_rf_phy_t*      phy, //
                           tx_monitor_control_t* ctrl) {

    LOG_FORMAT(debug, "Start (%s)", __func__);

    SPI_SDR_Write(phy->id_no, REG_TPM_MODE_ENABLE, (ctrl->one_shot_mode_en ? ONE_SHOT_MODE : 0) | TX_MON_DURATION(int_log2(ctrl->tx_mon_duration / 16)));

    SPI_SDR_Write(phy->id_no, REG_TX_MON_DELAY, ctrl->tx_mon_delay);

    SPI_SDR_Write(phy->id_no, REG_TX_MON_1_CONFIG, TX_MON_1_LO_CM(ctrl->tx1_mon_lo_cm) | TX_MON_1_GAIN(ctrl->tx1_mon_front_end_gain));

    SPI_SDR_Write(phy->id_no, REG_TX_MON_2_CONFIG, TX_MON_2_LO_CM(ctrl->tx2_mon_lo_cm) | TX_MON_2_GAIN(ctrl->tx2_mon_front_end_gain));

    SPI_SDR_Write(phy->id_no, REG_TX_ATTEN_THRESH, ctrl->low_high_gain_threshold_mdB / 250);

    SPI_SDR_Write(phy->id_no, REG_TX_MON_HIGH_GAIN, TX_MON_HIGH_GAIN(ctrl->high_gain_dB));

    SPI_SDR_Write(phy->id_no, REG_TX_MON_LOW_GAIN, (ctrl->tx_mon_track_en ? TX_MON_TRACK : 0) | TX_MON_LOW_GAIN(ctrl->low_gain_dB));

    return 0;
}

/**
 * Enable TXMON.
 * @param phy The AD9361 state structure.
 * @param en_mask The enable mask.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_txmon_control(ad9361_rf_phy_t* phy, //
                             int32_t          en_mask) {

    LOG_FORMAT(debug, "mask 0x%" PRIx32 " (%s)", en_mask, __func__);

    SPI_SDR_WriteF(phy->id_no, REG_ANALOG_POWER_DOWN_OVERRIDE, TX_MONITOR_POWER_DOWN(~0), ~en_mask);

    SPI_SDR_WriteF(phy->id_no, REG_TPM_MODE_ENABLE, TX1_MON_ENABLE, !!(en_mask & TX_1));

    SPI_SDR_WriteF(phy->id_no, REG_TPM_MODE_ENABLE, TX2_MON_ENABLE, !!(en_mask & TX_2));

    return 0;
}

/**
 * Setup the RF port.
 * Note:
 * val
 * 0	(RX1A_N &  RX1A_P) and (RX2A_N & RX2A_P) enabled; balanced
 * 1	(RX1B_N &  RX1B_P) and (RX2B_N & RX2B_P) enabled; balanced
 * 2	(RX1C_N &  RX1C_P) and (RX2C_N & RX2C_P) enabled; balanced
 *
 * 3	RX1A_N and RX2A_N enabled; unbalanced
 * 4	RX1A_P and RX2A_P enabled; unbalanced
 * 5	RX1B_N and RX2B_N enabled; unbalanced
 * 6	RX1B_P and RX2B_P enabled; unbalanced
 * 7	RX1C_N and RX2C_N enabled; unbalanced
 * 8	RX1C_P and RX2C_P enabled; unbalanced
 * 9 TX_MON1
 * 10 TX_MON2
 * 11 TX_MON1 & TX_MON2
 * @param phy The AD9361 state structure.
 * @param rx_inputs RX input option identifier
 * @param txb TX output option identifier
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_rf_port_setup(ad9361_rf_phy_t* phy, //
                             bool             is_out,
                             uint32_t         rx_inputs,
                             uint32_t         txb) {

    uint32_t val;

    if (rx_inputs > 11) {
        return -EINVAL;
    }

    if (!is_out) {
        if (rx_inputs > 8) {
            return ad9361_txmon_control(phy, rx_inputs & (TX_1 | TX_2));
        }
        else {
            ad9361_txmon_control(phy, 0);
        }
    }

    if (rx_inputs < 3) {
        val = (3 << (rx_inputs * 2));
    }
    else {
        val = (1 << (rx_inputs - 3));
    }

    if (txb) {
        val |= TX_OUTPUT; // Select TX1B, TX2B
    }

    LOG_FORMAT(debug, "INPUT_SELECT 0x%" PRIx32 " (%s)", val, __func__);

    SPI_SDR_Write(phy->id_no, REG_INPUT_SELECT, val);

    return 0;
}

/**
 * Setup the Parallel Port (Digital Data Interface).
 * @param phy The AD9361 state structure.
 * @param restore_c3 Set true, will restore the Parallel Port Configuration 3
 *                   register.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_pp_port_setup(ad9361_rf_phy_t* phy, //
                             bool             restore_c3) {

    ad9361_phy_platform_data_t* pd = &(phy->pdata);

    LOG_FORMAT(debug, "Start (%s)", __func__);

    if (restore_c3) {
        SPI_SDR_Write(phy->id_no, REG_PARALLEL_PORT_CONF_3, pd->port_ctrl.pp_conf[2]);
        return 0;
    }

    // Sanity check
    if (pd->port_ctrl.pp_conf[2] & LVDS_MODE) {
        pd->port_ctrl.pp_conf[2] &= ~(HALF_DUPLEX_MODE | SINGLE_DATA_RATE | SINGLE_PORT_MODE);
    }

    if (pd->port_ctrl.pp_conf[2] & FULL_PORT) {
        pd->port_ctrl.pp_conf[2] &= ~(HALF_DUPLEX_MODE | SINGLE_PORT_MODE);
    }

    SPI_SDR_Write(phy->id_no, REG_PARALLEL_PORT_CONF_1, pd->port_ctrl.pp_conf[0]);
    SPI_SDR_Write(phy->id_no, REG_PARALLEL_PORT_CONF_2, pd->port_ctrl.pp_conf[1]);
    SPI_SDR_Write(phy->id_no, REG_PARALLEL_PORT_CONF_3, pd->port_ctrl.pp_conf[2]);
    SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY, pd->port_ctrl.rx_clk_data_delay);
    SPI_SDR_Write(phy->id_no, REG_TX_CLOCK_DATA_DELAY, pd->port_ctrl.tx_clk_data_delay);

    SPI_SDR_Write(phy->id_no, REG_LVDS_BIAS_CTRL, pd->port_ctrl.lvds_bias_ctrl);
    SPI_SDR_Write(phy->id_no, REG_LVDS_INVERT_CTRL1, pd->port_ctrl.lvds_invert[0]);
    SPI_SDR_Write(phy->id_no, REG_LVDS_INVERT_CTRL2, pd->port_ctrl.lvds_invert[1]);

    if (pd->rx1rx2_phase_inversion_en || (pd->port_ctrl.pp_conf[1] & INVERT_RX2)) {
        SPI_SDR_WriteF(phy->id_no, REG_PARALLEL_PORT_CONF_2, INVERT_RX2, 1);
        SPI_SDR_WriteF(phy->id_no, REG_INVERT_BITS, INVERT_RX2_RF_DC_CGOUT_WORD, 0);
    }

    return 0;
}

/**
 * Setup the Gain Control Blocks (common function for MGC, AGC modes)
 * @param phy The AD9361 state structure.
 * @param ctrl The gain control settings.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_gc_setup(ad9361_rf_phy_t* phy, //
                        gain_control_t*  ctrl) {

    uint32_t reg, tmp1, tmp2;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    reg = DEC_PWR_FOR_GAIN_LOCK_EXIT | DEC_PWR_FOR_LOCK_LEVEL | DEC_PWR_FOR_LOW_PWR;

    if ((ctrl->rx1_mode == RF_GAIN_HYBRID_AGC) || (ctrl->rx2_mode == RF_GAIN_HYBRID_AGC)) {
        reg |= SLOW_ATTACK_HYBRID_MODE;
    }

    reg |= RX1_GAIN_CTRL_SETUP(ctrl->rx1_mode) | RX2_GAIN_CTRL_SETUP(ctrl->rx2_mode);

    phy->agc_mode[0] = ctrl->rx1_mode;
    phy->agc_mode[1] = ctrl->rx2_mode;

    SPI_SDR_Write(phy->id_no, REG_AGC_CONFIG_1, reg); // Gain Control Mode Select

    // AGC_USE_FULL_GAIN_TABLE handled in ad9361_load_gt()
    SPI_SDR_WriteF(phy->id_no, REG_AGC_CONFIG_2, MAN_GAIN_CTRL_RX1, ctrl->mgc_rx1_ctrl_inp_en);
    SPI_SDR_WriteF(phy->id_no, REG_AGC_CONFIG_2, MAN_GAIN_CTRL_RX2, ctrl->mgc_rx2_ctrl_inp_en);
    SPI_SDR_WriteF(phy->id_no, REG_AGC_CONFIG_2, DIG_GAIN_EN, ctrl->dig_gain_en);

    ctrl->adc_ovr_sample_size = clamp_t(uint8_t, ctrl->adc_ovr_sample_size, 1U, 8U);

    reg = ADC_OVERRANGE_SAMPLE_SIZE(ctrl->adc_ovr_sample_size - 1);

    if (phy->pdata.split_gt && (ctrl->mgc_rx1_ctrl_inp_en || ctrl->mgc_rx2_ctrl_inp_en)) {
        switch (ctrl->mgc_split_table_ctrl_inp_gain_mode) {
            case 1:
                reg &= ~INCDEC_LMT_GAIN;
                break;

            case 2:
                reg |= INCDEC_LMT_GAIN;
                break;

            default:
            case 0:
                reg |= USE_AGC_FOR_LMTLPF_GAIN;
                break;
        }
    }

    ctrl->mgc_inc_gain_step = clamp_t(uint8_t, ctrl->mgc_inc_gain_step, 1U, 8U);

    reg |= MANUAL_INCR_STEP_SIZE(ctrl->mgc_inc_gain_step - 1);

    SPI_SDR_Write(phy->id_no, REG_AGC_CONFIG_3, reg); // Incr Step Size, ADC Overrange Size

    if (phy->pdata.split_gt) {
        reg = SIZE_SPLIT_TABLE - 1;
    }
    else {
        reg = SIZE_FULL__TABLE - 1;
    }

    SPI_SDR_Write(phy->id_no, REG_MAX_LMT_FULL_GAIN, reg);        // Max Full/LMT Gain Table Index
    SPI_SDR_Write(phy->id_no, REG_RX1_MANUAL_LMT_FULL_GAIN, reg); // Rx1 Full/LMT Gain Index
    SPI_SDR_Write(phy->id_no, REG_RX2_MANUAL_LMT_FULL_GAIN, reg); // Rx2 Full/LMT Gain Index

    ctrl->mgc_dec_gain_step = clamp_t(uint8_t, ctrl->mgc_dec_gain_step, 1U, 8U);

    reg = MANUAL_CTRL_IN_DECR_GAIN_STP_SIZE(ctrl->mgc_dec_gain_step);

    SPI_SDR_Write(phy->id_no, REG_PEAK_WAIT_TIME, reg); // Decr Step Size, Peak Overload Time

    if (ctrl->dig_gain_en) {
        SPI_SDR_Write(phy->id_no, REG_DIGITAL_GAIN, MAXIMUM_DIGITAL_GAIN(ctrl->max_dig_gain) | DIG_GAIN_STP_SIZE(ctrl->dig_gain_step_size));
    }

    if (ctrl->adc_large_overload_thresh >= ctrl->adc_small_overload_thresh) {
        SPI_SDR_Write(phy->id_no, REG_ADC_SMALL_OVERLOAD_THRESH, ctrl->adc_small_overload_thresh); // ADC Small Overload Threshold
        SPI_SDR_Write(phy->id_no, REG_ADC_LARGE_OVERLOAD_THRESH, ctrl->adc_large_overload_thresh); // ADC Large Overload Threshold
    }
    else {
        SPI_SDR_Write(phy->id_no, REG_ADC_SMALL_OVERLOAD_THRESH, ctrl->adc_large_overload_thresh); // ADC Small Overload Threshold
        SPI_SDR_Write(phy->id_no, REG_ADC_LARGE_OVERLOAD_THRESH, ctrl->adc_small_overload_thresh); // ADC Large Overload Threshold
    }

    reg = (ctrl->lmt_overload_high_thresh / 16) - 1;
    reg = clamp(reg, 0U, 63U);

    SPI_SDR_Write(phy->id_no, REG_LARGE_LMT_OVERLOAD_THRESH, reg);

    reg = (ctrl->lmt_overload_low_thresh / 16) - 1;
    reg = clamp(reg, 0U, 63U);

    SPI_SDR_WriteF(phy->id_no, REG_SMALL_LMT_OVERLOAD_THRESH, SMALL_LMT_OVERLOAD_THRESH(~0), reg);

    if (phy->pdata.split_gt) {
        // REVIST
        SPI_SDR_Write(phy->id_no, REG_RX1_MANUAL_LPF_GAIN, 0x58);         // Rx1 LPF Gain Index
        SPI_SDR_Write(phy->id_no, REG_RX2_MANUAL_LPF_GAIN, 0x18);         // Rx2 LPF Gain Index
        SPI_SDR_Write(phy->id_no, REG_FAST_INITIAL_LMT_GAIN_LIMIT, 0x27); // Initial LMT Gain Limit
    }

    SPI_SDR_Write(phy->id_no, REG_RX1_MANUAL_DIGITALFORCED_GAIN, 0x00); // Rx1 Digital Gain Index
    SPI_SDR_Write(phy->id_no, REG_RX2_MANUAL_DIGITALFORCED_GAIN, 0x00); // Rx2 Digital Gain Index

    reg = clamp_t(uint8_t, ctrl->low_power_thresh, 0U, 64U) * 2;

    SPI_SDR_Write(phy->id_no, REG_FAST_LOW_POWER_THRESH, reg);   // Low Power Threshold
    SPI_SDR_Write(phy->id_no, REG_TX_SYMBOL_ATTEN_CONFIG, 0x00); // Tx Symbol Gain Control

    SPI_SDR_WriteF(phy->id_no, REG_DEC_POWER_MEASURE_DURATION_0, USE_HB1_OUT_FOR_DEC_PWR_MEAS, 1); // Power Measurement Duration

    SPI_SDR_WriteF(phy->id_no, REG_DEC_POWER_MEASURE_DURATION_0, ENABLE_DEC_PWR_MEAS, 1); // Power Measurement Duration

    if ((ctrl->rx1_mode == RF_GAIN_FASTATTACK_AGC) || (ctrl->rx2_mode == RF_GAIN_FASTATTACK_AGC)) {
        reg = int_log2(ctrl->f_agc_dec_pow_measuremnt_duration / 16);
    }
    else {
        reg = int_log2(ctrl->dec_pow_measuremnt_duration / 16);
    }

    SPI_SDR_WriteF(phy->id_no, REG_DEC_POWER_MEASURE_DURATION_0, DEC_POWER_MEASUREMENT_DURATION(~0), reg); // Power Measurement Duration

    // AGC

    tmp1 = reg = clamp_t(uint8_t, ctrl->agc_inner_thresh_high, 0U, 127U);

    SPI_SDR_WriteF(phy->id_no, REG_AGC_LOCK_LEVEL, AGC_LOCK_LEVEL_FAST_AGC_INNER_HIGH_THRESH_SLOW(~0), reg);

    tmp2 = reg = clamp_t(uint8_t, ctrl->agc_inner_thresh_low, 0U, 127U);

    reg |= (ctrl->adc_lmt_small_overload_prevent_gain_inc ? PREVENT_GAIN_INC : 0);

    SPI_SDR_Write(phy->id_no, REG_AGC_INNER_LOW_THRESH, reg);

    reg = AGC_OUTER_HIGH_THRESH(tmp1 - ctrl->agc_outer_thresh_high) | AGC_OUTER_LOW_THRESH(ctrl->agc_outer_thresh_low - tmp2);

    SPI_SDR_Write(phy->id_no, REG_OUTER_POWER_THRESHS, reg);

    reg = AGC_OUTER_HIGH_THRESH_EXED_STP_SIZE(ctrl->agc_outer_thresh_high_dec_steps) | AGC_OUTER_LOW_THRESH_EXED_STP_SIZE(ctrl->agc_outer_thresh_low_inc_steps);

    SPI_SDR_Write(phy->id_no, REG_GAIN_STP_2, reg);

    reg = ((ctrl->immed_gain_change_if_large_adc_overload) ? IMMED_GAIN_CHANGE_IF_LG_ADC_OVERLOAD : 0) | ((ctrl->immed_gain_change_if_large_lmt_overload) ? IMMED_GAIN_CHANGE_IF_LG_LMT_OVERLOAD : 0) |
          AGC_INNER_HIGH_THRESH_EXED_STP_SIZE(ctrl->agc_inner_thresh_high_dec_steps) | AGC_INNER_LOW_THRESH_EXED_STP_SIZE(ctrl->agc_inner_thresh_low_inc_steps);

    SPI_SDR_Write(phy->id_no, REG_GAIN_STP1, reg);

    reg = LARGE_ADC_OVERLOAD_EXED_COUNTER(ctrl->adc_large_overload_exceed_counter) | SMALL_ADC_OVERLOAD_EXED_COUNTER(ctrl->adc_small_overload_exceed_counter);

    SPI_SDR_Write(phy->id_no, REG_ADC_OVERLOAD_COUNTERS, reg);

    SPI_SDR_WriteF(phy->id_no, REG_GAIN_STP_CONFIG_2, LARGE_LPF_GAIN_STEP(~0), LARGE_LPF_GAIN_STEP(ctrl->adc_large_overload_inc_steps));

    reg = LARGE_LMT_OVERLOAD_EXED_COUNTER(ctrl->lmt_overload_large_exceed_counter) | SMALL_LMT_OVERLOAD_EXED_COUNTER(ctrl->lmt_overload_small_exceed_counter);

    SPI_SDR_Write(phy->id_no, REG_LMT_OVERLOAD_COUNTERS, reg);

    SPI_SDR_WriteF(phy->id_no, REG_GAIN_STP_CONFIG1, DEC_STP_SIZE_FOR_LARGE_LMT_OVERLOAD(~0), ctrl->lmt_overload_large_inc_steps);

    reg = DIG_SATURATION_EXED_COUNTER(ctrl->dig_saturation_exceed_counter) | (ctrl->sync_for_gain_counter_en ? ENABLE_SYNC_FOR_GAIN_COUNTER : 0);

    SPI_SDR_Write(phy->id_no, REG_DIGITAL_SAT_COUNTER, reg);

    // Fast AGC

    // Fast AGC - Low Power
    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, ENABLE_INCR_GAIN, ctrl->f_agc_allow_agc_gain_increase);

    SPI_SDR_Write(phy->id_no, REG_FAST_INCREMENT_TIME, ctrl->f_agc_lp_thresh_increment_time);

    reg = ctrl->f_agc_lp_thresh_increment_steps - 1;
    reg = clamp_t(uint32_t, reg, 0U, 7U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_ENERGY_DETECT_COUNT, INCREMENT_GAIN_STP_LPFLMT(~0), reg);

    // Fast AGC - Lock Level
    // Dual use see also agc_inner_thresh_high
    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, ENABLE_LMT_GAIN_INC_FOR_LOCK_LEVEL, ctrl->f_agc_lock_level_lmt_gain_increase_en);

    reg = ctrl->f_agc_lock_level_gain_increase_upper_limit;
    reg = clamp_t(uint32_t, reg, 0U, 63U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_AGCLL_UPPER_LIMIT, AGCLL_MAX_INCREASE(~0), reg);

    // Fast AGC - Peak Detectors and Final Settling
    reg = ctrl->f_agc_lpf_final_settling_steps;
    reg = clamp_t(uint32_t, reg, 0U, 3U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_ENERGY_LOST_THRESH, POST_LOCK_LEVEL_STP_SIZE_FOR_LPF_TABLE_FULL_TABLE(~0), reg);

    reg = ctrl->f_agc_lmt_final_settling_steps;
    reg = clamp_t(uint32_t, reg, 0U, 3U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_STRONGER_SIGNAL_THRESH, POST_LOCK_LEVEL_STP_FOR_LMT_TABLE(~0), reg);

    reg = ctrl->f_agc_final_overrange_count;
    reg = clamp_t(uint32_t, reg, 0U, 7U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_FINAL_OVER_RANGE_AND_OPT_GAIN, FINAL_OVER_RANGE_COUNT(~0), reg);

    // Fast AGC - Final Power Test
    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, ENABLE_GAIN_INC_AFTER_GAIN_LOCK, ctrl->f_agc_gain_increase_after_gain_lock_en);

    // Fast AGC - Unlocking the Gain
    // 0 = MAX Gain, 1 = Optimized Gain, 2 = Set Gain

    reg = ctrl->f_agc_gain_index_type_after_exit_rx_mode;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EXIT_RX_STATE, reg == SET_GAIN);
    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_OPTIMIZED_GAIN_IF_EXIT_RX_STATE, reg == OPTIMIZED_GAIN);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, USE_LAST_LOCK_LEVEL_FOR_SET_GAIN, ctrl->f_agc_use_last_lock_level_for_set_gain_en);

    reg = ctrl->f_agc_optimized_gain_offset;
    reg = clamp_t(uint32_t, reg, 0U, 15U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_FINAL_OVER_RANGE_AND_OPT_GAIN, OPTIMIZE_GAIN_OFFSET(~0), reg);

    tmp1 = !ctrl->f_agc_rst_gla_stronger_sig_thresh_exceeded_en || !ctrl->f_agc_rst_gla_engergy_lost_sig_thresh_exceeded_en || !ctrl->f_agc_rst_gla_large_adc_overload_en || !ctrl->f_agc_rst_gla_large_lmt_overload_en ||
           ctrl->f_agc_rst_gla_en_agc_pulled_high_en;

    SPI_SDR_WriteF(phy->id_no, REG_AGC_CONFIG_2, AGC_GAIN_UNLOCK_CTRL, tmp1);

    reg = !ctrl->f_agc_rst_gla_stronger_sig_thresh_exceeded_en;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_STRONG_SIGNAL_FREEZE, DONT_UNLOCK_GAIN_IF_STRONGER_SIGNAL, reg);

    reg = ctrl->f_agc_rst_gla_stronger_sig_thresh_above_ll;
    reg = clamp_t(uint32_t, reg, 0U, 63U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_STRONGER_SIGNAL_THRESH, STRONGER_SIGNAL_THRESH(~0), reg);

    reg = ctrl->f_agc_rst_gla_engergy_lost_sig_thresh_below_ll;
    reg = clamp_t(uint32_t, reg, 0U, 63U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_ENERGY_LOST_THRESH, ENERGY_LOST_THRESH(~0), reg);

    reg = ctrl->f_agc_rst_gla_engergy_lost_goto_optim_gain_en;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_OPT_GAIN_IF_ENERGY_LOST_OR_EN_AGC_HIGH, reg);

    reg = !ctrl->f_agc_rst_gla_engergy_lost_sig_thresh_exceeded_en;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, DONT_UNLOCK_GAIN_IF_ENERGY_LOST, reg);

    reg = ctrl->f_agc_energy_lost_stronger_sig_gain_lock_exit_cnt;
    reg = clamp_t(uint32_t, reg, 0U, 63U);

    SPI_SDR_WriteF(phy->id_no, REG_FAST_GAIN_LOCK_EXIT_COUNT, GAIN_LOCK_EXIT_COUNT(~0), reg);

    reg = !ctrl->f_agc_rst_gla_large_adc_overload_en || !ctrl->f_agc_rst_gla_large_lmt_overload_en;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, DONT_UNLOCK_GAIN_IF_LG_ADC_OR_LMT_OVRG, reg);

    reg = !ctrl->f_agc_rst_gla_large_adc_overload_en;

    SPI_SDR_WriteF(phy->id_no, REG_FAST_LOW_POWER_THRESH, DONT_UNLOCK_GAIN_IF_ADC_OVRG, reg);

    // 0 = Max Gain, 1 = Set Gain, 2 = Optimized Gain, 3 = No Gain Change

    if (ctrl->f_agc_rst_gla_en_agc_pulled_high_en) {
        switch (ctrl->f_agc_rst_gla_if_en_agc_pulled_high_mode) {
            case MAX_GAIN:
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, GOTO_MAX_GAIN_OR_OPT_GAIN_IF_EN_AGC_HIGH, 1);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EN_AGC_HIGH, 0);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_OPT_GAIN_IF_ENERGY_LOST_OR_EN_AGC_HIGH, 0);
                break;

            case SET_GAIN:
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, GOTO_MAX_GAIN_OR_OPT_GAIN_IF_EN_AGC_HIGH, 0);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EN_AGC_HIGH, 1);
                break;

            case OPTIMIZED_GAIN:
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, GOTO_MAX_GAIN_OR_OPT_GAIN_IF_EN_AGC_HIGH, 1);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EN_AGC_HIGH, 0);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_OPT_GAIN_IF_ENERGY_LOST_OR_EN_AGC_HIGH, 1);
                break;

            case NO_GAIN_CHANGE:
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EN_AGC_HIGH, 0);
                SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, GOTO_MAX_GAIN_OR_OPT_GAIN_IF_EN_AGC_HIGH, 0);
                break;

            default:
                LOG_FORMAT(error, "Wrong AGC pulled high mode (%s)", __func__);
                break;
        }
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_1, GOTO_SET_GAIN_IF_EN_AGC_HIGH, 0);
        SPI_SDR_WriteF(phy->id_no, REG_FAST_CONFIG_2_SETTLING_DELAY, GOTO_MAX_GAIN_OR_OPT_GAIN_IF_EN_AGC_HIGH, 0);
    }

    reg = int_log2(ctrl->f_agc_power_measurement_duration_in_state5 / 16);
    reg = clamp_t(uint32_t, reg, 0U, 15U);

    SPI_SDR_WriteF(phy->id_no, REG_RX1_MANUAL_LPF_GAIN, POWER_MEAS_IN_STATE_5(~0), reg);
    SPI_SDR_WriteF(phy->id_no, REG_RX1_MANUAL_LMT_FULL_GAIN, POWER_MEAS_IN_STATE_5_MSB, (reg >> 3));

    return ad9361_gc_update(phy);
}

/**
 * Set the Aux DAC.
 * @param phy The AD9361 state structure.
 * @param dac The DAC.
 * @param val_mV The value.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_auxdac_set(ad9361_rf_phy_t* phy, //
                          int32_t          dac,
                          int32_t          val_mV) {

    uint32_t val, tmp;

    LOG_FORMAT(debug, "DAC%" PRId32 " %" PRId32 " mV (%s)", dac, val_mV, __func__);

    SPI_SDR_WriteF(phy->id_no, REG_AUXDAC_ENABLE_CTRL, AUXDAC_MANUAL_BAR(dac), val_mV ? 0 : 1);

    if (val_mV < 306) {
        val_mV = 306;
    }

    if (val_mV < 1888) {
        val = ((val_mV - 306) * 1000) / 1404; // Vref = 1V, Step = 2
        tmp = AUXDAC_1_VREF(0);
    }
    else {
        val = ((val_mV - 1761) * 1000) / 1836; // Vref = 2.5V, Step = 2
        tmp = AUXDAC_1_VREF(3);
    }

    val = clamp_t(uint32_t, val, 0, 1023);

    switch (dac) {
        case 1:
            SPI_SDR_Write(phy->id_no, REG_AUXDAC_1_WORD, (val >> 2));
            SPI_SDR_Write(phy->id_no, REG_AUXDAC_1_CONFIG, AUXDAC_1_WORD_LSB(val) | tmp);
            phy->auxdac1_value = val_mV;
            break;

        case 2:
            SPI_SDR_Write(phy->id_no, REG_AUXDAC_2_WORD, (val >> 2));
            SPI_SDR_Write(phy->id_no, REG_AUXDAC_2_CONFIG, AUXDAC_2_WORD_LSB(val) | tmp);
            phy->auxdac2_value = val_mV;
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/**
 * Get the Aux DAC value.
 * @param phy The AD9361 state structure.
 * @param dac The DAC.
 * @return The value in case of success, negative error code otherwise.
 */
int32_t ad9361_auxdac_get(ad9361_rf_phy_t* phy, //
                          int32_t          dac) {

    switch (dac) {
        case 1:
            return phy->auxdac1_value;

        case 2:
            return phy->auxdac2_value;

        default:
            return -EINVAL;
    }

    return 0;
}

/**
 * Setup the AuxDAC.
 * @param phy The AD9361 state structure.
 * @param ctrl Pointer to auxdac_control structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_auxdac_setup(ad9361_rf_phy_t*  phy, //
                            auxdac_control_t* ctrl) {

    uint8_t tmp;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    ad9361_auxdac_set(phy, 1, ctrl->dac1_default_value);
    ad9361_auxdac_set(phy, 2, ctrl->dac2_default_value);

    tmp = ~(AUXDAC_AUTO_TX_BAR(ctrl->dac2_in_tx_en << 1 | ctrl->dac1_in_tx_en) | AUXDAC_AUTO_RX_BAR(ctrl->dac2_in_rx_en << 1 | ctrl->dac1_in_rx_en) | AUXDAC_INIT_BAR(ctrl->dac2_in_alert_en << 1 | ctrl->dac1_in_alert_en));

    SPI_SDR_WriteF(phy->id_no, REG_AUXDAC_ENABLE_CTRL, AUXDAC_AUTO_TX_BAR(~0) | AUXDAC_AUTO_RX_BAR(~0) | AUXDAC_INIT_BAR(~0), tmp); // Auto Control

    SPI_SDR_WriteF(phy->id_no, REG_EXTERNAL_LNA_CTRL, AUXDAC_MANUAL_SELECT, ctrl->auxdac_manual_mode_en);
    SPI_SDR_Write(phy->id_no, REG_AUXDAC1_RX_DELAY, ctrl->dac1_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_AUXDAC1_TX_DELAY, ctrl->dac1_tx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_AUXDAC2_RX_DELAY, ctrl->dac2_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_AUXDAC2_TX_DELAY, ctrl->dac2_tx_delay_us);

    return 0;
}

/**
 * Setup the AuxADC.
 * @param phy The AD9361 state structure.
 * @param ctrl The AuxADC settings.
 * @param bbpll_freq The BBPLL frequency [Hz].
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_auxadc_setup(ad9361_rf_phy_t*  phy, //
                            auxadc_control_t* ctrl,
                            uint32_t          bbpll_freq) {

    uint32_t val;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    val = DIV_ROUND_CLOSEST(ctrl->temp_time_inteval_ms * (bbpll_freq / 1000UL), (1 << 29));

    SPI_SDR_Write(phy->id_no, REG_TEMP_OFFSET, ctrl->offset);
    SPI_SDR_Write(phy->id_no, REG_START_TEMP_READING, 0x00);
    SPI_SDR_Write(phy->id_no, REG_TEMP_SENSE2, MEASUREMENT_TIME_INTERVAL(val) | (ctrl->periodic_temp_measuremnt ? TEMP_SENSE_PERIODIC_ENABLE : 0));
    SPI_SDR_Write(phy->id_no, REG_TEMP_SENSOR_CONFIG, TEMP_SENSOR_DECIMATION(int_log2(ctrl->temp_sensor_decimation) - 8));
    SPI_SDR_Write(phy->id_no, REG_AUXADC_CLOCK_DIVIDER, bbpll_freq / ctrl->auxadc_clock_rate);
    SPI_SDR_Write(phy->id_no, REG_AUXADC_CONFIG, AUX_ADC_DECIMATION(int_log2(ctrl->auxadc_decimation) - 8));

    return 0;
}

/**
 * Setup the Control Output pins.
 * @param phy The AD9361 state structure.
 * @param ctrl The Control Output pins settings.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_ctrl_outs_setup(ad9361_rf_phy_t*     phy, //
                               ctrl_outs_control_t* ctrl) {

    LOG_FORMAT(debug, "Start (%s)", __func__);

    SPI_SDR_Write(phy->id_no, REG_CTRL_OUTPUT_POINTER, ctrl->index);  // Ctrl Out index
    SPI_SDR_Write(phy->id_no, REG_CTRL_OUTPUT_ENABLE, ctrl->en_mask); // Ctrl Out [7:0] output enable

    return 0;
}

/**
 * Setup the GPO pins.
 * @param phy The AD9361 state structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_gpo_setup(ad9361_rf_phy_t* phy, //
                         gpo_control_t*   ctrl) {

    LOG_FORMAT(debug, "Start (%s)", __func__);

    SPI_SDR_Write(phy->id_no,
                  REG_AUTO_GPO,                                          //
                  GPO_ENABLE_AUTO_RX(ctrl->gpo0_slave_rx_en |            //
                                     (ctrl->gpo1_slave_rx_en << 1) |     //
                                     (ctrl->gpo2_slave_rx_en << 2) |     //
                                     (ctrl->gpo3_slave_rx_en << 3)) |    //
                      GPO_ENABLE_AUTO_TX(ctrl->gpo0_slave_tx_en |        //
                                         (ctrl->gpo1_slave_tx_en << 1) | //
                                         (ctrl->gpo2_slave_tx_en << 2) | //
                                         (ctrl->gpo3_slave_tx_en << 3)));

    SPI_SDR_Write(phy->id_no,
                  REG_GPO_FORCE_AND_INIT,                                   //
                  GPO_INIT_STATE(ctrl->gpo0_inactive_state_high_en |        //
                                 (ctrl->gpo1_inactive_state_high_en << 1) | //
                                 (ctrl->gpo2_inactive_state_high_en << 2) | //
                                 (ctrl->gpo3_inactive_state_high_en << 3)));

    SPI_SDR_Write(phy->id_no, REG_GPO0_RX_DELAY, ctrl->gpo0_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO0_TX_DELAY, ctrl->gpo0_tx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO1_RX_DELAY, ctrl->gpo1_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO1_TX_DELAY, ctrl->gpo1_tx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO2_RX_DELAY, ctrl->gpo2_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO2_TX_DELAY, ctrl->gpo2_tx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO3_RX_DELAY, ctrl->gpo3_rx_delay_us);
    SPI_SDR_Write(phy->id_no, REG_GPO3_TX_DELAY, ctrl->gpo3_tx_delay_us);

    return 0;
}

/**
 * Setup the RSSI.
 * @param phy The AD9361 state structure.
 * @param ctrl The RSSI settings.
 * @param is_update True if update
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_rssi_setup(ad9361_rf_phy_t* phy, //
                          rssi_control_t*  ctrl,
                          bool             is_update) {

    uint32_t total_weight, weight[4], total_dur = 0, temp;
    uint8_t  dur_buf[4] = {0};
    int32_t  val, i, j = 0;
    uint32_t rssi_delay;
    uint32_t rssi_wait;
    uint32_t rssi_duration;
    uint32_t rate;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    if (ctrl->rssi_unit_is_rx_samples) {
        if (is_update) {
            return 0; // no update required
        }

        rssi_delay    = ctrl->rssi_delay;
        rssi_wait     = ctrl->rssi_wait;
        rssi_duration = ctrl->rssi_duration;
    }
    else {
        // update sample based on RX rate
        rate = DIV_ROUND_CLOSEST(clk_get_rate(phy, &(phy->ref_clk_scale[RX_SAMPL_CLK])), 1000);
        // units are in us
        rssi_delay    = DIV_ROUND_CLOSEST(ctrl->rssi_delay * rate, 1000);
        rssi_wait     = DIV_ROUND_CLOSEST(ctrl->rssi_wait * rate, 1000);
        rssi_duration = DIV_ROUND_CLOSEST(ctrl->rssi_duration * rate, 1000);
    }

    if (ctrl->restart_mode == EN_AGC_PIN_IS_PULLED_HIGH) {
        rssi_delay = 0;
    }

    rssi_delay = clamp(rssi_delay / 8, 0U, 255U);
    rssi_wait  = clamp(rssi_wait / 4, 0U, 255U);

    do {
        for (i = 14; (rssi_duration > 0) && (i >= 0); i--) {
            val = 1 << i;

            if ((int64_t)rssi_duration >= val) {
                dur_buf[j++] = i;
                total_dur += val;
                rssi_duration -= val;
                break;
            }
        }
    }
    while ((j < 4) && (rssi_duration > 0));

    for (i = 0, total_weight = 0; i < 4; i++) {
        total_weight += weight[i] = DIV_ROUND_CLOSEST(RSSI_MAX_WEIGHT * (1 << dur_buf[i]), total_dur);
    }

    // total of all weights must be 0xFF
    val = total_weight - 0xFF;
    weight[j - 1] -= val;

    SPI_SDR_Write(phy->id_no, REG_MEASURE_DURATION_01, (dur_buf[1] << 4) | dur_buf[0]); // RSSI Measurement Duration 0, 1
    SPI_SDR_Write(phy->id_no, REG_MEASURE_DURATION_23, (dur_buf[3] << 4) | dur_buf[2]); // RSSI Measurement Duration 2, 3
    SPI_SDR_Write(phy->id_no, REG_RSSI_WEIGHT_0, weight[0]);                            // RSSI Weighted Multiplier 0
    SPI_SDR_Write(phy->id_no, REG_RSSI_WEIGHT_1, weight[1]);                            // RSSI Weighted Multiplier 1
    SPI_SDR_Write(phy->id_no, REG_RSSI_WEIGHT_2, weight[2]);                            // RSSI Weighted Multiplier 2
    SPI_SDR_Write(phy->id_no, REG_RSSI_WEIGHT_3, weight[3]);                            // RSSI Weighted Multiplier 3
    SPI_SDR_Write(phy->id_no, REG_RSSI_DELAY, rssi_delay);                              // RSSI Delay
    SPI_SDR_Write(phy->id_no, REG_RSSI_WAIT_TIME, rssi_wait);                           // RSSI Wait

    temp = RSSI_MODE_SELECT(ctrl->restart_mode);

    if (ctrl->restart_mode == SPI_WRITE_TO_REGISTER) {
        temp |= START_RSSI_MEAS;
    }

    SPI_SDR_Write(phy->id_no, REG_RSSI_CONFIG, temp); // RSSI Mode Select

    return 0;
}

/**
 * This function needs to be called whenever BBPLL changes.
 * @param phy The AD9361 state structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_bb_clk_change_handler(ad9361_rf_phy_t* phy) {

    int32_t _val;

    _val = ad9361_gc_update(phy);
    _val |= ad9361_rssi_setup(phy, &phy->pdata.rssi_ctrl, true);
    _val |= ad9361_auxadc_setup(phy, &phy->pdata.auxadc_ctrl, clk_get_rate(phy, &(phy->ref_clk_scale[BBPLL_CLK])));

    return _val;
}

/**
 * Set the desired Enable State Machine (ENSM) state.
 * @param phy The AD9361 state structure.
 * @param ensm_state The ENSM state [ENSM_STATE_SLEEP_WAIT, ENSM_STATE_ALERT,
 *                   ENSM_STATE_TX, ENSM_STATE_TX_FLUSH, ENSM_STATE_RX,
 *                   ENSM_STATE_RX_FLUSH, ENSM_STATE_FDD, ENSM_STATE_FDD_FLUSH].
 * @param pinctrl Set true, will enable the ENSM pin control.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_ensm_set_state(ad9361_rf_phy_t* phy, //
                              uint8_t          ensm_state,
                              bool             pinctrl) {

    int32_t  rc = 0;
    uint32_t val;
    uint32_t tmp;

    if (phy->curr_ensm_state == ensm_state) {
        LOG_FORMAT(warning, "Nothing to do, device is already in 0x%02X state (%s)", ensm_state, __func__);
        goto out;
    }

    LOG_FORMAT(debug, "Device is in 0x%02X state, moving to 0x%02X (%s)", phy->curr_ensm_state, ensm_state, __func__);

    if (phy->curr_ensm_state == ENSM_STATE_SLEEP) {

        SPI_SDR_Write(phy->id_no, REG_CLOCK_ENABLE,
                      DIGITAL_POWER_UP | CLOCK_ENABLE_DFLT | BBPLL_ENABLE | (phy->pdata.use_extclk ? XO_BYPASS : 0)); // Enable Clocks

        STIME_uSleep(20);

        SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, TO_ALERT | FORCE_ALERT_STATE);

        ad9361_trx_vco_cal_control(phy, false, true); // Enable VCO Cal
        ad9361_trx_vco_cal_control(phy, true, true);
    }

    val = (phy->pdata.ensm_pin_pulse_mode ? 0 : LEVEL_MODE) | (pinctrl ? ENABLE_ENSM_PIN_CTRL : 0) | (phy->txmon_tdd_en ? ENABLE_RX_DATA_PORT_FOR_CAL : 0) | TO_ALERT;

    switch (ensm_state) {
        case ENSM_STATE_TX:
            val |= FORCE_TX_ON;

            if (phy->pdata.fdd) {
                rc = -EINVAL;
            }
            else if (phy->curr_ensm_state != ENSM_STATE_ALERT) {
                rc = -EINVAL;
            }
            break;

        case ENSM_STATE_RX:
            val |= FORCE_RX_ON;

            if (phy->pdata.fdd) {
                rc = -EINVAL;
            }
            else if (phy->curr_ensm_state != ENSM_STATE_ALERT) {
                rc = -EINVAL;
            }
            break;

        case ENSM_STATE_FDD:
            val |= FORCE_TX_ON;

            if (!phy->pdata.fdd) {
                rc = -EINVAL;
            }
            break;

        case ENSM_STATE_ALERT:
            val &= ~(FORCE_TX_ON | FORCE_RX_ON);
            val |= TO_ALERT | FORCE_ALERT_STATE;
            break;

        case ENSM_STATE_SLEEP_WAIT:
            break;

        case ENSM_STATE_SLEEP:
            ad9361_trx_vco_cal_control(phy, false, false); // Disable VCO Cal
            ad9361_trx_vco_cal_control(phy, true, false);

            SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, 0); // Clear To Alert
            SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, (phy->pdata.fdd ? FORCE_TX_ON : FORCE_RX_ON));

            // Delay Flush Time 384 ADC clock cycles
            STIME_uSleep(384000000UL / clk_get_rate(phy, &(phy->ref_clk_scale[ADC_CLK])));

            SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, 0); // Move to Wait

            STIME_uSleep(1); // Wait for ENSM settle

            SPI_SDR_Write(phy->id_no, REG_CLOCK_ENABLE, (phy->pdata.use_extclk ? XO_BYPASS : 0)); // Turn off all clocks

            phy->curr_ensm_state = ensm_state;

            return 0;

        default:
            LOG_FORMAT(error, "No handling for forcing %d ensm state (%s)", ensm_state, __func__);
            goto out;
    }

    if (rc) {
        LOG_FORMAT(error, "Invalid ENSM state transition in %s mode (%s)", phy->pdata.fdd ? "FDD" : "TDD", __func__);
        goto out;
    }

    SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_1, val);

    if ((val & FORCE_RX_ON) && ((phy->agc_mode[0] == RF_GAIN_MGC) || (phy->agc_mode[1] == RF_GAIN_MGC))) {
        tmp = SPI_SDR_Read(phy->id_no, REG_SMALL_LMT_OVERLOAD_THRESH);

        SPI_SDR_Write(phy->id_no, REG_SMALL_LMT_OVERLOAD_THRESH, (tmp & SMALL_LMT_OVERLOAD_THRESH(~0)) | (phy->agc_mode[0] == RF_GAIN_MGC ? FORCE_PD_RESET_RX1 : 0) | (phy->agc_mode[1] == RF_GAIN_MGC ? FORCE_PD_RESET_RX2 : 0));

        SPI_SDR_Write(phy->id_no, REG_SMALL_LMT_OVERLOAD_THRESH, tmp & SMALL_LMT_OVERLOAD_THRESH(~0));
    }

    phy->curr_ensm_state = ensm_state;

out:

    return rc;
}

/**
 * Check if at least one of the clock rates is equal to the DATA_CLK (lvds) rate.
 * @param phy The AD9361 state structure.
 * @param rx_path_clks RX path rates buffer.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_validate_trx_clock_chain(ad9361_rf_phy_t* phy, //
                                        uint32_t*        rx_path_clks) {

    int      i;
    uint32_t data_clk;

    data_clk = (phy->pdata.rx2tx2 ? 4 : 2) * rx_path_clks[RX_SAMPL_FREQ];

    for (i = ADC_FREQ; i < RX_SAMPL_CLK; i++) {
        if (abs((int)(rx_path_clks[i] - data_clk)) < 4) {
            return 0;
        }
    }

    LOG_FORMAT(error, "Failed - at least one of the clock rates must be equal to the DATA_CLK (lvds) rate (%s)", __func__);
    return -EINVAL;
}

/**
 * Set the RX and TX path rates.
 * @param phy The AD9361 state structure.
 * @param rx_path_clks RX path rates buffer.
 * @param tx_path_clks TX path rates buffer.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_trx_clock_chain(ad9361_rf_phy_t* phy, //
                                   uint32_t*        rx_path_clks,
                                   uint32_t*        tx_path_clks) {

    int32_t _val, i, j, n;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    if (!rx_path_clks || !tx_path_clks) {
        return -EINVAL;
    }

    LOG_FORMAT(debug,
               "rx_path_clks: %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 " (%s)", //
               rx_path_clks[BBPLL_FREQ],                                                                          //
               rx_path_clks[ADC_FREQ],                                                                            //
               rx_path_clks[R2_FREQ],                                                                             //
               rx_path_clks[R1_FREQ],                                                                             //
               rx_path_clks[CLKRF_FREQ],                                                                          //
               rx_path_clks[RX_SAMPL_FREQ],                                                                       //
               __func__);

    LOG_FORMAT(debug,
               "tx_path_clks: %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 ", %" PRIu32 " (%s)", //
               tx_path_clks[BBPLL_FREQ],                                                                          //
               tx_path_clks[ADC_FREQ],                                                                            //
               tx_path_clks[R2_FREQ],                                                                             //
               tx_path_clks[R1_FREQ],                                                                             //
               tx_path_clks[CLKRF_FREQ],                                                                          //
               tx_path_clks[RX_SAMPL_FREQ],                                                                       //
               __func__);

    _val = ad9361_validate_trx_clock_chain(phy, rx_path_clks);
    if (_val < 0) {
        return _val;
    }

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[BBPLL_CLK]), rx_path_clks[BBPLL_FREQ]);
    if (_val < 0) {
        return _val;
    }

    for (i = ADC_CLK, j = DAC_CLK, n = ADC_FREQ; i <= RX_SAMPL_CLK; i++, j++, n++) {
        _val = clk_set_rate(phy, &(phy->ref_clk_scale[i]), rx_path_clks[n]);
        if (_val < 0) {
            LOG_FORMAT(error, "Failed to set BB ref clock rate %" PRId32 " (%s)", _val, __func__);
            return _val;
        }

        _val = clk_set_rate(phy, &(phy->ref_clk_scale[j]), tx_path_clks[n]);
        if (_val < 0) {
            LOG_FORMAT(error, "Failed to set BB ref clock rate %" PRId32 " (%s)", _val, __func__);
            return _val;
        }
    }

    return ad9361_bb_clk_change_handler(phy);
}

/**
 * Calculate the RX and TX path rates to obtain the desired sample rate.
 * @param phy The AD9361 state structure.
 * @param tx_sample_rate The desired sample rate.
 * @param rate_gov The rate governor option.
 * @param rx_path_clks RX path rates buffer.
 * @param tx_path_clks TX path rates buffer.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_calculate_rf_clock_chain(ad9361_rf_phy_t* phy, //
                                        uint32_t         tx_sample_rate,
                                        uint32_t         rate_gov,
                                        uint32_t*        rx_path_clks,
                                        uint32_t*        tx_path_clks) {

    uint32_t clktf, clkrf, adc_rate = 0, dac_rate = 0;
    uint64_t bbpll_rate;
    int32_t  i, index_rx = -1, index_tx = -1, tmp;
    uint32_t div, tx_intdec, rx_intdec, recursion = 1;

    const int8_t clk_dividers[][4] = {
        {12, 3, 2, 2},
        {8, 2, 2, 2},
        {6, 3, 1, 2},
        {4, 2, 2, 1},
        {3, 3, 1, 1},
        {2, 2, 1, 1},
        {1, 1, 1, 1},
    };

    if (phy->bypass_rx_fir) {
        rx_intdec = 1;
    }
    else {
        rx_intdec = phy->rx_fir_dec;
    }

    if (phy->bypass_tx_fir) {
        tx_intdec = 1;
    }
    else {
        tx_intdec = phy->tx_fir_int;
    }

    if ((rate_gov == 1) && ((rx_intdec * tx_sample_rate * 8) < MIN_ADC_CLK)) {
        recursion = 0;
        rate_gov  = 0;
    }

    LOG_FORMAT(debug,
               "Requested rate %" PRIu32 ", TXFIR int %" PRIu32 ", RXFIR dec %" PRIu32 ", mode %s (%s)", //
               tx_sample_rate,
               tx_intdec,
               rx_intdec,
               (rate_gov ? "Nominal" : "Highest OSR"),
               __func__);

    if (tx_sample_rate > (phy->pdata.rx2tx2 ? 61440000UL : 122880000UL)) {
        return -EINVAL;
    }

    clktf = tx_sample_rate * tx_intdec;
    clkrf = tx_sample_rate * rx_intdec * (phy->rx_eq_2tx ? 2 : 1);

    for (i = rate_gov; i < 7; i++) {
        adc_rate = clkrf * clk_dividers[i][0];
        dac_rate = clktf * clk_dividers[i][0];

        if ((adc_rate <= MAX_ADC_CLK) && (adc_rate >= MIN_ADC_CLK)) {
            if (dac_rate > adc_rate) {
                tmp = (dac_rate / adc_rate) * -1;
            }
            else {
                tmp = adc_rate / dac_rate;
            }

            if (adc_rate <= MAX_DAC_CLK) {
                index_rx = i;
                index_tx = i - ((tmp == 1) ? 0 : tmp);

                dac_rate = adc_rate; // ADC_CLK
                break;
            }
            else {
                dac_rate = adc_rate / 2; // ADC_CLK/2

                index_rx = i;

                if ((i == 4) && (tmp >= 0)) {
                    index_tx = 7; // STOP: 3/2 != 1
                }
                else {
                    index_tx = i + (((i == 5) && (tmp >= 0)) ? 1 : 2) - ((tmp == 1) ? 0 : tmp);
                }
                break;
            }
        }
    }

    if (((index_tx < 0) || (index_tx > 6) || (index_rx < 0) || (index_rx > 6)) && (rate_gov < 7) && recursion) {
        return ad9361_calculate_rf_clock_chain(phy, tx_sample_rate, ++rate_gov, rx_path_clks, tx_path_clks);
    }
    else if (((index_tx < 0) || (index_tx > 6) || (index_rx < 0) || (index_rx > 6))) {
        LOG_FORMAT(error,
                   "Failed to find suitable dividers %s (%s)", //
                   (adc_rate < MIN_ADC_CLK) ? "ADC clock below limit" : "BBPLL rate above limit",
                   __func__);
        return -EINVAL;
    }

    // Calculate target BBPLL rate
    div = MAX_BBPLL_DIV;

    do {
        bbpll_rate = (uint64_t)adc_rate * div;
        div >>= 1;
    }
    while ((bbpll_rate > MAX_BBPLL_FREQ) && (div >= MIN_BBPLL_DIV));

    rx_path_clks[BBPLL_FREQ]    = bbpll_rate;
    rx_path_clks[ADC_FREQ]      = adc_rate;
    rx_path_clks[R2_FREQ]       = rx_path_clks[ADC_FREQ] / clk_dividers[index_rx][1];
    rx_path_clks[R1_FREQ]       = rx_path_clks[R2_FREQ] / clk_dividers[index_rx][2];
    rx_path_clks[CLKRF_FREQ]    = rx_path_clks[R1_FREQ] / clk_dividers[index_rx][3];
    rx_path_clks[RX_SAMPL_FREQ] = rx_path_clks[CLKRF_FREQ] / rx_intdec;

    tx_path_clks[BBPLL_FREQ]    = bbpll_rate;
    tx_path_clks[DAC_FREQ]      = dac_rate;
    tx_path_clks[T2_FREQ]       = tx_path_clks[DAC_FREQ] / clk_dividers[index_tx][1];
    tx_path_clks[T1_FREQ]       = tx_path_clks[T2_FREQ] / clk_dividers[index_tx][2];
    tx_path_clks[CLKTF_FREQ]    = tx_path_clks[T1_FREQ] / clk_dividers[index_tx][3];
    tx_path_clks[TX_SAMPL_FREQ] = tx_path_clks[CLKTF_FREQ] / tx_intdec;

    return 0;
}

/**
 * Set the desired sample rate.
 * @param phy The AD9361 state structure.
 * @param freq The desired sample rate.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_trx_clock_chain_freq(ad9361_rf_phy_t* phy, //
                                        uint32_t         freq) {

    uint32_t rx[6], tx[6];
    int32_t  _val;

    _val = ad9361_calculate_rf_clock_chain(phy, freq, phy->rate_governor, rx, tx);
    if (_val < 0) {
        return _val;
    }

    return ad9361_set_trx_clock_chain(phy, rx, tx);
}

/**
 * Internal ENSM mode options helper function.
 * @param phy The AD9361 state structure.
 * @param fdd
 * @param pinctrl
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_ensm_mode(ad9361_rf_phy_t* phy, //
                             bool             fdd,
                             bool             pinctrl) {

    ad9361_phy_platform_data_t* pd  = &(phy->pdata);
    uint32_t                    val = 0;

    SPI_SDR_Write(phy->id_no, REG_ENSM_MODE, fdd ? FDD_MODE : 0);

    if (pd->use_ext_rx_lo) {
        val |= POWER_DOWN_RX_SYNTH;
    }

    if (pd->use_ext_tx_lo) {
        val |= POWER_DOWN_TX_SYNTH;
    }

    if (fdd) {
        SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_2, val | DUAL_SYNTH_MODE | (pd->fdd_independent_mode ? FDD_EXTERNAL_CTRL_ENABLE : 0));
    }
    else {
        SPI_SDR_Write(phy->id_no, REG_ENSM_CONFIG_2, val | (pd->tdd_use_dual_synth ? DUAL_SYNTH_MODE : 0) | (pd->tdd_use_dual_synth ? 0 : (pinctrl ? SYNTH_ENABLE_PIN_CTRL_MODE : TXNRX_SPI_CTRL)));
    }

    return 0;
}

/**
 * Fastlock read value.
 * @param spi - replaced
 * @param id_no AD9361 module identification number
 * @param tx
 * @param profile
 * @param word
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_fastlock_readval(uint8_t  id_no, //
                                bool     tx,
                                uint32_t profile,
                                uint32_t word) {

    uint32_t offs = 0;

    if (tx) {
        offs = REG_TX_FAST_LOCK_SETUP - REG_RX_FAST_LOCK_SETUP;
    }

    SPI_SDR_Write(id_no, REG_RX_FAST_LOCK_PROGRAM_ADDR + offs, RX_FAST_LOCK_PROFILE_ADDR(profile) | RX_FAST_LOCK_PROFILE_WORD(word));

    return SPI_SDR_Read(id_no, REG_RX_FAST_LOCK_PROGRAM_READ + offs);
}

/**
 * Fastlock prepare.
 * @param phy The AD9361 state structure.
 * @param tx
 * @param profile
 * @param prepare
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_fastlock_prepare(ad9361_rf_phy_t* phy, //
                                bool             tx,
                                uint32_t         profile,
                                bool             prepare) {

    uint32_t offs;
    uint32_t ready_mask;
    bool     is_prepared;

    LOG_FORMAT(debug, "%s profile %" PRIu32 " %s (%s)", tx ? "TX" : "RX", profile, prepare ? "Prepare" : "Un-Prepare", __func__);

    if (tx) {
        offs       = REG_TX_FAST_LOCK_SETUP - REG_RX_FAST_LOCK_SETUP;
        ready_mask = TX_SYNTH_READY_MASK;
    }
    else {
        offs       = 0;
        ready_mask = RX_SYNTH_READY_MASK;
    }

    is_prepared = !!phy->fastlock.current_profile[tx];

    if (prepare && !is_prepared) {
        SPI_SDR_Write(phy->id_no, REG_RX_FAST_LOCK_SETUP_INIT_DELAY + offs, (tx ? phy->pdata.tx_fastlock_delay_ns : phy->pdata.rx_fastlock_delay_ns) / 250);
        SPI_SDR_Write(phy->id_no, REG_RX_FAST_LOCK_SETUP_INIT_DELAY + offs, RX_FAST_LOCK_PROFILE(profile) | RX_FAST_LOCK_MODE_ENABLE);
        SPI_SDR_Write(phy->id_no, REG_RX_FAST_LOCK_SETUP_INIT_DELAY + offs, 0);

        SPI_SDR_WriteF(phy->id_no, REG_ENSM_CONFIG_2, ready_mask, 1);

        ad9361_trx_vco_cal_control(phy, tx, false);
    }
    else if (!prepare && is_prepared) {
        SPI_SDR_Write(phy->id_no, REG_RX_FAST_LOCK_SETUP + offs, 0);

        // Workaround: Exiting Fastlock Mode
        SPI_SDR_WriteF(phy->id_no, REG_RX_FORCE_ALC + offs, FORCE_ALC_ENABLE, 1);
        SPI_SDR_WriteF(phy->id_no, REG_RX_FORCE_VCO_TUNE_1 + offs, FORCE_VCO_TUNE, 1);
        SPI_SDR_WriteF(phy->id_no, REG_RX_FORCE_ALC + offs, FORCE_ALC_ENABLE, 0);
        SPI_SDR_WriteF(phy->id_no, REG_RX_FORCE_VCO_TUNE_1 + offs, FORCE_VCO_TUNE, 0);

        ad9361_trx_vco_cal_control(phy, tx, true);

        // SL : 2021.01.07 - replaced
        SPI_SDR_WriteF(phy->id_no, REG_ENSM_CONFIG_2, ready_mask, 0);

        phy->fastlock.current_profile[tx] = 0;
    }

    return 0;
}

/**
 * Determine the reference frequency value.
 * @param refin_Hz Maximum allowed frequency.
 * @param max Reference in frequency value.
 * @return Reference frequency value.
 */
uint32_t ad9361_ref_div_sel(uint32_t refin_Hz, //
                            uint32_t max) {

    uint32_t _val = 0;

    if (refin_Hz <= (max / 2)) {
        _val = refin_Hz * 2;
    }
    else if (refin_Hz <= max) {
        _val = refin_Hz;
    }
    else if (refin_Hz <= (max * 2)) {
        _val = refin_Hz / 2;
    }
    else if (refin_Hz <= (max * 4)) {
        _val = refin_Hz / 4;
    }

    return _val;
}

/**
 * Setup the AD9361 device.
 * @param phy The AD9361 state structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_setup(ad9361_rf_phy_t* phy) {

    uint32_t                    refin_Hz, ref_freq, bbpll_freq;
    ad9361_phy_platform_data_t* pd = &(phy->pdata);
    int32_t                     _val;
    uint32_t                    real_rx_bandwidth = pd->rf_rx_bandwidth_Hz / 2;
    uint32_t                    real_tx_bandwidth = pd->rf_tx_bandwidth_Hz / 2;

    LOG_FORMAT(debug, "Start (%s)", __func__);

    if (pd->fdd) {
        // FDD mode
        pd->tdd_skip_vco_cal = false;

        if (pd->ensm_pin_ctrl && pd->fdd_independent_mode) {
            LOG_FORMAT(warning, "Either set ENSM PINCTRL or FDD Independent Mode (%s)", __func__);
            pd->ensm_pin_ctrl = false;
        }
    }
    else {
        // TDD Mode
        if (pd->tdd_use_dual_synth || pd->tdd_skip_vco_cal) {
            pd->tdd_use_fdd_tables = true;
        }
    }

    if (pd->port_ctrl.pp_conf[2] & FDD_RX_RATE_2TX_RATE) {
        phy->rx_eq_2tx = true;
    }

    SPI_SDR_Write(phy->id_no, REG_CTRL, CTRL_ENABLE);
    SPI_SDR_Write(phy->id_no, REG_BANDGAP_CONFIG0, MASTER_BIAS_TRIM(0x0E));  // Enable Master Bias
    SPI_SDR_Write(phy->id_no, REG_BANDGAP_CONFIG1, BANDGAP_TEMP_TRIM(0x0E)); // Set Bandgap Trim

    ad9361_set_dcxo_tune(phy, pd->dcxo_coarse, pd->dcxo_fine);

    refin_Hz = phy->clk_refin.rate;

    ref_freq = ad9361_ref_div_sel(refin_Hz, MAX_BBPLL_FREF);
    if (ref_freq == 0) {
        return -EINVAL;
    }

    SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_1, RX_REF_RESET_BAR, 1);
    SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, TX_REF_RESET_BAR, 1);
    SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, TX_REF_DOUBLER_FB_DELAY(~0), 3); // FB DELAY
    SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, RX_REF_DOUBLER_FB_DELAY(~0), 3); // FB DELAY

    SPI_SDR_Write(phy->id_no, REG_CLOCK_ENABLE,
                  DIGITAL_POWER_UP | CLOCK_ENABLE_DFLT | BBPLL_ENABLE | (pd->use_extclk ? XO_BYPASS : 0)); // Enable Clocks

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[BB_REFCLK]), ref_freq);
    if (_val < 0) {
        LOG_FORMAT(error, "Failed to set BB ref clock rate %" PRId32 " (%s)", _val, __func__);
        return _val;
    }

    _val = ad9361_set_trx_clock_chain(phy, pd->rx_path_clks, pd->tx_path_clks);
    if (_val < 0) {
        return _val;
    }

    ad9361_en_dis_tx(phy, 1, TX_ENABLE);
    ad9361_en_dis_rx(phy, 1, RX_ENABLE);

    ad9361_en_dis_tx(phy, 2, pd->rx2tx2);
    ad9361_en_dis_rx(phy, 2, pd->rx2tx2);

    _val = ad9361_rf_port_setup(phy, true, pd->rf_rx_input_sel, pd->rf_tx_output_sel);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_pp_port_setup(phy, false);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_auxdac_setup(phy, &pd->auxdac_ctrl);
    if (_val < 0) {
        return _val;
    }

    bbpll_freq = clk_get_rate(phy, &(phy->ref_clk_scale[BBPLL_CLK]));

    _val = ad9361_auxadc_setup(phy, &pd->auxadc_ctrl, bbpll_freq);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_ctrl_outs_setup(phy, &pd->ctrl_outs_ctrl);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_gpo_setup(phy, &pd->gpo_ctrl);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_set_ref_clk_cycles(phy, refin_Hz);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_setup_ext_lna(phy, &pd->elna_ctrl);
    if (_val < 0) {
        return _val;
    }

    // This allows forcing a lower F_REF window
    // (worse phase noise, better fractional spurs)
    pd->trx_synth_max_fref = clamp_t(uint32_t, pd->trx_synth_max_fref, MIN_SYNTH_FREF, MAX_SYNTH_FREF);

    ref_freq = ad9361_ref_div_sel(refin_Hz, pd->trx_synth_max_fref);
    if (!ref_freq) {
        return -EINVAL;
    }

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[RX_REFCLK]), ref_freq);
    if (_val < 0) {
        LOG_FORMAT(error, "Failed to set RX Synth ref clock rate %" PRId32 " (%s)", _val, __func__);
        return _val;
    }

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[TX_REFCLK]), ref_freq);
    if (_val < 0) {
        LOG_FORMAT(error, "Failed to set TX Synth ref clock rate %" PRId32 " (%s)", _val, __func__);
        return _val;
    }

    _val = ad9361_txrx_synth_cp_calib(phy, ref_freq, false); // RXCP
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_txrx_synth_cp_calib(phy, ref_freq, true); // TXCP
    if (_val < 0) {
        return _val;
    }

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[RX_RFPLL]), ad9361_to_clk(pd->rx_synth_freq));
    if (_val < 0) {
        LOG_FORMAT(error, "Failed to set RX Synth rate %" PRId32 " (%s)", _val, __func__);
        return _val;
    }

    // REVISIT : add EXT LO clock
    if (pd->use_ext_rx_lo) {
        ad9361_trx_ext_lo_control(phy, false, pd->use_ext_rx_lo);
    }

    // Skip quad cal here we do it later again
    phy->last_tx_quad_cal_freq = pd->tx_synth_freq;

    _val = clk_set_rate(phy, &(phy->ref_clk_scale[TX_RFPLL]), ad9361_to_clk(pd->tx_synth_freq));
    if (_val < 0) {
        LOG_FORMAT(error, "[%s] Failed to set TX Synth rate (%" PRId32 ")", __func__, _val);
        return _val;
    }

    // REVISIT : add EXT LO clock
    if (pd->use_ext_tx_lo) {
        ad9361_trx_ext_lo_control(phy, true, pd->use_ext_tx_lo);
    }

    _val = ad9361_load_mixer_gm_subtable(phy);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_gc_setup(phy, &pd->gain_ctrl);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rx_bb_analog_filter_calib(phy, real_rx_bandwidth, bbpll_freq);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_tx_bb_analog_filter_calib(phy, real_tx_bandwidth, bbpll_freq);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rx_tia_calib(phy, real_rx_bandwidth);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_tx_bb_second_filter_calib(phy, real_tx_bandwidth);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rx_adc_setup(phy, bbpll_freq, clk_get_rate(phy, &(phy->ref_clk_scale[ADC_CLK])));
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_bb_dc_offset_calib(phy);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rf_dc_offset_calib(phy, ad9361_from_clk(clk_get_rate(phy, &(phy->ref_clk_scale[RX_RFPLL]))));
    if (_val < 0) {
        return _val;
    }

    phy->current_rx_bw_Hz       = pd->rf_rx_bandwidth_Hz;
    phy->current_tx_bw_Hz       = pd->rf_tx_bandwidth_Hz;
    phy->last_tx_quad_cal_phase = ~0;

    _val = ad9361_tx_quad_calib(phy, real_rx_bandwidth, real_tx_bandwidth, -1);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_tracking_control(phy, phy->bbdc_track_en, phy->rfdc_track_en, phy->quad_track_en);
    if (_val < 0) {
        return _val;
    }

    if (!pd->fdd) {
        ad9361_run_calibration(phy, TXMON_CAL);
    }

    ad9361_pp_port_setup(phy, true);

    _val = ad9361_set_ensm_mode(phy, pd->fdd, pd->ensm_pin_ctrl);
    if (_val < 0) {
        return _val;
    }

    SPI_SDR_WriteF(phy->id_no, REG_TX_ATTEN_OFFSET, MASK_CLR_ATTEN_UPDATE, 0);

    _val = ad9361_set_tx_atten(phy, pd->tx_atten, true, true, true);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_rssi_setup(phy, &pd->rssi_ctrl, false);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_clkout_control(phy, pd->ad9361_clkout_mode);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_txmon_setup(phy, &pd->txmon_ctrl);
    if (_val < 0) {
        return _val;
    }

    phy->curr_ensm_state = SPI_SDR_ReadF(phy->id_no, REG_STATE, ENSM_STATE(~0));

    ad9361_ensm_set_state(phy, pd->fdd ? ENSM_STATE_FDD : ENSM_STATE_RX, pd->ensm_pin_ctrl);

    phy->auto_cal_en        = true;
    phy->cal_threshold_freq = 100000000ULL; // 100 MHz

    return 0;
}

/**
 * Perform the selected calibration
 * @param phy The AD9361 state structure.
 * @param cal The selected calibration.
 * @param arg The argument of the calibration.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_do_calib_run(ad9361_rf_phy_t* phy, //
                            uint32_t         cal,
                            int32_t          arg) {

    int32_t _val;

    _val = ad9361_tracking_control(phy, false, false, false);
    if (_val < 0) {
        return _val;
    }

    ad9361_ensm_force_state(phy, ENSM_STATE_ALERT);

    switch (cal) {
        case TX_QUAD_CAL:
            _val = ad9361_tx_quad_calib(phy, phy->current_rx_bw_Hz / 2, phy->current_tx_bw_Hz / 2, arg);
            break;

        case RFDC_CAL:
            _val = ad9361_rf_dc_offset_calib(phy, ad9361_from_clk(clk_get_rate(phy, &(phy->ref_clk_scale[RX_RFPLL]))));
            break;

        default:
            _val = -EINVAL;
            break;
    }

    _val = ad9361_tracking_control(phy, phy->bbdc_track_en, phy->rfdc_track_en, phy->quad_track_en);

    ad9361_ensm_restore_prev_state(phy);

    return _val;
}

/**
 * Verify the FIR filter coefficients.
 * @param phy The AD9361 state structure.
 * @param dest Destination identifier (RX1,2 / TX1,2).
 * @param ntaps Number of filter Taps.
 * @param coef Pointer to filter coefficients.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_verify_fir_filter_coef(ad9361_rf_phy_t* phy, //
                                      fir_dest_t       dest,
                                      uint32_t         ntaps,
                                      short*           coef) {

    uint32_t val, offs = 0, gain = 0, conf, sel, cnt;
    int32_t  _val = 0;

    LOG_FORMAT(debug, "TAPS %" PRIu32 ", destination %d (%s)", ntaps, dest, __func__);

    if (dest & FIR_IS_RX) {
        gain = SPI_SDR_Read(phy->id_no, REG_RX_FILTER_GAIN);
        offs = REG_RX_FILTER_COEF_ADDR - REG_TX_FILTER_COEF_ADDR;
        SPI_SDR_Write(phy->id_no, REG_RX_FILTER_GAIN, 0);
    }

    conf = SPI_SDR_Read(phy->id_no, REG_TX_FILTER_CONF + offs);

    if ((dest & 3) == 3) {
        sel = 1;
        cnt = 2;
    }
    else {
        sel = (dest & 3);
        cnt = 1;
    }

    for (; cnt > 0; cnt--, sel++) {
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, FIR_NUM_TAPS(ntaps / 16 - 1) | FIR_SELECT(sel) | FIR_START_CLK);

        for (val = 0; val < ntaps; val++) {
            short tmp;

            SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_ADDR + offs, val);

            tmp = (SPI_SDR_Read(phy->id_no, REG_TX_FILTER_COEF_READ_DATA_1 + offs) & 0xFF) | (SPI_SDR_Read(phy->id_no, REG_TX_FILTER_COEF_READ_DATA_2 + offs) << 8);

            if (tmp != coef[val]) {
                LOG_FORMAT(error,
                           "%s%" PRIu32 " read verify failed TAP%" PRIu32 " %d =! %d (%s)", //
                           (dest & FIR_IS_RX) ? "RX" : "TX",
                           sel,
                           val,
                           tmp,
                           coef[val],
                           __func__);
                _val = -EIO;
            }
        }
    }

    if (dest & FIR_IS_RX) {
        SPI_SDR_Write(phy->id_no, REG_RX_FILTER_GAIN, gain);
    }

    SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, conf);

    return _val;
}

/**
 * Load the FIR filter coefficients.
 * @param phy The AD9361 state structure.
 * @param dest Destination identifier (RX1,2 / TX1,2).
 * @param gain_dB Gain option.
 * @param ntaps Number of filter Taps.
 * @param coef Pointer to filter coefficients.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_load_fir_filter_coef(ad9361_rf_phy_t* phy, //
                                    fir_dest_t       dest,
                                    int32_t          gain_dB,
                                    uint32_t         ntaps,
                                    int16_t*         coef) {

    uint32_t val, offs = 0, fir_conf = 0, fir_enable = 0;

    LOG_FORMAT(debug,
               "TAPS %" PRIu32 ", gain %" PRId32 ", destination %d (%s)", //
               ntaps,
               gain_dB,
               dest,
               __func__);

    if ((coef == NULL) || !ntaps || (ntaps > 128) || (ntaps % 16)) {
        LOG_FORMAT(error,
                   "Invalid parameters: TAPS %" PRIu32 ", gain %" PRId32 ", destination 0x%X (%s)", //
                   ntaps,
                   gain_dB,
                   dest,
                   __func__);
        return -EINVAL;
    }

    if (dest & FIR_IS_RX) {
        val = 3 - (gain_dB + 12) / 6;

        SPI_SDR_Write(phy->id_no, REG_RX_FILTER_GAIN, (val & 0x03));

        offs = REG_RX_FILTER_COEF_ADDR - REG_TX_FILTER_COEF_ADDR;

        phy->rx_fir_ntaps = ntaps;

        fir_enable = SPI_SDR_ReadF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_FIR_ENABLE_DECIMATION(~0));

        SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_FIR_ENABLE_DECIMATION(~0), (phy->rx_fir_dec == 4) ? 3 : phy->rx_fir_dec);
    }
    else {
        if (gain_dB == -6) {
            fir_conf = TX_FIR_GAIN_6DB;
        }

        phy->tx_fir_ntaps = ntaps;

        fir_enable = SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_FIR_ENABLE_INTERPOLATION(~0));

        SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_FIR_ENABLE_INTERPOLATION(~0), (phy->tx_fir_int == 4) ? 3 : phy->tx_fir_int);
    }

    val = ntaps / 16 - 1;

    fir_conf |= FIR_NUM_TAPS(val) | FIR_SELECT(dest) | FIR_START_CLK;

    SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, fir_conf);

    for (val = 0; val < ntaps; val++) {
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_ADDR + offs, val);
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_WRITE_DATA_1 + offs, (coef[val] & 0xFF));
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_WRITE_DATA_2 + offs, (coef[val] >> 8));
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, fir_conf | FIR_WRITE);
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_READ_DATA_2 + offs, 0);
        SPI_SDR_Write(phy->id_no, REG_TX_FILTER_COEF_READ_DATA_2 + offs, 0);
    }

    SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, fir_conf);

    fir_conf &= ~FIR_START_CLK;

    SPI_SDR_Write(phy->id_no, REG_TX_FILTER_CONF + offs, fir_conf);

    if (dest & FIR_IS_RX) {
        SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_FIR_ENABLE_DECIMATION(~0), fir_enable);
    }
    else {
        SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_FIR_ENABLE_INTERPOLATION(~0), fir_enable);
    }

    return ad9361_verify_fir_filter_coef(phy, dest, ntaps, coef);
}

/**
 * Set the multiplier and the divider for the selected refclk_scale structure.
 * @param priv The selected refclk_scale structure.
 * @param mul The multiplier value.
 * @param div The divider value.
 * @return 0 in case of success, negative error code otherwise.
 */
inline int32_t ad9361_set_muldiv(refclk_scale_t* priv, //
                                 uint32_t        mul,
                                 uint32_t        div) {

    priv->mult = mul;
    priv->div  = div;

    return 0;
}

/**
 * Get the clk scaler for the selected refclk_scale structure.
 * @param id_no AD9361 module identification number
 * @param priv The selected refclk_scale structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_get_clk_scaler(ad9361_rf_phy_t* phy, //
                              refclk_scale_t*  clk_priv) {

    uint32_t tmp_1, tmp_2;

    switch (clk_priv->source) {
        case BB_REFCLK:
            tmp_1 = SPI_SDR_Read(phy->id_no, REG_CLOCK_CTRL);
            tmp_1 &= 0x03;
            break;

        case RX_REFCLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_REF_DIVIDE_CONFIG_1, RX_REF_DIVIDER_MSB);
            tmp_2 = SPI_SDR_ReadF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, RX_REF_DIVIDER_LSB);
            tmp_1 = (tmp_1 << 1) | tmp_2;
            break;

        case TX_REFCLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, TX_REF_DIVIDER(~0));
            break;

        case ADC_CLK:
            tmp_1 = SPI_SDR_Read(phy->id_no, REG_BBPLL);
            return ad9361_set_muldiv(clk_priv, 1, 1 << (tmp_1 & 0x07));

        case R2_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, DEC3_ENABLE_DECIMATION(~0));
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case R1_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RHB2_EN);
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case CLKRF_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RHB1_EN);
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case RX_SAMPL_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_FIR_ENABLE_DECIMATION(~0));
            if (!tmp_1) {
                tmp_1 = 1; // bypass filter
            }
            else {
                tmp_1 = (1 << (tmp_1 - 1));
            }
            return ad9361_set_muldiv(clk_priv, 1, tmp_1);

        case DAC_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_BBPLL, BIT(3));
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case T2_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB3_ENABLE_INTERP(~0));
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);
        case T1_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB2_EN);
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case CLKTF_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB1_EN);
            return ad9361_set_muldiv(clk_priv, 1, tmp_1 + 1);

        case TX_SAMPL_CLK:
            tmp_1 = SPI_SDR_ReadF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_FIR_ENABLE_INTERPOLATION(~0));
            if (!tmp_1) {
                tmp_1 = 1; // bypass filter
            }
            else {
                tmp_1 = (1 << (tmp_1 - 1));
            }
            return ad9361_set_muldiv(clk_priv, 1, tmp_1);

        default:
            return -EINVAL;
    }

    // REFCLK Scaler
    switch (tmp_1) {
        case 0:
            ad9361_set_muldiv(clk_priv, 1, 1);
            break;

        case 1:
            ad9361_set_muldiv(clk_priv, 1, 2);
            break;

        case 2:
            ad9361_set_muldiv(clk_priv, 1, 4);
            break;

        case 3:
            ad9361_set_muldiv(clk_priv, 2, 1);
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/**
 * Calculate the REFCLK Scaler for the selected refclk_scale structure.
 * Note: REFCLK Scaler values - 00: x1; 01: x½; 10: x¼; 11: x2.
 * @param clk_priv The selected refclk_scale structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_to_refclk_scaler(refclk_scale_t* clk_priv) {
    int32_t _val = -EINVAL;

    // REFCLK Scaler
    switch (((clk_priv->mult & 0x0000000F) << 4) | (clk_priv->div & 0x0000000F)) {
        case 0x11:
            _val = 0;
            break;

        case 0x12:
            _val = 1;
            break;

        case 0x14:
            _val = 2;
            break;

        case 0x21:
            _val = 3;
            break;

        default:
            break;
    }

    return _val;
}

/**
 * Set clk scaler for the selected refclk_scale structure.
 * @param clk_priv The selected refclk_scale structure.
 * @param set Set true, the reference clock frequency will be scaled before
 *            it enters the BBPLL.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_clk_scaler(ad9361_rf_phy_t* phy, //
                              refclk_scale_t*  clk_priv,
                              bool             set) {

    uint32_t tmp;
    int32_t  _val;

    switch (clk_priv->source) {
        case BB_REFCLK:
            _val = ad9361_to_refclk_scaler(clk_priv);
            if (_val < 0) {
                return _val;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_CLOCK_CTRL, REF_FREQ_SCALER(~0), (uint8_t)_val);
                return 0;
            }
            break;

        case RX_REFCLK:
            _val = ad9361_to_refclk_scaler(clk_priv);
            if (_val < 0) {
                return _val;
            }

            if (set) {
                tmp = _val;
                SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_1, RX_REF_DIVIDER_MSB, tmp >> 1);
                SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, RX_REF_DIVIDER_LSB, tmp & 1);
                return 0;
            }
            break;

        case TX_REFCLK:
            _val = ad9361_to_refclk_scaler(clk_priv);
            if (_val < 0) {
                return _val;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_REF_DIVIDE_CONFIG_2, TX_REF_DIVIDER(~0), _val);
                return 0;
            }
            break;

        case ADC_CLK:
            tmp = int_log2((uint8_t)clk_priv->div);

            if ((clk_priv->mult != 1) || (tmp > 6) || (tmp < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_BBPLL, 0x07, tmp);
                return 0;
            }
            break;

        case R2_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 3) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, DEC3_ENABLE_DECIMATION(~0), clk_priv->div - 1);
                return 0;
            }
            break;

        case R1_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 2) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RHB2_EN, clk_priv->div - 1);
                return 0;
            }
            break;

        case CLKRF_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 2) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RHB1_EN, clk_priv->div - 1);
                return 0;
            }
            break;

        case RX_SAMPL_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 4) || (clk_priv->div < 1) || (clk_priv->div == 3)) {
                return -EINVAL;
            }

            if (phy->bypass_rx_fir) {
                tmp = 0;
            }
            else {
                tmp = int_log2(clk_priv->div) + 1;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_RX_ENABLE_FILTER_CTRL, RX_FIR_ENABLE_DECIMATION(~0), tmp);
                return 0;
            }
            break;

        case DAC_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 2) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_BBPLL, BIT(3), clk_priv->div - 1);
                return 0;
            }
            break;

        case T2_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 3) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB3_ENABLE_INTERP(~0), clk_priv->div - 1);
                // SL : 2020.12.23 - returned value to be reviewed
                return 0;
            }
            break;

        case T1_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 2) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB2_EN, clk_priv->div - 1);
                return 0;
            }
            break;

        case CLKTF_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 2) || (clk_priv->div < 1)) {
                return -EINVAL;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, THB1_EN, clk_priv->div - 1);
                return 0;
            }
            break;

        case TX_SAMPL_CLK:
            if ((clk_priv->mult != 1) || (clk_priv->div > 4) || (clk_priv->div < 1) || (clk_priv->div == 3)) {
                return -EINVAL;
            }

            if (phy->bypass_tx_fir) {
                tmp = 0;
            }
            else {
                tmp = int_log2(clk_priv->div) + 1;
            }

            if (set) {
                SPI_SDR_WriteF(phy->id_no, REG_TX_ENABLE_FILTER_CTRL, TX_FIR_ENABLE_INTERPOLATION(~0), tmp);
                return 0;
            }
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

/**
 * Recalculate the clock rate.
 * @id_no AD9361 module identification number
 * @param refclk_scale The refclk_scale structure.
 * @param parent_rate The parent clock rate.
 * @return The clock rate.
 */
uint32_t ad9361_clk_factor_recalc_rate(ad9361_rf_phy_t* phy, //
                                       refclk_scale_t*  clk_priv,
                                       uint32_t         parent_rate) {

    ad9361_get_clk_scaler(phy, clk_priv);

    uint64_t rate = (parent_rate * clk_priv->mult) / clk_priv->div;
    return (uint32_t)rate;
}

/**
 * Calculate the closest possible clock rate that can be set.
 * @param refclk_scale The refclk_scale structure.
 * @param rate The clock rate.
 * @param parent_rate The parent clock rate.
 * @return The closest possible clock rate that can be set.
 */
int32_t ad9361_clk_factor_round_rate(ad9361_rf_phy_t* phy, //
                                     refclk_scale_t*  clk_priv,
                                     uint32_t         rate,
                                     uint32_t*        prate) {

    int32_t _val;

    if (rate >= *prate) {
        clk_priv->mult = DIV_ROUND_CLOSEST(rate, *prate);
        clk_priv->div  = 1;
    }
    else {
        clk_priv->div  = DIV_ROUND_CLOSEST(*prate, rate);
        clk_priv->mult = 1;

        if (!clk_priv->div) {
            LOG_FORMAT(error, "Divide by zero (%s)", __func__);
            clk_priv->div = 1;
        }
    }

    _val = ad9361_set_clk_scaler(phy, clk_priv, false);
    if (_val >= 0) {
        _val = (*prate / clk_priv->div) * clk_priv->mult;
    }

    return _val;
}

/**
 * Set the clock rate.
 * @param refclk_scale The refclk_scale structure.
 * @param rate The clock rate.
 * @param parent_rate The parent clock rate.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_clk_factor_set_rate(ad9361_rf_phy_t* phy, //
                                   refclk_scale_t*  clk_priv,
                                   uint32_t         rate,
                                   uint32_t         parent_rate) {

    LOG_FORMAT(debug, "Rate %" PRIu32 " Hz, Parent Rate %" PRIu32 " Hz (%s)", rate, parent_rate, __func__);

    if (rate >= parent_rate) {
        clk_priv->mult = DIV_ROUND_CLOSEST(rate, parent_rate);
        clk_priv->div  = 1;
    }
    else {
        clk_priv->div  = DIV_ROUND_CLOSEST(parent_rate, rate);
        clk_priv->mult = 1;

        if (!clk_priv->div) {
            LOG_FORMAT(error, "Divide by zero (%s)", __func__);
            clk_priv->div = 1;
        }
    }

    return ad9361_set_clk_scaler(phy, clk_priv, true);
}

/**
 * Recalculate the clock rate.
 * @param id_no AD9361 module identification number
 * @param refclk_scale The refclk_scale structure.
 * @param parent_rate The parent clock rate.
 * @return The clock rate.
 */
uint32_t ad9361_bbpll_recalc_rate(ad9361_rf_phy_t* phy, //
                                  refclk_scale_t*  clk_priv,
                                  uint32_t         parent_rate) {

    UNUSED(clk_priv)

    uint64_t rate;
    uint32_t fract, integer;
    uint8_t  buf[4];

    SPI_SDR_ReadM(phy->id_no, REG_INTEGER_BB_FREQ_WORD, &buf[0], REG_INTEGER_BB_FREQ_WORD - REG_FRACT_BB_FREQ_WORD_1 + 1);

    fract   = (buf[3] << 16) | (buf[2] << 8) | buf[1];
    integer = buf[0];

    rate = ((uint64_t)parent_rate * fract);
    int_do_div(&rate, BBPLL_MODULUS);
    rate += ((uint64_t)parent_rate * integer);

    return (uint32_t)rate;
}

/**
 * Set the clock rate.
 * @param refclk_scale The refclk_scale structure.
 * @param rate The clock rate.
 * @param parent_rate The parent clock rate.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_bbpll_set_rate(ad9361_rf_phy_t* phy, //
                              refclk_scale_t*  clk_priv,
                              uint32_t         rate,
                              uint32_t         parent_rate) {

    UNUSED(clk_priv);

    uint32_t fract, integer;
    int32_t  icp_val;
    uint8_t  lf_defaults[3] = {0x35, 0x5B, 0xE8};
    uint64_t tmp_1, tmp_2;

    LOG_FORMAT(debug, "Rate %" PRIu32 " Hz, Parent Rate %" PRIu32 " Hz (%s)", rate, parent_rate, __func__);

    // Setup Loop Filter and CP Current
    // Scale is 150uA @ (1280MHz BBPLL, 40MHz REFCLK)
    tmp_1 = (rate >> 7) * 150ULL;
    int_do_div(&tmp_1, (parent_rate >> 7) * 32UL + (tmp_1 >> 1));

    // 25uA/LSB, Offset 25uA
    icp_val = DIV_ROUND_CLOSEST((uint32_t)tmp_1, 25U) - 1;
    icp_val = clamp(icp_val, 1, 64);

    SPI_SDR_Write(phy->id_no, REG_CP_CURRENT, icp_val);

    SPI_SDR_WriteM(phy->id_no, REG_LOOP_FILTER_3, lf_defaults, ARRAY_SIZE(lf_defaults));

    // Allow calibration to occur and set cal count to 1024 for max accuracy
    SPI_SDR_Write(phy->id_no, REG_VCO_CTRL, FREQ_CAL_ENABLE | FREQ_CAL_COUNT_LENGTH(3));

    // Set calibration clock to REFCLK/4 for more accuracy
    SPI_SDR_Write(phy->id_no, REG_SDM_CTRL, 0x10);

    // Calculate and set BBPLL frequency word
    tmp_2 = rate;
    tmp_1 = int_do_div(&tmp_2, parent_rate);
    rate  = tmp_2;
    tmp_1 = tmp_1 * (uint64_t)BBPLL_MODULUS + (parent_rate >> 1);

    int_do_div(&tmp_1, parent_rate);

    integer = rate;
    fract   = tmp_1;

    SPI_SDR_Write(phy->id_no, REG_INTEGER_BB_FREQ_WORD, integer);
    SPI_SDR_Write(phy->id_no, REG_FRACT_BB_FREQ_WORD_3, fract);
    SPI_SDR_Write(phy->id_no, REG_FRACT_BB_FREQ_WORD_2, fract >> 8);
    SPI_SDR_Write(phy->id_no, REG_FRACT_BB_FREQ_WORD_1, fract >> 16);

    SPI_SDR_Write(phy->id_no, REG_SDM_CTRL_1, INIT_BB_FO_CAL | BBPLL_RESET_BAR); // Start BBPLL Calibration
    SPI_SDR_Write(phy->id_no, REG_SDM_CTRL_1, BBPLL_RESET_BAR);                  // Clear BBPLL start calibration bit

    SPI_SDR_Write(phy->id_no, REG_VCO_PROGRAM_1, 0x86); // Increase BBPLL KV and phase margin
    SPI_SDR_Write(phy->id_no, REG_VCO_PROGRAM_2, 0x01); // Increase BBPLL KV and phase margin
    SPI_SDR_Write(phy->id_no, REG_VCO_PROGRAM_2, 0x05); // Increase BBPLL KV and phase margin

    return ad9361_check_cal_done(phy, REG_CH_1_OVERFLOW, BBPLL_LOCK, 1);
}

/**
 * Calculate the RFPLL frequency.
 * @param parent_rate The parent clock rate.
 * @param integer The integer value.
 * @param fract The fractional value.
 * @param vco_div The VCO divider.
 * @return The RFPLL frequency.
 */
uint64_t ad9361_calc_rfpll_freq(uint64_t parent_rate, //
                                uint64_t integer,
                                uint64_t fract,
                                uint32_t vco_div) {

    uint64_t rate = parent_rate * fract;

    int_do_div(&rate, RFPLL_MODULUS);

    rate += parent_rate * integer;

    return rate >> (vco_div + 1);
}

/**
 * Calculate the RFPLL dividers.
 * @param freq The RFPLL frequency.
 * @param parent_rate The parent clock rate.
 * @param integer The integer value.
 * @param fract The fractional value.
 * @param vco_div The VCO divider.
 * @param vco_freq The VCO frequency.
 * @return The RFPLL frequency.
 */
int32_t ad9361_calc_rfpll_divder(uint64_t  freq, //
                                 uint64_t  parent_rate,
                                 uint32_t* integer,
                                 uint32_t* fract,
                                 int32_t*  vco_div,
                                 uint64_t* vco_freq) {

    uint64_t tmp;
    int32_t  div;

    if ((freq > MAX_CARRIER_FREQ_HZ) || (freq < MIN_CARRIER_FREQ_HZ)) {
        return -EINVAL;
    }

    div = -1;

    while (freq <= MIN_VCO_FREQ_HZ) {
        freq <<= 1;
        div++;
    }

    *vco_div  = div;
    *vco_freq = freq;
    tmp       = int_do_div(&freq, parent_rate);
    tmp       = tmp * RFPLL_MODULUS + (parent_rate >> 1);

    int_do_div(&tmp, parent_rate);

    *integer = freq;
    *fract   = tmp;

    return 0;
}

/**
 * Recalculate the clock rate.
 * @param refclk_scale The refclk_scale structure.
 * @param parent_rate The parent clock rate.
 * @return The clock rate.
 */
uint32_t ad9361_rfpll_recalc_rate(ad9361_rf_phy_t* phy, //
                                  refclk_scale_t*  clk_priv,
                                  uint32_t         parent_rate) {

    uint32_t fract, integer;
    uint8_t  buf[5];
    uint32_t reg, div_mask, vco_div, profile;

    LOG_FORMAT(debug, "Parent rate %" PRIu32 " Hz (%s)", parent_rate, __func__);

    switch (clk_priv->source) {

        case RX_RFPLL:
            reg      = REG_RX_FRACT_BYTE_2;
            div_mask = RX_VCO_DIVIDER(~0);
            profile  = phy->fastlock.current_profile[0];
            break;

        case TX_RFPLL:
            reg      = REG_TX_FRACT_BYTE_2;
            div_mask = TX_VCO_DIVIDER(~0);
            profile  = phy->fastlock.current_profile[1];
            break;

        default:
            return -EINVAL;
    }

    if (profile) {
        bool tx = clk_priv->source == TX_RFPLL;
        profile = profile - 1;

        buf[0]  = ad9361_fastlock_readval(phy->id_no, tx, profile, 4);
        buf[1]  = ad9361_fastlock_readval(phy->id_no, tx, profile, 3);
        buf[2]  = ad9361_fastlock_readval(phy->id_no, tx, profile, 2);
        buf[3]  = ad9361_fastlock_readval(phy->id_no, tx, profile, 1);
        buf[4]  = ad9361_fastlock_readval(phy->id_no, tx, profile, 0);
        vco_div = ad9361_fastlock_readval(phy->id_no, tx, profile, 12) & 0x0000000F;
    }
    else {
        SPI_SDR_ReadM(phy->id_no, reg, &buf[0], ARRAY_SIZE(buf));
        vco_div = SPI_SDR_ReadF(phy->id_no, REG_RFPLL_DIVIDERS, div_mask);
    }

    fract   = (SYNTH_FRACT_WORD(buf[0]) << 16) | (buf[1] << 8) | buf[2];
    integer = (SYNTH_INTEGER_WORD(buf[3]) << 8) | buf[4];

    return ad9361_to_clk(ad9361_calc_rfpll_freq(parent_rate, integer, fract, vco_div));
}

/**
 * Set the clock rate.
 * @param refclk_scale The refclk_scale structure.
 * @param rate The clock rate.
 * @param parent_rate The parent clock rate.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_rfpll_set_rate(ad9361_rf_phy_t* phy, //
                              refclk_scale_t*  clk_priv,
                              uint32_t         rate,
                              uint32_t         parent_rate) {

    uint64_t vco;
    uint8_t  buf[5];
    uint32_t reg, div_mask, lock_reg, fract, integer;
    int32_t  vco_div, _val;

    LOG_FORMAT(debug, "Rate %" PRIu32 " Hz, Parent Rate %" PRIu32 " Hz (%s)", parent_rate, __func__);

    ad9361_fastlock_prepare(phy, clk_priv->source == TX_RFPLL, 0, false);

    _val = ad9361_calc_rfpll_divder(ad9361_from_clk(rate), parent_rate, &integer, &fract, &vco_div, &vco);
    if (_val < 0) {
        return _val;
    }

    switch (clk_priv->source) {
        case RX_RFPLL:
            reg      = REG_RX_FRACT_BYTE_2;
            lock_reg = REG_RX_CP_OVERRANGE_VCO_LOCK;
            div_mask = RX_VCO_DIVIDER(~0);
            break;

        case TX_RFPLL:
            reg      = REG_TX_FRACT_BYTE_2;
            lock_reg = REG_TX_CP_OVERRANGE_VCO_LOCK;
            div_mask = TX_VCO_DIVIDER(~0);
            break;

        default:
            return -EINVAL;
    }

    // Option to skip VCO cal in TDD mode when moving from TX/RX to Alert
    if (phy->pdata.tdd_skip_vco_cal) {
        ad9361_trx_vco_cal_control(phy, clk_priv->source == TX_RFPLL, true);
    }

    ad9361_rfpll_vco_init(phy, div_mask == TX_VCO_DIVIDER(~0), vco, parent_rate);

    buf[0] = SYNTH_FRACT_WORD(fract >> 16);
    buf[1] = fract >> 8;
    buf[2] = fract & 0x000000FF;
    buf[3] = integer >> 8;
    buf[3] = SYNTH_INTEGER_WORD(integer >> 8) | (~SYNTH_INTEGER_WORD(~0) & SPI_SDR_Read(phy->id_no, reg - 3));
    buf[4] = integer & 0x000000FF;

    SPI_SDR_WriteM(phy->id_no, reg, buf, 5);
    SPI_SDR_WriteF(phy->id_no, REG_RFPLL_DIVIDERS, div_mask, vco_div);

    // Load Gain Table
    if (clk_priv->source == RX_RFPLL) {
        _val = ad9361_load_gt(phy, ad9361_from_clk(rate), GT_RX1 + GT_RX2);
        if (_val < 0) {
            return _val;
        }
    }

    // For RX LO we typically have the tracking option enabled, so for now do nothing here.
    if (phy->auto_cal_en && (clk_priv->source == TX_RFPLL)) {
        if (std::abs((int64_t)(phy->last_tx_quad_cal_freq - ad9361_from_clk(rate))) > (int64_t)phy->cal_threshold_freq) {
            _val = ad9361_do_calib_run(phy, TX_QUAD_CAL, -1);
            if (_val < 0) {
                LOG_FORMAT(debug, "TX QUAD cal failed (%s)", __func__);
            }

            phy->last_tx_quad_cal_freq = ad9361_from_clk(rate);
        }
    }

    _val = ad9361_check_cal_done(phy, lock_reg, VCO_LOCK, 1);

    if (phy->pdata.tdd_skip_vco_cal) {
        ad9361_trx_vco_cal_control(phy, clk_priv->source == TX_RFPLL, false);
    }

    return _val;
}

/**
 * Register and initialize a new clock.
 * @param phy The AD9361 state structure.
 * @param source The source of the new clock.
 * @param parent_source The source of the parent clock.
 * @return A clk for the new clock or a negative error code.
 */
clk_t ad9361_clk_register(ad9361_rf_phy_t* phy, //
                          uint32_t         source,
                          uint32_t         parent_source) {

    refclk_scale_t clk_priv;
    clk_t          clk;

    clk_priv.source        = (ad9361_clocks_t)source;
    clk_priv.parent_source = (ad9361_clocks_t)parent_source;

    phy->ref_clk_scale[source] = clk_priv;

    switch (source) {
        case TX_REFCLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clk_refin.rate);
            break;

        case RX_REFCLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clk_refin.rate);
            break;

        case BB_REFCLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clk_refin.rate);
            break;

        case BBPLL_CLK:
            clk.rate = ad9361_bbpll_recalc_rate(phy, &clk_priv, phy->clks[BB_REFCLK].rate);
            break;

        case ADC_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[BBPLL_CLK].rate);
            break;

        case R2_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[ADC_CLK].rate);
            break;

        case R1_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[R2_CLK].rate);
            break;

        case CLKRF_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[R1_CLK].rate);
            break;

        case RX_SAMPL_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[CLKRF_CLK].rate);
            break;

        case DAC_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[ADC_CLK].rate);
            break;

        case T2_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[DAC_CLK].rate);
            break;

        case T1_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[T2_CLK].rate);
            break;

        case CLKTF_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[T1_CLK].rate);
            break;

        case TX_SAMPL_CLK:
            clk.rate = ad9361_clk_factor_recalc_rate(phy, &clk_priv, phy->clks[CLKTF_CLK].rate);
            break;

        case RX_RFPLL:
            clk.rate = ad9361_rfpll_recalc_rate(phy, &clk_priv, phy->clks[RX_REFCLK].rate);
            break;

        case TX_RFPLL:
            clk.rate = ad9361_rfpll_recalc_rate(phy, &clk_priv, phy->clks[TX_REFCLK].rate);
            break;

        default:
            break;
    }

    return clk;
}

/**
 * Register and initialize all the system clocks.
 * @param phy The AD9361 state structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t register_clocks(ad9361_rf_phy_t* phy) {
    // scaled reference clocks
    phy->clks[TX_REFCLK] = ad9361_clk_register(phy, TX_REFCLK, EXT_REF_CLK);
    phy->clks[RX_REFCLK] = ad9361_clk_register(phy, RX_REFCLK, EXT_REF_CLK);
    phy->clks[BB_REFCLK] = ad9361_clk_register(phy, BB_REFCLK, EXT_REF_CLK);
    // base-band PLL clock
    phy->clks[BBPLL_CLK]    = ad9361_clk_register(phy, BBPLL_CLK, BB_REFCLK);
    phy->clks[ADC_CLK]      = ad9361_clk_register(phy, ADC_CLK, BBPLL_CLK);
    phy->clks[R2_CLK]       = ad9361_clk_register(phy, R2_CLK, ADC_CLK);
    phy->clks[R1_CLK]       = ad9361_clk_register(phy, R1_CLK, R2_CLK);
    phy->clks[CLKRF_CLK]    = ad9361_clk_register(phy, CLKRF_CLK, R1_CLK);
    phy->clks[RX_SAMPL_CLK] = ad9361_clk_register(phy, RX_SAMPL_CLK, CLKRF_CLK);
    phy->clks[DAC_CLK]      = ad9361_clk_register(phy, DAC_CLK, ADC_CLK);
    phy->clks[T2_CLK]       = ad9361_clk_register(phy, T2_CLK, DAC_CLK);
    phy->clks[T1_CLK]       = ad9361_clk_register(phy, T1_CLK, T2_CLK);
    phy->clks[CLKTF_CLK]    = ad9361_clk_register(phy, CLKTF_CLK, T1_CLK);
    phy->clks[TX_SAMPL_CLK] = ad9361_clk_register(phy, TX_SAMPL_CLK, CLKTF_CLK);
    phy->clks[RX_RFPLL]     = ad9361_clk_register(phy, RX_RFPLL, RX_REFCLK);
    phy->clks[TX_RFPLL]     = ad9361_clk_register(phy, TX_RFPLL, TX_REFCLK);

    return 0;
}

/**
 * Digital tune.
 * @param phy The AD9361 state structure.
 * @param max_freq Maximum frequency.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_dig_tune(ad9361_rf_phy_t* phy, //
                        uint32_t         max_freq) {

    int32_t  _val, i, j, k, t, err = 0;
    uint32_t s0, s1, c0, c1;
    uint8_t  field[2][16];

    if (phy->pdata.dig_interface_tune_skipmode == 2) {
        // skip completely and use defaults
        SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY, phy->pdata.port_ctrl.rx_clk_data_delay);

        SPI_SDR_Write(phy->id_no, REG_TX_CLOCK_DATA_DELAY, phy->pdata.port_ctrl.tx_clk_data_delay);

        return 0;
    }

    if (!phy->pdata.fdd) {
        ad9361_set_ensm_mode(phy, true, false);

        ad9361_ensm_force_state(phy, ENSM_STATE_FDD);
    }
    else {
        ad9361_ensm_force_state(phy, ENSM_STATE_ALERT);

        ad9361_ensm_restore_prev_state(phy);
    }

    ad9361_bist_prbs(phy, BIST_INJ_RX);

    for (t = 0; t < 2; t++) {
        memset(field, 0, 32);

        for (k = 0; k < 2; k++) {
            if (max_freq) {
                ad9361_set_trx_clock_chain_freq(phy, k ? max_freq : 10000000UL);
            }

            for (i = 0; i < 2; i++) {
                for (j = 0; j < 16; j++) {
                    SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY + t, RX_DATA_DELAY(i == 0 ? j : 0) | DATA_CLK_DELAY(i ? j : 0));

                    STIME_mSleep(4);

                    if (t != 1) {
                        _val = 1;
                    }

                    field[i][j] |= _val;
                }
            }
        }

        c0 = ad9361_find_opt_delay(&field[0][0], 16, &s0);
        c1 = ad9361_find_opt_delay(&field[1][0], 16, &s1);

        if (!c0 && !c1) {
            LOG_FORMAT(error, "Tuning %s FAILED! (%s)", t ? "TX" : "RX", __func__);
            err |= -EIO;
        }

        if (c1 > c0) {
            SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY + t, DATA_CLK_DELAY(s1 + c1 / 2) | RX_DATA_DELAY(0));
        }
        else {
            SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY + t, DATA_CLK_DELAY(0) | RX_DATA_DELAY(s0 + c0 / 2));
        }

        if (t == 0) {
            // Now do the loopback and tune the digital out
            ad9361_bist_prbs(phy, BIST_DISABLE);

            if (phy->pdata.dig_interface_tune_skipmode == 1) {
                // skip TX
                phy->pdata.port_ctrl.rx_clk_data_delay = SPI_SDR_Read(phy->id_no, REG_RX_CLOCK_DATA_DELAY);

                if (!phy->pdata.fdd) {
                    ad9361_set_ensm_mode(phy, phy->pdata.fdd, phy->pdata.ensm_pin_ctrl);

                    ad9361_ensm_restore_prev_state(phy);
                }

                return 0;
            }

            ad9361_set_bist_loopback(phy, 1);
        }
        else {
            ad9361_set_bist_loopback(phy, 0);

            if (err == -EIO) {
                SPI_SDR_Write(phy->id_no, REG_RX_CLOCK_DATA_DELAY, phy->pdata.port_ctrl.rx_clk_data_delay);

                SPI_SDR_Write(phy->id_no, REG_TX_CLOCK_DATA_DELAY, phy->pdata.port_ctrl.tx_clk_data_delay);

                err = 0;
            }
            else {
                phy->pdata.port_ctrl.rx_clk_data_delay = SPI_SDR_Read(phy->id_no, REG_RX_CLOCK_DATA_DELAY);

                phy->pdata.port_ctrl.tx_clk_data_delay = SPI_SDR_Read(phy->id_no, REG_TX_CLOCK_DATA_DELAY);
            }

            if (!phy->pdata.fdd) {
                ad9361_set_ensm_mode(phy, phy->pdata.fdd, phy->pdata.ensm_pin_ctrl);

                ad9361_ensm_restore_prev_state(phy);
            }

            return err;
        }
    }

    return -EINVAL;
}

/**
 * Setup the AD9361 device.
 * @param phy The AD9361 state structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_post_setup(ad9361_rf_phy_t* phy) {

    int32_t rx2tx2 = phy->pdata.rx2tx2;
    int32_t tmp;
    int32_t _val;

    tmp = 0x00000000;

    if (rx2tx2) {
        tmp &= ~BIT(5);
    }

    _val = ad9361_dig_tune(phy, 61440000);
    if (_val < 0) {
        return _val;
    }

    _val = ad9361_set_trx_clock_chain(phy, phy->pdata.rx_path_clks, phy->pdata.tx_path_clks);

    ad9361_ensm_force_state(phy, ENSM_STATE_ALERT);
    ad9361_ensm_restore_prev_state(phy);

    return _val;
}

// Warning: diagnostic enabled
#pragma GCC diagnostic pop

/* *****************************************************************************
 End of File
 */
