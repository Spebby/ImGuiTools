#include "Logger.h"

//#include <GL/glew.h>   // Include GLEW for OpenGL extensions
//#include <GL/gl.h>	 // Include the standard OpenGL header

#include <ctime>
#include <iomanip> // nice time formatting
#include <sstream>
#include <algorithm>
#include <filesystem>

/**
 * @return string of the current time, formatted as Y:M:D H:M:S
 */
std::string formatTime(const std::chrono::system_clock::time_point& time) {
	std::time_t time_t = std::chrono::system_clock::to_time_t(time);

	// if i multithread don't use localtime b/c not thread safe.
	std::tm* local = std::localtime(&time_t);

	// if i care about optimising this, consider static.
	// if i care about multithreading, consider mutexes for thread safety
	std::ostringstream oss;
	oss << std::put_time(local, "%Y-%m-%d %H:%M:%S");
	return oss.str();
}

Logger::Logger() : outfile("prev.log"), startTime(std::chrono::system_clock::now()),
	visLevel(7), dirty(true) {
	log("Logger initialised");

	/**
	LoadTextureFromFile("img/infoicon.png",  &icons[0], 32, 32);
	LoadTextureFromFile("img/warnicon.png",  &icons[1], 32, 32);
	LoadTextureFromFile("img/erroricon.png", &icons[2], 32, 32);
	*/
}

Logger::~Logger() {
	outfile.close();
	saveLog();
}

void Logger::saveLog() {
	std::string now = formatTime(std::chrono::system_clock::now());
	std::string name = "[" + formatTime(startTime) + "][" + now + "].log";
	std::replace(name.begin(), name.end(), ':', '_');
	if (outfile.is_open()) {
		outfile << "[" << now << "]  log saved as " << name << std::endl;
		outfile.flush();
		rows.push_back(std::make_tuple(Level::INFO, "[" + now + "]", "log saved as " + name));
	}

	try {
		std::filesystem::copy("prev.log", name, std::filesystem::copy_options::overwrite_existing);
	} catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "Error during file copy: " << e.what() << std::endl;
	}
}

/**
 * @abstract Changes the visibilty layer for an inputted level.
 */
void Logger::setLevel(const Level level) {
	if (visLevel & level) {
		visLevel &= ~level;
	} else {
		visLevel |= level;
	}
}

void Logger::log(const std::string& message) {
	std::string time = "[" + formatTime(std::chrono::system_clock::now()) + "]";
	outfile  << time << "  " << message << std::endl;
	rows.push_back(std::make_tuple(Level::INFO, time, message));
	dirty = true;
}

void Logger::log(Logger::Level level, const std::string& message) {
	std::string time = "[" + formatTime(std::chrono::system_clock::now()) + "]";
	outfile  << time << "  [" << levelToStr(level) << "] " << message << std::endl;
	rows.push_back(std::make_tuple(level, time, message));
	dirty = true;
}

std::string Logger::levelToStr(const Level level) const {
	switch (level) {
		case Level::INFO:
			return "INFO";
		case Level::WARNING:
			return "WARNING";
		case Level::ERROR:
			return "ERROR";
		default:
			return "";
	}
}

const ImGuiTableFlags flags = ImGuiTableFlags_Borders |
							ImGuiTableFlags_Resizable |
							ImGuiTableFlags_RowBg |
							ImGuiTableFlags_Sortable;
// awesome gui stuff
void Logger::draw() {
	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);  // Default size, used only on the first appearance
	ImGui::SetNextWindowSizeConstraints(ImVec2(500, 300), ImVec2(FLT_MAX, FLT_MAX));  // Minimum size 500x300, no max size
	ImGui::Begin("Logger");

	ImGui::BeginChild("Log Options", ImVec2(0, 27), false);
	if (ImGui::Button("Clear")) {
		rows.clear();
	}
	ImGui::SameLine(0.0f, 11.0f);
	if (ImGui::Button("Save Log")) {
		saveLog();
	}

	{ // visibility toggles
		float buttonSize = 20;
		float window_width = ImGui::GetWindowWidth();

		ImGui::SameLine(0.0f, 11.0f);
		ImGui::SetCursorPosX(window_width - 90);
		if (visLevel & Level::INFO) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
		}
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.95f, 0.95f, 1.0f));
		if (ImGui::Button("##info_square", ImVec2(buttonSize, buttonSize))) {
			setLevel(Level::INFO);
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine(0.0f, 11.0f);
		if (visLevel & Level::WARNING) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.6f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
		}
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.85f, 0.0f, 1.0f));
		if (ImGui::Button("##warn_square", ImVec2(buttonSize, buttonSize))) {
			setLevel(Level::WARNING);
		}
		ImGui::PopStyleColor(3);

		ImGui::SameLine(0.0f, 11.0f);
		if (visLevel & Level::ERROR) {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
		} else {
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		}
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		if (ImGui::Button("##err_square", ImVec2(buttonSize, buttonSize))) {
			setLevel(Level::ERROR);
		}
		ImGui::PopStyleColor(3);
	}
	ImGui::EndChild();

	ImGui::BeginChild("Log Entries", ImVec2(0, 0), true); 
	if (ImGui::BeginTable("LoggerTable", 3, flags)) {
		ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_DefaultSort, 45.0f);
		ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed  | ImGuiTableColumnFlags_DefaultSort, 146.0f);
		ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_NoSort); // stretchable
		ImGui::TableHeadersRow();

		// imgui doesn't handle sorting for you, so here's my sorting implementation.
		ImGuiTableSortSpecs* sort_specs = ImGui::TableGetSortSpecs();
		if (sort_specs && sort_specs->SpecsDirty) {
			std::sort(rows.begin(), rows.end(), [&](const std::tuple<Level, std::string, std::string>& a, const std::tuple<Level, std::string, std::string>& b) {
				const ImGuiTableColumnSortSpecs* sort_spec = &sort_specs->Specs[0];
				if (sort_spec->ColumnIndex == 0) {
					// Sort by "Level" (custom enum or type)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) 
						   ? std::get<0>(a) < std::get<0>(b) 
						   : std::get<0>(a) > std::get<0>(b);
				} else if (sort_spec->ColumnIndex == 1) {
					// Sort by "Time" (string, assuming it's a formatted timestamp)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) 
						   ? std::get<1>(a) < std::get<1>(b) 
						   : std::get<1>(a) > std::get<1>(b);
				}
				return false; // Default case
			});
			sort_specs->SpecsDirty = false; // Mark specs as sorted
		}

		for(const std::tuple<Level, std::string, std::string>& data : rows) {
			Level level = std::get<0>(data);
			if(visLevel & level) {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%s", levelToStr(level).c_str());
				//ImGui::Image((ImTextureID)(intptr_t)icons[i], ImVec2(32, 32));

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", std::get<1>(data).c_str());

				ImGui::TableSetColumnIndex(2);
				ImGui::TextWrapped("%s", std::get<2>(data).c_str());
			}
		}

		ImGui::EndTable();
	}

	if (dirty) {
		// if we aren't at the bottom, don't scroll the user down.
		if(ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
			ImGui::SetScrollHereY(1.0f);
			// Scroll to the bottom
		}
	}
	ImGui::EndChild();
	ImGui::End();
	dirty = false;
}