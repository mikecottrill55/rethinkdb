#include <map>
#include <vector>

#include "clustering/administration/main/options.hpp"
#include "unittest/gtest.hpp"

namespace unittest {

std::vector<options::option_t> make_options() {
    return {
        options::option_t(options::names_t("--no-parameter"),
                          options::OPTIONAL_NO_PARAMETER),
        options::option_t(options::names_t("--optional"),
                          options::OPTIONAL),
        options::option_t(options::names_t("--optional-repeat", "-o"),
                          options::OPTIONAL_REPEAT),
        options::option_t(options::names_t("--mandatory"),
                          options::MANDATORY),
        options::option_t(options::names_t("--mandatory-repeat"),
                          options::MANDATORY_REPEAT),
        options::option_t(options::names_t("--override-default"),
                          options::OPTIONAL,
                          "default-not-overridden")
    };
}

std::map<std::string, std::vector<std::string> > without_source(const std::map<std::string, options::values_t> &opts) {
    std::map<std::string, std::vector<std::string> > ret;
    for (auto pair = opts.begin(); pair != opts.end(); ++pair) {
        ret.insert(std::make_pair(pair->first, pair->second.values));
    }
    return ret;
}

TEST(OptionsTest, CommandLineParsing) {
    const std::vector<const char *> command_line = {
        "--no-parameter",
        "--optional", "optional1",
        "--optional-repeat", "optional-repeat1",
        "--mandatory", "mandatory1",
        "--optional-repeat", "optional-repeat2",
        "-o", "optional-repeat3",
        "--mandatory-repeat", "mandatory-repeat1",
        "--mandatory-repeat", "mandatory-repeat2",
        "--mandatory-repeat", "mandatory-repeat3",
        "--override-default", "override-default1"
    };

    const std::vector<options::option_t> options = make_options();

    const std::map<std::string, std::vector<std::string> > opts
        = without_source(options::parse_command_line(command_line.size(), command_line.data(), options));

    const std::map<std::string, std::vector<std::string> > expected_parse = {
        { "--no-parameter", { "" } },
        { "--optional", { "optional1" } },
        { "--optional-repeat", { "optional-repeat1", "optional-repeat2", "optional-repeat3" } },
        { "--mandatory", { "mandatory1" } },
        { "--mandatory-repeat", { "mandatory-repeat1", "mandatory-repeat2", "mandatory-repeat3" } },
        { "--override-default", { "override-default1" } }
    };

    ASSERT_EQ(expected_parse, opts);
}

TEST(OptionsTest, ConfigFileParsing) {
    const std::string config_file =
        "# My First Config file\n"
        "no-parameter=\n"
        "optional=optional1\n"
        "optional-repeat = optional-repeat1\r\n"
        "optional-repeat = optional-repeat2\r\n"
        "optional-repeat = optional-repeat3\r\n"
        "   mandatory=    mandatory1   #comment\n"
        "# comment\n"
        "\n"
        "mandatory-repeat     =mandatory-repeat1#\n"
        "mandatory-repeat    =    mandatory-repeat2    \n"
        "    mandatory-repeat=mandatory-repeat3##comment\n"
        "override-default = override-default1";

    const std::vector<options::option_t> options = make_options();

    const std::map<std::string, std::vector<std::string> > opts
        = without_source(options::parse_config_file(config_file, "ConfigFileParsing file", options));

    const std::map<std::string, std::vector<std::string> > expected_parse = {
        { "--no-parameter", { "" } },
        { "--optional", { "optional1" } },
        { "--optional-repeat", { "optional-repeat1", "optional-repeat2", "optional-repeat3" } },
        { "--mandatory", { "mandatory1" } },
        { "--mandatory-repeat", { "mandatory-repeat1", "mandatory-repeat2", "mandatory-repeat3" } },
        { "--override-default", { "override-default1" } }
    };

    ASSERT_EQ(expected_parse, opts);
}



}  // namespace unittest
