#include "Kick.h"

dpp::slashcommand Commands::register_kick_command(dpp::cluster *bot) {
    auto command = dpp::slashcommand("kick", "Kick a member", bot->me.id);

    auto member = dpp::command_option(dpp::co_user, "member", "The member to kick", true);

    command.add_option(member);

    command.set_default_permissions(dpp::permissions::p_kick_members);

    return command;
}

auto Commands::handle_kick_command(const dpp::slashcommand_t &event) -> dpp::task<void> {
    dpp::cluster *bot = event.from->creator;

    auto member = std::get<dpp::snowflake>(event.get_parameter("member"));

    dpp::confirmation_callback_t completion = co_await bot->co_guild_member_kick(event.command.guild_id, member);

    if (completion.is_error()) {
        event.reply("Failed to kick member");
    } else {
        event.reply("Member kicked");
    }
}