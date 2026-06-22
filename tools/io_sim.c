#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dephy_industrial_io/industrial_io.h>
#include <dephy_industrial_io/posix_sim.h>

#define IO_SIM_MAX_CHANNELS 64
#define IO_SIM_MAX_LINES 512
#define IO_SIM_MAX_LINE 256
#define IO_SIM_NAME_LEN 64

typedef struct {
    char name[IO_SIM_NAME_LEN];
    dephy_io_channel_config_t cfg;
} channel_entry_t;

typedef struct {
    char lines[IO_SIM_MAX_LINES][IO_SIM_MAX_LINE];
    size_t count;
} scenario_t;

typedef enum {
    OUTPUT_TEXT = 0,
    OUTPUT_JSONL,
    OUTPUT_MQTT,
} output_format_t;

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

static channel_entry_t g_channel_entries[IO_SIM_MAX_CHANNELS];
static dephy_io_channel_config_t g_channels[IO_SIM_MAX_CHANNELS];
static size_t g_channel_count;
static int g_emit_mqtt;
static const char *g_site = "sim-site";
static const char *g_node = "sim-node";
static output_format_t g_output_format = OUTPUT_TEXT;

static void on_event(const dephy_io_event_t *event, void *user);

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

static int parse_type(const char *text, dephy_io_type_t *type)
{
    if (strcmp(text, "di") == 0) {
        *type = DEPHY_IO_DI;
        return 0;
    }
    if (strcmp(text, "do") == 0) {
        *type = DEPHY_IO_DO;
        return 0;
    }
    if (strcmp(text, "ai") == 0) {
        *type = DEPHY_IO_AI;
        return 0;
    }
    if (strcmp(text, "ao") == 0) {
        *type = DEPHY_IO_AO;
        return 0;
    }

    return -1;
}

static int parse_output_format(const char *text, output_format_t *format)
{
    if (strcmp(text, "text") == 0) {
        *format = OUTPUT_TEXT;
        return 0;
    }
    if (strcmp(text, "jsonl") == 0) {
        *format = OUTPUT_JSONL;
        return 0;
    }
    if (strcmp(text, "mqtt") == 0) {
        *format = OUTPUT_MQTT;
        return 0;
    }

    return -1;
}

static int add_channel(const char *name,
                       dephy_io_type_t type,
                       int driver_channel,
                       int debounce_ms,
                       int scale_num,
                       int scale_den,
                       int offset)
{
    channel_entry_t *entry;

    if (!name || driver_channel < 0 || debounce_ms < 0 ||
        scale_den == 0 || g_channel_count >= IO_SIM_MAX_CHANNELS) {
        return -1;
    }

    entry = &g_channel_entries[g_channel_count];
    snprintf(entry->name, sizeof(entry->name), "%s", name);
    entry->cfg.name = entry->name;
    entry->cfg.type = type;
    entry->cfg.driver_channel = (uint16_t)driver_channel;
    entry->cfg.debounce_ms = (uint32_t)debounce_ms;
    entry->cfg.scale_num = scale_num;
    entry->cfg.scale_den = scale_den;
    entry->cfg.offset = offset;
    g_channel_count++;
    return 0;
}

static int load_default_channels(void)
{
    size_t i;

    g_channel_count = 0;
    for (i = 0; i < sizeof(default_channels) / sizeof(default_channels[0]); ++i) {
        if (add_channel(default_channels[i].name,
                        default_channels[i].type,
                        default_channels[i].driver_channel,
                        default_channels[i].debounce_ms,
                        default_channels[i].scale_num,
                        default_channels[i].scale_den,
                        default_channels[i].offset) != 0) {
            return -1;
        }
    }

    return 0;
}

static int init_io(void)
{
    size_t i;

    for (i = 0; i < g_channel_count; ++i) {
        g_channels[i] = g_channel_entries[i].cfg;
    }

    dephy_io_posix_sim_reset();
    if (dephy_io_set_driver(dephy_io_posix_sim_driver()) != 0) {
        return -1;
    }
    if (dephy_io_init(g_channels, g_channel_count) != 0) {
        return -1;
    }
    dephy_io_set_event_callback(on_event, 0);
    return 0;
}

