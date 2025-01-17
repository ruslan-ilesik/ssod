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
#include <ssod/ssod.h>
#include <ssod/parser.h>
#include <ssod/database.h>

bool comparison(std::string condition, long C1, const std::string& C2, int g_dice) {
	long C = C2 == "dice" ? g_dice : atol(C2);
	condition = dpp::lowercase(condition);
	if (condition == "eq" && C1 == C) {
		return true;
	} else if (condition == "gt" && C1 > C) {
		return true;
	} else if (condition == "gte" && C1 >= C) {
		return true;
	} else if (condition == "lt" && C1 < C) {
		return true;
	} else if (condition == "lte" && C1 <= C) {
		return true;
	} else if (condition == "ne" && C1 != C) {
		return true;
	} else {
		return false;
	}
}

struct if_tag : public tag {
	if_tag() { register_tag<if_tag>(); }
	static constexpr bool overrides_display{true};
	static constexpr std::string_view tags[]{"<if"};
	static void route(paragraph& p, std::string& p_text, std::stringstream& paragraph_content, std::stringstream& output, player& current_player) {
		std::string condition;
		paragraph_content >> p_text;
		// -------------------------------------------------------
		// <if item multi-word-item-name>
		// -------------------------------------------------------
		if (dpp::lowercase(p_text) == "item") {
			paragraph_content >> p_text;
			extract_to_quote(p_text, paragraph_content, '>');
			p_text = remove_last_char(p_text);
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(current_player.has_herb(p_text) || current_player.has_spell(p_text) || current_player.has_possession(p_text));
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "!item") {
			paragraph_content >> p_text;
			extract_to_quote(p_text, paragraph_content, '>');
			p_text = remove_last_char(p_text);
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(!current_player.has_herb(p_text) && !current_player.has_spell(p_text) && !current_player.has_possession(p_text));
			} else {
				p.display.push_back(false);
			}
			return;
		} if (dpp::lowercase(p_text) == "has") {
			size_t number{};
			paragraph_content >> number;
			paragraph_content >> p_text;
			extract_to_quote(p_text, paragraph_content, '>');
			p_text = remove_last_char(p_text);
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				size_t n{};
				if (current_player.has_possession(p_text)) {
					for (const auto& item : current_player.possessions) {
						n += (dpp::lowercase(item.name) == dpp::lowercase(p_text));
					}
				}
				p.display.push_back(n >= number);
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "flag") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			std::string flag = "[gamestate_" + p_text;
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(current_player.gotfrom.find(flag) != std::string::npos || global_set(p_text));
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "!flag") {
			paragraph_content >> p_text;
			p_text = remove_last_char(p_text);
			std::string flag = "[gamestate_" + p_text;
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(current_player.gotfrom.find(flag) == std::string::npos && !global_set(p_text));
			} else {
				p.display.push_back(false);
			}
			return;
		}
		// -------------------------------------------------------
		// <if scorename gt|lt|eq value>
		// -------------------------------------------------------
		std::string scorename = dpp::lowercase(p_text);
		const std::map<std::string, long> scorename_map = {
			{ "exp", current_player.experience },
			{ "dice", p.g_dice },
			{ "stm", current_player.stamina },
			{ "skl", current_player.skill },
			{ "arm", current_player.armour.rating },
			{ "wpn", current_player.weapon.rating },
			{ "day", current_player.days },
			{ "spd", current_player.speed },
			{ "luck", current_player.luck },
			{ "scrolls", current_player.scrolls },
			{ "level", current_player.get_level() },
			{ "mana", current_player.mana },
			{ "notoriety", current_player.notoriety },
			{ "rations", current_player.rations },
		};
		auto check = scorename_map.find(scorename);
		if (check != scorename_map.end()) {
			paragraph_content >> condition;
			paragraph_content >> p_text;
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(comparison(condition, check->second, p_text, p.g_dice));
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "race") {
			// ------------------------------------------------------
			// <if race x>
			// ------------------------------------------------------
			// if false, nothing displayed until an <endif> is reached.
			paragraph_content >> p_text;
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(
					(dpp::lowercase(p_text) == "human>" && (current_player.race == race_human || current_player.race == race_barbarian))
					||
					(dpp::lowercase(p_text) == "orc>" && (current_player.race == race_orc || current_player.race == race_goblin))
					||
					(dpp::lowercase(p_text) == "elf>" && (current_player.race == race_elf || current_player.race == race_dark_elf))
					||
					(dpp::lowercase(p_text) == "dwarf>" && current_player.race == race_dwarf)
					||
					(dpp::lowercase(p_text) == "lesserorc>" && current_player.race == race_lesser_orc)
				);
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "prof") {
			// ------------------------------------------------------
			// <if prof x>
			// ------------------------------------------------------
			// if false, nothing displayed until an <endif> is reached.
			paragraph_content >> p_text;
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(
					(dpp::lowercase(p_text) == "warrior>" && (current_player.profession == prof_warrior || current_player.profession == prof_mercenary))
						||
					(dpp::lowercase(p_text) == "wizard>" && current_player.profession == prof_wizard)
						||
					(dpp::lowercase(p_text) == "thief>" && (current_player.profession == prof_thief || current_player.profession == prof_assassin))
						||
					(dpp::lowercase(p_text) == "woodsman>" && current_player.profession == prof_woodsman)
				);
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "mounted>") {
			// ------------------------------------------------------
			// <if mounted>
			// ------------------------------------------------------
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(current_player.has_flag("horse"));
			} else {
				p.display.push_back(false);
			}
			return;
		} else if (dpp::lowercase(p_text) == "premium>") {
			// ------------------------------------------------------
			// <if premium>
			// ------------------------------------------------------
			auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { current_player.event.command.usr.id });
			if (p.display.empty() || p.display[p.display.size() - 1]) {
				p.display.push_back(!rs.empty());
			} else {
				p.display.push_back(false);
			}
			return;
		}
	}
};

static if_tag self_init;
