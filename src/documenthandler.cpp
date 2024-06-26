// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-License-Identifier:  BSD-3-Clause

#include <QAbstractTextDocumentLayout>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QFileSelector>
#include <QFileSystemWatcher>
#include <QQmlFile>
#include <QQmlFileSelector>
#include <QQuickTextDocument>
#include <QStringDecoder>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QUrl>

#include <KLocalizedString>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#include <qstandardpaths.h>

#include "documenthandler.h"

#define AUTOSAVE_TIMEOUT 5000

/**
 * Global Variables
 */
KSyntaxHighlighting::Repository *DocumentHandler::m_repository = nullptr;
int DocumentHandler::m_instanceCount = 0;

void FileLoader::loadFile(const QUrl &url)
{
    if (QFile::exists(url.path())) {
        QFile file(url.toLocalFile());
        if (file.open(QFile::ReadOnly)) {
            const auto array = file.readAll();
            auto encoding = QStringConverter::encodingForData(array);
            if (encoding.has_value()) {
                Q_EMIT this->fileReady(QStringDecoder(*encoding).decode(array), url);
            } else {
                // If it's unknown encoding, try to give it to the user anyway
                Q_EMIT this->fileReady(QString::fromUtf8(array), url);
            }
        } else {
            qWarning() << "Failed to open file: " << file.errorString();
        }
    }
}

DocumentHandler::DocumentHandler(QObject *parent)
    : QObject(parent)
    , m_document(nullptr)
    , m_watcher(new QFileSystemWatcher(this))
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
    , m_highlighter(new KSyntaxHighlighting::SyntaxHighlighter(this))
{
    ++m_instanceCount;

    // start file loader thread implementation

    {
        FileLoader *m_loader = new FileLoader;
        m_loader->moveToThread(&m_worker);
        connect(&m_worker, &QThread::finished, m_loader, &QObject::deleteLater);
        connect(this, &DocumentHandler::loadFile, m_loader, &FileLoader::loadFile);
        connect(m_loader, &FileLoader::fileReady, [this](QString array, QUrl url) {
            this->setText(array);

            if (this->textDocument()) {
                this->textDocument()->setModified(false);

                this->isRich = Qt::mightBeRichText(this->text());
                Q_EMIT this->isRichChanged();
            }

            Q_EMIT this->loaded(url);

            reset();
        });
        m_worker.start();
    }

    // end file loader thread implementation

    connect(&m_autoSaveTimer, &QTimer::timeout, [this]() {
        if (m_autoSave && getModified() && !m_fileUrl.isEmpty()) {
            qDebug() << "Autosaving file" << m_fileUrl;
            saveAs(m_fileUrl);
            m_autoSaveTimer.start(AUTOSAVE_TIMEOUT);
        }
    });

    if (m_autoSave)
        m_autoSaveTimer.start(AUTOSAVE_TIMEOUT);

    connect(this, &DocumentHandler::cursorPositionChanged, [this]() {
        Q_EMIT this->currentLineIndexChanged();
    });

    connect(this->m_watcher, &QFileSystemWatcher::fileChanged, [this](QString url) {
        if (this->fileUrl() == QUrl::fromLocalFile(url)) {
            // THE FILE WAS REMOVED

            // THE FILE CHANGED BUT STILL EXISTS LOCALLY
            if (m_internallyModified) {
                m_internallyModified = false;
                return;
            }

            this->setExternallyModified(true);

            Q_EMIT this->loadFile(this->fileUrl());
        }
    });
}

DocumentHandler::~DocumentHandler()
{
    this->m_worker.quit();
    this->m_worker.wait();

    --DocumentHandler::m_instanceCount;

    if (!DocumentHandler::m_instanceCount) {
        delete DocumentHandler::m_repository;
        DocumentHandler::m_repository = nullptr;
    }
}

void DocumentHandler::setText(const QString &text)
{
    if (text != this->m_text) {
        this->m_text = text;
        Q_EMIT textChanged();
    }
}

bool DocumentHandler::getAutoReload() const
{
    return this->m_autoReload;
}

void DocumentHandler::setAutoReload(const bool &value)
{
    if (value == this->m_autoReload)
        return;

    this->m_autoReload = value;
    Q_EMIT this->autoReloadChanged();
}

bool DocumentHandler::autoSave() const
{
    return m_autoSave;
}

void DocumentHandler::setAutoSave(const bool &value)
{
    if (m_autoSave == value)
        return;

    m_autoSave = value;
    Q_EMIT autoSaveChanged();

    if (m_autoSave) {
        if (!m_autoSaveTimer.isActive())
            m_autoSaveTimer.start(AUTOSAVE_TIMEOUT);
    } else
        m_autoSaveTimer.stop();
}

