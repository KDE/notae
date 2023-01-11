// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QFile>
#include <QTextStream>

#include "filecontroller.h"

FileController::FileController(QObject *parent)
    : QObject(parent)
{
}

bool FileController::isEmptyFile()
{
    return m_isEmptyFile;
}

void FileController::open(QUrl filename)
{
    QFile file(filename.path());

    if (!filename.isEmpty()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            m_filename = filename.path();
            QTextStream in(&file);
            QString text = in.readAll();
            file.close();

            m_isEmptyFile = false;
            Q_EMIT isEmptyFileChanged();

            Q_EMIT fileChanged(text, filename.fileName());
        }
    }
}

void FileController::save(QString text)
{
    QFile file(m_filename);

    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << text;
        file.flush();
        file.close();
    }
}

void FileController::saveAs(QUrl filename, QString text)
{
    QFile file(filename.path());

    if (!filename.isEmpty()) {
        m_filename = filename.path();
        save(text);
    }
}
