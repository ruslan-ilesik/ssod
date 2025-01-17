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
#include <ssod/database.h>
#include <ssod/commands/profile.h>
#include <ssod/game_player.h>
#include <ssod/emojis.h>
#include <fmt/format.h>

dpp::slashcommand profile_command::register_command(dpp::cluster& bot) {
	return _(dpp::slashcommand("cmd_profile", "profile_desc", bot.me.id)
		.set_dm_permission(true)
		.add_option(dpp::command_option(dpp::co_string, "opt_user", "user_profile_desc", false)));
}

void profile_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;
	auto param = event.get_parameter("user");
	std::string user;
	player p;
	bool self{false};
	if (param.index() == 0) {
		if (player_is_live(event)) {
			p = get_live_player(event);
			user = p.name;
			self = true;
		}
	} else {
		user = std::get<std::string>(event.get_parameter("user"));
	}
	auto rs = db::query("SELECT * FROM game_users WHERE name = ?", {user});
	if (rs.empty()) {
		event.reply(dpp::message(_(self ? "NOPROFILE" : "NOSUCHUSER", event)).set_flags(dpp::m_ephemeral));
		return;
	}
	p.experience = atol(rs[0].at("experience"));
	auto g = db::query("SELECT * FROM guild_members JOIN guilds ON guild_id = guilds.id WHERE user_id = ?", {rs[0].at("user_id")});

	std::string content{"### " + _("LEVEL", event) + " " + std::to_string(p.get_level()) + " " + std::string(race((player_race)atoi(rs[0].at("race")))) + " " + std::string(profession((player_profession)atoi(rs[0].at("profession")))) +  "\n"};
	int percent = p.get_percent_of_current_level();
	for (int x = 0; x < 100; x += 10) {
		if (x < percent) {
			content += sprite::bar_green.get_mention();
		} else {
			content += sprite::bar_red.get_mention();
		}
	}
	content += " (" + std::to_string(percent) + "%)";

	if (!g.empty()) {
		content += "\n\n**" + _("GUILD", event) + ":** " + dpp::utility::markdown_escape(g[0].at("name"));
	}

	player p2(atol(rs[0].at("user_id")), false);

	std::string file = matrix_image((player_race)atoi(rs[0].at("race")), (player_profession)atoi(rs[0].at("profession")), rs[0].at("gender") == "male");

	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(_("PROFILE", event) + ": " + dpp::utility::markdown_escape(user))
		.set_footer(dpp::embed_footer{ 
			.text = _("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(content)
		.set_image(file)
		.add_field(_("Stamina", event), sprite::health_heart.get_mention() + " " + rs[0].at("stamina") + "/" + std::to_string(p2.max_stamina()), true)
		.add_field(_("Skill", event), sprite::book07.get_mention() + " " + rs[0].at("skill") + "/" + std::to_string(p2.max_skill()), true)
		.add_field(_("Luck", event), sprite::clover.get_mention() + " " + rs[0].at("luck") + "/" + std::to_string(p2.max_luck()), true)
		.add_field("XP", sprite::medal01.get_mention() + " " + rs[0].at("experience"), true)
		.add_field(_("Speed", event), sprite::shoes03.get_mention() + " " + rs[0].at("speed") + "/" + std::to_string(p2.max_speed()), true)
		.add_field(_("Sneak", event), sprite::throw05.get_mention() + " " + rs[0].at("sneak") + "/" + std::to_string(p2.max_sneak()), true)
		.add_field(_("Gold", event), sprite::gold_coin.get_mention() + " " + rs[0].at("gold") + "/" + std::to_string(p2.max_gold()), true)
		.add_field(_("Mana", event), sprite::hat02.get_mention() + " " + rs[0].at("mana") + "/" + std::to_string(p2.max_mana()), true)
		.add_field(_("Armour", event), sprite::helm03.get_mention() + " " + rs[0].at("armour_rating") + " (" + rs[0].at("armour") + ")", true)
		.add_field(_("Weapon", event), sprite::axe013.get_mention() + " " + rs[0].at("weapon_rating") + " (" + rs[0].at("weapon") + ")", true)
		.add_field(_("Notoriety", event), sprite::helm01.get_mention() + " " + rs[0].at("notoriety"), true)
		.add_field(_("Rations", event), sprite::cheese.get_mention() + " " + rs[0].at("rations"), true)
		.add_field(_("Scrolls", event), sprite::scroll.get_mention() + " " + rs[0].at("scrolls"), true)
		;

	auto premium = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { rs[0].at("user_id") });
	if (!premium.empty()) {
		auto bio = db::query("SELECT * FROM character_bio WHERE user_id = ?", { rs[0].at("user_id") });
		if (!bio.empty()) {
			if (!bio[0].at("bio").empty()) {
				embed.set_description(content + "\n### " + _("BIOGRAPHY", event) + "\n" + dpp::utility::markdown_escape(bio[0].at("bio")) + "\n\n");
			}
			if (!bio[0].at("image_name").empty()) {
				embed.set_image("https://premium.ssod.org/profiles/" + bio[0].at("image_name"));
			}
		}
	}
	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));	
}
