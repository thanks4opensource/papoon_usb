// papoon_usb: "Not Insane" USB library for STM32F103xx MCUs
// Copyright (C) 2019 Mark R. Rubin
//
// This file is part of papoon_usb.
//
// The regbits_stm program is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The regbits_stm program is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// (LICENSE.txt) along with the regbits_stm program.  If not, see
// <https://www.gnu.org/licenses/gpl.html>


#ifndef RANDOM_TEST_HXX
#define RANDOM_TEST_HXX

#include <stdint.h>

#define BITOPS_XORSHIFT_RANDOM_BYTES
#include <xorshift_random.hxx>


namespace unittest {

template<unsigned HIST_LENGTH> class RandomTest {
  public:
    constexpr
    RandomTest(
    const uint32_t  recv_seed,
    const uint32_t  send_seed,
    const uint32_t  lnth_seed,
    const uint32_t  lnth_max ,
    const uint32_t  send_max )
    :
        _recvs_btwn_sends{0           },
        _sends_btwn_recvs{0           },
        _recv_random     (recv_seed   ),
        _send_random     (send_seed   ),
        _lnth_random     (lnth_seed   ),
        _recv_seed       (recv_seed   ),
        _syncs_rcvd      (0           ),
        _send_max        (send_max    ),
        _recv_count      (0           ),
        _send_count      (0           ),
        _recv_bytes      (0           ),
        _send_bytes      (0           ),
        _prev_recv       (0           ),
        _prev_send       (0           ),
        _lnth_mask       (lnth_max - 1),
        _sent_b4_recv    (0           ),
        _synced          (false       )
    {}

    unsigned histogram_length() const { return HISTOGRAM_LENGTH; }

    unsigned recv_count() const { return _recv_count; }
    unsigned send_count() const { return _send_count; }
    unsigned recv_bytes() const { return _recv_bytes; }
    unsigned send_bytes() const { return _send_bytes; }

    bool synced() const volatile { return _synced; }

    unsigned recvs_btwn_sends(
    const unsigned      index)
    const
    {
        return _recvs_btwn_sends[index];
    }

    unsigned sends_btwn_recvs(
    const unsigned      index)
    const
    {
        return _sends_btwn_recvs[index];
    }


    bool recv_sync(
    const uint8_t* const    buffer       ,
    const unsigned          num_bytes = 4,
    const unsigned          sync_size = 4)
    {
        if (_synced) return true;

        unsigned    ndx ;
        for (ndx = 0 ; ndx < num_bytes && _syncs_rcvd < sync_size ; ++ndx) {
            if (buffer[ndx] != _recv_random.byte()) {
                _recv_random.seed(_recv_seed);
                _syncs_rcvd = 0;
                return false;
            }
            ++_syncs_rcvd;
        }

        if (_syncs_rcvd == sync_size) {
            if (ndx< num_bytes && !recv(buffer + ndx, num_bytes - ndx))
                return false;

            return _synced = true;
        }

        return false;
    }



    uint16_t send_sync(
          uint8_t* const    buffer       ,
    const unsigned          num_bytes = 4)
    {
        for (unsigned ndx = 0 ; ndx < num_bytes ; ++ndx)
            buffer[ndx] = _send_random.byte();
        return num_bytes;
    }



    bool recv(
    const uint8_t* const    buffer   ,
    const unsigned          num_bytes)
    {
        for (unsigned ndx = 0 ; ndx < num_bytes ; ++ndx)
            if (buffer[ndx] != _recv_random.byte())
                return false;

          _recv_bytes   += num_bytes;
        ++_recv_count               ;
          _sent_b4_recv  = 0        ;

        uint32_t    sends_between = _send_count - _prev_send;

        ++_sends_btwn_recvs[  sends_between < HISTOGRAM_LENGTH
                            ? sends_between
                            : HISTOGRAM_LENGTH - 1            ];

        _prev_send = _send_count;

        return true;
    }



    unsigned send(
    uint8_t* const  buffer)
    {
        if (!_synced) return 0;

        uint16_t     send_len    = (_lnth_random.word() & _lnth_mask) + 1;

        for (unsigned ndx = 0 ; ndx < send_len ; ++ndx)
            buffer[ndx] = _send_random.byte();

          _send_bytes   += send_len;
        ++_send_count              ;
          _sent_b4_recv += send_len;

        uint32_t    recvs_between = _recv_count - _prev_recv;

        ++_recvs_btwn_sends[  recvs_between < HISTOGRAM_LENGTH
                            ? recvs_between
                            : HISTOGRAM_LENGTH - 1            ];

        _prev_recv = _recv_count;

        return send_len;
    }



  protected:
    unsigned            _recvs_btwn_sends[HIST_LENGTH],
                        _sends_btwn_recvs[HIST_LENGTH];
    bitops::XorShift    _recv_random                  ,
                        _send_random                  ,
                        _lnth_random                  ;
    uint32_t            _recv_seed                    ,
                        _syncs_rcvd                   ,
                        _send_max                     ;
    unsigned            _recv_count                   ,
                        _send_count                   ,
                        _recv_bytes                   ,
                        _send_bytes                   ,
                        _prev_recv                    ,
                        _prev_send                    ;
    uint32_t            _lnth_mask                    ,
                        _sent_b4_recv                 ;
    bool                _synced                       ;

}; // class RandomTest

}  // namespace unittest {

#endif   // ifndef RANDOM_TEST_HXX
