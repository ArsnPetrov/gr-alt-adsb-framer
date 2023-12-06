/* -*- c++ -*- */
/*
 * Copyright 2023 gr-adsb_alt_framer author.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_H
#define INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_H

#include <gnuradio/adsb_alt_framer/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace adsb_alt_framer {

    /*!
     * \brief <+description of block+>
     * \ingroup adsb_alt_framer
     *
     */
    class ADSB_ALT_FRAMER_API alt_framer : virtual public gr::sync_block
    {
     public:
      typedef std::shared_ptr<alt_framer> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of adsb_alt_framer::alt_framer.
       *
       * To avoid accidental use of raw pointers, adsb_alt_framer::alt_framer's
       * constructor is in a private implementation
       * class. adsb_alt_framer::alt_framer::make is the public interface for
       * creating new instances.
       */
      static sptr make(float fs=2000000, float threshold=0.01);
    };

  } // namespace adsb_alt_framer
} // namespace gr

#endif /* INCLUDED_ADSB_ALT_FRAMER_ALT_FRAMER_H */
