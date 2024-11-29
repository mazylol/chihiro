#include "commands.h"

#include <stdlib.h>
#include <string.h>

void register_ban_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod) {
    struct discord_application_command_option options[] = {
        {.type = DISCORD_APPLICATION_OPTION_USER,
         .name = "user",
         .description = "The user to ban",
         .required = true},
        {.type = DISCORD_APPLICATION_OPTION_STRING,
         .name = "reason",
         .description = "The reason for the ban",
         .required = false},
        {.type = DISCORD_APPLICATION_OPTION_INTEGER,
         .name = "message_days",
         .description = "The days of messages to delete",
         .required = false}};

    if (prod) {
        struct discord_create_global_application_command params = {
            .name = "ban",
            .description = "Ban a user from the server",
            .options =
                &(struct discord_application_command_options){
                    .size = sizeof(options) / sizeof *options,
                    .array = options,
                },
            .default_permission = DISCORD_PERM_BAN_MEMBERS};

        discord_create_global_application_command(client, g_app_id, &params, NULL);
    } else {
        struct discord_create_guild_application_command params = {
            .name = "ban",
            .description = "Ban a user from the server",
            .options =
                &(struct discord_application_command_options){
                    .size = sizeof(options) / sizeof *options,
                    .array = options,
                },
            .default_permission = DISCORD_PERM_BAN_MEMBERS};

        discord_create_guild_application_command(client, g_app_id, guild_id, &params, NULL);
    }
};

void handle_ban_command(struct discord *client, const struct discord_interaction *event, sqlite3 *db) {
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, "SELECT * FROM Configuration WHERE id = ?", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_bind_int64(stmt, 1, event->guild_id);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);

    // check if ban command is enabled
    if (sqlite3_column_int(stmt, 1) == 0) {
        struct discord_interaction_response message_params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data){
                .content = "Ban command is disabled"}};

        discord_create_interaction_response(client, event->id, event->token, &message_params, NULL);

        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    u64snowflake user = {0};

    char *reason = "blank";

    int message_days = 0;

    for (int i = 0; i < event->data->options->size; ++i) {
        char *name = event->data->options->array[i].name;
        char *value = event->data->options->array[i].value;

        if (0 == strcmp(name, "user")) {
            sscanf(value, "%" SCNu64, &user);
        } else if (0 == strcmp(name, "reason")) {
            reason = value;
        } else if (0 == strcmp(name, "message_days")) {
            message_days = atoi(value);
        }
    }

    struct discord_create_guild_ban params = {
        .delete_message_days = message_days,
        .reason = reason};

    discord_create_guild_ban(client, event->guild_id, user, &params, NULL);

    struct discord_interaction_response message_params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data){
            .content = "User banned successfully"}};

    discord_create_interaction_response(client, event->id, event->token, &message_params, NULL);
}
