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
#include <ssod/commands/help.h>
#include <fmt/format.h>

dpp::slashcommand help_command::register_command(dpp::cluster& bot)
{
	return _(dpp::slashcommand("cmd_help", "help_description", bot.me.id).set_dm_permission(true));
}

void help_command::route(const dpp::slashcommand_t &event)
{
	dpp::cluster* bot = event.from->creator;
	dpp::embed embed = dpp::embed()
		.set_url("https://ssod.org/")
		.set_title(_("HELP_TITLE", event))
		.set_footer(dpp::embed_footer{ 
			.text = _("REQUESTED_BY", event, event.command.usr.format_username()),
			.icon_url = bot->me.get_avatar_url(), 
			.proxy_url = "",
		})
		.set_colour(EMBED_COLOUR)
		.set_description(_("HELP_TOP", event))
		.add_field(_("cmd_help", event), _("HELP_HELP_DESC", event), false)
		.add_field(_("cmd_start", event), _("HELP_START_DESC", event), false)
		.add_field(_("cmd_lore", event), _("HELP_LORE_DESC", event), false)
		.add_field(_("cmd_map", event), _("HELP_MAP_DESC", event), false)
		.add_field(_("cmd_vote", event), _("HELP_VOTE_DESC", event), false)
		.add_field(_("cmd_profile", event), _("HELP_PROFILE_DESC", event), false)
		.add_field(_("cmd_guild", event), _("HELP_GUILD_DESC", event), false)
		.add_field(_("cmd_gender", event), _("HELP_GENDER_DESC", event), false)
		.add_field(_("cmd_premium", event), _("HELP_PREMIUM_DESC", event), false)
		.add_field(_("cmd_bio", event), _("HELP_BIO_DESC", event), false)
		.add_field(_("cmd_rename", event), _("HELP_RENAME_DESC", event), false)
		.add_field(_("cmd_info", event), _("HELP_INFO_DESC", event), false)
		;

	event.reply(dpp::message().add_embed(embed).set_flags(dpp::m_ephemeral));
}
