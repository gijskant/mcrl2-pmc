// Author(s): Rimco Boudewijns
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

/**

  @file documentwidget.h
  @author R. Boudewijns

  A single QTextEdit for a document with built-in file operations

*/

#ifndef MCRL2XI_DOCUMENTWIDGET_H
#define MCRL2XI_DOCUMENTWIDGET_H

#include "mcrl2/utilities/numberedtextedit.h"

class DocumentWidget : public mcrl2::utilities::qt::NumberedTextEdit
{
    Q_OBJECT

  public:
    /**
     * @brief Constructor
     * @param parent The parent QWidget for the document
     */
    DocumentWidget(QWidget *parent = 0);

    /**
     * @brief Returns the filename for this document
     */
    QString getFileName();
    /**
     * @brief Returns true is the document was changed since the last open/save
     */
    bool isModified();

    /**
     * @brief Opens the file provided
     * @param fileName The file that should be opened
     */
    void openFile(QString fileName);
    /**
     * @brief Save the document to the provided @e fileName
     * @param fileName The file to which the contents should be saved
     */
    void saveFile(QString fileName);

  private:
    QString m_filename;         ///< The filename currently loaded, null String if none available
};

#endif // MCRL2XI_DOCUMENTWIDGET_H
