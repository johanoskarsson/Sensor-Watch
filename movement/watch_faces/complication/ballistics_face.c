/*
 * MIT License
 *
 * Copyright (c) 2024 Johan Oskarsson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include "ballistics_face.h"
#include "watch.h"
#include <ballistics/ballistics.h>


static uint16_t _ballistics_face_target_distance_int(ballistics_state_t *state) {
    uint16_t distance = 
            state->target_distance_yards[0] * 1000
           + state->target_distance_yards[1] * 100
           + state->target_distance_yards[2] * 10
           + state->target_distance_yards[3];

    printf("distance: %d\n", distance);   
    return distance;
}

static void _ballistics_face_update(movement_settings_t *settings, ballistics_state_t *state) {
    if (state->page == 0) {
        char buf[14];
        sprintf(buf, "DS  %04dyd", _ballistics_face_target_distance_int(state));
        watch_display_string(buf, 0);
        return;
    } else if (state->page == 1) {
        char buf[14];
        Ballistics* solution;
        double zeroAngle = zero_angle(
            state->df,
            state->bc,
            state->muzzle_velocity_fps,
            state->sight_height_inch,
            state->zero_distance_yards,
            0
        );
        int nsoln = Ballistics_solve(
            &solution,
            state->df,
            state->bc,
            state->muzzle_velocity_fps,
            state->sight_height_inch,
            state->angle_degrees,
            zeroAngle,
            state->wind_speed_mph,
            state->wind_angle_degrees
        );

        double moa = Ballistics_get_moa(solution, _ballistics_face_target_distance_int(state));

        sprintf(buf, "BA    %2.2f", moa);
        watch_display_string(buf, 0);

        printf("result: %2.2f\n", moa);

        Ballistics_free(solution);
    }
}

static void _ballistics_face_advance_digit(ballistics_state_t *state) {
    switch (state->page) {
        case 0:
            if (state->active_digit <= 3) {
                // We need to wrap the digits if we hit 9
                if (state->target_distance_yards[state->active_digit] == 9) {
                    state->target_distance_yards[state->active_digit] = 0;
                } else {
                    state->target_distance_yards[state->active_digit]++;
                }
            }
            
            break;
        case 1:
            break;
    }
}

void ballistics_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;
    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(ballistics_state_t));
        ballistics_state_t *state = (ballistics_state_t *)*context_ptr;
        memset(*context_ptr, 0, sizeof(ballistics_state_t));

        state->page = 0;
        // 500 yards to begin with
        state->target_distance_yards[1] = 5;

        state->bc = 0.425;
        state->df = G7;
        state->muzzle_velocity_fps = 2802;
        state->sight_height_inch = 2.7;
        state->angle_degrees = 0;
        state->zero_distance_yards = 100;
        state->wind_speed_mph = 0;
        state->wind_angle_degrees = 0;
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void ballistics_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    // ballistics_state_t *state = (ballistics_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
}

bool ballistics_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {
    ballistics_state_t *state = (ballistics_state_t *)context;

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // Show your initial UI here.

            _ballistics_face_update(settings, state);
            break;
        case EVENT_TICK:
            // If needed, update your display here.
            break;
        case EVENT_LIGHT_BUTTON_DOWN:
            if (state->page == 0) {
                state->active_digit++;
                if (state->active_digit > 3) {
                    state->active_digit = 0;
                    state->page = 1;
                }

                _ballistics_face_update(settings, state);
            }
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // You can use the Light button for your own purposes. Note that by default, Movement will also
            // illuminate the LED in response to EVENT_LIGHT_BUTTON_DOWN; to suppress that behavior, add an
            // empty case for EVENT_LIGHT_BUTTON_DOWN.
            break;
        case EVENT_ALARM_BUTTON_UP:
            if (state->page == 0) {
                _ballistics_face_advance_digit(state);
                _ballistics_face_update(settings, state);
            }
            break;
        case EVENT_ALARM_LONG_PRESS:
            if (state->page == 0) {
                state->page++;
            } else if (state->page == 1) {
                state->page--;
            }

            _ballistics_face_update(settings, state);
            break;
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            // movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            // Movement's default loop handler will step in for any cases you don't handle above:
            // * EVENT_LIGHT_BUTTON_DOWN lights the LED
            // * EVENT_MODE_BUTTON_UP moves to the next watch face in the list
            // * EVENT_MODE_LONG_PRESS returns to the first watch face (or skips to the secondary watch face, if configured)
            // You can override any of these behaviors by adding a case for these events to this switch statement.
            return movement_default_loop_handler(event, settings);
    }

    // return true if the watch can enter standby mode. Generally speaking, you should always return true.
    // Exceptions:
    //  * If you are displaying a color using the low-level watch_set_led_color function, you should return false.
    //  * If you are sounding the buzzer using the low-level watch_set_buzzer_on function, you should return false.
    // Note that if you are driving the LED or buzzer using Movement functions like movement_illuminate_led or
    // movement_play_alarm, you can still return true. This guidance only applies to the low-level watch_ functions.
    return true;
}

void ballistics_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}