bool DocumentHandler::getModified() const
{
    if (auto doc = this->textDocument())
        return doc->isModified();

    return false;
}

bool DocumentHandler::getExternallyModified() const
{
    return this->m_externallyModified;
}

void DocumentHandler::setExternallyModified(const bool &value)
{
    if (value == this->m_externallyModified)
        return;

    this->m_externallyModified = value;
    Q_EMIT this->externallyModifiedChanged();
}

void DocumentHandler::setStyle()
{
    if (!DocumentHandler::m_repository)
        DocumentHandler::m_repository = new KSyntaxHighlighting::Repository();

    qDebug() << "Setting ths tyle" << m_formatName;
    if (!m_enableSyntaxHighlighting || m_formatName == QStringLiteral("None")) {
        this->m_highlighter->setDocument(nullptr);
        return;
    }

    qDebug() << "Setting the style for syntax highligthing";

    const auto def = m_repository->definitionForName(this->m_formatName);
    if (!def.isValid()) {
        qDebug() << "Highliging definition is not valid" << def.name() << def.filePath() << def.author() << m_formatName;
        return;
    }

    if (!m_highlighter->document()) {
        this->m_highlighter->setDocument(this->textDocument());
    }

    qDebug() << "Highliging definition info" << def.name() << def.filePath() << def.author() << m_formatName;

    this->m_highlighter->setDefinition(def);

    if (m_theme.isEmpty()) {
        const bool isDark = DocumentHandler::isDark(this->m_backgroundColor);
        const auto style =
            DocumentHandler::m_repository->defaultTheme(isDark ? KSyntaxHighlighting::Repository::DarkTheme : KSyntaxHighlighting::Repository::LightTheme);
        this->m_highlighter->setTheme(style);
    } else {
        qDebug() << "Applying theme << " << m_theme << DocumentHandler::m_repository->theme(m_theme).isValid();
        const auto style = DocumentHandler::m_repository->theme(m_theme);
        this->m_highlighter->setTheme(style);
        this->m_highlighter->rehighlight();
    }

    refreshAllBlocks();
}

void DocumentHandler::refreshAllBlocks()
{
    if (textDocument()) {
        for (QTextBlock it = textDocument()->begin(); it != textDocument()->end(); it = it.next()) {
            Q_EMIT this->textDocument()->documentLayout()->updateBlock(it);
        }
    }
}

QString DocumentHandler::formatName() const
{
    return this->m_formatName;
}

void DocumentHandler::setFormatName(const QString &formatName)
{
    if (this->m_formatName != formatName) {
        this->m_formatName = formatName;
        Q_EMIT this->formatNameChanged();
    }

    this->setStyle();
}

QColor DocumentHandler::getBackgroundColor() const
{
    return this->m_backgroundColor;
}

void DocumentHandler::setBackgroundColor(const QColor &color)
{
    if (this->m_backgroundColor == color)
        return;

    this->m_backgroundColor = color;
    Q_EMIT this->backgroundColorChanged();

    if (!DocumentHandler::m_repository)
        DocumentHandler::m_repository = new KSyntaxHighlighting::Repository();
}

QQuickTextDocument *DocumentHandler::document() const
{
    return m_document;
}

void DocumentHandler::setDocument(QQuickTextDocument *document)
{
    this->m_document = document;
    Q_EMIT documentChanged();

    if (this->textDocument()) {
        this->textDocument()->setModified(false);
        connect(this->textDocument(), &QTextDocument::modificationChanged, this, &DocumentHandler::modifiedChanged);

        connect(this->textDocument(), &QTextDocument::blockCountChanged, this, &DocumentHandler::lineCountChanged);

        this->load(m_fileUrl);

        QTextOption textOptions = this->textDocument()->defaultTextOption();
        textOptions.setTabStopDistance(m_tabSpace);
        textDocument()->setDefaultTextOption(textOptions);
    }
}

int DocumentHandler::cursorPosition() const
{
    return m_cursorPosition;
}

void DocumentHandler::setCursorPosition(int position)
{
    if (m_cursorPosition == position) {
        return;
    }

    m_cursorPosition = position;
    Q_EMIT cursorPositionChanged();
}

int DocumentHandler::selectionStart() const
{
    return m_selectionStart;
}

void DocumentHandler::setSelectionStart(int position)
{
    if (position == m_selectionStart)
        return;

    m_selectionStart = position;
    Q_EMIT selectionStartChanged();
}

int DocumentHandler::selectionEnd() const
{
    return m_selectionEnd;
}

