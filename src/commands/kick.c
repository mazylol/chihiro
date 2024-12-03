#include "commands.h"

void register_kick_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod) {
    struct discord_application_command_option options[] = {
        {.type = DISCORD_APPLICATION_OPTION_USER,
         .name = "user",
         .description = "User to kick",
         .required = true},
        {.type = DISCORD_APPLICATION_OPTION_STRING,
         .name = "reason",
         .description = "Reason for kick",
         .required = false}};

    if (prod) {
        struct discord_create_global_application_command params = {
            .name = "kick",
            .description = "Kick a user",
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
            .description = "Kick a user",
            .options =
                &(struct discord_application_command_options){
                    .size = sizeof(options) / sizeof *options,
                    .array = options,
                },
            .default_permission = DISCORD_PERM_KICK_MEMBERS};

        discord_create_guild_application_command(client, g_app_id, guild_id, &params, NULL);
    }
}

void handle_kick_command(struct discord *client, const struct discord_interaction *event, sqlite3 *db) {
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, "SELECT * FROM Configuration WHERE id = ?", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_bind_int(stmt, 1, event->guild_id);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return;
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to step: %s\n", sqlite3_errmsg(db));
        return;
    }

    // check if kick command is enabled
    if (sqlite3_column_int64(stmt, 2) == 0) {
        struct discord_interaction_response message_params = {
            .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
            .data = &(struct discord_interaction_callback_data){
                .content = "Kick command is disabled"}};

        discord_create_interaction_response(client, event->id, event->token, &message_params, NULL);

        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    u64snowflake user = {0};

    char *reason = "blank";

    for (int i = 0; i < event->data->options->size; ++i) {
        char *name = event->data->options->array[i].name;
        char *value = event->data->options->array[i].value;

        if (0 == strcmp(name, "user")) {
            user = strtoull(value, NULL, 10);
        } else if (0 == strcmp(name, "reason")) {
            reason = value;
        }
    }

    struct discord_remove_guild_member kick_params = {.reason = reason};

    discord_remove_guild_member(client, event->guild_id, user, &kick_params, NULL);

    struct discord_interaction_response message_params = {
        .type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
        .data = &(struct discord_interaction_callback_data){
            .content = "User kicked successfully"}};

    discord_create_interaction_response(client, event->id, event->token, &message_params, NULL);
}
