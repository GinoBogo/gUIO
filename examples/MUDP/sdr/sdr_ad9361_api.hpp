/* ************************************************************************** */
/** SDR AD9361 API (SPI based)

  @Company
    Airbus Italia S.p.A.

  @File Name
    sdr_ad9361_api.hpp

  @Summary
    SDR AD9361 API (SPI based)

  @Description
    AD9361 API (SPI based).
                                                                              */
/* ************************************************************************** */

#ifndef SDR_AD9361_API_HPP
#define SDR_AD9361_API_HPP

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "sdr_ad9361.hpp"

#define sInt08 int8_t
#define sInt16 int16_t
#define sInt32 int32_t
#define sInt64 int64_t

#define uInt08 uint8_t
#define uInt16 uint16_t
#define uInt32 uint32_t
#define uInt64 uint64_t

// *****************************************************************************
// *****************************************************************************
// Section: Data types
// *****************************************************************************
// *****************************************************************************

typedef struct ad9361_init_parameters {
    // identification number
    uInt08 id_no;

    // reference clock
    uInt32 reference_clk_rate;

    // base configuration
    uInt08 two_rx_two_tx_mode_enable;                         // 2rx-2tx-mode-enable
    uInt08 frequency_division_duplex_mode_enable;             // frequency-division-duplex-mode-enable
    uInt08 frequency_division_duplex_independent_mode_enable; // frequency-division-duplex-independent-mode-enable
    uInt08 tdd_use_dual_synth_mode_enable;                    // tdd-use-dual-synth-mode-enable
    uInt08 tdd_skip_vco_cal_enable;                           // tdd-skip-vco-cal-enable
    uInt32 tx_fastlock_delay_ns;                              // tx-fastlock-delay-us
    uInt32 rx_fastlock_delay_ns;                              // rx-fastlock-delay-ns
    uInt08 rx_fastlock_pincontrol_enable;                     // rx-fastlock-pincontrol-enable
    uInt08 tx_fastlock_pincontrol_enable;                     // tx-fastlock-pincontrol-enable
    uInt08 external_rx_lo_enable;                             // external-rx-lo-enable
    uInt08 external_tx_lo_enable;                             // external-tx-lo-enable
    uInt08 dc_offset_tracking_update_event_mask;              // dc-offset-tracking-update-event-mask
    uInt08 dc_offset_attenuation_high_range;                  // dc-offset-attenuation-high-range
    uInt08 dc_offset_attenuation_low_range;                   // dc-offset-attenuation-low-range
    uInt08 dc_offset_count_high_range;                        // dc-offset-count-high-range
    uInt08 dc_offset_count_low_range;                         // dc-offset-count-low-range
    uInt08 tdd_use_fdd_vco_tables_enable;                     // tdd-use-fdd-vco-tables-enable
    uInt08 split_gain_table_mode_enable;                      // split-gain-table-mode-enable
    uInt32 trx_synthesizer_target_fref_overwrite_hz;          // trx-synthesizer-target-fref-overwrite-hz
    uInt08 qec_tracking_slow_mode_enable;                     // qec-tracking-slow-mode-enable

    // ENSM control
    uInt08 ensm_enable_pin_pulse_mode_enable; // ensm-enable-pin-pulse-mode-enable
    uInt08 ensm_enable_txnrx_control_enable;  // ensm-enable-txnrx-control-enable

    // LO control
    uInt64 rx_synthesizer_frequency_hz; // rx-synthesizer-frequency-hz
    uInt64 tx_synthesizer_frequency_hz; // tx-synthesizer-frequency-hz

    // rate & BW control
    uInt32 rx_path_clock_frequencies[6]; // rx-path-clock-frequencies
    uInt32 tx_path_clock_frequencies[6]; // tx-path-clock-frequencies
    uInt32 rf_rx_bandwidth_hz;           // rf-rx-bandwidth-hz
    uInt32 rf_tx_bandwidth_hz;           // rf-tx-bandwidth-hz

    // RF port control
    uInt32 rx_rf_port_input_select; // rx-rf-port-input-select
    uInt32 tx_rf_port_input_select; // tx-rf-port-input-select

    // TX attenuation control
    sInt32 tx_attenuation_mdB;             // tx-attenuation-mdB
    uInt08 update_tx_gain_in_alert_enable; // update-tx-gain-in-alert-enable

    // reference clock control
    uInt08 xo_disable_use_ext_refclk_enable; // xo-disable-use-ext-refclk-enable
    uInt32 dcxo_coarse_and_fine_tune[2];     // dcxo-coarse-and-fine-tune
    uInt32 clk_output_mode_select;           // clk-output-mode-select

    // gain control
    uInt08 gc_rx1_mode;                     // gc-rx1-mode
    uInt08 gc_rx2_mode;                     // gc-rx2-mode
    uInt08 gc_adc_large_overload_thresh;    // gc-adc-large-overload-thresh
    uInt08 gc_adc_ovr_sample_size;          // gc-adc-ovr-sample-size
    uInt08 gc_adc_small_overload_thresh;    // gc-adc-small-overload-thresh
    uInt16 gc_dec_pow_measurement_duration; // gc-dec-pow-measurement-duration
    uInt08 gc_dig_gain_enable;              // gc-dig-gain-enable
    uInt16 gc_lmt_overload_high_thresh;     // gc-lmt-overload-high-thresh
    uInt16 gc_lmt_overload_low_thresh;      // gc-lmt-overload-low-thresh
    uInt08 gc_low_power_thresh;             // gc-low-power-thresh
    uInt08 gc_max_dig_gain;                 // gc-max-dig-gain

    // MGC control
    uInt08 mgc_dec_gain_step;                  // mgc-dec-gain-step
    uInt08 mgc_inc_gain_step;                  // mgc-inc-gain-step
    uInt08 mgc_rx1_ctrl_inp_enable;            // mgc-rx1-ctrl-inp-enable
    uInt08 mgc_rx2_ctrl_inp_enable;            // mgc-rx2-ctrl-inp-enable
    uInt08 mgc_split_table_ctrl_inp_gain_mode; // mgc-split-table-ctrl-inp-gain-mode

    // AGC control
    uInt08 agc_adc_large_overload_exceed_counter;              // agc-adc-large-overload-exceed-counter
    uInt08 agc_adc_large_overload_inc_steps;                   // agc-adc-large-overload-inc-steps
    uInt08 agc_adc_lmt_small_overload_prevent_gain_inc_enable; // agc-adc-lmt-small-overload-prevent-gain-inc-enable
    uInt08 agc_adc_small_overload_exceed_counter;              // agc-adc-small-overload-exceed-counter
    uInt08 agc_dig_gain_step_size;                             // agc-dig-gain-step-size
    uInt08 agc_dig_saturation_exceed_counter;                  // agc-dig-saturation-exceed-counter
    uInt32 agc_gain_update_interval_us;                        // agc-gain-update-interval-us
    uInt08 agc_immed_gain_change_if_large_adc_overload_enable; // agc-immed-gain-change-if-large-adc-overload-enable
    uInt08 agc_immed_gain_change_if_large_lmt_overload_enable; // agc-immed-gain-change-if-large-lmt-overload-enable
    uInt08 agc_inner_thresh_high;                              // agc-inner-thresh-high
    uInt08 agc_inner_thresh_high_dec_steps;                    // agc-inner-thresh-high-dec-steps
    uInt08 agc_inner_thresh_low;                               // agc-inner-thresh-low
    uInt08 agc_inner_thresh_low_inc_steps;                     // agc-inner-thresh-low-inc-steps
    uInt08 agc_lmt_overload_large_exceed_counter;              // agc-lmt-overload-large-exceed-counter
    uInt08 agc_lmt_overload_large_inc_steps;                   // agc-lmt-overload-large-inc-steps
    uInt08 agc_lmt_overload_small_exceed_counter;              // agc-lmt-overload-small-exceed-counter
    uInt08 agc_outer_thresh_high;                              // agc-outer-thresh-high
    uInt08 agc_outer_thresh_high_dec_steps;                    // agc-outer-thresh-high-dec-steps
    uInt08 agc_outer_thresh_low;                               // agc-outer-thresh-low
    uInt08 agc_outer_thresh_low_inc_steps;                     // agc-outer-thresh-low-inc-steps
    uInt32 agc_attack_delay_extra_margin_us;                   // agc-attack-delay-extra-margin-us
    uInt08 agc_sync_for_gain_counter_enable;                   // agc-sync-for-gain-counter-enable

    // fast AGC
    uInt32 fagc_dec_pow_measuremnt_duration; // fagc-dec-pow-measurement-duration
    uInt32 fagc_state_wait_time_ns;          // fagc-state-wait-time-ns

    // fast AGC - low power
    uInt08 fagc_allow_agc_gain_increase;   // fagc-allow-agc-gain-increase-enable
    uInt32 fagc_lp_thresh_increment_time;  // fagc-lp-thresh-increment-time
    uInt32 fagc_lp_thresh_increment_steps; // fagc-lp-thresh-increment-steps

    // fast AGC - lock level
    uInt32 fagc_lock_level;                           // fagc-lock-level
    uInt08 fagc_lock_level_lmt_gain_increase_en;      // fagc-lock-level-lmt-gain-increase-enable
    uInt32 fagc_lock_level_gain_increase_upper_limit; // fagc-lock-level-gain-increase-upper-limit

    // fast AGC - peak detectors and final settling
    uInt32 fagc_lpf_final_settling_steps; // fagc-lpf-final-settling-steps
    uInt32 fagc_lmt_final_settling_steps; // fagc-lmt-final-settling-steps
    uInt32 fagc_final_overrange_count;    // fagc-final-overrange-count

    // fast AGC - final power test
    uInt08 fagc_gain_increase_after_gain_lock_en; // fagc-gain-increase-after-gain-lock-enable

    // fast AGC - unlocking the gain
    uInt32 fagc_gain_index_type_after_exit_rx_mode;          // fagc-gain-index-type-after-exit-rx-mode
    uInt08 fagc_use_last_lock_level_for_set_gain_en;         // fagc-use-last-lock-level-for-set-gain-enable
    uInt08 fagc_rst_gla_stronger_sig_thresh_exceeded_en;     // fagc-rst-gla-stronger-sig-thresh-exceeded-enable
    uInt32 fagc_optimized_gain_offset;                       // fagc-optimized-gain-offset
    uInt32 fagc_rst_gla_stronger_sig_thresh_above_ll;        // fagc-rst-gla-stronger-sig-thresh-above-ll
    uInt08 fagc_rst_gla_engergy_lost_sig_thresh_exceeded_en; // fagc-rst-gla-engergy-lost-sig-thresh-exceeded-enable
    uInt08 fagc_rst_gla_engergy_lost_goto_optim_gain_en;     // fagc-rst-gla-engergy-lost-goto-optim-gain-enable
    uInt32 fagc_rst_gla_engergy_lost_sig_thresh_below_ll;    // fagc-rst-gla-engergy-lost-sig-thresh-below-ll
    uInt32 fagc_energy_lost_stronger_sig_gain_lock_exit_cnt; // fagc-energy-lost-stronger-sig-gain-lock-exit-cnt
    uInt08 fagc_rst_gla_large_adc_overload_en;               // fagc-rst-gla-large-adc-overload-enable
    uInt08 fagc_rst_gla_large_lmt_overload_en;               // fagc-rst-gla-large-lmt-overload-enable
    uInt08 fagc_rst_gla_en_agc_pulled_high_en;               // fagc-rst-gla-en-agc-pulled-high-enable
    uInt32 fagc_rst_gla_if_en_agc_pulled_high_mode;          // fagc-rst-gla-if-en-agc-pulled-high-mode
    uInt32 fagc_power_measurement_duration_in_state5;        // fagc-power-measurement-duration-in-state5

    // RSSI control
    uInt32 rssi_delay;                     // rssi-delay
    uInt32 rssi_duration;                  // rssi-duration
    uInt08 rssi_restart_mode;              // rssi-restart-mode
    uInt08 rssi_unit_is_rx_samples_enable; // rssi-unit-is-rx-samples-enable
    uInt32 rssi_wait;                      // rssi-wait

    // aux ADC control
    uInt32 aux_adc_decimation; // aux-adc-decimation
    uInt32 aux_adc_rate;       // aux-adc-rate

    // aux DAC control
    uInt08 aux_dac_manual_mode_enable;      // aux-dac-manual-mode-enable
    uInt32 aux_dac1_default_value_mV;       // aux-dac1-default-value-mV
    uInt08 aux_dac1_active_in_rx_enable;    // aux-dac1-active-in-rx-enable
    uInt08 aux_dac1_active_in_tx_enable;    // aux-dac1-active-in-tx-enable
    uInt08 aux_dac1_active_in_alert_enable; // aux-dac1-active-in-alert-enable
    uInt32 aux_dac1_rx_delay_us;            // aux-dac1-rx-delay-us
    uInt32 aux_dac1_tx_delay_us;            // aux-dac1-tx-delay-us
    uInt32 aux_dac2_default_value_mV;       // aux-dac2-default-value-mV
    uInt08 aux_dac2_active_in_rx_enable;    // aux-dac2-active-in-rx-enable
    uInt08 aux_dac2_active_in_tx_enable;    // aux-dac2-active-in-tx-enable
    uInt08 aux_dac2_active_in_alert_enable; // aux-dac2-active-in-alert-enable
    uInt32 aux_dac2_rx_delay_us;            // aux-dac2-rx-delay-us
    uInt32 aux_dac2_tx_delay_us;            // aux-dac2-tx-delay-us

    // temperature sensor control
    uInt32 temp_sense_decimation;                  // temp-sense-decimation
    uInt16 temp_sense_measurement_interval_ms;     // temp-sense-measurement-interval-ms
    sInt08 temp_sense_offset_signed;               // temp-sense-offset-signed
    uInt08 temp_sense_periodic_measurement_enable; // temp-sense-periodic-measurement-enable

    // control out setup
    uInt08 ctrl_outs_enable_mask; // ctrl-outs-enable-mask
    uInt08 ctrl_outs_index;       // ctrl-outs-index

    // external LNA control
    uInt32 elna_settling_delay_ns;       // elna-settling-delay-ns
    uInt32 elna_gain_mdB;                // elna-gain-mdB
    uInt32 elna_bypass_loss_mdB;         // elna-bypass-loss-mdB
    uInt08 elna_rx1_gpo0_control_enable; // elna-rx1-gpo0-control-enable
    uInt08 elna_rx2_gpo1_control_enable; // elna-rx2-gpo1-control-enable

    // digital interface control
    uInt32 digital_interface_tune_skip_mode;  // digital-interface-tune-skip-mode
    uInt08 pp_tx_swap_enable;                 // pp-tx-swap-enable
    uInt08 pp_rx_swap_enable;                 // pp-rx-swap-enable
    uInt08 tx_channel_swap_enable;            // tx-channel-swap-enable
    uInt08 rx_channel_swap_enable;            // rx-channel-swap-enable
    uInt08 rx_frame_pulse_mode_enable;        // rx-frame-pulse-mode-enable
    uInt08 two_t_two_r_timing_enable;         // 2t2r-timing-enable
    uInt08 invert_data_bus_enable;            // invert-data-bus-enable
    uInt08 invert_data_clk_enable;            // invert-data-clk-enable
    uInt08 fdd_alt_word_order_enable;         // fdd-alt-word-order-enable
    uInt08 invert_rx_frame_enable;            // invert-rx-frame-enable
    uInt08 fdd_rx_rate_2tx_enable;            // fdd-rx-rate-2tx-enable
    uInt08 swap_ports_enable;                 // swap-ports-enable
    uInt08 single_data_rate_enable;           // single-data-rate-enable
    uInt08 lvds_mode_enable;                  // lvds-mode-enable
    uInt08 half_duplex_mode_enable;           // half-duplex-mode-enable
    uInt08 single_port_mode_enable;           // single-port-mode-enable
    uInt08 full_port_enable;                  // full-port-enable
    uInt08 full_duplex_swap_bits_enable;      // full-duplex-swap-bits-enable
    uInt32 delay_rx_data;                     // delay-rx-data
    uInt32 rx_data_clock_delay;               // rx-data-clock-delay
    uInt32 rx_data_delay;                     // rx-data-delay
    uInt32 tx_fb_clock_delay;                 // tx-fb-clock-delay
    uInt32 tx_data_delay;                     // tx-data-delay
    uInt32 lvds_bias_mV;                      // lvds-bias-mV
    uInt08 lvds_rx_onchip_termination_enable; // lvds-rx-onchip-termination-enable
    uInt08 rx1rx2_phase_inversion_en;         // rx1-rx2-phase-inversion-enable
    uInt08 lvds_invert1_control;              // lvds-invert1-control
    uInt08 lvds_invert2_control;              // lvds-invert2-control

    // GPO control
    uInt08 gpo0_inactive_state_high_enable; // gpo0-inactive-state-high-enable
    uInt08 gpo1_inactive_state_high_enable; // gpo1-inactive-state-high-enable
    uInt08 gpo2_inactive_state_high_enable; // gpo2-inactive-state-high-enable
    uInt08 gpo3_inactive_state_high_enable; // gpo3-inactive-state-high-enable
    uInt08 gpo0_slave_rx_enable;            // gpo0-slave-rx-enable
    uInt08 gpo0_slave_tx_enable;            // gpo0-slave-tx-enable
    uInt08 gpo1_slave_rx_enable;            // gpo1-slave-rx-enable
    uInt08 gpo1_slave_tx_enable;            // gpo1-slave-tx-enable
    uInt08 gpo2_slave_rx_enable;            // gpo2-slave-rx-enable
    uInt08 gpo2_slave_tx_enable;            // gpo2-slave-tx-enable
    uInt08 gpo3_slave_rx_enable;            // gpo3-slave-rx-enable
    uInt08 gpo3_slave_tx_enable;            // gpo3-slave-tx-enable
    uInt08 gpo0_rx_delay_us;                // gpo0-rx-delay-us
    uInt08 gpo0_tx_delay_us;                // gpo0-tx-delay-us
    uInt08 gpo1_rx_delay_us;                // gpo1-rx-delay-us
    uInt08 gpo1_tx_delay_us;                // gpo1-tx-delay-us
    uInt08 gpo2_rx_delay_us;                // gpo2-rx-delay-us
    uInt08 gpo2_tx_delay_us;                // gpo2-tx-delay-us
    uInt08 gpo3_rx_delay_us;                // gpo3-rx-delay-us
    uInt08 gpo3_tx_delay_us;                // gpo3-tx-delay-us

    // TX monitor control
    uInt32 low_high_gain_threshold_mdB; // txmon-low-high-thresh
    uInt32 low_gain_dB;                 // txmon-low-gain
    uInt32 high_gain_dB;                // txmon-high-gain
    uInt08 tx_mon_track_en;             // txmon-dc-tracking-enable
    uInt08 one_shot_mode_en;            // txmon-one-shot-mode-enable
    uInt32 tx_mon_delay;                // txmon-delay
    uInt32 tx_mon_duration;             // txmon-duration
    uInt32 tx1_mon_front_end_gain;      // txmon-1-front-end-gain
    uInt32 tx2_mon_front_end_gain;      // txmon-2-front-end-gain
    uInt32 tx1_mon_lo_cm;               // txmon-1-lo-cm
    uInt32 tx2_mon_lo_cm;               // txmon-2-lo-cm

} ad9361_init_parameters_t;

