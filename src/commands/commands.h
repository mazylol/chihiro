#include <concord/discord.h>
#include <sqlite3.h>

void register_ban_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod);
void handle_ban_command(struct discord *client, const struct discord_interaction *event, sqlite3 *db);

void register_kick_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod);
void handle_kick_command(struct discord *client, const struct discord_interaction *event, sqlite3 *db);
