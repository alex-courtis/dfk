#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/time.h>

#define DUR_MILLIS(start, end) \
    ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000)

/* https://www.kernel.org/doc/html/latest/input/event-codes.html */
#define INPUT_VAL_PRESS 1
#define INPUT_VAL_RELEASE 0
#define INPUT_VAL_REPEAT 2

static Cfg cfg;

int
read_event(struct input_event *event) {
    return fread(event, sizeof(struct input_event), 1, stdin) == 1;
}

void
write_event(const struct input_event *event) {
    if (fwrite(event, sizeof(struct input_event), 1, stdout) != 1)
        exit(EXIT_FAILURE);
}

void
handle_press(Mapping *m, struct input_event *input) {

    // state
    switch (m->state) {
        case TAPPED:
        case DOUBLETAPPED:
            if (DUR_MILLIS(m->changed, input->time) < cfg.double_tap_millis)
                m->state = DOUBLETAPPED;
            else
                m->state = PRESSED;
            break;
        case RELEASED:
        case CONSUMED:
            m->state = PRESSED;
            break;
        case PRESSED:
            break;
    }
    m->changed = input->time;

    // action
    switch (m->state) {
        case TAPPED:
        case DOUBLETAPPED:
            input->code = m->tap;
            break;
        case RELEASED:
        case PRESSED:
        case CONSUMED:
            input->code = m->hold;
            break;
    }

    write_event(input);
}

void
handle_release(Mapping *m, struct input_event *input) {

    // state
    switch (m->state) {
        case PRESSED:
            if (DUR_MILLIS(m->changed, input->time) < cfg.tap_millis)
                m->state = TAPPED;
            else
                m->state = RELEASED;
            break;
        case TAPPED:
        case DOUBLETAPPED:
            break;
        case CONSUMED:
            m->state = RELEASED;
            break;
        case RELEASED:
            break;
    }
    m->changed = input->time;

    // action
    switch (m->state) {
        case TAPPED:
            // release
            input->code = m->hold;
            write_event(input);

            // synthesize tap press/release
            input->value = 1;
            input->code = m->tap;
            write_event(input);
            input->value = 0;
            write_event(input);
            break;
        case DOUBLETAPPED:
            input->code = m->tap;
            write_event(input);
            break;
        case CONSUMED:
        case RELEASED:
        case PRESSED:
            input->code = m->hold;
            write_event(input);
            break;
    }
}

void
consume_pressed() {

    // state
    for (Mapping *m = cfg.m; m; m = m->n) {
        switch (m->state) {
            case PRESSED:
                m->state = CONSUMED;
                break;
            case TAPPED:
            case DOUBLETAPPED:
            case RELEASED:
            case CONSUMED:
                break;
        }
    }
}

void
loop() {
    struct input_event input;
    Mapping *m;

    while (read_event(&input)) {
        // uinput doesn't need sync events
        if (input.type == EV_MSC && input.code == MSC_SCAN)
            continue;

        // forward anything that is not a key event
        if (input.type != EV_KEY) {
            write_event(&input);
            continue;
        }

        // uinput doesn't need hardware repeats; squish them all
        if (input.value == INPUT_VAL_REPEAT)
            continue;

        // consume all taps that are incomplete
        if (input.value == INPUT_VAL_PRESS)
            consume_pressed();

        // is this our key?
        for (m = cfg.m; m && m->key != input.code; m = m->n);

        // forward all other key events
        if (!m) {
            write_event(&input);
            continue;
        }

        switch (input.value) {
            case INPUT_VAL_PRESS:
                handle_press(m, &input);
                break;
            case INPUT_VAL_RELEASE:
                handle_release(m, &input);
                break;
        }
    }
}

int
main(void) {

    setbuf(stdin, NULL), setbuf(stdout, NULL);

    read_cfg(&cfg);

    loop();
}

