#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dotenv.h>

#include "commands/commands.h"

struct env_vars {
    const char *dev_token;
    const char *prod_token;
    u64snowflake guild_id;
    bool prod;
};

struct env_vars load_env_vars() {
    struct env_vars env_vars = {
        .prod = false,
        .guild_id = 0,
        .prod_token = "",
        .dev_token = ""};

    env_load(".", false);

    char *id = getenv("GUILD_ID");
    sscanf(id, "%" SCNu64, &env_vars.guild_id);

    env_vars.dev_token = getenv("DEV_TOKEN");
    env_vars.prod_token = getenv("PROD_TOKEN");

    char *prodValue = getenv("PROD");

    if (prodValue == NULL) {
        env_vars.prod = false;
        return env_vars;
    };

    if (strcmp(prodValue, "0") == 0) {
        env_vars.prod = false;
    } else if (strcmp(prodValue, "1") == 0) {
        env_vars.prod = true;
    }

    return env_vars;
}

struct env_vars vars = {};

void on_ready(struct discord *client, const struct discord_ready *event) {
    struct discord_create_guild_application_command params = {
        .name = "ping", .description = "Ping command!"};

    discord_create_guild_application_command(client, event->application->id,
                                             vars.guild_id, &params, NULL);

    register_kick_command(client, event->application->id, vars.guild_id, vars.prod);
    register_ban_command(client, event->application->id, vars.guild_id, vars.prod);
}

void on_interaction(struct discord *client, const struct discord_interaction *event) {
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) {
        return;
    }

    if (strcmp(event->data->name, "ping") == 0) {
        struct discord_interaction_response params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data){
                .content = "pong"}};

        discord_create_interaction_response(client, event->id, event->token, &params, NULL);
    } else if (strcmp(event->data->name, "kick") == 0) {
        handle_kick_command(client, event);
    } else if (strcmp(event->data->name, "ban") == 0) {
        handle_ban_command(client, event);
    }
}

int main() {
    vars = load_env_vars();

    struct discord *client = {0};

    if (vars.prod) {
        client = discord_init(vars.prod_token);
    } else {
        client = discord_init(vars.dev_token);
    }

    discord_set_on_ready(client, *on_ready);
    discord_set_on_interaction_create(client, *on_interaction);

    discord_run(client);
}
