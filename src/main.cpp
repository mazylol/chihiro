#include <dotenvloader.hpp>
#include <dpp/dpp.h>
#include <fmt/format.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "commands/Kick.h"

int main() {
    dotenvloader::load();

    bool prod = std::getenv("PROD");

    std::string token;

    if (prod) {
        token = std::getenv("PROD_TOKEN");
    } else {
        token = std::getenv("DEV_TOKEN");
    }

    dpp::cluster bot(token, dpp::intents::i_all_intents);

    bot.on_log(dpp::utility::cout_logger());

    auto commandRegisters = std::vector<dpp::slashcommand>();
    commandRegisters.push_back(Commands::register_kick_command(&bot));

    bot.on_slashcommand([&bot, &prod](const dpp::slashcommand_t &event) -> dpp::task<void> {
        if (const std::string cmdName = event.command.get_command_name();
            cmdName == "shutdown" && !prod) {

            event.reply("Bot shutting down...");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            bot.shutdown();
        } else if (cmdName == "kick") {
            Commands::handle_kick_command(event);
        }

        co_return;
    });

    bot.on_ready([&bot, &prod, &commandRegisters](const dpp::ready_t & /*event*/) {
        bot.log(dpp::ll_info, fmt::format("Logged in as {}#{}", bot.me.username, bot.me.discriminator));

        std::string guildId(std::getenv("DEV_GUILD_ID"));

        bot.log(dpp::ll_info, fmt::format("Running in {} mode", prod ? "production" : "development"));

        if (dpp::run_once<struct register_bot_commands>()) {
            if (!prod) {
                bot.guild_bulk_command_create(commandRegisters, guildId);

                bot.guild_command_create(
                    dpp::slashcommand("shutdown", "Turn off the bot", bot.me.id).set_default_permissions(dpp::permissions::p_administrator), guildId);
            } else {
                bot.global_bulk_command_create(commandRegisters);
            }
        }

        bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_watching, "you"));
    });

    bot.start(dpp::st_wait);

    return 0;
}
