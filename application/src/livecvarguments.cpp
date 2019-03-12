/****************************************************************************
**
** Copyright (C) 2014-2018 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#include "livecvarguments.h"
#include "live/commandlineparser.h"
#include "live/mlnodetojson.h"
#include <QFile>

namespace lv{

LiveCVArguments::LiveCVArguments(const std::string& header)
    : m_parser(new CommandLineParser(header))
{
}

LiveCVArguments::~LiveCVArguments(){
    delete m_parser;
}

bool LiveCVArguments::pluginInfoFlag() const{
    return m_pluginInfoFlag;
}

bool LiveCVArguments::helpFlag() const{
    return m_parser->isSet(m_parser->helpOption());
}

void LiveCVArguments::initialize(int argc, const char* const argv[]){
    CommandLineParser::Option* launchOption  = m_parser->addFlag({"-l" , "--launch"},
        "Launches the live cv project with the UI enabled. Does not load the editor.");
    CommandLineParser::Option* runOption     = m_parser->addFlag({"-r" , "--run"},
        "Runs the live cv project in cli mode. Enables console logging by default.");
    CommandLineParser::Option* monitorOption  = m_parser->addOption({"-m", "--monitor"},
        "Opens the list of paths in monitor mode.", "list");
    CommandLineParser::Option* pluginInfoOption = m_parser->addOption({"--plugininfo"},
        "Outputs the plugin info to a specified import (e.g. --plugininfo \"live 1.0\".", "string");

    CommandLineParser::Option* logToConsoleOption = m_parser->addFlag({"-c", "--log-toconsole"},
        "Output log data to the console. This is on by default if Live CV is in run mode.");
    CommandLineParser::Option* logLevelOption = m_parser->addOption({"--log-level"},
        "Log level for the application (Fatal|Error|Warning|Info|Debug|Verbose).", "level");
    CommandLineParser::Option* logToNetworkOption = m_parser->addOption({"--log-tonetwork"},
        "Stream log data to the network through TCP/IP.", "url");
    CommandLineParser::Option* logToNoViewOption = m_parser->addFlag({"--log-noview"},
        "Disable logging to the live cvs logger.");
    CommandLineParser::Option* logFileOption = m_parser->addOption({"-o", "--log-file"},
        "Output log data to a log file.", "path");
    CommandLineParser::Option* logDailyOption = m_parser->addFlag({"--log-daily"},
        "Create daily log files. The \'--log-file\' option is used as the pattern to generate the actual files."
        "The pattern is specific to Live CVs log date format (i.e. path/to/logfile_%Y_%m_%d.txt). "
        "See the documentation on log prefixes for more details).");
    CommandLineParser::Option* logPrefixOption = m_parser->addOption({"--log-prefix"},
        "Default prefix for messages. See the documentation on logging for more info. To use the standard prefix, "
        "use (\'--log-prefix %p\').", "string");
    CommandLineParser::Option* logConfigOption = m_parser->addOption({"--log-config"},
        "Custom configuration for each log parameter. This will cancel out all other log flags. See the "
        "documentation on logging for more info.", "string");

    CommandLineParser::Option* logConfigFileOption = m_parser->addOption({"--log-config-file"},
        "Json file configuration for each log parameter. This will cancel out all other log flags. See the "
        "documentation on logging for more inf.", "path");

    m_parser->parse(argc, argv);

    m_launchFlag       = m_parser->isSet(launchOption);
    m_runFlag          = m_parser->isSet(runOption);
    m_pluginInfoFlag   = m_parser->isSet(pluginInfoOption);
    m_pluginInfoImport = QString::fromStdString(m_parser->value(pluginInfoOption));

    if ( m_parser->isSet(logConfigFileOption) ){
        m_logConfiguration = MLNode(MLNode::Type::Object);
        QFile lcf(QString::fromStdString(m_parser->value(logConfigFileOption)));
        if ( !lcf.open(QIODevice::ReadOnly) )
            THROW_EXCEPTION(lv::Exception, "Failed to open log configuration file: " + lcf.fileName().toStdString(), Exception::toCode("Init"));

        ml::fromJson(lcf.readAll(), m_logConfiguration);
    } else if ( m_parser->isSet(logConfigOption) ){
        // i.e: level=error; global:level=error; global:prefix=%p

        m_logConfiguration = MLNode(MLNode::Type::Object);

        // separate assignments, and check for optional configuration name
        QStringList cf = QString::fromStdString(m_parser->value(logConfigOption)).split(";");
        for( auto it = cf.begin(); it != cf.end(); ++it ){
            QString& cfs = *it;
            if ( cfs.isEmpty() )
                continue;
            int apos = cfs.indexOf('=');
            if ( apos == -1 )
                THROW_EXCEPTION(lv::Exception, "Failed to parse configuration segment: " + cfs.toStdString(), Exception::toCode("Init"));

            QString configurationName = "global";
            QString configurationKey = "";
            QString configurationValue = "";
            int tpos = cfs.indexOf(':');

            if ( tpos != -1 && tpos < apos){
                configurationName  = cfs.mid(0, tpos);
                configurationKey   = cfs.mid(tpos + 1, apos - tpos - 1);
                configurationValue = cfs.mid(apos + 1);
            } else {
                configurationKey   = cfs.mid(0, apos);
                configurationValue = cfs.mid(apos + 1);
            }

            if ( configurationName.isEmpty() || configurationKey.isEmpty() || configurationValue.isEmpty() )
                THROW_EXCEPTION(lv::Exception, "Failed to parse configuration segment: " + cfs.toStdString(), Exception::toCode("Init"));

            MLNode::StringType cfgname = configurationName.toStdString();
            MLNode::StringType cfgkey  = configurationKey.toStdString();
            if ( !m_logConfiguration.hasKey(configurationName.toStdString()) )
                m_logConfiguration[cfgname] = MLNode(MLNode::Type::Object);

            if ( configurationKey == "logDaily" || configurationKey == "toView" || configurationKey == "toConsole" ){
                m_logConfiguration[cfgname][cfgkey] = configurationValue.toLower() == "true" ? true : false;
            } else {
                m_logConfiguration[cfgname][cfgkey] = configurationValue.toStdString();
            }
        }


    } else {
        m_logConfiguration = MLNode(MLNode::Type::Object);
        m_logConfiguration["global"] = MLNode(MLNode::Type::Object);
        if ( m_parser->isSet(logPrefixOption) ){
            m_logConfiguration["global"]["prefix"] = m_parser->value(logPrefixOption);
        }
        if ( m_parser->isSet(logDailyOption) ){
            m_logConfiguration["global"]["daily"] = true;
        }
        if ( m_parser->isSet(logToNoViewOption) ) {
            m_logConfiguration["global"]["toView"] = false;
        }
        if ( m_parser->isSet(logToNetworkOption) ){
            m_logConfiguration["global"]["toNetwork"] = m_parser->value(logToNetworkOption);
        }
        if ( m_parser->isSet(logLevelOption) ){
            m_logConfiguration["global"]["level"] = m_parser->value(logLevelOption);
        }
        if ( m_parser->isSet(logToConsoleOption) ){
            m_logConfiguration["global"]["toConsole"] = true;
        }
        if ( m_parser->isSet(logFileOption) ){
            m_logConfiguration["global"]["file"] = m_parser->value(logFileOption);
        }
    }

    QString monitoredList = QString::fromStdString(m_parser->value(monitorOption));
    if ( !monitoredList.isEmpty() ){
        m_monitoredFiles = monitoredList.split(";");
    }

}

const MLNode &LiveCVArguments::getLogConfiguration(){
    return m_logConfiguration;
}

bool LiveCVArguments::versionFlag() const{
    return m_parser->isSet(m_parser->versionOption());
}

std::string LiveCVArguments::helpString() const{
    return m_parser->helpString();
}

const std::vector<std::string> &LiveCVArguments::scriptArguments() const{
    return m_parser->scriptArguments();
}

const std::string &LiveCVArguments::script() const{
    return m_parser->script();
}

}// namespace
