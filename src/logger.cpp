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
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <dpp/dpp.h>
#include <ssod/sentry.h>

namespace logger {

	using spdlog_level = spdlog::level::level_enum;

	constexpr int max_log_size = 1024 * 1024 * 5;

	static std::shared_ptr<spdlog::logger> async_logger;

	void init(const std::string& log_file) {
		/* This shuts up libleptonica, who tf logs errors to stderr in a lib?! */
		int fd = ::open("/dev/null", O_WRONLY);
		::dup2(fd, 2);
		::close(fd);

		/* Set up spdlog logger */
		spdlog::init_thread_pool(8192, 2);
		std::vector<spdlog::sink_ptr> sinks = {
			std::make_shared<spdlog::sinks::stdout_color_sink_mt >(),
			std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, max_log_size, 10)
		};

		async_logger = std::make_shared<spdlog::async_logger>("file_and_console", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block);
		async_logger->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
		async_logger->set_level(spdlog_level::debug);
		
		spdlog::register_logger(async_logger);
	}

	void log(const dpp::log_t & event) {
		if (event.message == "You have attached an event to cluster::on_message_create() but have not specified the privileged intent dpp::i_message_content. Message content, embeds, attachments, and components on received guild messages will be empty.") {
			return;
		}
		switch (event.severity) {
			case dpp::ll_trace:
				async_logger->trace("{}", event.message);
				break;
			case dpp::ll_debug:
				async_logger->debug("{}", event.message);
				break;
			case dpp::ll_info:
				async_logger->info("{}", event.message);
				break;
			case dpp::ll_warning:
				async_logger->warn("{}", event.message);
				break;
			case dpp::ll_error:
				async_logger->error("{}", event.message);
				break;
			case dpp::ll_critical:
				async_logger->critical("{}", event.message);
				break;
		}
	}
}