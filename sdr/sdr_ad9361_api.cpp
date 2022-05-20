/* ************************************************************************** */
/** SDR AD9361 API (SPI based)

  @Company
    Airbus Italia S.p.A.

  @File Name
    sdr_ad9361_api.cpp

  @Summary
    SDR AD9361 API (SPI based)

  @Description
    AD9361 API (SPI based).
                                                                              */
/* ************************************************************************** */

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

// STD libraries
#include "sdr_ad9361_api.hpp" // SDR AD9361 API

#include <cstring> // memset

// project libraries
#include "GLogger.hpp"
#include "sdr_ad9361.hpp" // SDR AD9361 driver
#include "sdr_if.hpp"     // SDR control (SPI interface)
#include "spi_if.hpp"     // SPI interface API

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

// WARNING: unused global variables
// ad9361_rf_phy_t _phy;
// clk_t           _clk_refin;

// *****************************************************************************
// *****************************************************************************
// Section: Local Functions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

/**
 * Initialize the AD9361 part.
 * @param ad9361_phy ...
 * @param init_param The structure that contains the AD9361 initial parameters.
 * @return A structure that contains the AD9361 current state in case of success, negative error code otherwise.
 */
int32_t ad9361_init(ad9361_rf_phy_t*          phy, //
                    ad9361_init_parameters_t* init_param) {

    memset(phy, 0, sizeof(ad9361_rf_phy_t));

    // identification number
    phy->id_no = init_param->id_no;

    // reference clock
    phy->clk_refin.rate = init_param->reference_clk_rate;

    // base configuration
    phy->pdata.fdd                        = init_param->frequency_division_duplex_mode_enable != 0;
    phy->pdata.fdd_independent_mode       = init_param->frequency_division_duplex_independent_mode_enable != 0;
    phy->pdata.rx2tx2                     = init_param->two_rx_two_tx_mode_enable != 0;
    phy->pdata.tdd_use_dual_synth         = init_param->tdd_use_dual_synth_mode_enable != 0;
    phy->pdata.tdd_skip_vco_cal           = init_param->tdd_skip_vco_cal_enable != 0;
    phy->pdata.rx_fastlock_delay_ns       = init_param->rx_fastlock_delay_ns;
    phy->pdata.tx_fastlock_delay_ns       = init_param->tx_fastlock_delay_ns;
    phy->pdata.trx_fastlock_pinctrl_en[0] = init_param->rx_fastlock_pincontrol_enable != 0;
    phy->pdata.trx_fastlock_pinctrl_en[1] = init_param->tx_fastlock_pincontrol_enable != 0;
    phy->pdata.use_ext_rx_lo              = init_param->external_rx_lo_enable != 0;
    phy->pdata.use_ext_tx_lo              = init_param->external_tx_lo_enable != 0;
    phy->pdata.dc_offset_update_events    = init_param->dc_offset_tracking_update_event_mask;
    phy->pdata.dc_offset_attenuation_high = init_param->dc_offset_attenuation_high_range;
    phy->pdata.dc_offset_attenuation_low  = init_param->dc_offset_attenuation_low_range;
    phy->pdata.rf_dc_offset_count_high    = init_param->dc_offset_count_high_range;
    phy->pdata.rf_dc_offset_count_low     = init_param->dc_offset_count_low_range;
    phy->pdata.tdd_use_fdd_tables         = init_param->tdd_use_fdd_vco_tables_enable != 0;
    phy->pdata.split_gt                   = init_param->split_gain_table_mode_enable != 0;
    phy->pdata.trx_synth_max_fref         = init_param->trx_synthesizer_target_fref_overwrite_hz;
    phy->pdata.qec_tracking_slow_mode_en  = init_param->qec_tracking_slow_mode_enable != 0;

    // ENSM control
    phy->pdata.ensm_pin_pulse_mode = init_param->ensm_enable_pin_pulse_mode_enable != 0;
    phy->pdata.ensm_pin_ctrl       = init_param->ensm_enable_txnrx_control_enable != 0;

    // LO control
    phy->pdata.rx_synth_freq = init_param->rx_synthesizer_frequency_hz;
    phy->pdata.tx_synth_freq = init_param->tx_synthesizer_frequency_hz;

    // rate & BW control
    for (auto i{0}; i < 6; ++i) {
        phy->pdata.rx_path_clks[i] = init_param->rx_path_clock_frequencies[i];
    }

    for (auto i{0}; i < 6; ++i) {
        phy->pdata.tx_path_clks[i] = init_param->tx_path_clock_frequencies[i];
    }
    phy->pdata.rf_rx_bandwidth_Hz = init_param->rf_rx_bandwidth_hz;
    phy->pdata.rf_tx_bandwidth_Hz = init_param->rf_tx_bandwidth_hz;

    // RF port control
    phy->pdata.rf_rx_input_sel  = init_param->rx_rf_port_input_select;
    phy->pdata.rf_tx_output_sel = init_param->tx_rf_port_input_select;

    // TX attenuation control
    phy->pdata.tx_atten                 = init_param->tx_attenuation_mdB;
    phy->pdata.update_tx_gain_via_alert = init_param->update_tx_gain_in_alert_enable != 0;

    // reference clock control
    phy->pdata.use_extclk         = init_param->xo_disable_use_ext_refclk_enable != 0;
    phy->pdata.dcxo_coarse        = init_param->dcxo_coarse_and_fine_tune[0];
    phy->pdata.dcxo_fine          = init_param->dcxo_coarse_and_fine_tune[1];
    phy->pdata.ad9361_clkout_mode = (ad9361_clkout_t)init_param->clk_output_mode_select;

    // gain control
    phy->pdata.gain_ctrl.rx1_mode                    = (rf_gain_ctrl_mode_t)init_param->gc_rx1_mode;
    phy->pdata.gain_ctrl.rx2_mode                    = (rf_gain_ctrl_mode_t)init_param->gc_rx2_mode;
    phy->pdata.gain_ctrl.adc_large_overload_thresh   = init_param->gc_adc_large_overload_thresh;
    phy->pdata.gain_ctrl.adc_ovr_sample_size         = init_param->gc_adc_ovr_sample_size;
    phy->pdata.gain_ctrl.adc_small_overload_thresh   = init_param->gc_adc_small_overload_thresh;
    phy->pdata.gain_ctrl.dec_pow_measuremnt_duration = init_param->gc_dec_pow_measurement_duration;
    phy->pdata.gain_ctrl.dig_gain_en                 = init_param->gc_dig_gain_enable != 0;
    phy->pdata.gain_ctrl.lmt_overload_high_thresh    = init_param->gc_lmt_overload_high_thresh;
    phy->pdata.gain_ctrl.lmt_overload_low_thresh     = init_param->gc_lmt_overload_low_thresh;
    phy->pdata.gain_ctrl.low_power_thresh            = init_param->gc_low_power_thresh;
    phy->pdata.gain_ctrl.max_dig_gain                = init_param->gc_max_dig_gain;

    // MGC control
    phy->pdata.gain_ctrl.mgc_dec_gain_step                  = init_param->mgc_dec_gain_step;
    phy->pdata.gain_ctrl.mgc_inc_gain_step                  = init_param->mgc_inc_gain_step;
    phy->pdata.gain_ctrl.mgc_rx1_ctrl_inp_en                = init_param->mgc_rx1_ctrl_inp_enable != 0;
    phy->pdata.gain_ctrl.mgc_rx2_ctrl_inp_en                = init_param->mgc_rx2_ctrl_inp_enable != 0;
    phy->pdata.gain_ctrl.mgc_split_table_ctrl_inp_gain_mode = init_param->mgc_split_table_ctrl_inp_gain_mode;

    // AGC control
    phy->pdata.gain_ctrl.adc_large_overload_exceed_counter       = init_param->agc_adc_large_overload_exceed_counter;
    phy->pdata.gain_ctrl.adc_large_overload_inc_steps            = init_param->agc_adc_large_overload_inc_steps;
    phy->pdata.gain_ctrl.adc_lmt_small_overload_prevent_gain_inc = init_param->agc_adc_lmt_small_overload_prevent_gain_inc_enable != 0;
    phy->pdata.gain_ctrl.adc_small_overload_exceed_counter       = init_param->agc_adc_small_overload_exceed_counter;
    phy->pdata.gain_ctrl.dig_gain_step_size                      = init_param->agc_dig_gain_step_size;
    phy->pdata.gain_ctrl.dig_saturation_exceed_counter           = init_param->agc_dig_saturation_exceed_counter;
    phy->pdata.gain_ctrl.gain_update_interval_us                 = init_param->agc_gain_update_interval_us;
    phy->pdata.gain_ctrl.immed_gain_change_if_large_adc_overload = init_param->agc_immed_gain_change_if_large_adc_overload_enable != 0;
    phy->pdata.gain_ctrl.immed_gain_change_if_large_lmt_overload = init_param->agc_immed_gain_change_if_large_lmt_overload_enable != 0;
    phy->pdata.gain_ctrl.agc_inner_thresh_high                   = init_param->agc_inner_thresh_high;
    phy->pdata.gain_ctrl.agc_inner_thresh_high_dec_steps         = init_param->agc_inner_thresh_high_dec_steps;
    phy->pdata.gain_ctrl.agc_inner_thresh_low                    = init_param->agc_inner_thresh_low;
    phy->pdata.gain_ctrl.agc_inner_thresh_low_inc_steps          = init_param->agc_inner_thresh_low_inc_steps;
    phy->pdata.gain_ctrl.lmt_overload_large_exceed_counter       = init_param->agc_lmt_overload_large_exceed_counter;
    phy->pdata.gain_ctrl.lmt_overload_large_inc_steps            = init_param->agc_lmt_overload_large_inc_steps;
    phy->pdata.gain_ctrl.lmt_overload_small_exceed_counter       = init_param->agc_lmt_overload_small_exceed_counter;
    phy->pdata.gain_ctrl.agc_outer_thresh_high                   = init_param->agc_outer_thresh_high;
    phy->pdata.gain_ctrl.agc_outer_thresh_high_dec_steps         = init_param->agc_outer_thresh_high_dec_steps;
    phy->pdata.gain_ctrl.agc_outer_thresh_low                    = init_param->agc_outer_thresh_low;
    phy->pdata.gain_ctrl.agc_outer_thresh_low_inc_steps          = init_param->agc_outer_thresh_low_inc_steps;
    phy->pdata.gain_ctrl.agc_attack_delay_extra_margin_us        = init_param->agc_attack_delay_extra_margin_us;
    phy->pdata.gain_ctrl.sync_for_gain_counter_en                = init_param->agc_sync_for_gain_counter_enable != 0;

    // fast AGC
    phy->pdata.gain_ctrl.f_agc_dec_pow_measuremnt_duration = init_param->fagc_dec_pow_measuremnt_duration;
    phy->pdata.gain_ctrl.f_agc_state_wait_time_ns          = init_param->fagc_state_wait_time_ns;

    // fast AGC - low power
    phy->pdata.gain_ctrl.f_agc_allow_agc_gain_increase   = init_param->fagc_allow_agc_gain_increase != 0;
    phy->pdata.gain_ctrl.f_agc_lp_thresh_increment_time  = init_param->fagc_lp_thresh_increment_time;
    phy->pdata.gain_ctrl.f_agc_lp_thresh_increment_steps = init_param->fagc_lp_thresh_increment_steps;

    // fast AGC - lock level
    phy->pdata.gain_ctrl.f_agc_lock_level                           = init_param->fagc_lock_level;
    phy->pdata.gain_ctrl.f_agc_lock_level_lmt_gain_increase_en      = init_param->fagc_lock_level_lmt_gain_increase_en != 0;
    phy->pdata.gain_ctrl.f_agc_lock_level_gain_increase_upper_limit = init_param->fagc_lock_level_gain_increase_upper_limit;

    // fast AGC - peak detectors and final settling
    phy->pdata.gain_ctrl.f_agc_lpf_final_settling_steps = init_param->fagc_lpf_final_settling_steps;
    phy->pdata.gain_ctrl.f_agc_lmt_final_settling_steps = init_param->fagc_lmt_final_settling_steps;
    phy->pdata.gain_ctrl.f_agc_final_overrange_count    = init_param->fagc_final_overrange_count;

    // fast AGC - final power test
    phy->pdata.gain_ctrl.f_agc_gain_increase_after_gain_lock_en = init_param->fagc_gain_increase_after_gain_lock_en != 0;

    // fast AGC - unlocking the gain
    phy->pdata.gain_ctrl.f_agc_gain_index_type_after_exit_rx_mode          = (f_agc_target_gain_index_t)init_param->fagc_gain_index_type_after_exit_rx_mode;
    phy->pdata.gain_ctrl.f_agc_use_last_lock_level_for_set_gain_en         = init_param->fagc_use_last_lock_level_for_set_gain_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_stronger_sig_thresh_exceeded_en     = init_param->fagc_rst_gla_stronger_sig_thresh_exceeded_en != 0;
    phy->pdata.gain_ctrl.f_agc_optimized_gain_offset                       = init_param->fagc_optimized_gain_offset;
    phy->pdata.gain_ctrl.f_agc_rst_gla_stronger_sig_thresh_above_ll        = init_param->fagc_rst_gla_stronger_sig_thresh_above_ll;
    phy->pdata.gain_ctrl.f_agc_rst_gla_engergy_lost_sig_thresh_exceeded_en = init_param->fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_engergy_lost_goto_optim_gain_en     = init_param->fagc_rst_gla_engergy_lost_goto_optim_gain_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_engergy_lost_sig_thresh_below_ll    = init_param->fagc_rst_gla_engergy_lost_sig_thresh_below_ll;
    phy->pdata.gain_ctrl.f_agc_energy_lost_stronger_sig_gain_lock_exit_cnt = init_param->fagc_energy_lost_stronger_sig_gain_lock_exit_cnt;
    phy->pdata.gain_ctrl.f_agc_rst_gla_large_adc_overload_en               = init_param->fagc_rst_gla_large_adc_overload_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_large_lmt_overload_en               = init_param->fagc_rst_gla_large_lmt_overload_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_en_agc_pulled_high_en               = init_param->fagc_rst_gla_en_agc_pulled_high_en != 0;
    phy->pdata.gain_ctrl.f_agc_rst_gla_if_en_agc_pulled_high_mode          = (f_agc_target_gain_index_t)init_param->fagc_rst_gla_if_en_agc_pulled_high_mode;
    phy->pdata.gain_ctrl.f_agc_power_measurement_duration_in_state5        = init_param->fagc_power_measurement_duration_in_state5;

    // RSSI control
    phy->pdata.rssi_ctrl.rssi_delay              = init_param->rssi_delay;
    phy->pdata.rssi_ctrl.rssi_duration           = init_param->rssi_duration;
    phy->pdata.rssi_ctrl.restart_mode            = (rssi_restart_mode_t)init_param->rssi_restart_mode;
    phy->pdata.rssi_ctrl.rssi_unit_is_rx_samples = init_param->rssi_unit_is_rx_samples_enable != 0;
    phy->pdata.rssi_ctrl.rssi_wait               = init_param->rssi_wait;

    // aux ADC control
    phy->pdata.auxadc_ctrl.auxadc_decimation = init_param->aux_adc_decimation;
    phy->pdata.auxadc_ctrl.auxadc_clock_rate = init_param->aux_adc_rate;

    // aux DAC control
    phy->pdata.auxdac_ctrl.auxdac_manual_mode_en = init_param->aux_dac_manual_mode_enable != 0;
    phy->pdata.auxdac_ctrl.dac1_default_value    = init_param->aux_dac1_default_value_mV;
    phy->pdata.auxdac_ctrl.dac1_in_rx_en         = init_param->aux_dac1_active_in_rx_enable != 0;
    phy->pdata.auxdac_ctrl.dac1_in_tx_en         = init_param->aux_dac1_active_in_tx_enable != 0;
    phy->pdata.auxdac_ctrl.dac1_in_alert_en      = init_param->aux_dac1_active_in_alert_enable != 0;
    phy->pdata.auxdac_ctrl.dac1_rx_delay_us      = init_param->aux_dac1_rx_delay_us;
    phy->pdata.auxdac_ctrl.dac1_tx_delay_us      = init_param->aux_dac1_tx_delay_us;
    phy->pdata.auxdac_ctrl.dac2_default_value    = init_param->aux_dac2_default_value_mV;
    phy->pdata.auxdac_ctrl.dac2_in_rx_en         = init_param->aux_dac2_active_in_rx_enable != 0;
    phy->pdata.auxdac_ctrl.dac2_in_tx_en         = init_param->aux_dac2_active_in_tx_enable != 0;
    phy->pdata.auxdac_ctrl.dac2_in_alert_en      = init_param->aux_dac2_active_in_alert_enable != 0;
    phy->pdata.auxdac_ctrl.dac2_rx_delay_us      = init_param->aux_dac2_rx_delay_us;
    phy->pdata.auxdac_ctrl.dac2_tx_delay_us      = init_param->aux_dac2_tx_delay_us;

    // temperature sensor control
    phy->pdata.auxadc_ctrl.temp_sensor_decimation   = init_param->temp_sense_decimation;
    phy->pdata.auxadc_ctrl.temp_time_inteval_ms     = init_param->temp_sense_measurement_interval_ms;
    phy->pdata.auxadc_ctrl.offset                   = init_param->temp_sense_offset_signed;
    phy->pdata.auxadc_ctrl.periodic_temp_measuremnt = init_param->temp_sense_periodic_measurement_enable != 0;

    // control out setup
    phy->pdata.ctrl_outs_ctrl.en_mask = init_param->ctrl_outs_enable_mask;
    phy->pdata.ctrl_outs_ctrl.index   = init_param->ctrl_outs_index;

    // external LNA control
    phy->pdata.elna_ctrl.settling_delay_ns = init_param->elna_settling_delay_ns;
    phy->pdata.elna_ctrl.gain_mdB          = init_param->elna_gain_mdB;
    phy->pdata.elna_ctrl.bypass_loss_mdB   = init_param->elna_bypass_loss_mdB;
    phy->pdata.elna_ctrl.elna_1_control_en = init_param->elna_rx1_gpo0_control_enable != 0;
    phy->pdata.elna_ctrl.elna_2_control_en = init_param->elna_rx2_gpo1_control_enable != 0;

    // digital interface control
    phy->pdata.dig_interface_tune_skipmode = (init_param->digital_interface_tune_skip_mode);
    phy->pdata.port_ctrl.pp_conf[0]        = (init_param->pp_tx_swap_enable << 7);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->pp_rx_swap_enable << 6);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->tx_channel_swap_enable << 5);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->rx_channel_swap_enable << 4);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->rx_frame_pulse_mode_enable << 3);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->two_t_two_r_timing_enable << 2);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->invert_data_bus_enable << 1);
    phy->pdata.port_ctrl.pp_conf[0] |= (init_param->invert_data_clk_enable << 0);
    phy->pdata.port_ctrl.pp_conf[1] = (init_param->fdd_alt_word_order_enable << 7);
    phy->pdata.port_ctrl.pp_conf[1] |= (init_param->invert_rx_frame_enable << 2);
    phy->pdata.port_ctrl.pp_conf[2] = (init_param->fdd_rx_rate_2tx_enable << 7);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->swap_ports_enable << 6);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->single_data_rate_enable << 5);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->lvds_mode_enable << 4);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->half_duplex_mode_enable << 3);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->single_port_mode_enable << 2);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->full_port_enable << 1);
    phy->pdata.port_ctrl.pp_conf[2] |= (init_param->full_duplex_swap_bits_enable << 0);
    phy->pdata.port_ctrl.pp_conf[1] |= (init_param->delay_rx_data & 0x03);
    phy->pdata.port_ctrl.rx_clk_data_delay = DATA_CLK_DELAY(init_param->rx_data_clock_delay);
    phy->pdata.port_ctrl.rx_clk_data_delay |= RX_DATA_DELAY(init_param->rx_data_delay);
    phy->pdata.port_ctrl.tx_clk_data_delay = FB_CLK_DELAY(init_param->tx_fb_clock_delay);
    phy->pdata.port_ctrl.tx_clk_data_delay |= TX_DATA_DELAY(init_param->tx_data_delay);
    phy->pdata.port_ctrl.lvds_bias_ctrl = (init_param->lvds_bias_mV / 75) & 0x07;
    phy->pdata.port_ctrl.lvds_bias_ctrl |= (init_param->lvds_rx_onchip_termination_enable << 5);
    phy->pdata.rx1rx2_phase_inversion_en = init_param->rx1rx2_phase_inversion_en != 0;

    // GPO control
    phy->pdata.gpo_ctrl.gpo0_inactive_state_high_en = init_param->gpo0_inactive_state_high_enable != 0;
    phy->pdata.gpo_ctrl.gpo1_inactive_state_high_en = init_param->gpo1_inactive_state_high_enable != 0;
    phy->pdata.gpo_ctrl.gpo2_inactive_state_high_en = init_param->gpo2_inactive_state_high_enable != 0;
    phy->pdata.gpo_ctrl.gpo3_inactive_state_high_en = init_param->gpo3_inactive_state_high_enable != 0;

    phy->pdata.gpo_ctrl.gpo0_slave_rx_en = init_param->gpo0_slave_rx_enable != 0;
    phy->pdata.gpo_ctrl.gpo0_slave_tx_en = init_param->gpo0_slave_tx_enable != 0;
    phy->pdata.gpo_ctrl.gpo1_slave_rx_en = init_param->gpo1_slave_rx_enable != 0;
    phy->pdata.gpo_ctrl.gpo1_slave_tx_en = init_param->gpo1_slave_tx_enable != 0;
    phy->pdata.gpo_ctrl.gpo2_slave_rx_en = init_param->gpo2_slave_rx_enable != 0;
    phy->pdata.gpo_ctrl.gpo2_slave_tx_en = init_param->gpo2_slave_tx_enable != 0;
    phy->pdata.gpo_ctrl.gpo3_slave_rx_en = init_param->gpo3_slave_rx_enable != 0;
    phy->pdata.gpo_ctrl.gpo3_slave_tx_en = init_param->gpo3_slave_tx_enable != 0;

    phy->pdata.gpo_ctrl.gpo0_rx_delay_us = init_param->gpo0_rx_delay_us;
    phy->pdata.gpo_ctrl.gpo0_tx_delay_us = init_param->gpo0_tx_delay_us;
    phy->pdata.gpo_ctrl.gpo1_rx_delay_us = init_param->gpo1_rx_delay_us;
    phy->pdata.gpo_ctrl.gpo1_tx_delay_us = init_param->gpo1_tx_delay_us;
    phy->pdata.gpo_ctrl.gpo2_rx_delay_us = init_param->gpo2_rx_delay_us;
    phy->pdata.gpo_ctrl.gpo2_tx_delay_us = init_param->gpo2_tx_delay_us;
    phy->pdata.gpo_ctrl.gpo3_rx_delay_us = init_param->gpo3_rx_delay_us;
    phy->pdata.gpo_ctrl.gpo3_tx_delay_us = init_param->gpo3_tx_delay_us;

    // TX monitor control
    phy->pdata.txmon_ctrl.low_high_gain_threshold_mdB = init_param->low_high_gain_threshold_mdB;
    phy->pdata.txmon_ctrl.low_gain_dB                 = init_param->low_gain_dB;
    phy->pdata.txmon_ctrl.high_gain_dB                = init_param->high_gain_dB;
    phy->pdata.txmon_ctrl.tx_mon_track_en             = init_param->tx_mon_track_en != 0;
    phy->pdata.txmon_ctrl.one_shot_mode_en            = init_param->one_shot_mode_en != 0;
    phy->pdata.txmon_ctrl.tx_mon_delay                = init_param->tx_mon_delay;
    phy->pdata.txmon_ctrl.tx_mon_duration             = init_param->tx_mon_duration;
    phy->pdata.txmon_ctrl.tx1_mon_front_end_gain      = init_param->tx1_mon_front_end_gain;
    phy->pdata.txmon_ctrl.tx2_mon_front_end_gain      = init_param->tx2_mon_front_end_gain;
    phy->pdata.txmon_ctrl.tx1_mon_lo_cm               = init_param->tx1_mon_lo_cm;
    phy->pdata.txmon_ctrl.tx2_mon_lo_cm               = init_param->tx2_mon_lo_cm;

    phy->pdata.debug_mode = true;

    phy->pdata.port_ctrl.digital_io_ctrl = 0;
    phy->pdata.port_ctrl.lvds_invert[0]  = init_param->lvds_invert1_control;
    phy->pdata.port_ctrl.lvds_invert[1]  = init_param->lvds_invert2_control;

    phy->rx_eq_2tx = false;

    phy->current_table = RXGAIN_TBLS_END;
    phy->bypass_tx_fir = true;
    phy->bypass_rx_fir = true;
    phy->rate_governor = 1;
    phy->rfdc_track_en = true;
    phy->bbdc_track_en = true;
    phy->quad_track_en = true;

    phy->bist_loopback_mode = 0;
    phy->bist_prbs_mode     = BIST_DISABLE;
    phy->bist_tone_mode     = BIST_DISABLE;
    phy->bist_tone_freq_Hz  = 0;
    phy->bist_tone_level_dB = 0;
    phy->bist_tone_mask     = 0;

    SDR_Reset(phy->id_no);

    SDR_SoftReset(phy->id_no);

    int32_t _ret;
    int32_t _rev;

    _ret = SPI_SDR_Read(phy->id_no, REG_PRODUCT_ID);
    if ((_ret & PRODUCT_ID_MASK) != PRODUCT_ID_9361) {
        LOG_FORMAT(error, "Unsupported PRODUCT_ID 0x%02X for SDR %d (%s)", _ret, phy->id_no, __func__);
        _ret = -ENODEV;
        goto out;
    }

    // get revision number
    _rev = _ret & REV_MASK;

    _ret = register_clocks(phy);
    if (_ret < 0) {
        goto out;
    }

    ad9361_init_gain_tables(phy);

    // AD9361 devices setup
    _ret = ad9361_setup(phy);
    if (_ret < 0) {
        goto out;
    }

    _ret = ad9361_post_setup(phy);
    if (_ret < 0) {
        goto out;
    }

    LOG_FORMAT(info, "AD9361 Rev %d successfully initialized (%s)", _rev, __func__);
    return 0;

