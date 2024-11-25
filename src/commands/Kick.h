#ifndef CHIHIRO_COMMAND_KICK_H
#define CHIHIRO_COMMAND_KICK_H

#include <dpp/dpp.h>

namespace Commands {
    dpp::slashcommand register_kick_command(dpp::cluster *bot);
    void handle_kick_command(dpp::cluster *bot, const dpp::slashcommand_t *event);
}

#endif // CHIHIRO_COMMAND_KICK_H