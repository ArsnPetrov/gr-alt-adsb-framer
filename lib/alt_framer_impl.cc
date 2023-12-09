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

      in0_pulses.clear();
      in0_transitions.clear();
      in0_pulses.reserve(N);
      in0_transitions.reserve(N-1);

      in0_rise_edge_idxs.clear();
      in0_fall_edge_idxs.clear();

      pulse_idxs.clear();
      
      in0_pulses.push_back(prev_in0 > threshold ? 1 : 0);
      for (int i = 1; i < N; i++) {
        in0_pulses.push_back(in[i-1] > threshold ? 1 : 0);
      }

      prev_in0 = in[N - 1];

      for (int i = 0; i < N; i++) {
        in0_transitions.push_back(in0_pulses[i+1] - in0_pulses[i]);
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

      // импульсов не найдено - ранний выход
      if (in0_fall_edge_idxs.size() == 0 || in0_rise_edge_idxs.size() == 0) {
        std::memcpy(out, in + N_hist - 1, noutput_items * sizeof(float));
        return noutput_items;
      }

      // Обеспечить, чтобы первый фронт был растущим
      if (in0_fall_edge_idxs[0] - in0_rise_edge_idxs[0] < 0) {
        in0_fall_edge_idxs.pop_front();
      }

      int rise_fall_cout_diff = in0_rise_edge_idxs.size() - in0_fall_edge_idxs.size();
      if (rise_fall_cout_diff > 0) {
        in0_rise_edge_idxs.pop_back();

        if (rise_fall_cout_diff > 1) {
          std::cout << "Ошибка детектирования символов: " << rise_fall_cout_diff << std::endl;
        }
      }

      for (int i = 0; i < in0_rise_edge_idxs.size(); i++) {
        int av_idx = (in0_rise_edge_idxs[i] + in0_fall_edge_idxs[i]) / 2;
        pulse_idxs.push_back(av_idx);
      }

      // Для каждого импульса проверяем, является ли он началом преамбулы
      for (int pulse_idx : pulse_idxs) {
        if (pulse_idx <= prev_eob_idx) {
          continue;
        }

        prev_eob_idx = -1;

        bool correlation_found = true;

        for (int i = 0; i < adsb_alt_framer::NUM_PREAMBLE_PULSES; i++) {
          int pulse = in[pulse_idx + i] > in[pulse_idx] / 2 ? 1 : 0;
          if (pulse != adsb_alt_framer::preamble[i]) {
            correlation_found = false;
            break;
          }
        }

        // add_item_tag(0, (nitems_written(0) - (N_hist-1)) + pulse_idx, 
        //               pmt::mp("pulse"),
        //               pmt::mp("1"),
        //               pmt::mp("framer"));

        if (correlation_found) {
          std::vector<float> noise_sample(NUM_NOISE_SAMPLES);
          
          if (pulse_idx < NUM_NOISE_SAMPLES) {
            noise_sample.insert(noise_sample.end(), in, in + pulse_idx);
          }
          else {
            noise_sample.insert(noise_sample.end(), in + pulse_idx - NUM_NOISE_SAMPLES, in + pulse_idx);
          }

          std::nth_element(noise_sample.begin(), noise_sample.begin() + noise_sample.size() / 2, noise_sample.end());

          float median = noise_sample[noise_sample.size() / 2];

          float snr = 10.0f * std::log10(in[pulse_idx] / median) + 1.6f;

          prev_eob_idx = pulse_idx + (NUM_PREAMBLE_BITS + MIN_NUM_BITS - 1) * sps;

          //std::cout << "bburst: snr is " << snr << " sample is " << pulse_idx << std::endl;

          add_item_tag(0, (nitems_written(0) - (N_hist-1)) + pulse_idx, 
                      pmt::mp("burst"),
                      pmt::make_tuple(pmt::mp("SOB"), pmt::mp(snr)),
                      pmt::mp("framer"));
        }
      }

      if (prev_eob_idx >= N) {
        prev_eob_idx -= N;
      }

      std::memcpy(out, in + N_hist - 1, noutput_items * sizeof(float));
      //out = (float*)in + N_hist - 1;
      return noutput_items;
    }
  } /* namespace adsb_alt_framer */
} /* namespace gr */
