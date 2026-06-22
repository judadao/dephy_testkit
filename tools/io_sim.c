#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dephy_industrial_io/industrial_io.h>
#include <dephy_industrial_io/posix_sim.h>

static const dephy_io_channel_config_t default_channels[] = {
    {
        .name = "door",
        .type = DEPHY_IO_DI,
        .driver_channel = 0,
        .debounce_ms = 20,
        .scale_num = 1,
        .scale_den = 1,
    },
    {
        .name = "relay",
        .type = DEPHY_IO_DO,
        .driver_channel = 1,
        .scale_num = 1,
        .scale_den = 1,
    },
    {
        .name = "pressure_ma_x100",
        .type = DEPHY_IO_AI,
        .driver_channel = 2,
        .scale_num = 100,
        .scale_den = 1,
    },
};

static const char *event_name(dephy_io_event_type_t type)
{
    switch (type) {
    case DEPHY_IO_EVENT_CHANGED:
        return "changed";
    case DEPHY_IO_EVENT_RISING:
        return "rising";
    case DEPHY_IO_EVENT_FALLING:
        return "falling";
    case DEPHY_IO_EVENT_FAULT:
        return "fault";
    default:
        return "unknown";
    }
}

static const char *type_name(dephy_io_type_t type)
{
    switch (type) {
    case DEPHY_IO_DI:
        return "di";
    case DEPHY_IO_DO:
        return "do";
    case DEPHY_IO_AI:
        return "ai";
    case DEPHY_IO_AO:
        return "ao";
    default:
        return "unknown";
    }
}

static void on_event(const dephy_io_event_t *event, void *user)
{
    (void)user;

    printf("event %s %s %s %d %u %u\n",
           event_name(event->type),
           event->sample.name,
           type_name(event->sample.type),
           event->sample.value,
           event->sample.fault,
           event->sample.changed_at_ms);
}

static int print_read(const char *name)
{
    dephy_io_sample_t sample;

    if (dephy_io_read(name, &sample) != 0) {
        fprintf(stderr, "error: read failed for channel '%s'\n", name);
        return -1;
    }

    printf("read %s %s %d %u %u\n",
           sample.name,
           type_name(sample.type),
           sample.value,
           sample.fault,
           sample.changed_at_ms);
    return 0;
}

static int run_line(char *line)
{
    char cmd[32];
    char name[64];
    int channel;
    int value;
    int advance_ms;

    if (line[0] == '\0' || line[0] == '\n' || line[0] == '#') {
        return 0;
    }

    if (sscanf(line, "%31s", cmd) != 1) {
        return 0;
    }

    if (strcmp(cmd, "set") == 0) {
        if (sscanf(line, "%31s %d %d %d", cmd, &channel, &value, &advance_ms) != 4) {
            fprintf(stderr, "error: set syntax: set <driver_channel> <raw> <advance_ms>\n");
            return -1;
        }
        if (dephy_io_posix_sim_set_raw((uint16_t)channel, value) != 0) {
            fprintf(stderr, "error: set failed for driver channel %d\n", channel);
            return -1;
        }
        dephy_io_posix_sim_advance_ms((uint32_t)advance_ms);
        return dephy_io_poll();
    }

    if (strcmp(cmd, "sleep") == 0) {
        if (sscanf(line, "%31s %d", cmd, &advance_ms) != 2) {
            fprintf(stderr, "error: sleep syntax: sleep <advance_ms>\n");
            return -1;
        }
        dephy_io_posix_sim_advance_ms((uint32_t)advance_ms);
        return dephy_io_poll();
    }

    if (strcmp(cmd, "write") == 0) {
        if (sscanf(line, "%31s %63s %d", cmd, name, &value) != 3) {
            fprintf(stderr, "error: write syntax: write <name> <value>\n");
            return -1;
        }
        return dephy_io_write(name, value);
    }

    if (strcmp(cmd, "read") == 0) {
        if (sscanf(line, "%31s %63s", cmd, name) != 2) {
            fprintf(stderr, "error: read syntax: read <name>\n");
            return -1;
        }
        return print_read(name);
    }

    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    return -1;
}

static int run_file(FILE *file)
{
    char line[256];
    unsigned int line_no = 0;

    while (fgets(line, sizeof(line), file)) {
        line_no++;
        if (run_line(line) != 0) {
            fprintf(stderr, "error: scenario failed at line %u\n", line_no);
            return 1;
        }
    }

    return 0;
}

static int run_default_demo(void)
{
    char *lines[] = {
        "read door",
        "set 0 1 10",
        "read door",
        "sleep 10",
        "read door",
        "set 2 12 1",
        "read pressure_ma_x100",
        "write relay 1",
        "read relay",
        "set 0 0 20",
        "read door",
    };
    size_t i;

    for (i = 0; i < sizeof(lines) / sizeof(lines[0]); ++i) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s", lines[i]);
        if (run_line(buf) != 0) {
            return 1;
        }
    }

    return 0;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "usage: %s [SCENARIO]\n\n"
            "Commands:\n"
            "  set <driver_channel> <raw> <advance_ms>\n"
            "  sleep <advance_ms>\n"
            "  write <name> <value>\n"
            "  read <name>\n",
            argv0);
}

int main(int argc, char **argv)
{
    FILE *file;

    if (argc > 2) {
        usage(argv[0]);
        return 2;
    }

    dephy_io_posix_sim_reset();
    if (dephy_io_set_driver(dephy_io_posix_sim_driver()) != 0) {
        return 1;
    }
    if (dephy_io_init(default_channels,
                      sizeof(default_channels) / sizeof(default_channels[0])) != 0) {
        return 1;
    }
    dephy_io_set_event_callback(on_event, 0);

    if (argc == 1) {
        return run_default_demo();
    }

    file = fopen(argv[1], "r");
    if (!file) {
        perror(argv[1]);
        return 1;
    }

    int rc = run_file(file);
    fclose(file);
    return rc;
}

