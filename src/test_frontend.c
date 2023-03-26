#include <SDL_events.h>
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include "backend.h"
#include "frontend.h"

static const char *name;
static FILE *data_writer;
static FILE *input_sim;
static FILE *starting_data;
static FILE *save_file;
static bool run_completed = false;
static bool run_succeeded = true;
static bool finished = false;

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
        fclose(save_file);
        data_writer = NULL;
        quit_in_a_bit();
    } else if (save_file) {
        if (run_succeeded) {
            printf("%s SUCCEEDED\n", name);
        } else {
            printf("%s FAILED\n", name);
            
        }
        fclose(save_file);
        save_file = NULL;
        finished = true;
        quit_in_a_bit();
    }
}
static int onEvent(void *userdata, SDL_Event *event) {
    (void)userdata;
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        SDL_MouseButtonEvent *mouse = &event->button;
        if (save_file) return 1;
        if (mouse->button == SDL_BUTTON_LEFT) {
            onClick(mouse->x, mouse->y);
            fprintf(data_writer, "CLICK %d %d\n", mouse->x, mouse->y);
        } else if (mouse->button == SDL_BUTTON_RIGHT) {
            onAltClick(mouse->x, mouse->y);
            fprintf(data_writer, "ALTCLICK %d %d\n", mouse->x, mouse->y);
        }
    } else if (event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_RETURN && finished) {
        quit();
    }
    return 1;
}

bool openSaveReader(void) {
    char file_name[1024];
    strcpy(file_name, name);
    strcat(file_name, ".initial");
    starting_data = fopen(file_name, "r");
    if (starting_data == NULL) {
        printf("failed to open %s: (%s)\n", file_name, strerror(errno));
        return false;
    }
    return true;
}
Uint8 readByte(void) {
    return fgetc(starting_data);
}
bool openSaveWriter(void) {
    if (finished) return false;
    char save_file_name[1024];
    strcpy(save_file_name, name);
    strcat(save_file_name, ".expected");
    if (data_writer != NULL) {
        // Recording test output
        save_file = fopen(save_file_name, "w");
        if (save_file == NULL) {
            printf("Failed to open %s: %s\n", save_file_name, strerror(errno));
            return false;
        }
        return true;
    } else {
        // Running test -- checking output
        if (!run_completed) return false;
        save_file = fopen(save_file_name, "r");
        if (save_file == NULL) {
            printf("Failed to open %s: %s\n", save_file_name, strerror(errno));
            return false;
        }
        return true;
    }
}

int writeByte(Uint8 value) {
    if (data_writer) {
        return fputc(value, save_file) != EOF;
    } else {
        assert(save_file);
        uint8_t expected = fgetc(save_file);
        if (run_succeeded && value != expected) {
            run_succeeded = false;
            printf("expected %c (%d) but got %c (%d)\n", expected, expected, value, value);
        }
        if (value == 'z') {
            printf("getting seed...\n");
        }
        return 1;
    }
}

#define INTERVAL 100

Uint32 process_next_command(Uint32 interval, void *param) {
    (void)param;
    if (input_sim == NULL) return 0;
    int x, y;
    if (fscanf(input_sim, "CLICK %d %d\n", &x, &y) == 2) {
        onClick(x, y);
        return interval;
    } else if (fscanf(input_sim, "ALTCLICK %d %d\n", &x, &y) == 2) {
        onAltClick(x, y);
        return interval;
    } else if (feof(input_sim)) {
        run_completed = true;
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
    input_sim = fopen(name, "r");
    if (input_sim == NULL) {
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

void frontend_init(char **arg) {
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