typedef struct ad9361_rx_fir_config {
    uInt32 rx;      // 1, 2, 3(both)
    sInt32 rx_gain; // -12, -6, 0, 6
    uInt32 rx_dec;  // 1, 2, 4
    sInt16 rx_coef[128];
    uInt08 rx_coef_size;

} ad9361_rx_fir_config_t;

typedef struct ad9361_tx_fir_config {
    uInt32 tx;      // 1, 2, 3(both)
    sInt32 tx_gain; // -6, 0
    uInt32 tx_int;  // 1, 2, 4
    sInt16 tx_coef[128];
    uInt08 tx_coef_size;

} ad9361_tx_fir_config_t;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Functions
// *****************************************************************************
// *****************************************************************************

// Initialize the AD9361 part.
int32_t ad9361_init(ad9361_rf_phy_t* phy, ad9361_init_parameters_t* init_param);
// Set the RX LO frequency.
int32_t ad9361_set_rx_lo_freq(ad9361_rf_phy_t* phy, uint64_t lo_freq_hz);
// Get current RX LO frequency.
int32_t ad9361_get_rx_lo_freq(ad9361_rf_phy_t* phy, uint64_t* lo_freq_hz);
// Set the RX FIR filter configuration.
int32_t ad9361_set_rx_fir_config(ad9361_rf_phy_t* phy, ad9361_rx_fir_config_t fir_cfg);
// Set the transmit attenuation for the selected channel.
int32_t ad9361_set_tx_attenuation(ad9361_rf_phy_t* phy, uint8_t ch, uint32_t attenuation_mdb);
// Get current transmit attenuation for the selected channel.
int32_t ad9361_get_tx_attenuation(ad9361_rf_phy_t* phy, uint8_t ch, uint32_t* attenuation_mdb);
// Set the TX LO frequency.
int32_t ad9361_set_tx_lo_freq(ad9361_rf_phy_t* phy, uint64_t lo_freq_hz);
// Get current TX LO frequency.
int32_t ad9361_get_tx_lo_freq(ad9361_rf_phy_t* phy, uint64_t* lo_freq_hz);
// Set the TX FIR filter configuration.
int32_t ad9361_set_tx_fir_config(ad9361_rf_phy_t* phy, ad9361_tx_fir_config_t fir_cfg);
// Perform the selected calibration.
int32_t ad9361_do_calib(ad9361_rf_phy_t* phy, uint32_t cal, int32_t arg);

#endif /* SDR_AD9361_API_HPP */

/* *****************************************************************************
 End of File
 */
