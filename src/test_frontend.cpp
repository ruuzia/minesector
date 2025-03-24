#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <cassert>
#include <cstdbool>
#include <fstream>
#include "backend.h"
#include "frontend.h"


constexpr uint32_t INTERVAL = 100;
constexpr uint32_t AUTOQUIT_PERIOD = 5000;

static enum { RUNNING, RECORDING, FINISHED } state;

static struct {
    std::ofstream expected;
    std::ofstream file;
} recorder;

static struct {
    std::ifstream expected;
    std::ifstream sim_input;
    bool completed;
    bool succeeded;
} runner;

static std::string name;
static std::ifstream inital_savedata;

static Uint32 quit_timer(Uint32 interval, void *param) {
    (void)param;
    (void)interval;
    quit();
    return 0;
}

static void quit_in_a_bit(void) {
    SDL_AddTimer(AUTOQUIT_PERIOD, quit_timer, NULL);
}

void closeSaveFile(void) {
    if (inital_savedata.is_open()) {
        inital_savedata.close();
        return;
    }

    switch (state) {
    case RECORDING:
        recorder.file.close();
        recorder.expected.close();
        state = FINISHED;
        quit_in_a_bit();
        return;

    case RUNNING:
        if (runner.completed) {
            printf("%s SUCCEEDED\n", name.c_str());
        } else {
            printf("%s FAILED\n", name.c_str());
            
        }
        runner.expected.close();
        state = FINISHED;
        quit_in_a_bit();
        return;

    case FINISHED:
        // do nothing
        return;
    }
}
static int onEvent(void *userdata, SDL_Event *event) {
    (void)userdata;
    if (state == RECORDING && event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *mouse = &event->button;
        if (mouse->button == SDL_BUTTON_LEFT) {
            onClick(mouse->x, mouse->y);
            recorder.file << "CLICK " << mouse->x << " " << mouse->y << '\n';
        } else if (mouse->button == SDL_BUTTON_RIGHT) {
            onAltClick(mouse->x, mouse->y);
            recorder.file << "ALTCLICK " << mouse->x << " " << mouse->y << '\n';
        }
    } else if (state == FINISHED && event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN) {
        quit();
    }
    return 1;
}

bool openSaveReader(void) {
    assert(state == RUNNING || state == RECORDING);
    std::string file_name = name + ".initial";
    inital_savedata.open(file_name);
    if (!inital_savedata.is_open()) {
        printf("failed to open %s\n", file_name.c_str());
        return false;
    }
    return true;
}
Uint8 readByte(void) {
    assert(state == RUNNING || state == RECORDING);
    return inital_savedata.get();
}
bool openSaveWriter(void) {
    std::string save_file_name = name + ".expected";

    switch (state) {
    case FINISHED:
        return false;
    case RECORDING:
        recorder.expected.open(save_file_name);
        if (!recorder.expected.is_open()) {
            printf("Failed to open %s\n", save_file_name.c_str());
            return false;
        }
        return true;
    case RUNNING:
        if (!runner.completed) return false;
        runner.expected.open(save_file_name);
        if (!runner.expected.is_open()) {
            printf("Failed to open %s\n", save_file_name.c_str());
            return false;
        }
        return true;
    }
}

int writeByte(Uint8 value) {
    switch (state) {
    case RECORDING:
        recorder.expected << value;
        return 1;

    case RUNNING: {
        uint8_t expected = runner.expected.get();
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

Uint32 process_next_command(Uint32 interval, void *param) {
    (void)param;
    assert(state == RUNNING);
    std::string cmd;
    runner.sim_input >> cmd;
    if (cmd == "CLICK") {
        int x, y;
        runner.sim_input >> x;
        runner.sim_input >> y;
        onClick(x, y);
        return interval;
    } else if (cmd == "ALTCLICK") {
        int x, y;
        runner.sim_input >> x;
        runner.sim_input >> y;
        onAltClick(x, y);
        return interval;
    } else if (cmd == "") {
        runner.completed = true;
        save();
        return 0;
    } else {
        fprintf(stderr, "input sim unexpected \"%s\"\n", cmd.c_str());
        exit(1);
        return 0;
    }
}

static void usage(void) {
    printf("Usage: run|record <file>\n");
    exit(1);
}

static void run() {
    runner.sim_input.open(name);
    if (!runner.sim_input.is_open()) {
        fprintf(stderr, "Failed to open %s (%s)\n", name.c_str(), strerror(errno));
        exit(1);
    }
    SDL_AddTimer(INTERVAL, process_next_command, NULL);
}

static void record() {
    printf("Creating test at %s\n", name.c_str());
    recorder.file.open(name);
    if (!recorder.file.is_open()) {
        fprintf(stderr, "Failed to open %s (%s)\n", name.c_str(), strerror(errno));
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

