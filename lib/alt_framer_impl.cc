/* -*- c++ -*- */
/*
 * Copyright 2023 gr-adsb_alt_framer author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <gnuradio/io_signature.h>
#include <algorithm>
#include "alt_framer_impl.h"

namespace gr {
  namespace adsb_alt_framer {
    using input_type = float;
    using output_type = float;
    alt_framer::sptr
    alt_framer::make(float fs, float threshold)
    {
      return gnuradio::make_block_sptr<alt_framer_impl>(
        fs, threshold);
    }


    /*
     * The private constructor
     */
    alt_framer_impl::alt_framer_impl(float fs, float threshold)
      : gr::sync_block("alt_framer",
              gr::io_signature::make(1 /* min inputs */, 1 /* max inputs */, sizeof(input_type)),
              gr::io_signature::make(1 /* min outputs */, 1 /*max outputs */, sizeof(output_type))), 
              fs(fs), threshold(threshold)
    {
      if ((int)fs % adsb_alt_framer::SYMBOL_RATE != 0) {
        std::cerr << "Предусмотрена работа блока только при частоте дискретизации, кратной количеству символов в секунду в сигнале АЗС-Н (k * 1 МГц)" << std::endl;
      }

      sps = (int)fs / adsb_alt_framer::SYMBOL_RATE;
      prev_in0 = 0;
      prev_eob_idx = -1;
      N_hist = adsb_alt_framer::NUM_PREAMBLE_BITS * sps;
      set_history(N_hist);

      set_tag_propagation_policy(TPP_ONE_TO_ONE);
    }

    /*
     * Our virtual destructor.
     */
    alt_framer_impl::~alt_framer_impl()
    {
    }

    int
    alt_framer_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const int N = noutput_items;

      const float* in = static_cast<const input_type*>(input_items[0]);
      float*       out = static_cast<output_type*>(output_items[0]);

      in0_pulses.reserve(N);
      in0_transitions.reserve(N-1);

      in0_rise_edge_idxs.clear();
      in0_fall_edge_idxs.clear();
      
      in0_pulses[0] = prev_in0 > threshold ? 1 : 0;
      for (int i = 1; i < N; i++) {
        in0_pulses[i] = in[i-1] > threshold ? 1 : 0;
      }

      prev_in0 = in[N - 1];

      for (int i = 0; i < N; i++) {
        in0_transitions[i] = in0_pulses[2*i + 1] - in0_pulses[2*i];
      }

      // Заполнить массивы индексами восходящих и нисходящих фронтов
      for (int8_t &transition : in0_transitions) {
        if (transition > 0) {
          in0_rise_edge_idxs.push_back(&transition - &in0_transitions[0]);
        }
        else {
          in0_fall_edge_idxs.push_back(&transition - &in0_transitions[0]);
        }
      }

      // Обеспечить, чтобы первый фронт был растущим
      if (in0_fall_edge_idxs[0] - in0_rise_edge_idxs[0] < 0) {
        in0_fall_edge_idxs.pop_front();
      }

      size_t rise_fall_cout_diff = in0_rise_edge_idxs.size() - in0_fall_edge_idxs.size();
      if (rise_fall_cout_diff > 0) {
        in0_rise_edge_idxs.pop_back();

        if (rise_fall_cout_diff > 1) {
          std::cout << "Ошибка детектирования символов" << std::endl;
        }
      }

      long mean = 0;

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace adsb_alt_framer */
} /* namespace gr */
