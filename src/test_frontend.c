#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include "backend.h"
#include "frontend.h"

static enum { RUNNING, RECORDING, FINISHED } state;

static struct {
    FILE *expected;
    FILE *file;
} recorder;

static struct {
    FILE *expected;
    FILE *sim_input;
    bool completed;
    bool succeeded;
} runner;

static const char *name;
static FILE *inital_savedata;

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
    if (inital_savedata) {
        // Reader
        fclose(inital_savedata);
        inital_savedata = NULL;
        return;
    }

    switch (state) {
    case RECORDING:
        assert(recorder.file != NULL);
        assert(recorder.expected != NULL);
        fclose(recorder.file);
        fclose(recorder.expected);
        recorder.file = NULL;
        state = FINISHED;
        quit_in_a_bit();
        return;

    case RUNNING:
        if (runner.completed) {
            printf("%s SUCCEEDED\n", name);
        } else {
            printf("%s FAILED\n", name);
            
        }
        fclose(runner.expected);
        runner.expected = NULL;
        state = FINISHED;
        quit_in_a_bit();
        return;

    case FINISHED:
        // do nothing
        assert(recorder.file == NULL);
        assert(recorder.expected == NULL);
        assert(inital_savedata == NULL);
        return;
    }
}
static int onEvent(void *userdata, SDL_Event *event) {
    (void)userdata;
    if (state == RECORDING && event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *mouse = &event->button;
        if (recorder.expected) return 1;
        if (mouse->button == SDL_BUTTON_LEFT) {
            onClick(mouse->x, mouse->y);
            fprintf(recorder.file, "CLICK %d %d\n", mouse->x, mouse->y);
        } else if (mouse->button == SDL_BUTTON_RIGHT) {
            onAltClick(mouse->x, mouse->y);
            fprintf(recorder.file, "ALTCLICK %d %d\n", mouse->x, mouse->y);
        }
    } else if (state == FINISHED && event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
        quit();
    }
    return 1;
}

bool openSaveReader(void) {
    assert(state == RUNNING || state == RECORDING);
    char file_name[1024];
    strcpy(file_name, name);
    strcat(file_name, ".initial");
    inital_savedata = fopen(file_name, "r");
    if (inital_savedata == NULL) {
        printf("failed to open %s: (%s)\n", file_name, strerror(errno));
        return false;
    }
    return true;
}
Uint8 readByte(void) {
    assert(state == RUNNING || state == RECORDING);
    return fgetc(inital_savedata);
}
bool openSaveWriter(void) {
    char save_file_name[1024];
    strcpy(save_file_name, name);
    strcat(save_file_name, ".expected");

    switch (state) {
    case FINISHED:
        return false;
    case RECORDING:
        recorder.expected = fopen(save_file_name, "w");
        if (recorder.expected == NULL) {
            printf("Failed to open %s: %s\n", save_file_name, strerror(errno));
            return false;
        }
        return true;
    case RUNNING:
        if (!runner.completed) return false;
        runner.expected = fopen(save_file_name, "r");
        if (runner.expected == NULL) {
            printf("Failed to open %s: %s\n", save_file_name, strerror(errno));
            return false;
        }
        return true;
    }
}

int writeByte(Uint8 value) {
    switch (state) {
    case RECORDING:
        return fputc(value, recorder.expected) != EOF;
    case RUNNING: {
        uint8_t expected = fgetc(runner.expected);
        if (runner.completed && value != expected) {
            runner.completed = false;
            printf("expected %c (%d) but got %c (%d)\n", expected, expected, value, value);
        }
        return 1;
    }
    case FINISHED:
        assert(false && "writing data while finished");
        return 0;
    }
}

#define INTERVAL 100

Uint32 process_next_command(Uint32 interval, void *param) {
    (void)param;
    assert(state == RUNNING);
    int x, y;
    if (fscanf(runner.sim_input, "CLICK %d %d\n", &x, &y) == 2) {
        onClick(x, y);
        return interval;
    } else if (fscanf(runner.sim_input, "ALTCLICK %d %d\n", &x, &y) == 2) {
        onAltClick(x, y);
        return interval;
    } else if (feof(runner.sim_input)) {
        runner.completed = true;
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
    runner.sim_input = fopen(name, "r");
    if (runner.sim_input == NULL) {
        fprintf(stderr, "Failed to open %s (%s)\n", name, strerror(errno));
        exit(1);
    }
    SDL_AddTimer(INTERVAL, process_next_command, NULL);
}

static void record() {
    printf("Creating test at %s\n", name);
    recorder.file = fopen(name, "w");
    if (recorder.file == NULL) {
        fprintf(stderr, "Failed to open %s (%s)\n", name, strerror(errno));
        exit(1);
    }
}

void frontend_init(char **arg) {
    if (arg[0] == NULL || arg[1] == NULL) {
        usage();
    }
    if (strcmp(arg[0], "run") == 0) {
        state = RUNNING;
        name = arg[1];
        run();
    } else if (strcmp(arg[0], "record") == 0) {
        state = RECORDING;
        name = arg[1];
        record();
    } else {
        usage();
    }
    SDL_AddEventWatch(onEvent, NULL);
}

