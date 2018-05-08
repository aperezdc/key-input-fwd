/*
 * Author: Philippe Normand <philn@igalia.com>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <termios.h>


#define KK(kc, lc, uc)  [lc] = { kc }, [uc] = { kc, .shift = 1 }
#define KL(kc, uc)      KK(kc, uc + ('a' - 'A'), uc)

static const struct {
    int key;
    unsigned shift : 1;
} s_keytable[256] = {
    KK(KEY_0, '0', ')'),
    KK(KEY_1, '1', '!'),
    KK(KEY_2, '2', '@'),
    KK(KEY_3, '3', '#'),
    KK(KEY_4, '4', '$'),
    KK(KEY_5, '5', '%'),
    KK(KEY_6, '6', '^'),
    KK(KEY_7, '7', '&'),
    KK(KEY_8, '8', '*'),
    KK(KEY_9, '9', '('),

    KL(KEY_A, 'A'), KL(KEY_B, 'B'), KL(KEY_C, 'C'), KL(KEY_D, 'D'),
    KL(KEY_E, 'E'), KL(KEY_F, 'F'), KL(KEY_G, 'G'), KL(KEY_H, 'H'),
    KL(KEY_I, 'I'), KL(KEY_J, 'J'), KL(KEY_K, 'K'), KL(KEY_L, 'L'),
    KL(KEY_M, 'M'), KL(KEY_N, 'N'), KL(KEY_O, 'O'), KL(KEY_P, 'P'),
    KL(KEY_Q, 'Q'), KL(KEY_R, 'R'), KL(KEY_S, 'S'), KL(KEY_T, 'T'),
    KL(KEY_U, 'U'), KL(KEY_V, 'V'), KL(KEY_W, 'W'), KL(KEY_X, 'X'),
    KL(KEY_Y, 'Y'), KL(KEY_Z, 'Z'),

    KK(KEY_APOSTROPHE, '\'', '"'),
    KK(KEY_BACKSLASH, '\\', '|'),
    KK(KEY_COMMA, ',', '<'),
    KK(KEY_DOT, '.', '>'),
    KK(KEY_EQUAL, '=', '+'),
    KK(KEY_GRAVE, '`', '~'),
    KK(KEY_LEFTBRACE, '[', '{'),
    KK(KEY_RIGHTBRACE, ']', '}'),
    KK(KEY_SEMICOLON, ';', ':'),
    KK(KEY_SLASH, '/', '?'),

    [ 27 ] = { KEY_ESC },
    [ 65 ] = { KEY_UP },
    [ 66 ] = { KEY_DOWN },
    [ 67 ] = { KEY_RIGHT },
    [ 68 ] = { KEY_LEFT },
    [127 ] = { KEY_BACKSPACE },
    ['-' ] = { KEY_MINUS },
    [' ' ] = { KEY_SPACE },
    ['\b'] = { KEY_BACKSPACE },
    ['\n'] = { KEY_ENTER },
    ['\t'] = { KEY_TAB },
};

#undef KL
#undef KK


static struct termios old, new;
static int fd;

/* Initialize new terminal i/o settings */
void initTermios() {
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    new = old; /* make new settings same as old settings */
    new.c_lflag &= ~ICANON; /* disable buffered i/o */
    new.c_lflag &= ~ECHO; /* disable echo mode */
    tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) {
    tcsetattr(0, TCSANOW, &old);
}

void send_key_event(int fd, unsigned int keycode, int keyvalue) {
    struct input_event event;
    gettimeofday(&event.time, NULL);

    event.type = EV_KEY;
    event.code = keycode;
    event.value = keyvalue;

    if (write(fd, &event, sizeof(event)) < 0)
        printf("simulate key error\n");

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    write(fd, &event, sizeof(event));
    if (write(fd, &event, sizeof(event)) < 0)
        printf("simulate key error\n");
}

void intHandler(int dummy) {
    resetTermios();
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    exit(0);
}

int main(int argc, char** argv) {
    struct uinput_user_dev uidev;

    signal(SIGINT, intHandler);

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Failed to open uinput\n");
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);

    ioctl(fd, UI_SET_KEYBIT, KEY_LEFTSHIFT);

    for (unsigned i = 0; i < sizeof(s_keytable) / sizeof(s_keytable[0]); i++) {
        if (s_keytable[i].key) {
            ioctl(fd, UI_SET_KEYBIT, s_keytable[i].key);
        }
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1234;
    uidev.id.product = 0xfedc;
    uidev.id.version = 1;
    write(fd, &uidev, sizeof(uidev));

    ioctl(fd, UI_DEV_CREATE);
    sleep(1);
    initTermios();

    while (1) {
        int cmd = getchar();
        if (cmd == 27) {
            cmd = getchar();
            cmd = getchar();
        }

        int keycode = s_keytable[cmd & 0xFF].key;
        int shift = s_keytable[cmd & 0xFF].shift;

        if (keycode) {
            if (shift)
                send_key_event(fd, KEY_LEFTSHIFT, 1);
            send_key_event(fd, keycode, 1);
            send_key_event(fd, keycode, 0);
            if (shift)
                send_key_event(fd, KEY_LEFTSHIFT, 0);
        } else {
            fprintf(stderr, "unhandled key code: %d\n", cmd);
        }
    }

    resetTermios();
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