out:
    LOG_FORMAT(error, "AD9361 initialization error (%s)", __func__);
    return -ENODEV;
}

/**
 * Set the RX LO frequency.
 * @param phy The AD9361 current state structure.
 * @param lo_freq_hz The desired frequency (Hz).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_rx_lo_freq(ad9361_rf_phy_t* phy, //
                              uint64_t         lo_freq_hz) {

    return clk_set_rate(phy, &(phy->ref_clk_scale[RX_RFPLL]), ad9361_to_clk(lo_freq_hz));
}

/**
 * Get current RX LO frequency.
 * @param phy The AD9361 current state structure.
 * @param lo_freq_hz A variable to store the frequency value (Hz).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_get_rx_lo_freq(ad9361_rf_phy_t* phy, //
                              uint64_t*        lo_freq_hz) {

    *lo_freq_hz = ad9361_from_clk(clk_get_rate(phy, &(phy->ref_clk_scale[RX_RFPLL])));

    return 0;
}

/**
 * Set the RX FIR filter configuration.
 * @param phy The AD9361 current state structure.
 * @param fir_cfg FIR filter configuration.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_rx_fir_config(ad9361_rf_phy_t*       phy, //
                                 ad9361_rx_fir_config_t fir_cfg) {

    int32_t _ret = ad9361_load_fir_filter_coef(phy, (fir_dest_t)(fir_cfg.rx | FIR_IS_RX), fir_cfg.rx_gain, fir_cfg.rx_coef_size, fir_cfg.rx_coef);

    phy->rx_fir_dec = fir_cfg.rx_dec;

    return _ret;
}

/**
 * Set the transmit attenuation for the selected channel.
 * @param phy The AD9361 current state structure.
 * @param ch The desired channel number (0, 1).
 * @param attenuation_mdb The attenuation (mdB).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_tx_attenuation(ad9361_rf_phy_t* phy, //
                                  uint8_t          ch,
                                  uint32_t         attenuation_mdb) {

    return ad9361_set_tx_atten(phy, attenuation_mdb, ch == 0, ch == 1, !phy->pdata.update_tx_gain_via_alert);
}

/**
 * Get current transmit attenuation for the selected channel.
 * @param phy The AD9361 current state structure.
 * @param ch The desired channel number (0, 1).
 * @param attenuation_mdb A variable to store the attenuation value (mdB).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_get_tx_attenuation(ad9361_rf_phy_t* phy, //
                                  uint8_t          ch,
                                  uint32_t*        attenuation_mdb) {

    *attenuation_mdb = (uint32_t)ad9361_get_tx_atten(phy, ch + 1);

    return (int32_t)*attenuation_mdb > 0 ? 0 : -1;
}

/**
 * Set the TX LO frequency.
 * @param phy The AD9361 current state structure.
 * @param lo_freq_hz The desired frequency (Hz).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_tx_lo_freq(ad9361_rf_phy_t* phy, //
                              uint64_t         lo_freq_hz) {

    return clk_set_rate(phy, &(phy->ref_clk_scale[TX_RFPLL]), ad9361_to_clk(lo_freq_hz));
}

/**
 * Get current TX LO frequency.
 * @param phy The AD9361 current state structure.
 * @param lo_freq_hz A variable to store the frequency value (Hz).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_get_tx_lo_freq(ad9361_rf_phy_t* phy, //
                              uint64_t*        lo_freq_hz) {

    *lo_freq_hz = ad9361_from_clk(clk_get_rate(phy, &(phy->ref_clk_scale[TX_RFPLL])));

    return 0;
}

/**
 * Set the TX FIR filter configuration.
 * @param phy The AD9361 current state structure.
 * @param fir_cfg FIR filter configuration.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_set_tx_fir_config(ad9361_rf_phy_t*       phy, //
                                 ad9361_tx_fir_config_t fir_cfg) {

    auto _ret = ad9361_load_fir_filter_coef(phy, (fir_dest_t)fir_cfg.tx, fir_cfg.tx_gain, fir_cfg.tx_coef_size, fir_cfg.tx_coef);

    phy->tx_fir_int = fir_cfg.tx_int;

    return _ret;
}

/**
 * Perform the selected calibration.
 * @param phy The AD9361 state structure.
 * @param cal The selected calibration (TX_QUAD_CAL, RFDC_CAL).
 * @param arg For TX_QUAD_CAL - the optional RX phase value overwrite (set to zero).
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad9361_do_calib(ad9361_rf_phy_t* phy, //
                        uint32_t         cal,
                        int32_t          arg) {

    return ad9361_do_calib_run(phy, cal, arg);
}

/* *****************************************************************************
 End of File
 */
