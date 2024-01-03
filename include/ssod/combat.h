/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#pragma once
#include <dpp/dpp.h>
#include <ssod/game_player.h>

struct combat_state {
	dpp::snowflake opponent{};
	bool accepted{false};
	bool my_turn;
};

bool has_active_pvp(const dpp::snowflake id);

void remove_pvp(const dpp::snowflake id);

player end_pvp_combat(const dpp::interaction_create_t& event);

void challenge_pvp(const dpp::interaction_create_t& event, const dpp::snowflake id);

void update_opponent_message(const dpp::interaction_create_t& event, const dpp::message& m);

dpp::message get_pvp_round(const dpp::interaction_create_t& event);

player set_in_pvp_combat(const dpp::interaction_create_t& event);

dpp::snowflake get_pvp_opponent_id(const dpp::snowflake id);

player get_pvp_opponent(const dpp::snowflake id, dpp::discord_client* shard);

void accept_pvp(const dpp::snowflake id1, const dpp::snowflake id2);

bool pvp_combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts);

bool combat_nav(const dpp::button_click_t& event, player p, const std::vector<std::string>& parts);

void continue_combat(const dpp::interaction_create_t& event, player p);

void continue_pvp_combat(const dpp::interaction_create_t& event, player p);