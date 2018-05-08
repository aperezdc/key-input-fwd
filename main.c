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
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <signal.h>
#include <termios.h>

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
    unsigned int keycode;
    int cmd;

    signal(SIGINT, intHandler);

    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        printf("Failed to open uinput\n");
        return -1;
    }

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_SYN);
    ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
    ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, KEY_UP);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
    ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);
    ioctl(fd, UI_SET_KEYBIT, KEY_ESC);
    ioctl(fd, UI_SET_KEYBIT, KEY_DELETE);
    ioctl(fd, UI_SET_KEYBIT, KEY_BACKSPACE);
    ioctl(fd, UI_SET_KEYBIT, KEY_A);
    ioctl(fd, UI_SET_KEYBIT, KEY_B);
    ioctl(fd, UI_SET_KEYBIT, KEY_C);
    ioctl(fd, UI_SET_KEYBIT, KEY_D);
    ioctl(fd, UI_SET_KEYBIT, KEY_E);
    ioctl(fd, UI_SET_KEYBIT, KEY_F);
    ioctl(fd, UI_SET_KEYBIT, KEY_G);
    ioctl(fd, UI_SET_KEYBIT, KEY_H);
    ioctl(fd, UI_SET_KEYBIT, KEY_I);
    ioctl(fd, UI_SET_KEYBIT, KEY_J);
    ioctl(fd, UI_SET_KEYBIT, KEY_K);
    ioctl(fd, UI_SET_KEYBIT, KEY_L);
    ioctl(fd, UI_SET_KEYBIT, KEY_M);
    ioctl(fd, UI_SET_KEYBIT, KEY_N);
    ioctl(fd, UI_SET_KEYBIT, KEY_O);
    ioctl(fd, UI_SET_KEYBIT, KEY_P);
    ioctl(fd, UI_SET_KEYBIT, KEY_Q);
    ioctl(fd, UI_SET_KEYBIT, KEY_R);
    ioctl(fd, UI_SET_KEYBIT, KEY_S);
    ioctl(fd, UI_SET_KEYBIT, KEY_T);
    ioctl(fd, UI_SET_KEYBIT, KEY_U);
    ioctl(fd, UI_SET_KEYBIT, KEY_V);
    ioctl(fd, UI_SET_KEYBIT, KEY_W);
    ioctl(fd, UI_SET_KEYBIT, KEY_X);
    ioctl(fd, UI_SET_KEYBIT, KEY_Y);
    ioctl(fd, UI_SET_KEYBIT, KEY_Z);
    ioctl(fd, UI_SET_KEYBIT, KEY_0);
    ioctl(fd, UI_SET_KEYBIT, KEY_1);
    ioctl(fd, UI_SET_KEYBIT, KEY_2);
    ioctl(fd, UI_SET_KEYBIT, KEY_3);
    ioctl(fd, UI_SET_KEYBIT, KEY_4);
    ioctl(fd, UI_SET_KEYBIT, KEY_5);
    ioctl(fd, UI_SET_KEYBIT, KEY_6);
    ioctl(fd, UI_SET_KEYBIT, KEY_7);
    ioctl(fd, UI_SET_KEYBIT, KEY_8);
    ioctl(fd, UI_SET_KEYBIT, KEY_9);
    ioctl(fd, UI_SET_KEYBIT, KEY_DOT);

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
        cmd = getchar();
        if (cmd == 27) {
            cmd = getchar();
            cmd = getchar();
        }

        switch(cmd) {
        case 27:
            keycode = KEY_ESC;
            break;
        case 46:
            keycode = KEY_DOT;
            break;
        case 48:
            keycode = KEY_0;
            break;
        case 49:
            keycode = KEY_1;
            break;
        case 50:
            keycode = KEY_2;
            break;
        case 51:
            keycode = KEY_3;
            break;
        case 52:
            keycode = KEY_4;
            break;
        case 53:
            keycode = KEY_5;
            break;
        case 54:
            keycode = KEY_6;
            break;
        case 55:
            keycode = KEY_7;
            break;
        case 56:
            keycode = KEY_8;
            break;
        case 59:
            keycode = KEY_9;
            break;
        case 65:
            keycode = KEY_UP;
            break;
        case 66:
            keycode = KEY_DOWN;
            break;
        case 67:
            keycode = KEY_RIGHT;
            break;
        case 68:
            keycode = KEY_LEFT;
            break;
        case 10:
            keycode = KEY_ENTER;
            break;
        case 127:
            keycode = KEY_BACKSPACE;
            break;
        case 97:
            keycode = KEY_A;
            break;
        case 98:
            keycode = KEY_B;
            break;
        case 99:
            keycode = KEY_C;
            break;
        case 100:
            keycode = KEY_D;
            break;
        case 101:
            keycode = KEY_E;
            break;
        case 102:
            keycode = KEY_F;
            break;
        case 103:
            keycode = KEY_G;
            break;
        case 104:
            keycode = KEY_H;
            break;
        case 105:
            keycode = KEY_I;
            break;
        case 106:
            keycode = KEY_J;
            break;
        case 107:
            keycode = KEY_K;
            break;
        case 108:
            keycode = KEY_L;
            break;
        case 109:
            keycode = KEY_M;
            break;
        case 110:
            keycode = KEY_N;
            break;
        case 111:
            keycode = KEY_O;
            break;
        case 112:
            keycode = KEY_P;
            break;
        case 113:
            keycode = KEY_Q;
            break;
        case 114:
            keycode = KEY_R;
            break;
        case 115:
            keycode = KEY_S;
            break;
        case 116:
            keycode = KEY_T;
            break;
        case 117:
            keycode = KEY_U;
            break;
        case 118:
            keycode = KEY_V;
            break;
        case 119:
            keycode = KEY_W;
            break;
        case 120:
            keycode = KEY_X;
            break;
        case 121:
            keycode = KEY_Y;
            break;
        case 122:
            keycode = KEY_Z;
            break;
        default:
            keycode = 0;
            printf("unhandled key code: %d\n", cmd);
            break;
        }

        if (keycode) {
            send_key_event(fd, keycode, 1);
            send_key_event(fd, keycode, 0);
        }
    }

    resetTermios();
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);
    return 0;
}
