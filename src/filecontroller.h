// SPDX-FileCopyrightText: 2023 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include <QObject>
#include <QUrl>

class FileController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isEmptyFile MEMBER m_isEmptyFile NOTIFY isEmptyFileChanged)

public:
    explicit FileController(QObject* parent = nullptr);

    bool isEmptyFile();

    Q_INVOKABLE void open(QUrl filename);
    Q_SIGNAL void fileChanged(QString text, QString title);

    Q_INVOKABLE void save(QString text);
    Q_INVOKABLE void saveAs(QUrl filename, QString text);

    Q_SIGNAL void isEmptyFileChanged();

private:
    QString m_filename;
    bool m_isEmptyFile = true;
};
