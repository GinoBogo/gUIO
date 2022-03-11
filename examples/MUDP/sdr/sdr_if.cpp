/* ************************************************************************** */
/** SDR (AD9361) interface API

  @Company
    Airbus Italia S.p.A.

  @File Name
    sdr_if.cpp

  @Summary
    SDR (AD9361) interface API (SPI)

  @Description
    Interface API for configuring and controlling SDR (AD9361) modules via SPI.
                                                                              */
/* ************************************************************************** */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

// STD libraries
#include "definitions.hpp" // SYS function prototypes

#include <inttypes.h>

// project libraries
#include "GLogger.hpp"
#include "GRegisters.hpp"     // set_bit, to_bits
#include "sdr_ad9361_api.hpp" // SDR AD9361 API
#include "spi_if.hpp"         // SPI interface API

// *****************************************************************************
// *****************************************************************************
// Section: Macros
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: File Scope or Global Data
// *****************************************************************************
// *****************************************************************************

ad9361_rf_phy_t ad9361_phy[SPI_SDR_NUM];

ad9361_init_parameters_t init_params = {
    // identification number
    (uInt08)SPI_SDR1_CS, // id_no

    // reference clock
    (uInt32)40000000UL, // reference_clk_rate

    // base configuration
    (uInt08)0,              // two_rx_two_tx_mode_enable
    (uInt08)1,              // frequency_division_duplex_mode_enable
    (uInt08)0,              // frequency_division_duplex_independent_mode_enable
    (uInt08)0,              // tdd_use_dual_synth_mode_enable
    (uInt08)0,              // tdd_skip_vco_cal_enable
    (uInt32)0,              // tx_fastlock_delay_ns
    (uInt32)0,              // rx_fastlock_delay_ns
    (uInt08)0,              // rx_fastlock_pincontrol_enable
    (uInt08)0,              // tx_fastlock_pincontrol_enable
    (uInt08)0,              // external_rx_lo_enable
    (uInt08)0,              // external_tx_lo_enable
    (uInt08)5,              // dc_offset_tracking_update_event_mask
    (uInt08)6,              // dc_offset_attenuation_high_range
    (uInt08)5,              // dc_offset_attenuation_low_range
    (uInt08)0x28,           // dc_offset_count_high_range
    (uInt08)0x32,           // dc_offset_count_low_range
    (uInt08)0,              // tdd_use_fdd_vco_tables_enable
    (uInt08)0,              // split_gain_table_mode_enable
    (uInt32)MAX_SYNTH_FREF, // trx_synthesizer_target_fref_overwrite_hz
    (uInt08)0,              // qec_tracking_slow_mode_enable

    // ENSM control
    (uInt08)0, // ensm_enable_pin_pulse_mode_enable
    (uInt08)0, // ensm_enable_txnrx_control_enable

    // LO control
    (uInt64)400000000, // rx_synthesizer_frequency_hz
    (uInt64)400000000, // tx_synthesizer_frequency_hz

    // rate & BW control
    {
        (uInt32)800000000, // uint32_t rx_path_clock_frequencies[6]
        (uInt32)200000000, //
        (uInt32)100000000, //
        (uInt32)50000000,  //
        (uInt32)25000000,  //
        (uInt32)25000000   //
    },
    {
        (uInt32)800000000, // uint32_t tx_path_clock_frequencies[6]
        (uInt32)200000000, //
        (uInt32)100000000, //
        (uInt32)50000000,  //
        (uInt32)25000000,  //
        (uInt32)25000000   //
    },
    (uInt32)18000000, // rf_rx_bandwidth_hz
    (uInt32)18000000, // rf_tx_bandwidth_hz

    // RF port control
    (uInt32)0, // rx_rf_port_input_select
    (uInt32)0, // tx_rf_port_input_select

    // TX attenuation control
    (sInt32)10000, // tx_attenuation_mdB
    (uInt08)0,     // update_tx_gain_in_alert_enable

    // reference clock control
    (uInt08)0, // xo_disable_use_ext_refclk_enable
    {
        (uInt32)8,   // dcxo_coarse_and_fine_tune[2]
        (uInt32)5920 //
    },
    (uInt32)0, // clk_output_mode_select

    // gain control
    (uInt08)2,    // gc_rx1_mode
    (uInt08)2,    // gc_rx2_mode
    (uInt08)58,   // gc_adc_large_overload_thresh
    (uInt08)4,    // gc_adc_ovr_sample_size
    (uInt08)47,   // gc_adc_small_overload_thresh
    (uInt16)8192, // gc_dec_pow_measurement_duration
    (uInt08)0,    // gc_dig_gain_enable
    (uInt16)800,  // gc_lmt_overload_high_thresh
    (uInt16)704,  // gc_lmt_overload_low_thresh
    (uInt08)24,   // gc_low_power_thresh
    (uInt08)15,   // gc_max_dig_gain

    // MGC control
    (uInt08)2, // mgc_dec_gain_step
    (uInt08)2, // mgc_inc_gain_step
    (uInt08)0, // mgc_rx1_ctrl_inp_enable
    (uInt08)0, // mgc_rx2_ctrl_inp_enable
    (uInt08)0, // mgc_split_table_ctrl_inp_gain_mode

    // AGC control
    (uInt08)10,   // agc_adc_large_overload_exceed_counter
    (uInt08)2,    // agc_adc_large_overload_inc_steps
    (uInt08)0,    // agc_adc_lmt_small_overload_prevent_gain_inc_enable
    (uInt08)10,   // agc_adc_small_overload_exceed_counter
    (uInt08)4,    // agc_dig_gain_step_size
    (uInt08)3,    // agc_dig_saturation_exceed_counter
    (uInt32)1000, // agc_gain_update_interval_us
    (uInt08)0,    // agc_immed_gain_change_if_large_adc_overload_enable
    (uInt08)0,    // agc_immed_gain_change_if_large_lmt_overload_enable
    (uInt08)10,   // agc_inner_thresh_high
    (uInt08)1,    // agc_inner_thresh_high_dec_steps
    (uInt08)12,   // agc_inner_thresh_low
    (uInt08)1,    // agc_inner_thresh_low_inc_steps
    (uInt08)10,   // agc_lmt_overload_large_exceed_counter
    (uInt08)2,    // agc_lmt_overload_large_inc_steps
    (uInt08)10,   // agc_lmt_overload_small_exceed_counter
    (uInt08)5,    // agc_outer_thresh_high
    (uInt08)2,    // agc_outer_thresh_high_dec_steps
    (uInt08)18,   // agc_outer_thresh_low
    (uInt08)2,    // agc_outer_thresh_low_inc_steps
    (uInt32)1,    // agc_attack_delay_extra_margin_us;
    (uInt08)0,    // agc_sync_for_gain_counter_enable

    // fast AGC
    (uInt32)64,  // fagc_dec_pow_measuremnt_duration
    (uInt32)260, // fagc_state_wait_time_ns

    // fast AGC - low power
    (uInt08)0, // fagc_allow_agc_gain_increase
    (uInt32)5, // fagc_lp_thresh_increment_time
    (uInt32)1, // fagc_lp_thresh_increment_steps

    // fast AGC - lock level
    (uInt32)10, // fagc_lock_level
    (uInt08)1,  // fagc_lock_level_lmt_gain_increase_en
    (uInt32)5,  // fagc_lock_level_gain_increase_upper_limit

    // fast AGC - peak detectors and final settling
    (uInt32)1, // fagc_lpf_final_settling_steps
    (uInt32)1, // fagc_lmt_final_settling_steps
    (uInt32)3, // fagc_final_overrange_count

    // fast AGC - final power test
    (uInt08)0, // fagc_gain_increase_after_gain_lock_en

    // fast AGC - unlocking the gain
    (uInt32)0,  // fagc_gain_index_type_after_exit_rx_mode
    (uInt08)1,  // fagc_use_last_lock_level_for_set_gain_en
    (uInt08)1,  // fagc_rst_gla_stronger_sig_thresh_exceeded_en
    (uInt32)5,  // fagc_optimized_gain_offset
    (uInt32)10, // fagc_rst_gla_stronger_sig_thresh_above_ll
    (uInt08)1,  // fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en
    (uInt08)1,  // fagc_rst_gla_engergy_lost_goto_optim_gain_en
    (uInt32)10, // fagc_rst_gla_engergy_lost_sig_thresh_below_ll
    (uInt32)8,  // fagc_energy_lost_stronger_sig_gain_lock_exit_cnt
    (uInt08)1,  // fagc_rst_gla_large_adc_overload_en
    (uInt08)1,  // fagc_rst_gla_large_lmt_overload_en
    (uInt08)0,  // fagc_rst_gla_en_agc_pulled_high_en
    (uInt32)0,  // fagc_rst_gla_if_en_agc_pulled_high_mode
    (uInt32)64, // fagc_power_measurement_duration_in_state5

    // RSSI control
    (uInt32)1,    // rssi_delay
    (uInt32)1000, // rssi_duration
    (uInt08)3,    // rssi_restart_mode
    (uInt08)0,    // rssi_unit_is_rx_samples_enable
    (uInt32)1,    // rssi_wait

    // aux ADC control
    (uInt32)256,      // aux_adc_decimation
    (uInt32)40000000, // aux_adc_rate

    // aux DAC control
    (uInt08)1, // aux_dac_manual_mode_enable
    (uInt32)0, // aux_dac1_default_value_mV
    (uInt08)0, // aux_dac1_active_in_rx_enable
    (uInt08)0, // aux_dac1_active_in_tx_enable
    (uInt08)0, // aux_dac1_active_in_alert_enable
    (uInt32)0, // aux_dac1_rx_delay_us
    (uInt32)0, // aux_dac1_tx_delay_us
    (uInt32)0, // aux_dac2_default_value_mV
    (uInt08)0, // aux_dac2_active_in_rx_enable
    (uInt08)0, // aux_dac2_active_in_tx_enable
    (uInt08)0, // aux_dac2_active_in_alert_enable
    (uInt32)0, // aux_dac2_rx_delay_us
    (uInt32)0, // aux_dac2_tx_delay_us

    // temperature sensor control
    (uInt32)256,  // temp_sense_decimation
    (uInt16)1000, // temp_sense_measurement_interval_ms
    (sInt08)0xCE, // temp_sense_offset_signed
    (uInt08)1,    // temp_sense_periodic_measurement_enable

    // control out setup
    (uInt08)0xFF, // ctrl_outs_enable_mask
    (uInt08)0,    // ctrl_outs_index

    // external LNA control
    (uInt32)0, // elna_settling_delay_ns
    (uInt32)0, // elna_gain_mdB
    (uInt32)0, // elna_bypass_loss_mdB
    (uInt08)0, // elna_rx1_gpo0_control_enable
    (uInt08)0, // elna_rx2_gpo1_control_enable

    // digital interface control
    (uInt32)1,    // digital_interface_tune_skip_mode
    (uInt08)1,    // pp_tx_swap_enable
    (uInt08)1,    // pp_rx_swap_enable
    (uInt08)0,    // tx_channel_swap_enable
    (uInt08)0,    // rx_channel_swap_enable
    (uInt08)1,    // rx_frame_pulse_mode_enable
    (uInt08)0,    // two_t_two_r_timing_enable
    (uInt08)0,    // invert_data_bus_enable
    (uInt08)0,    // invert_data_clk_enable
    (uInt08)1,    // fdd_alt_word_order_enable
    (uInt08)0,    // invert_rx_frame_enable
    (uInt08)0,    // fdd_rx_rate_2tx_enable
    (uInt08)0,    // swap_ports_enable
    (uInt08)0,    // single_data_rate_enable
    (uInt08)1,    // lvds_mode_enable
    (uInt08)0,    // half_duplex_mode_enable
    (uInt08)0,    // single_port_mode_enable
    (uInt08)0,    // full_port_enable
    (uInt08)0,    // full_duplex_swap_bits_enable
    (uInt32)0,    // delay_rx_data
    (uInt32)0,    // rx_data_clock_delay
    (uInt32)15,   // rx_data_delay
    (uInt32)7,    // tx_fb_clock_delay
    (uInt32)0,    // tx_data_delay
    (uInt32)150,  // lvds_bias_mV
    (uInt08)1,    // lvds_rx_onchip_termination_enable
    (uInt08)0,    // rx1rx2_phase_inversion_en
    (uInt08)0xFF, // lvds_invert1_control
    (uInt08)0x0F, // lvds_invert2_control

    // GPO control
    (uInt08)0, // gpo0_inactive_state_high_enable
    (uInt08)0, // gpo1_inactive_state_high_enable
    (uInt08)0, // gpo2_inactive_state_high_enable
    (uInt08)0, // gpo3_inactive_state_high_enable
    (uInt08)0, // gpo0_slave_rx_enable
    (uInt08)0, // gpo0_slave_tx_enable
    (uInt08)0, // gpo1_slave_rx_enable
    (uInt08)0, // gpo1_slave_tx_enable
    (uInt08)0, // gpo2_slave_rx_enable
    (uInt08)0, // gpo2_slave_tx_enable
    (uInt08)0, // gpo3_slave_rx_enable
    (uInt08)0, // gpo3_slave_tx_enable
    (uInt08)0, // gpo0_rx_delay_us
    (uInt08)0, // gpo0_tx_delay_us
    (uInt08)0, // gpo1_rx_delay_us
    (uInt08)0, // gpo1_tx_delay_us
    (uInt08)0, // gpo2_rx_delay_us
    (uInt08)0, // gpo2_tx_delay_us
    (uInt08)0, // gpo3_rx_delay_us
    (uInt08)0, // gpo3_tx_delay_us

    // Tx monitor control
    (uInt32)37000, // low_high_gain_threshold_mdB
    (uInt32)0,     // low_gain_dB
    (uInt32)0,     // high_gain_dB
    (uInt08)0,     // tx_mon_track_en
    (uInt08)0,     // one_shot_mode_en
    (uInt32)511,   // tx_mon_delay
    (uInt32)8192,  // tx_mon_duration
    (uInt32)2,     // tx1_mon_front_end_gain
    (uInt32)2,     // tx2_mon_front_end_gain
    (uInt32)48,    // tx1_mon_lo_cm
    (uInt32)48     // tx2_mon_lo_cm
};

