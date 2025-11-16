#ifndef TESTUTILS_HPP
#define TESTUTILS_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/wait.h>

std::string operator+(std::string str, const std::filesystem::path& path)
{
	return str.append(path.string());
}

namespace testutils
{
	using namespace std::chrono_literals;

	class safe_stringbuf : public std::stringbuf {
		mutable std::mutex mutex{};
		using std::stringbuf::stringbuf; // inherit constructors
	protected:
		void imbue(const std::locale& loc) override
		{
			std::lock_guard lock{mutex};
			std::stringbuf::imbue(loc);
		}

		std::streambuf* setbuf(std::stringbuf::char_type* s, std::streamsize n) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::setbuf(s, n);
		}

		std::stringbuf::pos_type seekoff(std::stringbuf::off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::seekoff(off, dir, which);
		}

		std::stringbuf::pos_type seekpos(std::stringbuf::pos_type pos, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::seekpos(pos, which);
		}

		int sync() override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::sync();
		}

		std::streamsize showmanyc() override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::showmanyc();
		}

		std::stringbuf::int_type underflow() override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::underflow();
		}

		std::stringbuf::int_type uflow() override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::uflow();
		}

		std::streamsize xsgetn(std::stringbuf::char_type* s, std::streamsize count) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::xsgetn(s, count);
		}

		std::streamsize xsputn(const std::stringbuf::char_type* s, std::streamsize count) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::xsputn(s, count);
		}

		std::stringbuf::int_type overflow(std::stringbuf::int_type ch = std::stringbuf::traits_type::eof()) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::overflow(ch);
		}

		std::stringbuf::int_type pbackfail(std::stringbuf::int_type c = std::stringbuf::traits_type::eof()) override
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::pbackfail(c);
		}

	public:
		void str(const std::string& s)
		{
			std::lock_guard lock{mutex};
			std::stringbuf::str(s);
		}

		std::string str() const
		{
			std::lock_guard lock{mutex};
			return std::stringbuf::str();
		}
	};

	class safe_sstream : public std::stringstream {
		safe_stringbuf buf{};
	public:
		safe_sstream()
		{
			set_rdbuf(&buf);
		}

		void str(const std::string& s)
		{
			buf.str(s);
		}

		std::string str() const
		{
			return buf.str();
		}
	};

	std::filesystem::path WORKDIR;

	bool is_strm_eof(std::istream& strm)
	{
		return strm.peek() == std::istream::traits_type::eof();
	}

	/* Wait a given time to allow supplier thread to process and return data into strm */
	bool is_strm_eof_delayed(std::istream& strm, std::chrono::milliseconds ms = 20ms)
	{
		std::this_thread::sleep_for(ms);
		std::this_thread::yield(); // make really sure the other thread could run
		return is_strm_eof(strm);
	}

	std::string getline(std::istream& strm)
	{
		std::string output{};
		std::getline(strm, output);
		return output;
	}

	std::vector<std::string> get_current_strm_block(std::istream& strm)
	{
		std::vector<std::string> arr{};
		std::string line;
		while(not (is_strm_eof(strm) or (line = getline(strm)).empty())) {
			arr.push_back(line);
		}
		return arr;
	}

	std::string skip_to_blank_or_eof(std::istream& strm)
	{
		std::string last;
		while(not (is_strm_eof(strm) or (last = getline(strm)).empty()));
		return last;
	}

	bool execute(const std::string& cmd)
	{
		const int stat = std::system(cmd.c_str());
		if (WIFEXITED(stat) and WEXITSTATUS(stat) == 0) {
			return true;
		}
		return false;
	}

	int get_count_empty_lines(std::istream& strm)
	{
		int empty_lines = 0;
		while (not is_strm_eof(strm)) {
			if (getline(strm).empty()) {
				empty_lines += 1;
			}
		}
		return empty_lines;
	}

	std::string get_rnd_hex_str(const std::size_t length)
	{
		static std::mt19937 eng{};
		static std::uniform_int_distribution<int> dist(0, 15);

		std::ostringstream oss{};
		oss.setf(std::ios_base::hex, std::ios_base::basefield);

		for (std::size_t i{}; i < length; ++i) {
			oss << dist(eng);
		}
		return oss.str();
	}
}

namespace testutils::rclone
{
	bool rclone_cmd(const std::filesystem::path& rclone_cfg, const std::string& cmd)
	{
		return execute("RCLONE_CONFIG=" + rclone_cfg + " rclone " + cmd);
	}
}

namespace testutils::setup
{
	std::filesystem::path get_binary_search_path()
	{
		const char *const cpath = std::getenv("BINARY_SEARCH_PATH");
		if (nullptr == cpath) {
			throw std::runtime_error("missing env BINARY_SEARCH_PATH");
		}
		const std::filesystem::path search_path(std::filesystem::absolute(cpath));
		if (not std::filesystem::is_directory(search_path)) {
			throw std::runtime_error("not a directory: BINARY_SEARCH_PATH: " + search_path);
		}
		return search_path;
	}