void DocumentHandler::setSelectionEnd(int position)
{
    if (position == m_selectionEnd)
        return;

    m_selectionEnd = position;
    Q_EMIT selectionEndChanged();
}

QString DocumentHandler::fontFamily() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QString();
    QTextCharFormat format = cursor.charFormat();
    return format.font().family();
}

void DocumentHandler::setFontFamily(const QString &family)
{
    QTextCharFormat format;
    format.setFontFamilies({family});
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontFamilyChanged();
}

QColor DocumentHandler::textColor() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return QColor(Qt::black);
    QTextCharFormat format = cursor.charFormat();
    return format.foreground().color();
}

void DocumentHandler::setTextColor(const QColor &color)
{
    QTextCharFormat format;
    format.setForeground(QBrush(color));
    mergeFormatOnWordOrSelection(format);
    Q_EMIT textColorChanged();
}

Qt::Alignment DocumentHandler::alignment() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return Qt::AlignLeft;
    return textCursor().blockFormat().alignment();
}

void DocumentHandler::setAlignment(Qt::Alignment alignment)
{
    QTextBlockFormat format;
    format.setAlignment(alignment);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    Q_EMIT alignmentChanged();
}

bool DocumentHandler::bold() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontWeight() == QFont::Bold;
}

void DocumentHandler::setBold(bool bold)
{
    QTextCharFormat format;
    format.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT boldChanged();
}

bool DocumentHandler::uppercase() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontCapitalization() == QFont::AllUppercase;
}

void DocumentHandler::setUppercase(bool uppercase)
{
    QTextCharFormat format;
    format.setFontCapitalization(uppercase ? QFont::AllUppercase : QFont::AllLowercase);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT uppercaseChanged();
}

bool DocumentHandler::italic() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontItalic();
}

void DocumentHandler::setItalic(bool italic)
{
    QTextCharFormat format;
    format.setFontItalic(italic);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT italicChanged();
}

bool DocumentHandler::underline() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return false;
    return textCursor().charFormat().fontUnderline();
}

void DocumentHandler::setUnderline(bool underline)
{
    QTextCharFormat format;
    format.setFontUnderline(underline);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT underlineChanged();
}

bool DocumentHandler::getIsRich() const
{
    return this->isRich;
}

int DocumentHandler::fontSize() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;
    QTextCharFormat format = cursor.charFormat();
    return format.font().pointSize();
}

void DocumentHandler::setFontSize(int size)
{
    if (size <= 0)
        return;

    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return;

    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);

    if (cursor.charFormat().property(QTextFormat::FontPointSize).toInt() == size)
        return;

    QTextCharFormat format;
    format.setFontPointSize(size);
    mergeFormatOnWordOrSelection(format);
    Q_EMIT fontSizeChanged();
}

void DocumentHandler::setTabSpace(qreal value)
{
    if (m_tabSpace == value)
        return;

    m_tabSpace = value;

    if (textDocument()) {
        QTextOption textOptions = this->textDocument()->defaultTextOption();
        textOptions.setTabStopDistance(m_tabSpace);
        textDocument()->setDefaultTextOption(textOptions);
    }

    Q_EMIT tabSpaceChanged();
    refreshAllBlocks();
}

qreal DocumentHandler::tabSpace() const
{
    return m_tabSpace;
}

QString DocumentHandler::fileName() const
{
    const QString filePath = QQmlFile::urlToLocalFileOrQrc(m_fileUrl);
    const QString fileName = QFileInfo(filePath).fileName();
    if (fileName.isEmpty())
        return QStringLiteral("untitled.txt");
    return fileName;
}

QString DocumentHandler::fileType() const
{
    return QFileInfo(fileName()).suffix();
}

QUrl DocumentHandler::fileUrl() const
{
    return m_fileUrl;
}

void DocumentHandler::setFileUrl(const QUrl &url)
{
    if (url == m_fileUrl)
        return;

    m_fileUrl = url;

    load(m_fileUrl);

    Q_EMIT fileUrlChanged();
    Q_EMIT fileInfoChanged();
}

QVariantMap DocumentHandler::fileInfo() const
{
    const QFileInfo file(m_fileUrl.toLocalFile());
    if (file.exists()) {
        return QVariantMap();
    }

    QVariantMap map = {};

    return map;
}