ad9361_tx_fir_config_t tx_fir_config = {
    // BPF PASSBAND 3/20 fs to 1/4 fs
    (uInt32)3,  // tx
    (sInt32)-6, // tx_gain
    (uInt32)1,  // tx_int
    {
        (sInt16)-4,    (sInt16)-6,    (sInt16)-37,    (sInt16)35,    (sInt16)186,   (sInt16)86,     (sInt16)-284,  (sInt16)-315,  // tx_coef[128]
        (sInt16)107,   (sInt16)219,   (sInt16)-4,     (sInt16)271,   (sInt16)558,   (sInt16)-307,   (sInt16)-1182, (sInt16)-356,  //
        (sInt16)658,   (sInt16)157,   (sInt16)207,    (sInt16)1648,  (sInt16)790,   (sInt16)-2525,  (sInt16)-2553, (sInt16)748,   //
        (sInt16)865,   (sInt16)-476,  (sInt16)3737,   (sInt16)6560,  (sInt16)-3583, (sInt16)-14731, (sInt16)-5278, (sInt16)14819, //
        (sInt16)14819, (sInt16)-5278, (sInt16)-14731, (sInt16)-3583, (sInt16)6560,  (sInt16)3737,   (sInt16)-476,  (sInt16)865,   //
        (sInt16)748,   (sInt16)-2553, (sInt16)-2525,  (sInt16)790,   (sInt16)1648,  (sInt16)207,    (sInt16)157,   (sInt16)658,   //
        (sInt16)-356,  (sInt16)-1182, (sInt16)-307,   (sInt16)558,   (sInt16)271,   (sInt16)-4,     (sInt16)219,   (sInt16)107,   //
        (sInt16)-315,  (sInt16)-284,  (sInt16)86,     (sInt16)186,   (sInt16)35,    (sInt16)-37,    (sInt16)-6,    (sInt16)-4,    //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0      //
    },
    (uInt08)64 // tx_coef_size
};

