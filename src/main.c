#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dotenv.h>
#include <sqlite3.h>

#include "commands/commands.h"
#include "util/util.h"

sqlite3 *db;

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

    env_load("../", false);

    char *id = getenv("GUILD_ID");
    env_vars.guild_id = strtoull(id, NULL, 10);

    env_vars.dev_token = getenv("DEV_TOKEN");
    env_vars.prod_token = getenv("PROD_TOKEN");

    char *prodValue = getenv("PROD");

    if (prodValue == NULL) {
        env_vars.prod = false;
        return env_vars;
    }

    if (strcmp(prodValue, "0") == 0) {
        env_vars.prod = false;
    } else if (strcmp(prodValue, "1") == 0) {
        env_vars.prod = true;
    }

    return env_vars;
}

struct env_vars vars = {};

void on_ready(struct discord *client, const struct discord_ready *event) {
    struct discord_activity activities[] = {
        {
            .name = "with Enten",
            .type = DISCORD_ACTIVITY_GAME,
            .details = "Hunting down my fathers killers",
        },
    };

    struct discord_presence_update status = {
        .activities =
            &(struct discord_activities){
                .size = sizeof(activities) / sizeof *activities,
                .array = activities,
            },
        .status = "online",
        .afk = false,
        .since = discord_timestamp(client),
    };

    discord_update_presence(client, &status);

    register_kick_command(client, event->application->id, vars.guild_id, vars.prod);
    register_ban_command(client, event->application->id, vars.guild_id, vars.prod);
}

void on_interaction(struct discord *client, const struct discord_interaction *event) {
    if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) {
        return;
    }

    if (strcmp(event->data->name, "kick") == 0) {
        handle_kick_command(client, event, db);
    } else if (strcmp(event->data->name, "ban") == 0) {
        handle_ban_command(client, event, db);
    }
}

void on_guild_create(struct discord *client, const struct discord_guild *event) {
    sqlite3_stmt *stmt;

    int rc = sqlite3_prepare_v2(db, "SELECT * FROM Configuration WHERE id = ?", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_bind_int64(stmt, 1, event->id);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    rc = sqlite3_prepare_v2(db, "INSERT INTO Configuration (id, ban_command_enabled, kick_command_enabled) VALUES (?, 0, 0)", -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_bind_int64(stmt, 1, event->id);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        return;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert row: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_finalize(stmt);

    printf("Added guild %lu to the database\n", event->id);
}

int main() {
    vars = load_env_vars();

    int rc;

    if (!file_exists("../chihiro.db")) {
        rc = sqlite3_open("../chihiro.db", &db);

        if (rc != SQLITE_OK) {
            printf("Error opening SQLite DB: %s\n", sqlite3_errmsg(db));
        }

        rc = sqlite3_exec(db, "CREATE TABLE Configuration (id INTEGER PRIMARY KEY, ban_command_enabled INTEGER, kick_command_enabled INTEGER)", 0, 0, 0);

        if (rc != SQLITE_OK) {
            printf("Error creating table: %s\n", sqlite3_errmsg(db));
        }
    } else {
        rc = sqlite3_open("../chihiro.db", &db);

        if (rc != SQLITE_OK) {
            printf("Error opening SQLite DB in memory: %s\n", sqlite3_errmsg(db));
        }
    }

    struct discord *client = {0};

    if (vars.prod) {
        client = discord_init(vars.prod_token);
    } else {
        client = discord_init(vars.dev_token);
    }

    discord_set_on_ready(client, &on_ready);
    discord_set_on_interaction_create(client, &on_interaction);
    discord_set_on_guild_create(client, &on_guild_create);

    discord_run(client);

    sqlite3_close(db);
}
