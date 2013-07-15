#include <llvm/Option/OptTable.h>
#include <llvm/Option/Option.h>
#include <llvm/Support/CommandLine.h>
#include <clang/Driver/Options.h>
#include <clang/Tooling/CommonOptionsParser.h>

#include "oclint/ConfigFile.h"
#include "oclint/Debug.h"
#include "oclint/Options.h"
#include "oclint/RuleConfiguration.h"

using namespace oclint;

/* ----------------
   input and output
   ---------------- */

static llvm::cl::opt<std::string> argOutput("o",
    llvm::cl::desc("Write output to <path>"),
    llvm::cl::value_desc("path"),
    llvm::cl::init("-"));

/* --------------------
   oclint configuration
   -------------------- */

static llvm::cl::opt<std::string> argReportType("report-type",
    llvm::cl::desc("Change output report type"),
    llvm::cl::value_desc("name"),
    llvm::cl::init("text"));
static llvm::cl::list<std::string> argRulesPath("R",
    llvm::cl::Prefix,
    llvm::cl::desc("Add directory to rule loading path"),
    llvm::cl::value_desc("directory"),
    llvm::cl::ZeroOrMore);
static llvm::cl::list<std::string> argRuleConfiguration("rc",
    llvm::cl::desc("Override the default behavior of rules"),
    llvm::cl::value_desc("parameter>=<value"),
    llvm::cl::ZeroOrMore);
static llvm::cl::list<std::string> argEnabledRules("rule",
    llvm::cl::desc("Explicitly pick rules"),
    llvm::cl::value_desc("rule name"),
    llvm::cl::ZeroOrMore);
static llvm::cl::list<std::string> argDisabledRules("disable-rule",
    llvm::cl::desc("Disable rules"),
    llvm::cl::value_desc("rule name"),
    llvm::cl::ZeroOrMore);
static llvm::cl::opt<bool> argListEnabledRules("list-enabled-rules",
    llvm::cl::desc("List enabled rules"),
    llvm::cl::init(false));
static llvm::cl::opt<int> argMaxP1("max-priority-1",
    llvm::cl::desc("The max allowed number of priority 1 violations"),
    llvm::cl::value_desc("threshold"),
    llvm::cl::init(0));
static llvm::cl::opt<int> argMaxP2("max-priority-2",
    llvm::cl::desc("The max allowed number of priority 2 violations"),
    llvm::cl::value_desc("threshold"),
    llvm::cl::init(10));
static llvm::cl::opt<int> argMaxP3("max-priority-3",
    llvm::cl::desc("The max allowed number of priority 3 violations"),
    llvm::cl::value_desc("threshold"),
    llvm::cl::init(20));
static llvm::cl::opt<bool> argClangChecker("enable-clang-static-analyzer",
    llvm::cl::desc("Enable Clang Static Analyzer, and integrate results into OCLint report"),
    llvm::cl::init(false));

/* -------------
   libTooling cl
   ------------- */

static llvm::cl::extrahelp CommonHelp(clang::tooling::CommonOptionsParser::HelpMessage);
static llvm::cl::extrahelp MoreHelp(
    "For more information, please visit http://oclint.org\n"
);
static llvm::OwningPtr<llvm::opt::OptTable> Options(clang::driver::createDriverOptTable());

/* -------
   options
   ------- */

static oclint::RulesetFilter filter;

template <typename T>
void updateArgIfSet(llvm::cl::opt<T> &argValue, const llvm::Optional<T> &configValue)
{
    if (configValue.hasValue() && argValue.getNumOccurrences() == 0)
    {
        argValue.setValue(configValue.getValue());
    }
}

static void processConfigFile(const oclint::option::ConfigFile &config)
{
    for (const oclint::option::RuleConfigurationPair &ruleConfig : config.ruleConfigurations())
    {
        oclint::RuleConfiguration::addConfiguration(ruleConfig.key(), ruleConfig.value());
    }
    filter.enableRules(config.rules().begin(), config.rules().end());
    filter.disableRules(config.disableRules().begin(), config.disableRules().end());

    updateArgIfSet(argOutput, config.output());
    updateArgIfSet(argReportType, config.reportType());
    updateArgIfSet(argMaxP1, config.maxP1());
    updateArgIfSet(argMaxP2, config.maxP2());
    updateArgIfSet(argMaxP3, config.maxP3());
}

static void processConfigFiles()
{
    const std::vector<oclint::option::ConfigFile> configFiles = oclint::option::readConfigFiles();
    for_each(configFiles.begin(), configFiles.end(), processConfigFile);
}

void oclint::option::process()
{
    processConfigFiles();
    for (unsigned i = 0; i < argRuleConfiguration.size(); ++i)
    {
        std::string configuration = argRuleConfiguration[i];
        int indexOfSeparator = configuration.find_last_of("=");
        std::string key = configuration.substr(0, indexOfSeparator);
        std::string value = configuration.substr(indexOfSeparator + 1,
            configuration.size() - indexOfSeparator - 1);
        oclint::RuleConfiguration::addConfiguration(key, value);
    }

    filter.enableRules(argEnabledRules.begin(), argEnabledRules.end());
    filter.disableRules(argDisabledRules.begin(), argDisabledRules.end());
}

bool oclint::option::hasOutputPath()
{
    return argOutput != "-";
}

std::string oclint::option::outputPath()
{
    return argOutput;
}

std::string oclint::option::reportType()
{
    return argReportType;
}

bool oclint::option::hasCustomRulesPath()
{
    return argRulesPath.size() > 0;
}

std::vector<std::string> oclint::option::rulesPath()
{
    return argRulesPath;
}

const oclint::RulesetFilter &oclint::option::rulesetFilter()
{
    return filter;
}

int oclint::option::maxP1()
{
    return argMaxP1;
}

int oclint::option::maxP2()
{
    return argMaxP2;
}

int oclint::option::maxP3()
{
    return argMaxP3;
}

bool oclint::option::showEnabledRules()
{
    return argListEnabledRules;
}

bool oclint::option::enableClangChecker()
{
    return argClangChecker;
}