ad9361_rx_fir_config_t rx_fir_config = {
    // BPF PASSBAND 3/20 fs to 1/4 fs
    (uInt32)3, // rx
    (uInt32)0, // rx_gain
    (uInt32)1, // rx_dec
    {
        (sInt16)-4,    (sInt16)-6,    (sInt16)-37,    (sInt16)35,    (sInt16)186,   (sInt16)86,     (sInt16)-284,  (sInt16)-315,  // rx_coef[128]F
        (sInt16)107,   (sInt16)219,   (sInt16)-4,     (sInt16)271,   (sInt16)558,   (sInt16)-307,   (sInt16)-1182, (sInt16)-356,  //
        (sInt16)658,   (sInt16)157,   (sInt16)207,    (sInt16)1648,  (sInt16)790,   (sInt16)-2525,  (sInt16)-2553, (sInt16)748,   //
        (sInt16)865,   (sInt16)-476,  (sInt16)3737,   (sInt16)6560,  (sInt16)-3583, (sInt16)-14731, (sInt16)-5278, (sInt16)14819, //
        (sInt16)14819, (sInt16)-5278, (sInt16)-14731, (sInt16)-3583, (sInt16)6560,  (sInt16)3737,   (sInt16)-476,  (sInt16)865,   //
        (sInt16)748,   (sInt16)-2553, (sInt16)-2525,  (sInt16)790,   (sInt16)1648,  (sInt16)207,    (sInt16)157,   (sInt16)658,   //
        (sInt16)-356,  (sInt16)-1182, (sInt16)-307,   (sInt16)558,   (sInt16)271,   (sInt16)-4,     (sInt16)219,   (sInt16)107,   //
        (sInt16)-315,  (sInt16)-284,  (sInt16)86,     (sInt16)186,   (sInt16)35,    (sInt16)-37,    (sInt16)-6,    (sInt16)-4,    //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     //
        (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0,     (sInt16)0,      (sInt16)0,     (sInt16)0      //
    },
    (uInt08)64 // rx_coef_size
};

// *****************************************************************************
// *****************************************************************************
// Section: local functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* void SDR_DumpRegs(uint8_t module)

  Summary:
    Dump to UART a full SDR (AD9361) internal registers image.

  Description:
    Dump to UART a full SDR (AD9361) internal registers image. Target module
    is selected by 'module' input parameter.

  Remarks:
    None.
*/
void SDR_DumpRegs(uint8_t module) {
    LOG_FORMAT(debug, "SDR (AD9361) dump started (%s)", __func__);

    // loop over all SDR internal registers...
    for (uint16_t reg{0x000}; reg < 0x400; ++reg) {
        auto _val = SPI_SDR_Read(module, reg);
        LOG_FORMAT(debug, "  GET register 0x%03X : 0x%02X (%3d)", reg, _val, _val);
    }

    LOG_FORMAT(debug, "SDR (AD9361) dump stopped (%s)", __func__);
}

// *****************************************************************************
// *****************************************************************************
// Section: interface functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* void SDR_Reset(uint8_t module)

  Summary:
    Reset SDR (AD9361) module.

  Description:
    Reset SDR (AD9361) module. Selected module is defined by 'module' input
    parameter.

  Remarks:
    None.
*/
void SDR_Reset(uint8_t module) {
    LOG_FORMAT(debug, "Assert reset for module %d (%s)", module, __func__);
    SPI_FPGA_Write(AD9361_RESET_ADDR, AD9361_RESET_ASSERT);

    LOG_FORMAT(debug, "Release reset for module %d (%s)", module, __func__);
    SPI_FPGA_Write(AD9361_RESET_ADDR, AD9361_RESET_FORBID);
}

// *****************************************************************************
/* void SDR_SoftReset(uint8_t module)

  Summary:
    Software reset SDR (AD9361) module.

  Description:
    Software reset SDR (AD9361) module. Selected module is defined by 'module' input parameter.

  Remarks:
    None.
*/
void SDR_SoftReset(uint8_t module) {
    LOG_FORMAT(debug, "Assert soft-reset for module %d (%s)", module, __func__);
    SPI_SDR_Write(module, REG_SPI_CONF, SOFT_RESET | _SOFT_RESET);

    LOG_FORMAT(debug, "Release soft-reset for module %d (%s)", module, __func__);
    SPI_SDR_Write(module, REG_SPI_CONF, 0x00);
}

// *****************************************************************************
/* void SDR_SelfTest(uint8_t module, bool pre_reset)

  Summary:
    SDR (AD9361) module self-test procedure.

  Description:
    SDR (AD9361) module self-test procedure. Selected module is defined by 'module'
    input parameter.

  Remarks:
    None.
*/
void SDR_SelfTest(uint8_t module, bool pre_reset) {
    uint8_t _val{0};

    LOG_FORMAT(debug, "SDR (AD9361) test started (%s)", __func__);

    if (pre_reset) {
        SDR_Reset(module);

        SDR_SoftReset(module);
    }

    // write then read SDR internal register: 0x0028
    SPI_SDR_Write(module, 0x028, 0);
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  SET register 0x028 : %s", to_bits<8>(_val).c_str());

    // write SDR internal register field: 0x028, bit D4 = 1
    SPI_SDR_WriteF(module, 0x028, 0x10, 1);
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  SET bit D4 - 0x028 : %s", to_bits<8>(_val).c_str());

    // write SDR internal register field: 0x028, bit D7 = 1
    SPI_SDR_WriteF(module, 0x028, 0x80, 1);
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  SET bit D7 - 0x028 : %s", to_bits<8>(_val).c_str());

    // write SDR internal register field: 0x028, bit D3 = 1
    SPI_SDR_WriteF(module, 0x028, 0x08, 1);
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  SET bit D3 - 0x028 : %s", to_bits<8>(_val).c_str());

    // write SDR internal register field: 0x028, bit D2 = 1
    SPI_SDR_WriteF(module, 0x028, 0x04, 1);
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  SET bit D2 - 0x028 : %s", to_bits<8>(_val).c_str());

    // write SDR internal register field: 0x028, bit D3 = 0
    SPI_SDR_WriteF(module, 0x028, 0x08, 0);
    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  CLS bit D3 - 0x028 : %s", to_bits<8>(_val).c_str());

    // read back SDR internal register fields: 0x028
    for (auto i{7}; i >= 0; --i) {
        _val = SPI_SDR_ReadF(module, 0x028, set_bit<uint8_t>(i));
        LOG_FORMAT(debug, "  GET bit D%d - 0x028 : %d", i, _val);
    }

    // multiple read/write buffers
    uint8_t sdr_wr_data[4] = {0x01, 0x02, 0x03, 0x04};
    uint8_t sdr_rd_data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // multiple write
    SPI_SDR_WriteM(module, 0x02B, sdr_wr_data, sizeof(sdr_wr_data));
    // read back values
    for (uint8_t i{0}; i < sizeof(sdr_rd_data); ++i) {
        _val = SPI_SDR_Read(module, 0x028 + i);
        LOG_FORMAT(debug, "  SET register 0x%03X : 0x%02X", (0x0028 + i), _val);
    }

    // multiple read
    SPI_SDR_ReadM(module, 0x02F, sdr_rd_data, sizeof(sdr_rd_data));
    // read back values
    LOG_WRITE(debug, "  SDR read block - 0x02F (8 bytes) : ");
    for (uint8_t i{0}; i < sizeof(sdr_rd_data); ++i) {
        LOG_FORMAT(debug, "  (%d) 0x%02X ", i, sdr_rd_data[i]);
    }

    // write SDR internal register (single location)
    SPI_SDR_Write(module, 0x028, 0xA5);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x028);
    LOG_FORMAT(debug, "  GET register 0x028 : 0x%02X", _val);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x28B);
    LOG_FORMAT(debug, "  GET register 0x28B : 0x%02X", _val);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x037);
    LOG_FORMAT(debug, "  GET register 0x037 : 0x%02X", _val);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x002);
    LOG_FORMAT(debug, "  GET register 0x002 : 0x%02X", _val);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x009);
    LOG_FORMAT(debug, "  GET register 0x009 : 0x%02X", _val);

    // read SDR internal register (single location)
    _val = SPI_SDR_Read(module, 0x00B);
    LOG_FORMAT(debug, "  GET register 0x00B : 0x%02X", _val);

    LOG_FORMAT(debug, "SDR (AD9361) test stopped (%s)", __func__);
}

