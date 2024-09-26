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

#ifndef BALLISTICS_FACE_H_
#define BALLISTICS_FACE_H_

#include "movement.h"
#include "ballistics/ballistics.h"

/*
 * A DESCRIPTION OF YOUR WATCH FACE
 *
 * and a description of how use it
 *
 */

typedef struct {
    // Page 0 is the input target distance page
    // Page 1 displays the elevation calculation
    uint8_t page;

    // When inputting target distance this represents the active digit
    uint8_t active_digit;
    // Target distance. Max is 9999. We split this out into individual numbers to make input easier.
    uint8_t target_distance_yards[4];

    // Ballistics coefficient (usually in G7 these days, but G1 is also common)
    double bc;
    DragFunction df;
    // In feet per second
    double muzzle_velocity_fps;
    // Sight height over bore, in inches
    double sight_height_inch;
    // Shooting angle in degrees
    double angle_degrees;
    // Distance the rifle was zeroed at, in yards.
    double zero_distance_yards;
    // Miles per hour
    double wind_speed_mph;
    // Wind angle in degrees
    double wind_angle_degrees;
} ballistics_state_t;

void ballistics_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void ballistics_face_activate(movement_settings_t *settings, void *context);
bool ballistics_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void ballistics_face_resign(movement_settings_t *settings, void *context);

#define ballistics_face ((const watch_face_t){ \
    ballistics_face_setup, \
    ballistics_face_activate, \
    ballistics_face_loop, \
    ballistics_face_resign, \
    NULL, \
})

#endif // BALLISTICS_FACE_H_

