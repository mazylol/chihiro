#include "commands.h"
#include <stdio.h>
#include <string.h>

void register_kick_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod) {
    struct discord_application_command_option options[] = {
        {.type = DISCORD_APPLICATION_OPTION_USER,
         .name = "user",
         .description = "The user to kick",
         .required = true},
        {.type = DISCORD_APPLICATION_OPTION_STRING,
         .name = "reason",
         .description = "The reason for kick",
         .required = false}};

    if (prod) {
        struct discord_create_global_application_command params = {
            .name = "kick",
            .description = "Kick a user from the server",
            .options =
                &(struct discord_application_command_options){
                    .size = sizeof(options) / sizeof *options,
                    .array = options,
                },
            .default_permission = DISCORD_PERM_KICK_MEMBERS};

        discord_create_global_application_command(client, g_app_id, &params, NULL);
    } else {
        struct discord_create_guild_application_command params = {
            .name = "kick",
            .description = "Kick a user from the server",
            .options =
                &(struct discord_application_command_options){
                    .size = sizeof(options) / sizeof *options,
                    .array = options,
                },
            .default_permission = DISCORD_PERM_KICK_MEMBERS};

        discord_create_guild_application_command(client, g_app_id, guild_id, &params, NULL);
    }
};

void kick_command_handler(struct discord *client, const struct discord_interaction *event) {
    u64snowflake user = {0};

    char *reason = "blank";

    for (int i = 0; i < event->data->options->size; ++i) {
        char *name = event->data->options->array[i].name;
        char *value = event->data->options->array[i].value;

        if (0 == strcmp(name, "user")) {
            sscanf(value, "%" SCNu64, &user);
        } else if (0 == strcmp(name, "reason")) {
            reason = value;
        }
    }

    struct discord_remove_guild_member kick_params = {.reason = reason};

    printf("%lu\n", user);
    printf("%s\n", reason);

    discord_remove_guild_member(client, event->guild_id, user, &kick_params, NULL);

    struct discord_interaction_response message_params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data){
            .content = "User kicked successfully"}};

    discord_create_interaction_response(client, event->id, event->token, &message_params, NULL);
}