// *****************************************************************************
/* bool SDR_Configure(uint8_t module)

  Summary:
    Configure SDR (AD9361) module.

  Description:
    Configure SDR (AD9361) module. Selected module is defined by 'module'
    input parameter.

  Remarks:
    None.
*/
bool SDR_Configure(uint8_t module) {

    // check module parameter is in allowed range
    if (module < SPI_SDR_NUM) {
        // AD9361 initialize device
        ad9361_init(&ad9361_phy[module], &init_params);

        // AD9361 set TX fir configuration
        ad9361_set_tx_fir_config(&ad9361_phy[module], tx_fir_config);

        // AD9361 set RX fir configuration
        ad9361_set_rx_fir_config(&ad9361_phy[module], rx_fir_config);

        // check ENSM internal state
        uint8_t ENSM_state = ENSM_STATE(SPI_SDR_Read(SPI_SDR1_CS, REG_STATE));
        if (ENSM_state == ENSM_STATE_FDD) {
            LOG_FORMAT(info, "ENSM in FDD state %d (%s)", ENSM_state, __func__);
            return true;
        }
        else {
            LOG_FORMAT(error, "ENSM in state %d (%s)", ENSM_state, __func__);
            LOG_FORMAT(error, "ENSM in FDD state (%d) expected (%s)", ENSM_STATE_FDD, __func__);
        }
    }
    return false;
}

