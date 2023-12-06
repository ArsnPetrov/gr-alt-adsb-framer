/* -*- c++ -*- */
/*
 * Copyright 2023 gr-adsb_alt_framer author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_IMPL_H
#define INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_IMPL_H

#include <gnuradio/adsb_alt_framer/alt_framer.h>

namespace gr {
  namespace adsb_alt_framer {
    const int SYMBOL_RATE         = 1000000; // symbols/second
    const int NUM_PREAMBLE_BITS   = 8;
    const int MIN_NUM_BITS        = 56;
    const int NUM_PREAMBLE_PULSES = NUM_PREAMBLE_BITS*2;
    const int NUM_NOISE_SAMPLES   = 100;
    
    // преамбула кадра (2 отсчета на символ)
    const float preamble[16] = { 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0 };

    class alt_framer_impl : public alt_framer
    {
     private:
      float fs; // частота дискретизации
      int   sps; // отсчетов на символ
      float threshold; // 

      float prev_in0;
      int prev_eob_idx;
      int N_hist;

      std::vector<int8_t> in0_pulses;
      std::vector<int8_t> in0_transitions;

      std::deque<int> in0_rise_edge_idxs, in0_fall_edge_idxs;

     public:
      alt_framer_impl(float fs, float threshold);
      ~alt_framer_impl();

      // Where all the action really happens
      int work(
              int noutput_items,
              gr_vector_const_void_star &input_items,
              gr_vector_void_star &output_items
      );
    };

  } // namespace adsb_alt_framer
} // namespace gr

#endif /* INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_IMPL_H */
