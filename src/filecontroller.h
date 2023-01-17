// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QUrl>

#include <KDirWatch>

class FileController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isEmptyFile MEMBER m_isEmptyFile NOTIFY isEmptyFileChanged)
    Q_PROPERTY(QString documentText READ documentText WRITE setDocumentText NOTIFY documentTextChanged)

public:
    explicit FileController(QObject* parent = nullptr);

    bool isEmptyFile();
    QString documentText();
    void setDocumentText(const QString &text);

    Q_INVOKABLE void newFile();
    Q_INVOKABLE void open(QUrl filename);
    Q_INVOKABLE void save();
    Q_INVOKABLE void saveAs(QUrl filename);

    Q_SIGNAL void fileChanged(QString title);
    Q_SIGNAL void isEmptyFileChanged();
    Q_SIGNAL void documentTextChanged();

private Q_SLOTS:
    void slotFileChanged(const QString &path);
    void slotFileDeleted();

private:
    QString m_filename;
    QString m_documentText;
    bool m_isEmptyFile = true;

    KDirWatch* m_watcher;
};