// *****************************************************************************
/* void SDR_BIST_Start(uint8_t module)

  Summary:
    Start SDR (AD9361) module BIST.

  Description:
    Start SDR (AD9361) module BIST. Selected module is defined by 'module'
    input parameter.

  Remarks:
    None.
*/
void SDR_BIST_Start(uint8_t module, bool prbs_mode) {

    LOG_FORMAT(info, "SDR (AD9361) BIST started (%s)", __func__);

    SPI_SDR_Write(module, REG_OBSERVE_CONFIG, 0x40);

    SPI_SDR_Write(module, REG_BIST_AND_DATA_PORT_TEST_CONFIG, 0x00);

    if (prbs_mode) {
        SPI_SDR_Write(module, REG_BIST_CONFIG, 0x01);
    }
    else {
        SPI_SDR_Write(module, REG_BIST_CONFIG, 0xC3);
    }
}

// *****************************************************************************
/* void SDR_BIST_Stop(uint8_t module)

  Summary:
    Stop SDR (AD9361) module BIST.

  Description:
    Stop SDR (AD9361) module BIST. Selected module is defined by 'module'
    input parameter.

  Remarks:
    None.
*/
void SDR_BIST_Stop(uint8_t module) {

    LOG_FORMAT(info, "SDR (AD9361) BIST stopped (%s)", __func__);

    SPI_SDR_Write(module, REG_OBSERVE_CONFIG, 0x00);

    SPI_SDR_Write(module, REG_BIST_AND_DATA_PORT_TEST_CONFIG, 0x00);

    SPI_SDR_Write(module, REG_BIST_CONFIG, 0x00);
}

