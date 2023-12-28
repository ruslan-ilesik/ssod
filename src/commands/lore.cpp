/************************************************************************************
 * 
 * The Seven Spells Of Destruction
 *
 * Copyright 1993,2001,2023 Craig Edwards <support@sporks.gg>
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
#include <ssod/commands/lore.h>
#include <ssod/database.h>
#include <ssod/game_date.h>
#include <ssod/component_builder.h>
#include <filesystem>
#include <set>

namespace fs = std::filesystem;


std::string to_title(std::string s)
{
	bool last{true};
	for (char& c : s) {
		c = last ? std::toupper(c) : std::tolower(c);
		last = std::isspace(c);
	}
	return s;
}

uint32_t word_count(const std::string& s) {
	uint32_t count{1};
	for (const char& c : s) {
		if (std::isspace(c)) {
			++count;
		}
	}
	return count;
}

uint32_t mins_read_time(const std::string& s) {
	return ceil((double)word_count(s) / 200.0);
}

void page(const dpp::interaction_create_t& event, bool document, const std::string& path = "") {
	dpp::cluster* bot = event.from->creator;
	fs::path fullpath(path);
	size_t pages = 1;
	std::string label = fullpath.filename();
	std::string file_content = document ? dpp::utility::read_file(path) : "";
	std::string whole_doc{file_content};
	std::string title = replace_string(to_title(replace_string(label, "-", " ")), ".md", "");
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(document ? title : "Encyclopaedia Cryptillius")
		.set_author(dpp::embed_author{ 
			.name = "Encyclopaedia Cryptillius", 
			.url = "", 
			.icon_url = "",
			.proxy_icon_url = "",
		})
		.set_colour(0xd5b994)
		.set_image("attachment://app_logo.png")
		.set_description("Select a choice from the options below to read information about the game world, its characters and your quest's background.");
	dpp::message m;
	component_builder cb(m);
	if (document) {
		embed.set_description(file_content);
		fs::path fullpath(path);
		fullpath.remove_filename();
		if (isdigit(label[0])) {
			/* Get name without number on start */
			std::string partial_name = label.substr(label.find("-"), label.length() - label.find("-"));
			size_t current_page = atoi(label);
			/* Count pages */
			while (fs::exists(fullpath.string() + std::to_string(pages) + partial_name)) {
				pages++;
			}
			pages--;
			whole_doc.clear();
			for (size_t p = 1; p <= pages; ++p) {
				cb.add_component(dpp::component()
					.set_type(dpp::cot_button)
					.set_id("lore-read;" + fullpath.string() + std::to_string(p) + partial_name)
					.set_label("Page " + std::to_string(p) + " of " + std::to_string(pages))
					.set_style(dpp::cos_secondary)
					.set_emoji("📘", 0, false)
					.set_disabled(current_page == p)
				);
				whole_doc.append(dpp::utility::read_file(fullpath.string() + std::to_string(p) + partial_name));
			}
			title = replace_string(title, std::to_string(current_page) + " ", "");
		}
		cb.add_component(dpp::component()
			.set_type(dpp::cot_button)
			.set_id("lore;" + fullpath.string() + "/")
			.set_label("Back")
			.set_style(dpp::cos_secondary)
			.set_emoji("⬆", 0, false)
		);
		embed.set_title(title);
		embed.set_footer(dpp::embed_footer{ 
			.text = document ? std::to_string(pages) + " page" + (pages > 1 ? "s" : "") + ", " + std::to_string(mins_read_time(whole_doc)) + " minutes read time" : "Lore Information", 
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		});

		m = cb.get_message();
		for (const auto& ext : std::array<std::string, 3>{"webp", "png", "jpg"}) {
			std::string image_file = replace_string(path, ".md", "") + "." + ext;
			if (fs::exists(image_file)) {
				m.add_file("image." + ext, dpp::utility::read_file(image_file));
				embed.set_image("attachment://image." + ext);
				break;
			}
		}
	} else {
		std::set<fs::directory_entry> sorted_entries;
		for (const auto& entry : fs::directory_iterator(path.empty() ? "../resource/lore/" : path)) {
			sorted_entries.insert(entry);
		}
		for (const auto& entry : sorted_entries) {
			if (entry.is_directory()) {
				/* Show directories as categories */
				cb.add_component(dpp::component()
					.set_type(dpp::cot_button)
					.set_id("lore;" + entry.path().string())
					.set_label(to_title(replace_string(replace_string(entry.path(), "../resource/lore/", ""), "-", " ")))
					.set_style(dpp::cos_secondary)
					.set_emoji("📂", 0, false)
				);
			} else if (entry.is_regular_file()) {
				/* Show regular files as entries */
				fs::path fullpath(entry.path());
				std::string label = fullpath.filename();
				if (fullpath.extension() != ".md") {
					continue;
				}
				if (isdigit(label[0])) {
					/* Paginated content, split into 4k per page */
					if (atoi(label) == 1) {
						/* Only display the first page, take the page number off the name */
						label = label.substr(2, label.length() - 2);
					} else {
						continue;
					}
				}
				fullpath.remove_filename();
				cb.add_component(dpp::component()
					.set_type(dpp::cot_button)
					.set_id("lore-read;" + entry.path().string())
					.set_label(replace_string(to_title(replace_string(label, "-", " ")), ".md", ""))
					.set_style(dpp::cos_secondary)
					.set_emoji("📃", 0, false)
				);
			}
		}
		if (!document && path != "../resource/lore/" && path != "") {
			cb.add_component(dpp::component()
				.set_type(dpp::cot_button)
				.set_id("lore;../resource/lore/")
				.set_label("Back")
				.set_style(dpp::cos_secondary)
				.set_emoji("⬆", 0, false)
			);
		}
		m = cb.get_message();
		m.add_file("app_logo.png", dpp::utility::read_file("../resource/app_logo.png"));
	}
	m.add_embed(embed);
	if (event.command.type == dpp::it_component_button) {
		event.reply(dpp::ir_update_message, m.set_flags(dpp::m_ephemeral), [bot](const auto& cc) {
			if (cc.is_error()) {{
				bot->log(dpp::ll_error, cc.http_info.body);
			}}
		});
	} else {
		event.reply(m.set_flags(dpp::m_ephemeral));
	}
}

dpp::slashcommand lore_command::register_command(dpp::cluster& bot) {
	bot.on_button_click([](const dpp::button_click_t& event) {
		std::vector<std::string> parts = dpp::utility::tokenize(event.custom_id, ";");
		if ((parts[0] == "lore" || parts[0] == "lore-read") && parts.size() >= 2) {
			page(event, parts[0] == "lore-read", parts[1]);
		}
	});
	return dpp::slashcommand("lore", "Show lore pages about the game world", bot.me.id);
}

void lore_command::route(const dpp::slashcommand_t &event) {
	page(event, false);
}
