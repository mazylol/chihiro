#ifndef CHIHIRO_COMMAND_KICK_H
#define CHIHIRO_COMMAND_KICK_H

#include <dpp/dpp.h>

namespace Commands {
    dpp::slashcommand register_kick_command(dpp::cluster *bot);
    auto handle_kick_command(const dpp::slashcommand_t &event) -> dpp::task<void>;
}

#endif // CHIHIRO_COMMAND_KICK_H