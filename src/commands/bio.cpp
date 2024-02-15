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
#include <ssod/commands/bio.h>
#include <ssod/game_util.h>
#include <ssod/database.h>
#include <ssod/aes.h>

dpp::slashcommand bio_command::register_command(dpp::cluster& bot) {
	return dpp::slashcommand("bio", "Update player biography", bot.me.id)
		.set_dm_permission(true)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "picture", "Set a custom profile picture")
			.add_option(dpp::command_option(dpp::co_attachment, "image", "Image to upload", true))
		)
                .add_option(
			dpp::command_option(dpp::co_sub_command, "text", "Set custom biography")
			.add_option(dpp::command_option(dpp::co_string, "text", "Biography to set", true))
		);
}

void bio_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster& bot = *event.from->creator;

	dpp::command_interaction cmd_data = event.command.get_command_interaction();
	auto subcommand = cmd_data.options[0];

	dpp::embed embed;
	embed.set_url("https://ssod.org/")
		.set_title("Custom Player Bio")
		.set_footer(dpp::embed_footer{ 
			.text = "Requested by " + event.command.usr.format_username(), 
			.icon_url = bot.me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(0xd5b994);


	auto rs = db::query("SELECT * FROM premium_credits WHERE user_id = ? AND active = 1", { event.command.usr.id });
	if (rs.empty()) {
		premium_required(event);
	} else  if (subcommand.name == "text") {
		auto param = subcommand.options[0].value;
		std::string text = std::get<std::string>(param);
		db::query("INSERT INTO character_bio (user_id, bio) VALUES(?, ?) ON DUPLICATE KEY UPDATE bio = ?", { event.command.usr.id, text, text });
		embed.set_description("Your custom bio text has been set to:\n\n" + text);
		event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
	} else if (subcommand.name == "picture") {
		auto param = subcommand.options[0].value;
            	dpp::snowflake file_id = std::get<dpp::snowflake>(param);
		dpp::attachment att = event.command.get_resolved_attachment(file_id);
		bot.request(att.url, dpp::m_get, [embed, att, event](const dpp::http_request_completion_t& data) {
			std::string filename = event.command.usr.id.str() + fs::path(att.filename).extension().c_str();
			std::fstream file;
			dpp::embed e = embed;
			file.open("../uploads/" + filename, std::ios::app | std::ios::binary);
			file.write(data.body.data(), data.body.length());
			file.close();
			db::query("INSERT INTO character_bio (user_id, image_name) VALUES(?, ?) ON DUPLICATE KEY UPDATE image_name = ?", { event.command.usr.id, filename, filename });
			e.set_description("Your custom bio profile picture has been uploaded.");
			e.set_image("attachment://" + filename);
			event.reply(dpp::message().add_embed(e).add_file(filename, data.body).set_flags(dpp::m_ephemeral));
		});
	}

}
