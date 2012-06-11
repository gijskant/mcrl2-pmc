// Author(s): Rimco Boudewijns
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#include "toolinformation.h"
#include "mcrl2/utilities/logger.h"
#include <QTextStream>

ToolInformation::ToolInformation(QString name, QString input, QString output)
  : name(name), input(input), output(output), valid(false)
{}

void ToolInformation::load()
{
  QProcess *toolProcess = new QProcess();

  QDir appDir = QDir(QCoreApplication::applicationDirPath());

#ifdef __APPLE__
  appDir.cdUp();
  appDir.cdUp();
#endif

  QString executable = appDir.absoluteFilePath(name);
#ifdef _WIN32
  executable.append(".exe");
#endif
#ifdef __APPLE__
  executable.append(".app/Contents/MacOS/"+ name);
#endif

  toolProcess->start(executable, QStringList("--generate-xml"), QIODevice::ReadOnly);
  if (!toolProcess->waitForFinished(3000))
  {
    mCRL2log(mcrl2::log::error) << toolProcess->errorString().toStdString() << " (" << name.toStdString() << ")" << std::endl;
    delete toolProcess;
    return;
  }
  QByteArray xmlText = toolProcess->readAllStandardOutput();
  delete toolProcess;

  QString errorMsg;
  QDomDocument xml;
  if(!xml.setContent(xmlText, false, &errorMsg))
  {
    mCRL2log(mcrl2::log::error) << "Could not parse XML output of " << name.toStdString() << ": " << errorMsg.toStdString() << std::endl;
    return;
  }

  QDomElement root = xml.documentElement();
  if(root.tagName() != "tool")
  {
    mCRL2log(mcrl2::log::error) << "XML output of " << name.toStdString() << " contains no valid tool information" << std::endl;
    return;
  }

  valid = true;

  QDomElement descElement = root.firstChildElement("description");
  QDomElement authorElement = root.firstChildElement("author");
  QDomElement optionsElement = root.firstChildElement("options");

  if (!descElement.isNull())
  {
    QByteArray data;
    QTextStream out(&data);
    descElement.save(out, 2);
    desc = QString(data);
  }
  else
  {
    mCRL2log(mcrl2::log::warning) << "XML output of " << name.toStdString() << " contains no description element" << std::endl;
  }

  if (!authorElement.isNull())
  {
    author = authorElement.text();
  }
  else
  {
    mCRL2log(mcrl2::log::warning) << "XML output of " << name.toStdString() << " contains no author element" << std::endl;
  }

  if (!optionsElement.isNull())
  {
    parseOptions(optionsElement);
  }
  else
  {
    mCRL2log(mcrl2::log::warning) << "XML output of " << name.toStdString() << " contains no options element" << std::endl;
  }


  appDir.mkdir("xml");

  if (!appDir.exists("xml/"+name+".xml"))
  {
    QFile data(appDir.absoluteFilePath("xml/"+name+".xml"));
    if (data.open(QFile::WriteOnly | QFile::Truncate)) {
        QTextStream out(&data);
        xml.save(out, 2);
    }
  }
}

void ToolInformation::parseOptions(QDomElement optionsElement)
{
  QDomElement optionElement = optionsElement.firstChildElement("option");

  while (!optionElement.isNull())
  {
    bool standard = (optionElement.attribute("standard") == "yes");
    QString optshort = optionElement.firstChildElement("short").text();
    QString optlong = optionElement.firstChildElement("long").text();
    QString optdescription = optionElement.firstChildElement("description").text();
    ToolOption option(standard, optshort, optlong, optdescription);

    QDomElement argElement = optionElement.firstChildElement("option_argument");

    if (!argElement.isNull())
    {
      bool optional = (argElement.attribute("optional") == "yes");
      QString typeStr = argElement.attribute("type");
      QString name = argElement.firstChildElement("name").text();


      ToolArgument argument(optional, guessType(typeStr, name), name);
      option.argument = argument;
    }

    options.append(option);
    optionElement = optionElement.nextSiblingElement("option");
  }

}

ArgumentType ToolInformation::guessType(QString type, QString name)
{
  return UnknownArgument;
}







