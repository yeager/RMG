/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef ROMBROWSERWIDGET_HPP
#define ROMBROWSERWIDGET_HPP

#include "Thread/RomSearcherThread.hpp"
#include "UserInterface/NoFocusDelegate.hpp"

#include "RomBrowserListViewWidget.hpp"
#include "RomBrowserGridViewWidget.hpp"

#include <QHeaderView>
#include <QList>
#include <QStandardItemModel>
#include <QString>
#include <QTableView>
#include <QMenu>
#include <QAction>
#include <QGridLayout>
#include <QListWidget>
#include <QStackedWidget>

namespace UserInterface
{
namespace Widget
{
class RomBrowserWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    RomBrowserWidget(QWidget *);
    ~RomBrowserWidget(void);

    void RefreshRomList(void);
    bool IsRefreshingRomList(void);
    void StopRefreshRomList(void);

  private:
    Widget::RomBrowserListViewWidget* listViewWidget = nullptr;
    QStandardItemModel* listViewModel                = nullptr;
    Widget::RomBrowserGridViewWidget* gridViewWidget = nullptr;
    QStandardItemModel* gridViewModel                = nullptr;

    Thread::RomSearcherThread* romSearcherThread = nullptr;

    QString coversDirectory;

  private slots:
    void on_DoubleClicked(const QModelIndex& index);

    void on_ZoomIn(void);
    void on_ZoomOut(void);

    void on_RomBrowserThread_RomFound(QString file, CoreRomHeader header, CoreRomSettings settings);

  signals:
    void PlayGame(QString);
    void PlayGameWithDisk(QString);
    void FileDropped(QDropEvent *);
    void EditGameSettings(QString);
    void Cheats(QString);
    void ChooseRomDirectory(void);
    void RomInformation(QString);
};
} // namespace Widget
} // namespace UserInterface

#endif // ROMBROWSERWIDGET_HPP