void DocumentHandler::load(const QUrl &url)
{
    qDebug() << "TRYING TO LOAD FILE << " << url << url.isEmpty();
    if (!textDocument())
        return;

    if (m_fileUrl.isLocalFile() && !QFile::exists(url.path()))
        return;

    QQmlEngine *engine = qmlEngine(this);
    if (!engine) {
        qWarning() << "load() called before DocumentHandler has QQmlEngine";
        return;
    }

    this->m_watcher->removePaths(this->m_watcher->files());
    this->m_watcher->addPath(m_fileUrl.toLocalFile());

    Q_EMIT this->loadFile(m_fileUrl);

    if (m_enableSyntaxHighlighting) {
        this->setFormatName(DocumentHandler::getLanguageNameFromFileName(m_fileUrl));
    }
}

void DocumentHandler::saveAs(const QUrl &url)
{
    if (url.isEmpty() || !url.isValid())
        return;

    QTextDocument *doc = this->textDocument();
    if (!doc)
        return;

    this->m_internallyModified = true;

    const QString filePath = url.toLocalFile();
    const bool isHtml = QFileInfo(filePath).suffix().contains(QLatin1String("htm"));
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Truncate | (isHtml ? QFile::NotOpen : QFile::Text))) {
        Q_EMIT error(i18nd("notae", "Cannot save: ") + file.errorString());

        return;
    }
    file.write((isHtml ? doc->toHtml() : doc->toPlainText()).toUtf8());
    file.close();
    Q_EMIT fileSaved();

    doc->setModified(false);

    if (url == m_fileUrl)
        return;

    m_fileUrl = url;
    Q_EMIT fileUrlChanged();
}

const QString DocumentHandler::getLanguageNameFromFileName(const QUrl &fileName)
{
    if (!DocumentHandler::m_repository)
        DocumentHandler::m_repository = new KSyntaxHighlighting::Repository();

    const auto res = DocumentHandler::m_repository->definitionForFileName(fileName.toString());

    return res.isValid() ? res.name() : QString();
}

const QStringList DocumentHandler::getLanguageNameList()
{
    if (!DocumentHandler::m_repository)
        m_repository = new KSyntaxHighlighting::Repository();

    const auto definitions = DocumentHandler::m_repository->definitions();

    QStringList names;

    for (auto definition : definitions) {
        names.append(definition.name());
    }

    return names;
}

const QStringList DocumentHandler::getThemes()
{
    if (!DocumentHandler::m_repository)
        DocumentHandler::m_repository = new KSyntaxHighlighting::Repository();

    const auto themes = DocumentHandler::m_repository->themes();

    QStringList names;

    for (auto theme : themes) {
        names << theme.name();
    }

    return names;
}

void DocumentHandler::reset()
{
    Q_EMIT fontFamilyChanged();
    Q_EMIT alignmentChanged();
    Q_EMIT boldChanged();
    Q_EMIT italicChanged();
    Q_EMIT underlineChanged();
    Q_EMIT fontSizeChanged();
    Q_EMIT textColorChanged();
}

