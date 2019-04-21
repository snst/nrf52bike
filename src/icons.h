#ifndef ICONS_H
#define ICONS_H

#include "../icons/ic_nav_arrow_finish.xbm"
#include "../icons/ic_nav_arrow_turn_hard_left.xbm"
#include "../icons/ic_nav_roundabout_ccw1_3.xbm"
#include "../icons/ic_nav_roundabout_cw2_3.xbm"
#include "../icons/ic_nav_arrow_fork_left.xbm"
#include "../icons/ic_nav_arrow_turn_hard_right.xbm"
#include "../icons/ic_nav_roundabout_ccw2_2.xbm"
#include "../icons/ic_nav_roundabout_cw3_3.xbm"
#include "../icons/ic_nav_arrow_fork_right.xbm"
#include "../icons/ic_nav_arrow_turn_left.xbm"
#include "../icons/ic_nav_roundabout_ccw2_3.xbm"
#include "../icons/ic_nav_roundabout_exit_ccw.xbm"
#include "../icons/ic_nav_arrow_goto_start.xbm"
#include "../icons/ic_nav_arrow_turn_right.xbm"
#include "../icons/ic_nav_roundabout_ccw3_3.xbm"
#include "../icons/ic_nav_roundabout_exit_cw.xbm"
#include "../icons/ic_nav_arrow_keep_going.xbm"
#include "../icons/ic_nav_arrow_uturn.xbm"
#include "../icons/ic_nav_roundabout_cw1_1.xbm"
#include "../icons/ic_nav_roundabout_fallback.xbm"
#include "../icons/ic_nav_arrow_keep_left.xbm"
#include "../icons/ic_nav_outof_route.xbm"
#include "../icons/ic_nav_roundabout_cw1_2.xbm"
#include "../icons/ic_nav_arrow_keep_right.xbm"
#include "../icons/ic_nav_roundabout_ccw1_1.xbm"
#include "../icons/ic_nav_roundabout_cw1_3.xbm"
#include "../icons/ic_nav_arrow_start.xbm"
#include "../icons/ic_nav_roundabout_ccw1_2.xbm"
#include "../icons/ic_nav_roundabout_cw2_2.xbm"

const uint8_t *nav_icons[] = {
    NULL,
    ic_nav_arrow_keep_going_bits,      // 1
    ic_nav_arrow_start_bits,           // 2
    ic_nav_arrow_finish_bits,          // 3
    ic_nav_arrow_keep_left_bits,       // 4
    ic_nav_arrow_turn_left_bits,       // 5
    ic_nav_arrow_turn_hard_left_bits,  // 6
    ic_nav_arrow_turn_hard_right_bits, // 7
    ic_nav_arrow_turn_right_bits,      // 8
    ic_nav_arrow_keep_right_bits,      // 9
    ic_nav_arrow_fork_right_bits,      // 10
    ic_nav_arrow_fork_left_bits,       // 11
    ic_nav_arrow_uturn_bits,           // 12
    NULL,                              // 13
    NULL,                              // 14
    ic_nav_roundabout_exit_cw_bits,    // 15
    ic_nav_roundabout_exit_ccw_bits,   // 16
    ic_nav_roundabout_ccw1_1_bits,     // 17
    ic_nav_roundabout_ccw1_2_bits,     // 18
    ic_nav_roundabout_ccw1_3_bits,     // 19
    ic_nav_roundabout_ccw2_2_bits,     // 20
    ic_nav_roundabout_ccw2_3_bits,     // 21
    ic_nav_roundabout_ccw3_3_bits,     // 22
    ic_nav_roundabout_cw1_1_bits,      // 23
    ic_nav_roundabout_cw1_2_bits,      // 24
    ic_nav_roundabout_cw1_3_bits,      // 25
    ic_nav_roundabout_cw2_2_bits,      // 26
    ic_nav_roundabout_cw2_3_bits,      // 27
    ic_nav_roundabout_cw3_3_bits,      // 28
    ic_nav_roundabout_fallback_bits,   // 29
    ic_nav_outof_route_bits            //30
};

const uint8_t *GetNavIcon(uint8_t i)
{
    return i <= 30 ? nav_icons[i] : NULL;
}

#endif