#include <concord/discord.h>

void register_kick_command(struct discord *client, u64snowflake g_app_id, u64snowflake guild_id, bool prod);
void kick_command_handler(struct discord *client, const struct discord_interaction *event);