// *****************************************************************************
/* void SDR_TXRX_LO_Test(uint8_t module, uint32_t rx_lo_offset, bool bist_mode)

  Summary:
    SDR (AD9361) module LO TX frequencies frequencies tests.

  Description:
    Test SDR (AD9361) module LO TX frequencies. Selected module is defined by
    'module' input parameter.

  Remarks:
    None.
*/
void SDR_TXRX_LO_Test(uint8_t module, uint32_t rx_lo_offset, bool bist_mode) {

    // TX LO frequency plan
    uint64_t tx_lo_frequency_table[6] = {530000000UL,  2270000000UL, 2470000000UL, //
                                         5293500000UL, 5495000000UL, 5696500000UL};

    // RX LO frequency offset (wrt TX LO frequency offset)
    uint64_t rx_lo_frequency_offset = rx_lo_offset;

    // TX and RX LO frequency read-back value
    uint64_t tx_lo_frequency_rvalue = 0UL;
    uint64_t rx_lo_frequency_rvalue = 0UL;

    // tx attenuation
    uint32_t tx_attenuation = 0;

    // check module parameter is in allowed range
    if (module < SPI_SDR_NUM) {

        // test all frequency
        for (uint8_t i = 0; i < 6; i += 1) {

            // set TX and RX LO frequency
            ad9361_set_tx_lo_freq(&ad9361_phy[module], tx_lo_frequency_table[i]);
            ad9361_set_rx_lo_freq(&ad9361_phy[module], tx_lo_frequency_table[i] + rx_lo_frequency_offset);

            // read back PLL dividers values
            auto RFPLL_Dividers = SPI_SDR_Read(module, REG_RFPLL_DIVIDERS);
            LOG_FORMAT(debug, "RFPLL dividers = 0x%02X (%s)", RFPLL_Dividers, __func__);

            // DC offset calibration
            LOG_FORMAT(debug, "Start DC offset calibration... (%s)", __func__);
            ad9361_do_calib(&ad9361_phy[module], RFDC_CAL, -1);

            // TX quadrature calibration
            LOG_FORMAT(debug, "Start TX quadrature calibration... (%s)", __func__);
            ad9361_do_calib(&ad9361_phy[module], TX_QUAD_CAL, -1);

            // get TX and RX LO frequency
            ad9361_get_tx_lo_freq(&ad9361_phy[module], &tx_lo_frequency_rvalue);
            ad9361_get_rx_lo_freq(&ad9361_phy[module], &rx_lo_frequency_rvalue);
            LOG_FORMAT(debug, "TX LO frequency = %" PRIu64 " [Hz] (%s)", tx_lo_frequency_rvalue, __func__);
            LOG_FORMAT(debug, "RX LO frequency = %" PRIu64 " [Hz] (%s)", rx_lo_frequency_rvalue, __func__);

            // read back received signal power
            auto rx_power = SPI_SDR_Read(module, REG_CH1_RX_FILTER_POWER);
            LOG_FORMAT(debug, "RX signal power = %d (0x%02X) (%s)", rx_power, rx_power, __func__);

            // start transmission
            LOG_FORMAT(debug, "Start transmission (%s)", __func__);
            if (bist_mode) {
                SDR_BIST_Start(module, false);
            }

            // read back received signal power
            rx_power = SPI_SDR_Read(module, REG_CH1_RX_FILTER_POWER);
            LOG_FORMAT(debug, "RX signal power = %d (0x%02X) (%s)", rx_power, rx_power, __func__);

            // get tx attenuation
            ad9361_get_tx_attenuation(&ad9361_phy[module], 0, &tx_attenuation);

            // increase tx attenuation by 3dB ( 3000 mdB )
            tx_attenuation += 3000;

            // set tx attenuation
            ad9361_set_tx_attenuation(&ad9361_phy[module], 0, tx_attenuation);
            LOG_FORMAT(debug, "TX attenuation +3dB (%s)", __func__);

            // read back received signal power
            rx_power = SPI_SDR_Read(module, REG_CH1_RX_FILTER_POWER);
            LOG_FORMAT(debug, "rx signal power = %d (0x%02X) (%s)", rx_power, rx_power, __func__);

            // stop transmission
            LOG_FORMAT(debug, "Stop transmission (%s)", __func__);
            if (bist_mode) {
                SDR_BIST_Stop(module);
            }

            // restore tx attenuation
            tx_attenuation -= 3000;
            ad9361_set_tx_attenuation(&ad9361_phy[module], 0, tx_attenuation);
        }
    }
}

// *****************************************************************************
/* void SDR_TX_Atten_Test(uint8_t module)

  Summary:
    Test SDR (AD9361) module TX attenuator.

  Description:
    Test SDR (AD9361) module TX attenuator. Selected module is defined by
    'module' input parameter.

  Remarks:
    None.
*/
void SDR_TX_Atten_Test(uint8_t module) {
    // TX attenuation [mdB]
    uint32_t tx_attenuation = 45000;

    while (1) {
        // set TX attenuation - channel 1
        ad9361_set_tx_attenuation(&ad9361_phy[module], 0, tx_attenuation);
        // update TX attenuation
        tx_attenuation -= 3000;
    }
}

/* *****************************************************************************
 End of File
 */
