#include <ssod/game_date.h>
#include <fmt/format.h>
#include <array>

const std::array<std::string, 5> day{
	"Lunae",
	"Martis",
	"Jovis",
	"Veneris",
	"Saturni"
};

const std::array<std::string, 6> month{
	"Ivonium",
	"Hornath",
	"Illium",
	"Vernum",
	"Ulium",
	"Nixium"
};

const std::array<std::string, 4> fraction{
	"Dead hours",
	"Morning",
	"Afternoon",
	"Evening"
};

const int year_modifier = 1542;

std::string game_date() {
	time_t a = time(NULL);
	tm* realtime = localtime(&a);
	int yday = realtime->tm_yday + 1;
	int game_month = (yday / 60);
	int game_index = (yday % 31)+1;
	int game_day = yday % 5;
	int game_year = (realtime->tm_year / 2) + year_modifier;
	int frac = (((realtime->tm_hour)+1) / 6);
	frac = std::min(3, std::max(0, frac));
	game_day = std::min(4, std::max(0, game_day));
	game_month = std::min(5, std::max(0, game_month));
	return fmt::format("{}, {}, {} {} in the year {}\n{} days remaining this round.", fraction[frac], day[game_day], game_index, month[game_month], game_year, 50);
}