static void on_event(const dephy_io_event_t *event, void *user)
{
    const char *base_topic;

    (void)user;

    if (g_output_format == OUTPUT_TEXT) {
        printf("event %s %s %s %d %u %u\n",
               event_name(event->type),
               event->sample.name,
               type_name(event->sample.type),
               event->sample.value,
               event->sample.fault,
               event->sample.changed_at_ms);
    } else if (g_output_format == OUTPUT_JSONL) {
        printf("{\"kind\":\"event\",\"event\":\"%s\",\"name\":\"%s\","
               "\"type\":\"%s\",\"value\":%d,\"fault\":%u,\"t_ms\":%u}\n",
               event_name(event->type),
               event->sample.name,
               type_name(event->sample.type),
               event->sample.value,
               event->sample.fault,
               event->sample.changed_at_ms);
    }

    if (g_emit_mqtt || g_output_format == OUTPUT_MQTT) {
        base_topic = event->sample.fault ? "fault" : "event";

        printf("mqtt site/%s/node/%s/io/%s/%s "
               "{\"event\":\"%s\",\"type\":\"%s\",\"value\":%d,"
               "\"fault\":%u,\"t_ms\":%u}\n",
               g_site,
               g_node,
               event->sample.name,
               base_topic,
               event_name(event->type),
               type_name(event->sample.type),
               event->sample.value,
               event->sample.fault,
               event->sample.changed_at_ms);
        printf("mqtt site/%s/node/%s/io/%s/state "
               "{\"type\":\"%s\",\"value\":%d,\"fault\":%u,\"t_ms\":%u}\n",
               g_site,
               g_node,
               event->sample.name,
               type_name(event->sample.type),
               event->sample.value,
               event->sample.fault,
               event->sample.changed_at_ms);
    }
}

static int print_read(const char *name)
{
    dephy_io_sample_t sample;

    if (dephy_io_read(name, &sample) != 0) {
        fprintf(stderr, "error: read failed for channel '%s'\n", name);
        return -1;
    }

    if (g_output_format == OUTPUT_TEXT) {
        printf("read %s %s %d %u %u\n",
               sample.name,
               type_name(sample.type),
               sample.value,
               sample.fault,
               sample.changed_at_ms);
    } else if (g_output_format == OUTPUT_JSONL) {
        printf("{\"kind\":\"read\",\"name\":\"%s\",\"type\":\"%s\","
               "\"value\":%d,\"fault\":%u,\"t_ms\":%u}\n",
               sample.name,
               type_name(sample.type),
               sample.value,
               sample.fault,
               sample.changed_at_ms);
    }
    return 0;
}

static int expect_sample(const char *name, int expected_value, int expected_fault)
{
    dephy_io_sample_t sample;

    if (dephy_io_read(name, &sample) != 0) {
        fprintf(stderr, "error: expect failed to read channel '%s'\n", name);
        return -1;
    }

    if (sample.value != expected_value || sample.fault != (uint8_t)expected_fault) {
        fprintf(stderr,
                "error: expect %s value=%d fault=%d, got value=%d fault=%u\n",
                name,
                expected_value,
                expected_fault,
                sample.value,
                sample.fault);
        return -1;
    }

    if (g_output_format == OUTPUT_TEXT) {
        printf("ok expect %s %d %d\n", name, expected_value, expected_fault);
    } else if (g_output_format == OUTPUT_JSONL) {
        printf("{\"kind\":\"ok\",\"assert\":\"expect\",\"name\":\"%s\","
               "\"value\":%d,\"fault\":%d}\n",
               name,
               expected_value,
               expected_fault);
    }
    return 0;
}

static int expect_between(const char *name, int min_value, int max_value, int expected_fault)
{
    dephy_io_sample_t sample;

    if (dephy_io_read(name, &sample) != 0) {
        fprintf(stderr, "error: expect_between failed to read channel '%s'\n", name);
        return -1;
    }

    if (sample.value < min_value ||
        sample.value > max_value ||
        sample.fault != (uint8_t)expected_fault) {
        fprintf(stderr,
                "error: expect_between %s min=%d max=%d fault=%d, "
                "got value=%d fault=%u\n",
                name,
                min_value,
                max_value,
                expected_fault,
                sample.value,
                sample.fault);
        return -1;
    }

    if (g_output_format == OUTPUT_TEXT) {
        printf("ok expect_between %s %d %d %d\n",
               name,
               min_value,
               max_value,
               expected_fault);
    } else if (g_output_format == OUTPUT_JSONL) {
        printf("{\"kind\":\"ok\",\"assert\":\"expect_between\",\"name\":\"%s\","
               "\"min\":%d,\"max\":%d,\"fault\":%d}\n",
               name,
               min_value,
               max_value,
               expected_fault);
    }
    return 0;
}

