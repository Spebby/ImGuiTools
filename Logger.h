#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <vector>
#include <tuple>
#include "imgui/imgui.h"

class Logger {
	public:
	enum Level {
		INFO    = 1 << 0, // 001
		WARNING = 1 << 1, // 010
		ERROR   = 1 << 2  // 100
	};

	Logger();
	~Logger();

	void saveLog();
	/**
	 * @abstract Which levels should be rendered.
	 */
	void setLevel(const Level);
	void log(const std::string&);
	void log(const Level, const std::string&);
	std::string levelToStr(const Logger::Level) const;

	// awesome gui stuff
	void draw();

	private:
	// rows to be rendered by the logger.
	std::vector<std::tuple<Level, std::string, std::string>> rows;
	/** Is the GUI out of date? */
	bool dirty;
	short visLevel;
	std::ofstream outfile;
    const std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
};