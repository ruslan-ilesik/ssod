#pragma once
#include <string>
#include <vector>
#include <dpp/dpp.h>
#include <ssod/game_player.h>
#include <dpp/unicode_emoji.h>

inline const char* directions[10] = {
	dpp::unicode_emoji::zero,
	dpp::unicode_emoji::one,
	dpp::unicode_emoji::two,
	dpp::unicode_emoji::three,
	dpp::unicode_emoji::four,
	dpp::unicode_emoji::five,
	dpp::unicode_emoji::six,
	dpp::unicode_emoji::seven,
	dpp::unicode_emoji::eight,
	dpp::unicode_emoji::nine,
};

enum nav_link_type {
	nav_type_disabled_link,
	nav_type_link,
	nav_type_paylink,
	nav_type_autolink,
};

struct nav_link {
	long paragraph;
	nav_link_type type;
	long cost;
};

struct paragraph {
	uint32_t id{};
	std::string text;
	std::vector<nav_link> navigation_links;
	std::vector<item> dropped_items;
	bool combat_disabled{};
	bool magic_disabled{};
	bool theft_disabled{};
	bool chat_disabled{};

	paragraph() = default;
	~paragraph() = default;
	paragraph(uint32_t paragraph_id, player current, dpp::snowflake user_id);

private:
	void parse(player current_player, dpp::snowflake user_id);
};