static int expect_raw(int driver_channel, int expected_raw)
{
    int32_t raw = 0;

    if (driver_channel < 0 ||
        dephy_io_posix_sim_get_raw((uint16_t)driver_channel, &raw) != 0) {
        fprintf(stderr, "error: expect_raw failed for driver channel %d\n", driver_channel);
        return -1;
    }

    if (raw != expected_raw) {
        fprintf(stderr,
                "error: expect_raw channel=%d raw=%d, got raw=%d\n",
                driver_channel,
                expected_raw,
                raw);
        return -1;
    }

    if (g_output_format == OUTPUT_TEXT) {
        printf("ok expect_raw %d %d\n", driver_channel, expected_raw);
    } else if (g_output_format == OUTPUT_JSONL) {
        printf("{\"kind\":\"ok\",\"assert\":\"expect_raw\","
               "\"driver_channel\":%d,\"raw\":%d}\n",
               driver_channel,
               expected_raw);
    }
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

    if (strcmp(cmd, "channel") == 0) {
        return 0;
    }

    if (strcmp(cmd, "fault") == 0) {
        char state[16];

        if (sscanf(line, "%31s %d %15s %d", cmd, &channel, state, &advance_ms) != 4) {
            fprintf(stderr, "error: fault syntax: fault <driver_channel> <on|off> <advance_ms>\n");
            return -1;
        }
        if (dephy_io_posix_sim_set_fault((uint16_t)channel,
                                         strcmp(state, "on") == 0) != 0) {
            fprintf(stderr, "error: fault failed for driver channel %d\n", channel);
            return -1;
        }
        dephy_io_posix_sim_advance_ms((uint32_t)advance_ms);
        return dephy_io_poll();
    }

    if (strcmp(cmd, "stuck") == 0) {
        char state[16];

        if (sscanf(line, "%31s %d %15s %d %d",
                   cmd, &channel, state, &value, &advance_ms) != 5) {
            fprintf(stderr,
                    "error: stuck syntax: stuck <driver_channel> <on|off> <raw> <advance_ms>\n");
            return -1;
        }
        if (dephy_io_posix_sim_set_stuck((uint16_t)channel,
                                         strcmp(state, "on") == 0,
                                         value) != 0) {
            fprintf(stderr, "error: stuck failed for driver channel %d\n", channel);
            return -1;
        }
        dephy_io_posix_sim_advance_ms((uint32_t)advance_ms);
        return dephy_io_poll();
    }

    if (strcmp(cmd, "noise") == 0) {
        if (sscanf(line, "%31s %d %d %d", cmd, &channel, &value, &advance_ms) != 4) {
            fprintf(stderr, "error: noise syntax: noise <driver_channel> <raw_span> <advance_ms>\n");
            return -1;
        }
        if (dephy_io_posix_sim_set_noise((uint16_t)channel, value) != 0) {
            fprintf(stderr, "error: noise failed for driver channel %d\n", channel);
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

    if (strcmp(cmd, "expect") == 0) {
        int expected_fault;

        if (sscanf(line, "%31s %63s %d %d", cmd, name, &value, &expected_fault) != 4) {
            fprintf(stderr, "error: expect syntax: expect <name> <value> <fault>\n");
            return -1;
        }
        return expect_sample(name, value, expected_fault);
    }

    if (strcmp(cmd, "expect_between") == 0) {
        int min_value;
        int max_value;
        int expected_fault;

        if (sscanf(line, "%31s %63s %d %d %d",
                   cmd, name, &min_value, &max_value, &expected_fault) != 5) {
            fprintf(stderr,
                    "error: expect_between syntax: "
                    "expect_between <name> <min> <max> <fault>\n");
            return -1;
        }
        return expect_between(name, min_value, max_value, expected_fault);
    }

    if (strcmp(cmd, "expect_raw") == 0) {
        if (sscanf(line, "%31s %d %d", cmd, &channel, &value) != 3) {
            fprintf(stderr, "error: expect_raw syntax: expect_raw <driver_channel> <raw>\n");
            return -1;
        }
        return expect_raw(channel, value);
    }

    fprintf(stderr, "error: unknown command '%s'\n", cmd);
    return -1;
}

static int load_scenario(FILE *file, scenario_t *scenario)
{
    char line[IO_SIM_MAX_LINE];

    scenario->count = 0;
    while (fgets(line, sizeof(line), file)) {
        if (scenario->count >= IO_SIM_MAX_LINES) {
            fprintf(stderr, "error: too many scenario lines\n");
            return -1;
        }
        snprintf(scenario->lines[scenario->count],
                 sizeof(scenario->lines[scenario->count]),
                 "%s",
                 line);
        scenario->count++;
    }

    return 0;
}

static int configure_channels_from_scenario(const scenario_t *scenario)
{
    size_t i;
    int saw_channel = 0;

    g_channel_count = 0;
    for (i = 0; i < scenario->count; ++i) {
        char cmd[32];
        char name[IO_SIM_NAME_LEN];
        char type_text[16];
        dephy_io_type_t type;
        int driver_channel;
        int debounce_ms;
        int scale_num;
        int scale_den;
        int offset = 0;
        int fields;

        if (scenario->lines[i][0] == '\0' ||
            scenario->lines[i][0] == '\n' ||
            scenario->lines[i][0] == '#') {
            continue;
        }

        if (sscanf(scenario->lines[i], "%31s", cmd) != 1 ||
            strcmp(cmd, "channel") != 0) {
            continue;
        }

        fields = sscanf(scenario->lines[i],
                        "%31s %63s %15s %d %d %d %d %d",
                        cmd,
                        name,
                        type_text,
                        &driver_channel,
                        &debounce_ms,
                        &scale_num,
                        &scale_den,
                        &offset);
        if (fields != 7 && fields != 8) {
            fprintf(stderr,
                    "error: channel syntax: channel <name> <di|do|ai|ao> "
                    "<driver_channel> <debounce_ms> <scale_num> <scale_den> [offset]\n");
            return -1;
        }
        if (parse_type(type_text, &type) != 0 ||
            add_channel(name, type, driver_channel, debounce_ms,
                        scale_num, scale_den, offset) != 0) {
            fprintf(stderr, "error: invalid channel definition at scenario line %zu\n", i + 1);
            return -1;
        }
        saw_channel = 1;
    }

    if (!saw_channel) {
        return load_default_channels();
    }

    return 0;
}

static int run_scenario(const scenario_t *scenario)
{
    size_t i;

    for (i = 0; i < scenario->count; ++i) {
        char line[IO_SIM_MAX_LINE];

        snprintf(line, sizeof(line), "%s", scenario->lines[i]);
        if (run_line(line) != 0) {
            fprintf(stderr, "error: scenario failed at line %zu\n", i + 1);
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

    if (load_default_channels() != 0 || init_io() != 0) {
        return 1;
    }

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
            "usage: %s [--mqtt] [--format text|jsonl|mqtt] "
            "[--site SITE] [--node NODE] [SCENARIO]\n\n"
            "Commands:\n"
            "  channel <name> <di|do|ai|ao> <driver_channel> "
            "<debounce_ms> <scale_num> <scale_den> [offset]\n"
            "  set <driver_channel> <raw> <advance_ms>\n"
            "  fault <driver_channel> <on|off> <advance_ms>\n"
            "  stuck <driver_channel> <on|off> <raw> <advance_ms>\n"
            "  noise <driver_channel> <raw_span> <advance_ms>\n"
            "  sleep <advance_ms>\n"
            "  write <name> <value>\n"
            "  read <name>\n"
            "  expect <name> <value> <fault>\n"
            "  expect_between <name> <min> <max> <fault>\n"
            "  expect_raw <driver_channel> <raw>\n",
            argv0);
}

int main(int argc, char **argv)
{
    FILE *file;
    scenario_t scenario;
    int rc;
    const char *scenario_path = 0;
    int i;

    for (i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--mqtt") == 0) {
            g_emit_mqtt = 1;
        } else if (strcmp(argv[i], "--format") == 0) {
            if (++i >= argc || parse_output_format(argv[i], &g_output_format) != 0) {
                usage(argv[0]);
                return 2;
            }
        } else if (strcmp(argv[i], "--site") == 0) {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }
            g_site = argv[i];
        } else if (strcmp(argv[i], "--node") == 0) {
            if (++i >= argc) {
                usage(argv[0]);
                return 2;
            }
            g_node = argv[i];
        } else if (strcmp(argv[i], "--help") == 0) {
            usage(argv[0]);
            return 0;
        } else if (!scenario_path) {
            scenario_path = argv[i];
        } else {
            usage(argv[0]);
            return 2;
        }
    }

    if (!scenario_path) {
        return run_default_demo();
    }

    file = fopen(scenario_path, "r");
    if (!file) {
        perror(scenario_path);
        return 1;
    }

    if (load_scenario(file, &scenario) != 0) {
        fclose(file);
        return 1;
    }
    fclose(file);

    if (configure_channels_from_scenario(&scenario) != 0 || init_io() != 0) {
        return 1;
    }

    rc = run_scenario(&scenario);
    return rc;
}
