#include "Kick.h"

dpp::slashcommand Commands::register_kick_command(dpp::cluster *bot) {
    auto command = dpp::slashcommand("kick", "Kick a member", bot->me.id);

    auto member = dpp::command_option(dpp::co_user, "member", "The member to kick", true);

    command.add_option(member);

    command.set_default_permissions(dpp::permissions::p_kick_members);

    return command;
}

void Commands::handle_kick_command(dpp::cluster *bot, const dpp::slashcommand_t *event) {
    dpp::snowflake target_id = std::get<dpp::snowflake>(event->get_parameter("user"));
    dpp::guild_member target = event->command.get_resolved_member(target_id);

    bot->guild_member_kick(target.guild_id, target.user_id, [event](const dpp::confirmation_callback_t &callback) -> void {
        if (callback.is_error()) {
            event->reply(dpp::message("Failed to kick user").set_flags(dpp::m_ephemeral));
        } else {
            event->reply(dpp::message("Successfully kicked user").set_flags(dpp::m_ephemeral));
        }
    });
}