#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <assert.h>
#include <errno.h>
#include <errno.h>
#include "backend.h"
#include "frontend.h"

static const char *name;
static FILE *data_writer;
static FILE *expected_data;
static FILE *starting_data;
static bool completed = false;
static bool run_succeeded = true;

static Uint32 quit_timer(Uint32 interval, void *param) {
    (void)param;
    (void)interval;
    quit();
    return 0;
}

static void quit_in_a_bit(void) {
    SDL_AddTimer(5000, quit_timer, NULL);
}

void closeSaveFile(void) {
    if (starting_data) {
        fclose(starting_data);
        starting_data = NULL;
    } else if (data_writer) {
        fclose(data_writer);
        data_writer = NULL;
        quit_in_a_bit();
    } else if (expected_data) {
        if (run_succeeded) {
            printf("%s SUCCEEDED\n", name);
        } else {
            printf("%s FAILED\n", name);
            
        }
        fclose(expected_data);
        expected_data = NULL;
        quit_in_a_bit();
    }
}
static int onEvent(void *userdata, SDL_Event *event) {
    (void)userdata;
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *mouse = &event->button;
        if (expected_data) return 1;
        if (mouse->button == SDL_BUTTON_LEFT) {
            onClick(mouse->x, mouse->y);
            fprintf(data_writer, "CLICK %d %d\n", mouse->x, mouse->y);
        } else if (mouse->button == SDL_BUTTON_RIGHT) {
            onAltClick(mouse->x, mouse->y);
            fprintf(data_writer, "ALTCLICK %d %d\n", mouse->x, mouse->y);
        }
    }
    return 1;
}

void test_frontend_mouse_down(SDL_MouseButtonEvent *mouse) {
    (void)mouse;
}

bool openSaveReader(void) {
    starting_data = fopen("test_save.bin", "r");
    assert(starting_data != NULL && "failed to open test start starting_data");
    return true;
}
Uint8 readByte(void) {
    return fgetc(starting_data);
}
bool openSaveWriter(void) {
    if (data_writer != NULL) {
        fprintf(data_writer, "SAVEDATA");
        return true;
    } else if (expected_data != NULL) {
        if (!completed) {
            fclose(expected_data);
            expected_data = NULL;
        }
        return completed;
    } else {
        return false;
    }
}

int writeByte(Uint8 value) {
    if (data_writer) {
        return fputc(value, data_writer) != EOF;
    } else {
        assert(expected_data);
        char expected = fgetc(expected_data);
        if (run_succeeded && value != expected) {
            run_succeeded = false;
            printf("expected %c (%d) but got %c (%d)\n", expected, expected, value, value);
        }
        return 1;
    }
}

#define INTERVAL 100

Uint32 process_next_command(Uint32 interval, void *param) {
    (void)param;
    if (expected_data == NULL) return 0;
    int x, y;
    if (fscanf(expected_data, "CLICK %d %d\n", &x, &y) == 2) {
        onClick(x, y);
        return interval;
    } else if (fscanf(expected_data, "ALTCLICK %d %d\n", &x, &y) == 2) {
        onAltClick(x, y);
        return interval;
    } else if (fscanf(expected_data, "SAVEDATA\n") == 0) {
        completed = true;
        save();
        return 0;
    } else {
        fprintf(stderr, "INVALID TEST SAVE FILE");
        return 0;
    }
}

static void usage(void) {
    printf("Usage: run|record <file>\n");
    exit(1);
}

static void run() {
    expected_data = fopen(name, "r");
    if (expected_data == NULL) {
        fprintf(stderr, "Failed to open %s (%s)\n", name, strerror(errno));
        exit(1);
    }
    SDL_AddTimer(INTERVAL, process_next_command, NULL);
}

static void record() {
    printf("Creating test at %s\n", name);
    data_writer = fopen(name, "w");
    if (data_writer == NULL) {
        fprintf(stderr, "Failed to open %s (%s)\n", name, strerror(errno));
        exit(1);
    }
}

void test_frontend_main(char **arg) {
    if (arg[0] == NULL || arg[1] == NULL) {
        usage();
    }
    if (strcmp(arg[0], "run") == 0) {
        name = arg[1];
        run();
    } else if (strcmp(arg[0], "record") == 0) {
        name = arg[1];
        record();
    } else {
        usage();
    }
    SDL_AddEventWatch(onEvent, NULL);
}
