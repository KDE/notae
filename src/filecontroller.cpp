// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QFile>
#include <QTextStream>

#include "filecontroller.h"

#include "config.h"

FileController::FileController(QObject *parent)
    : QObject(parent)
{
    m_watcher = new KDirWatch;
    QObject::connect(m_watcher, &KDirWatch::dirty, this, &FileController::slotFileChanged);
    QObject::connect(m_watcher, &KDirWatch::deleted, this, &FileController::slotFileDeleted);
}

bool FileController::isEmptyFile()
{
    return m_isEmptyFile;
}

QString FileController::documentText()
{
    return m_documentText;
}

void FileController::setDocumentText(const QString &text) {
    if (text != m_documentText) {
        m_documentText = text;
        Q_EMIT documentTextChanged();
    }
}

void FileController::newFile()
{
    m_filename = QString();
    Q_EMIT fileChanged(QString());

    m_isEmptyFile = true;
    Q_EMIT isEmptyFileChanged();

    m_documentText = QString();
    Q_EMIT documentTextChanged();
}

void FileController::open(QUrl filename)
{
    QFile file(filename.path());

    if (!filename.isEmpty()) {
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            m_filename = filename.path();
            Q_EMIT fileChanged(filename.fileName());

            m_watcher->addFile(m_filename);

            QTextStream in(&file);
            QString text = in.readAll();
            file.close();

            if (Config::self()->rememberMostRecentFile()) {
                Config::self()->setMostRecentFile(m_filename);
                Config::self()->save();
            }

            m_isEmptyFile = false;
            Q_EMIT isEmptyFileChanged();

            m_documentText = text;
            Q_EMIT documentTextChanged();
        }
    }
}

void FileController::save()
{
    QFile file(m_filename);

    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << m_documentText;
        file.flush();
        file.close();
    }
}

void FileController::saveAs(QUrl filename)
{
    QFile file(filename.path());

    if (!filename.isEmpty()) {
        m_filename = filename.path();
        Q_EMIT fileChanged(filename.fileName());

        m_watcher->addFile(m_filename);

        if (Config::self()->rememberMostRecentFile()) {
            Config::self()->setMostRecentFile(m_filename);
            Config::self()->save();
        }

        m_isEmptyFile = false;
        Q_EMIT isEmptyFileChanged();

        save();
    }
}

void FileController::slotFileChanged(const QString &path)
{
    if (m_watcher->contains(path)) {
        open(path);
    }
}

void FileController::slotFileDeleted()
{
    newFile();
}
