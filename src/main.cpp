#include <dotenvloader.hpp>
#include <dpp/dpp.h>
#include <fmt/format.h>

#include <chrono>
#include <string>
#include <thread>

int main() {
    dotenvloader::load();

    bool prod = std::getenv("PROD");

    std::string token;

    if (prod) {
        token = std::getenv("PROD_TOKEN");
    } else {
        token = std::getenv("DEV_TOKEN");
    }

    dpp::cluster bot(token);

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([&bot, &prod](const dpp::slashcommand_t &event) {
        if (const std::string cmdName = event.command.get_command_name();
            cmdName == "shutdown" && !prod) {

            event.reply("Bot shutting down...");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            bot.shutdown();
        }
    });

    bot.on_ready([&bot, &prod](const dpp::ready_t & /*event*/) {
        bot.log(dpp::ll_info, fmt::format("Logged in as {}#{}", bot.me.username, bot.me.discriminator));

        std::string guildId(std::getenv("DEV_GUILD_ID"));

        bot.log(dpp::ll_info, fmt::format("Running in {} mode", prod ? "production" : "development"));

        if (dpp::run_once<struct register_bot_commands>()) {
            if (!prod) {
                bot.guild_command_create(
                    dpp::slashcommand("shutdown", "Turn off the bot", bot.me.id).set_default_permissions(dpp::permissions::p_administrator), guildId);
            }
        }

        bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_watching, "you"));
    });

    bot.start(dpp::st_wait);

    return 0;
}