QTextCursor DocumentHandler::textCursor() const
{
    QTextDocument *doc = textDocument();
    if (!doc)
        return QTextCursor();

    QTextCursor cursor = QTextCursor(doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

QTextDocument *DocumentHandler::textDocument() const
{
    if (!m_document)
        return nullptr;

    return m_document->textDocument();
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
}

void DocumentHandler::find(const QString &query, const bool &forward)
{
    qDebug() << "Asked to find" << query;
    QTextDocument *doc = textDocument();

    if (!doc) {
        return;
    }

    QTextDocument::FindFlags searchFlags;
    QTextDocument::FindFlags newFlags = searchFlags;

    if (!forward) {
        newFlags = searchFlags | QTextDocument::FindBackward;
    }

    if (m_findCaseSensitively) {
        newFlags = newFlags | QTextDocument::FindCaseSensitively;
    }

    if (m_findWholeWords) {
        newFlags = newFlags | QTextDocument::FindWholeWords;
    }

    QTextCursor start = this->textCursor();

    if (query != m_searchQuery) {
        start.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
        m_searchQuery = query;
    }

    if (!start.isNull() && !start.atEnd()) {
        QTextCursor found = doc->find(m_searchQuery, start, newFlags);
        if (found.isNull()) {
            if (!forward)
                start.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
            else
                start.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

            this->setCursorPosition(start.position());

            found = doc->find(m_searchQuery, start, newFlags);
        }

        if (!found.isNull()) {
            setSelectionStart(found.selectionStart());
            setSelectionEnd(found.selectionEnd());
            setCursorPosition(found.position());
            Q_EMIT searchFound(selectionStart(), selectionEnd());
        }
    }
}

void DocumentHandler::replace(const QString &query, const QString &value)
{
    if (value.isEmpty()) {
        return;
    }

    if (this->textDocument()) {
        if (m_searchQuery.isEmpty() || query != m_searchQuery) {
            find(query);
        }

        auto cursor = this->textCursor();
        cursor.beginEditBlock();
        cursor.insertText(value);
        cursor.endEditBlock();

        find(query);
    }
}

void DocumentHandler::replaceAll(const QString &query, const QString &value)
{
    QTextDocument *doc = textDocument();

    if (!doc) {
        return;
    }

    QTextCursor newCursor(doc);
    newCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

    if (newCursor.isNull() || newCursor.atEnd()) {
        return;
    }

    QTextDocument::FindFlags searchFlags;
    QTextDocument::FindFlags newFlags = searchFlags;

    if (m_findCaseSensitively) {
        newFlags = searchFlags | QTextDocument::FindCaseSensitively;
    }

    if (m_findWholeWords) {
        newFlags = searchFlags | QTextDocument::FindWholeWords;
    }

    while (!newCursor.isNull() && !newCursor.atEnd()) {
        newCursor = doc->find(query, newCursor, newFlags);

        if (!newCursor.isNull()) {
            newCursor.beginEditBlock();
            newCursor.insertText(value);
            newCursor.endEditBlock();
        }
    }
}

bool DocumentHandler::isFoldable(const int &line) const
{
    if (!m_highlighter)
        return false;

    if (auto doc = this->textDocument()) {
        return m_highlighter->startsFoldingRegion(doc->findBlockByLineNumber(line));
    }

    return false;
}

bool DocumentHandler::isFolded(const int &line) const
{
    if (!m_highlighter)
        return false;

    if (auto doc = this->textDocument()) {
        auto block = doc->findBlockByLineNumber(line);

        if (!block.isValid())
            return false;

        const auto nextBlock = block.next();

        if (!nextBlock.isValid())
            return false;

        return !nextBlock.isVisible();
    }

    return false;
}

void DocumentHandler::toggleFold(const int &line)
{
    if (!m_highlighter)
        return;

    if (auto doc = this->textDocument()) {
        auto startBlock = doc->findBlockByLineNumber(line);

        // we also want to fold the last line of the region, therefore the ".next()"
        const auto endBlock = m_highlighter->findFoldingRegionEnd(startBlock).next();

        qDebug() << "Fold line" << line << startBlock.position() << endBlock.position() << doc->blockCount();
        // fold
        auto block = startBlock.next();
        while (block.isValid() && block != endBlock) {
            block.setVisible(false);
            block.setLineCount(0);
            block = block.next();
        }

        for (QTextBlock it = startBlock; it != endBlock; it = it.next()) {
            Q_EMIT this->textDocument()->documentLayout()->updateBlock(it);
        }

        // redraw document
        //         doc->markContentsDirty(startBlock.position(), endBlock.position());
        qDebug() << "Fold line" << line << startBlock.position() << endBlock.position() << doc->blockCount();

        //         // update scrollbars
        Q_EMIT doc->documentLayout()->documentSizeChanged(doc->documentLayout()->documentSize());
    }
}

int DocumentHandler::lineHeight(const int &line)
{
    QTextDocument *doc = textDocument();

    if (!doc) {
        return 0;
    }

    return int(doc->documentLayout()->blockBoundingRect(doc->findBlockByLineNumber(line)).height());
}

int DocumentHandler::lineCount()
{
    if (!this->textDocument())
        return 0;
    return this->textDocument()->blockCount();
}

int DocumentHandler::getCurrentLineIndex()
{
    if (!this->textDocument())
        return -1;

    return this->textDocument()->findBlock(m_cursorPosition).blockNumber();
}

int DocumentHandler::goToLine(const int &line)
{
    if (!this->textDocument())
        return this->cursorPosition();
    const auto block = this->textDocument()->findBlockByLineNumber(line);
    return block.position() + block.length() - 1;
}

void DocumentHandler::setEnableSyntaxHighlighting(const bool &value)
{
    if (m_enableSyntaxHighlighting == value) {
        return;
    }

    m_enableSyntaxHighlighting = value;

    if (!m_enableSyntaxHighlighting) {
        this->setFormatName(QStringLiteral("None"));
    } else {
        this->setFormatName(DocumentHandler::getLanguageNameFromFileName(m_fileUrl));
    }

    Q_EMIT enableSyntaxHighlightingChanged();
}

bool DocumentHandler::enableSyntaxHighlighting() const
{
    return m_enableSyntaxHighlighting;
}

void DocumentHandler::setTheme(const QString &theme)
{
    if (m_theme == theme)
        return;

    m_theme = theme;
    setStyle();
    qDebug() << "changinf the theme<< " << theme << m_theme;
    Q_EMIT themeChanged();
}

QString DocumentHandler::theme() const
{
    return m_theme;
}