	void set_env(const std::string& env, const std::string& val)
	{
		if (setenv(env.c_str(), val.c_str(), 1)) {
			throw std::runtime_error("failed to set env " + env + " to: " + val + ": " + std::strerror(errno));
		}
	}

	void add_to_search_path(const std::filesystem::path& dir)
	{
		const char *const cpath_env = std::getenv("PATH");
		if (nullptr == cpath_env) {
			throw std::runtime_error("missing env PATH");
		}
		set_env("PATH", dir + ":" + std::string(cpath_env));
	}

	void setup_git_debug_env()
	{
		set_env("GIT_TRACE2", "1");
		set_env("GIT_TRACE2_ENV_VARS", "PATH");
	}

	void setup_rclone_conf(const std::filesystem::path& test_case_dir)
	{
		const std::filesystem::path rclone_cfg = test_case_dir / "rclone.conf";
		if (not rclone::rclone_cmd(rclone_cfg, "config create --non-interactive --obscure remote crypt password=git-remote-rclone \"remote=" + test_case_dir / "remote" + "\"")) {
			throw std::runtime_error("cannot setup rclone config for crypt remote");
		}
		if (not std::filesystem::is_regular_file(rclone_cfg)) {
			throw std::runtime_error("cannot create rclone config (: " + rclone_cfg);
		}
	}

	std::filesystem::path setup_sub_workdir(const std::string& sub_workdir_name)
	{
		const std::filesystem::path sub_workdir = WORKDIR / sub_workdir_name;
		if (not std::filesystem::create_directory(sub_workdir)) {
			throw std::runtime_error("cannot create sub-work directory: " + sub_workdir);
		}
		setup_rclone_conf(sub_workdir);
		return sub_workdir;
	}

	void setup_workdir(const std::filesystem::path& workdir_base)
	{
		WORKDIR = workdir_base + ".workdir";
		// Workdir may already exist; cleanup for a clean state
		std::filesystem::remove_all(WORKDIR);
		if (not std::filesystem::create_directory(WORKDIR)) {
			throw std::runtime_error("cannot create work directory: " + WORKDIR);
		}
		add_to_search_path(setup::get_binary_search_path());
		setup_git_debug_env();
	}
}

namespace testutils::git
{
	// Global definitions of operator+s not visible from here; need to extend scope.
	// Required for using globally defined operator+(std::string, std::filesystem::path&)
	using ::operator+;

	class git_repo {
		int ncommits;
		const std::filesystem::path repo;

		friend git_repo init_repo(const std::filesystem::path&);
	protected:
		explicit git_repo(const std::filesystem::path repo) : ncommits(1), repo(repo) {}
	public:
		int trigger_ncommits()
		{
			return ncommits++;
		}

		int get_ncommits() const
		{
			return ncommits;
		}

		std::filesystem::path get_repo_path() const
		{
			return repo;
		}

	};

	std::string operator+(std::string str, const git_repo& repo)
	{
		return str.append(repo.get_repo_path());
	}

	bool git_cmd(const std::string& cmd, const git_repo& repo)
	{
		return execute("git -C \"" + repo + "\" " + cmd);
	}

	bool commit(git_repo& repo)
	{
		const std::string cmd = "commit --message=commit@" + std::to_string(repo.trigger_ncommits()) + ":" + get_rnd_hex_str(16);
		return git_cmd(cmd, repo);
	}

	bool add_all(const git_repo& repo)
	{
		return git_cmd("add --all", repo);
	}

	bool push(const git_repo& repo)
	{
		return git_cmd("push --set-upstream origin master", repo);
	}

	void append_test_data(const git_repo& repo)
	{
		const std::filesystem::path path(repo.get_repo_path() / "testfile");
		std::ofstream testfile{path};
		testfile << "append@" << std::to_string(repo.get_ncommits()) << ":" << get_rnd_hex_str(16) << std::endl;
	}

	bool add_remote(const git_repo& repo, const std::string remote_url = "rclone://remote:subdir")
	{
		// remote_url: "<proto>://": <proto> effectively sets what helper git will execute -> git-remote-<proto>
		return git_cmd("remote add origin " + remote_url, repo);
	}

	git_repo init_repo(const std::filesystem::path& repo)
	{
		if (not std::filesystem::create_directory(repo)) {
			throw std::runtime_error("cannot create directory: " + repo);
		}
		const git_repo g_repo{repo};
		if (not git_cmd("init --initial-branch=master", g_repo)) {
			throw std::runtime_error("cannot init git repo: " + repo);
		}
		return g_repo;
	}
}

#endif /* TESTUTILS_HPP */
