#include <iostream>
#include <thread>
#include "Discord.hpp"
#include "Utils.hpp"

std::string DiscordBot::token;
std::string DiscordBot::channel;
Artist* DiscordBot::artistPtr;

DiscordBot::DiscordBot():bot(token, dpp::i_default_intents | dpp::i_message_content) {
  /* Output simple log messages to stdout */
	bot.on_log(dpp::utility::cout_logger());

	/* Handle slash command */
	bot.on_slashcommand([](const dpp::slashcommand_t& event) {
		if (event.command.get_command_name() == "ping") {
			event.reply("Pong!");
		} else {
      std::cout << event.command.get_command_name() << std::endl;
    }
	});

	/* Register slash command here in on_ready */
	bot.on_ready([&bot = bot](const dpp::ready_t& event) {
		/* Wrap command registration in run_once to make sure it doesnt run on every full reconnection */
		if (dpp::run_once<struct register_bot_commands>()) {
			bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));
		}
	});

  bot.on_message_create([](const dpp::message_create_t &event) {
	if (event.msg.channel_id.str() != channel) return;
	
    std::cout << event.msg.content << std::endl;
    std::cout << event.msg.author.format_username() << std::endl;
    std::cout << event.msg.author.global_name << std::endl;

	// CmdHandler(event.msg.content, artistPtr);
  });
}

void DiscordBot::start(){
	/* Start the bot */
	dcThread = std::thread(&dpp::cluster::start, std::ref(bot), dpp::st_wait);
}

void DiscordBot::sendDCMessage(std::string text) {
  dpp::message m(channel, text);
  bot.message_create(m);
}

DiscordBot& DiscordBot::getDiscordBot() {
	static DiscordBot ref;
	return ref;
}

void DiscordBot::init(std::string _token, std::string _ch, Artist* _ptr) {
	token = _token;
	channel = _ch;
	artistPtr = _ptr;
}