#include <iostream>
#include <string>
#include <limits.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <fstream>
#include <sys/sysinfo.h>
#include <array>
#include <memory>
#include <cassert>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <sstream>

#include "ascii_art.h"
#include "ansi.h"

struct utsname os;

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string& s) {
	size_t start = s.find_first_not_of(WHITESPACE);
	return (start == std::string::npos) ? "" : s.substr(start);
}

bool string_ends_with(std::string const &fullString, std::string const &ending) {
	if (fullString.length() >= ending.length()) {
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

std::string read_file(std::string location) {
	std::ifstream file(location);
	if (!file.good()) exit(1);
	std::string line;
	getline(file, line);
	file.close();
	return line;
}

std::string execute_command(std::string command) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
	if (!pipe) {
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	result.pop_back(); //Remove newline from end of output
	return result;
}

std::string get_username() {
	return getenv("USER");
}

std::string get_hostname() {
	char hostname[HOST_NAME_MAX];
	gethostname(hostname, HOST_NAME_MAX);
	return std::string(hostname);
}

std::string get_operating_system() {
	std::string os_name = read_file("/etc/os-release");

	//Extract OS name (when read it is in format 'NAME="<name>"')
	os_name = os_name.substr(6);
	os_name.pop_back();
	
	return os_name + " " + os.machine;
}

std::string get_host() {
	std::string device_name = read_file("/sys/devices/virtual/dmi/id/product_name");
	std::string device_version = read_file("/sys/devices/virtual/dmi/id/product_version");

	return device_name + " " + device_version;
}

std::string get_kernel() {
	return os.release;
}

std::string get_uptime() {
	struct sysinfo system_info;
	sysinfo(&system_info);
	
	//FIXME: This code doesn't handle leap years, or differing amounts of days in months
	long uptime_seconds = system_info.uptime;
	int seconds = uptime_seconds % 60;
	uptime_seconds /= 60;
	int minutes = uptime_seconds % 60;
	uptime_seconds /= 60;
	int hours = uptime_seconds % 24;
	uptime_seconds /= 24;
	int days = uptime_seconds % 31;
	uptime_seconds /= 31;
	int years = uptime_seconds % 365;
	uptime_seconds /= 365;

	if (system_info.uptime < 60) {
		std::string uptime_string = std::to_string(seconds) + "second";
		if (seconds != 1) uptime_string += "s";
		return uptime_string;
	}

	std::string uptime_string = "";
	if (years > 0) {
		uptime_string += std::to_string(years) + " year";
		if (years != 1) uptime_string += "s";
		uptime_string += ", ";
	}

	if (days > 0) {
		uptime_string += std::to_string(days) + " day";
		if (days != 1) uptime_string += "s";
		uptime_string += ", ";
	}

	if (hours > 0) {
		uptime_string += std::to_string(hours) + " hour";
		if (hours != 1) uptime_string += "s";
		uptime_string += ", ";
	}

	if (minutes > 0) {
		uptime_string += std::to_string(minutes) + " min";
		if (minutes != 1) uptime_string += "s";
	}

	if (uptime_string.back() == ' ') { //String has trailing ", ", remove it
		uptime_string.pop_back(); 
		uptime_string.pop_back(); 
	}
	return uptime_string;
}

std::string get_packages() {
	return execute_command("pacman -Q | wc -l") + " (pacman)";
}

std::string get_shell() {
	std::string shell_path = getenv("SHELL");
	assert(shell_path.length() != 0);
	shell_path.erase(0, 1);
	std::string shell = shell_path.substr(shell_path.find("/") + 1); 

	std::string shell_with_version = execute_command("pacman -Q " + shell);
	return shell_with_version;
}

std::string get_resolutions() {
	Display *d = XOpenDisplay(NULL);
	XRRScreenResources *screen = XRRGetScreenResources(d, DefaultRootWindow(d));

	std::vector<std::string> resolutions;
	
	for (int i = 0; i < screen->ncrtc; i++) {
		XRRCrtcInfo *crtc_info = XRRGetCrtcInfo(d, screen, screen->crtcs[i]);
		for (int j = 0; j < crtc_info->noutput; j++) {
			XRROutputInfo *output_info = XRRGetOutputInfo(d, screen, crtc_info->outputs[j]);
			if (output_info->connection == RR_Connected) resolutions.push_back(std::to_string(crtc_info->width) + "x" + std::to_string(crtc_info->height));
		}
	}
	XCloseDisplay(d);

	std::string resolution_string = "";
	std::vector<std::string>::iterator it = resolutions.begin();
	int resolution_count = resolutions.size();
	for (int i = 0; i < resolution_count; i++) {
		resolution_string += *it;
		if (it != --resolutions.end()) resolution_string += ", ";
		it++;
	}

	return resolution_string;
}

std::string get_wm() {
	std::string process_list = execute_command("ps a");
	std::stringstream iss(process_list);

	int command_start = -1;
	bool xorg_started = false;
	while(iss.good()) {
		std::string line;
		getline(iss, line, '\n');
	
		if (command_start == -1) {
			command_start = line.find("COMMAND");
		} else {
			std::string command = line.substr(command_start);
			std::string process_name = command;
			if (command.find(" ") != std::string::npos) process_name = command.substr(0, command.find(" "));
			if (xorg_started) {
				if (string_ends_with(process_name, "wm") || string_ends_with(process_name, "dm"))	{
					return process_name;
				}
			} else {
				if (process_name.find("Xorg") != std::string::npos) xorg_started = true;
			}
		}
	}

	__builtin_unreachable();
}

std::string get_theme(bool icon_theme) {
	std::string command = "gsettings get org.gnome.desktop.interface ";
	if (icon_theme) {
		command += "icon-theme";
	} else {
		command += "gtk-theme";
	}
	std::string theme = execute_command(command);
	//Remove single quotes wrapping theme name
	theme.erase(0, 1);
	theme.pop_back();
	return theme;
}

std::string get_terminal() {
	//This entire section gets the name of the parent process of the parent process of the currently executing process, perhaps there is a cleaner way to do this
	std::string stat = read_file("/proc/" + std::to_string(getppid()) + "/stat");
	
	std::istringstream ss(stat);
	std::string ppid;
	for (int i = 0; i < 4; i++)	ss >> ppid;

	std::string terminal = read_file("/proc/" + ppid + "/comm");
	return terminal;
}

std::string get_terminal_font(std::string terminal) {
	if (terminal == "st") {
		std::string font = execute_command("strings $(which st) | grep pixelsize=");
		if (font.find(":") != std::string::npos) font = font.substr(0, font.find(":"));
		return font;
	}

	__builtin_unreachable();
}

std::string get_cpu() {
	std::string data = execute_command("lscpu");
	std::stringstream iss(data);

	int data_start = -1;
	std::string cpu_name = "";
	std::string core_count = "";
	std::string clock_speed = "";
	std::string vendor = "";
	while(iss.good()) {
		std::string line;
		getline(iss, line, '\n');

		if (data_start == -1) {
			int pos = line.find(":") + 1;
			for (;;) {
				if (line[pos++] != ' ') break;
			}
			data_start = --pos;
		}

		std::string line_data = line.substr(data_start);
		std::cout << line << std::endl;
		if (line.rfind("Model name") == 0) cpu_name = line_data;
		if (line.rfind("CPU(s)") == 0) core_count = line_data;
		if (line.rfind("CPU max MHz") == 0) clock_speed = line_data;
		if (line.rfind("Vendor ID") == 0) vendor = line_data;
	}

	if (cpu_name.find("@")) cpu_name = cpu_name.substr(0, cpu_name.find(" @"));

	float clock_speed_mhz = stof(clock_speed);
	float clock_speed_ghz = clock_speed_mhz / 1000;
	clock_speed = std::to_string(clock_speed_ghz).substr(0, 5) + "GHz";

	cpu_name = cpu_name.substr(cpu_name.find(" ") + 1);
	if (vendor == "GenuineIntel") cpu_name = cpu_name.substr(cpu_name.find(" ") + 1);
	if (cpu_name.find("CPU") != std::string::npos) cpu_name = cpu_name.substr(0, cpu_name.find(" CPU"));
	
	if (vendor == "GenuineIntel") vendor = "Intel";
	if (vendor == "AuthenticAMD") vendor = "AMD";
	cpu_name = vendor + " " + cpu_name;

	return cpu_name + " (" + core_count + ") @ " + clock_speed;
}

std::string get_gpu() {
	std::string data = execute_command("lspci | grep VGA");
	return data.substr(data.find(": ") + 2);
}

std::string get_memory() {
	std::string data = execute_command("cat /proc/meminfo | grep -P \"MemTotal|Shmem:|MemFree|Buffers|^Cached|SReclaimable\"");
	std::stringstream iss(data);
	
	int data_start = -1;
	std::vector<long> mem_data;
	while(iss.good()) {
		std::string line;
		getline(iss, line, '\n');

		if (data_start == -1) {
			int pos = line.find(":") + 1;
			for (;;) {
				if (line[pos++] != ' ') break;
			}
			data_start = --pos;
		}

		std::string line_data = ltrim(line.substr(data_start));
		line_data = line_data.substr(0, line_data.find(" "));
		mem_data.push_back(stol(line_data));
	}

	std::string total_ram = std::to_string(mem_data[0] / 1024) + "MiB";
	//          MemUsed =                  Memtotal    + Shmem       - MemFree     - Buffers     - Cached      - SReclaimable
	std::string used_ram = std::to_string((mem_data[0] + mem_data[4] - mem_data[1] - mem_data[2] - mem_data[3] - mem_data[5]) / 1024) + "MiB";
	return used_ram + " / " + total_ram;
}

ASCIIArt arch_art({
	"                   -`",
	"                  .o+`",
	"                 `ooo/",
	"                `+oooo:",
	"               `+oooooo:",
	"               -+oooooo+:",
	"             `/:-:++oooo+:",
	"            `/++++/+++++++:",
	"           `/++++++++++++++:",
	"          `/+++ooooooooooooo/`",
	"         ./ooosssso++osssssso+`",
	"        .oossssso-````/ossssss+`",
	"       -osssssso.      :ssssssso.",
	"      :osssssss/        osssso+++.",
	"     /ossssssss/        +ssssooo/-",
	"   `/ossssso+/:-        -:/+osssso+-",
	"  `+sso+:-`                 `.-/+oso:",
	" `++:.                           `-/+/",
	" .`                                 `/"
});

int main(void) {
	//Get OS details, this is done in main because multiple functions use this information and we shouldn't make multiple calls to uname
	uname(&os);
	
	//Get terminal name, this is done in main because get terminal font often requires the terminal name to acquire the font name
	std::string terminal = get_terminal();

	std::vector<std::string> data;
	data.push_back(get_username() + RESET_COLORS + "@" + BRIGHT_CYAN_COLOR + get_hostname());
	data.push_back(RESET_COLORS + "--------------");
	data.push_back("OS: " + RESET_COLORS + get_operating_system());
	data.push_back("Host: " + RESET_COLORS + get_host());
	data.push_back("Kernel: " + RESET_COLORS + get_kernel());
	data.push_back("Uptime: " + RESET_COLORS + get_uptime());
	data.push_back("Packages: " + RESET_COLORS + get_packages());
	data.push_back("Shell: " + RESET_COLORS + get_shell());
	data.push_back("Resolution: " + RESET_COLORS + get_resolutions());
	data.push_back("WM: " + RESET_COLORS + get_wm());
	data.push_back("Theme: " + RESET_COLORS + get_theme(false));
	data.push_back("Icons: " + RESET_COLORS + get_theme(true));
	data.push_back("Terminal: " + RESET_COLORS + terminal);
	data.push_back("Terminal Font: " + RESET_COLORS + get_terminal_font(terminal));
	data.push_back("CPU: " + RESET_COLORS + get_cpu());
	data.push_back("GPU: " + RESET_COLORS + get_gpu());
	data.push_back("Memory: " + RESET_COLORS + get_memory());
	data.push_back("");
	data.push_back(_BLACK_COLOR + "   " + _RED_COLOR + "   " + _GREEN_COLOR + "   " + _YELLOW_COLOR + "   " + _BLUE_COLOR + "   " + _MAGENTA_COLOR + "   " + _CYAN_COLOR + "   " + _WHITE_COLOR + "   " + _BLACK_COLOR);
	data.push_back(_BRIGHT_BLACK_COLOR + "   " + _BRIGHT_RED_COLOR + "   " + _BRIGHT_GREEN_COLOR + "   " + _BRIGHT_YELLOW_COLOR + "   " + _BRIGHT_BLUE_COLOR + "   " + _BRIGHT_MAGENTA_COLOR + "   " + _BRIGHT_CYAN_COLOR + "   " + _BRIGHT_WHITE_COLOR + "   " + _BLACK_COLOR);
	arch_art.print(data);
	return 0;
}
