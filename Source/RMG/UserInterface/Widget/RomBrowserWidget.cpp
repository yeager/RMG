/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "RomBrowserWidget.hpp"
#include "ColumnID.hpp"
#include "UserInterface/Dialog/CheatsDialog.hpp"

#include <RMG-Core/Core.hpp>

#include <QDir>
#include <QDragMoveEvent>

#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <qabstractitemmodel.h>
#include <qboxlayout.h>
#include <qlist.h>
#include <qlistview.h>
#include <qlistwidget.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <QListWidget>
#include <qstandarditemmodel.h>
#include <vector>
#include <QList>

using namespace UserInterface::Widget;


RomBrowserWidget::RomBrowserWidget(QWidget *parent) : QStackedWidget(parent)
{
    // configure rom searcher thread
    this->romSearcherThread = new Thread::RomSearcherThread(this);
    connect(this->romSearcherThread, &Thread::RomSearcherThread::RomFound, this,&RomBrowserWidget::on_RomBrowserThread_RomFound);

    // configure list view widget
    this->listViewWidget = new Widget::RomBrowserListViewWidget(this);
    this->listViewModel  = new QStandardItemModel(this);
    this->listViewWidget->setModel(this->listViewModel);
    this->listViewWidget->setItemDelegate(new NoFocusDelegate(this));
    this->listViewWidget->setWordWrap(false);
    this->listViewWidget->setShowGrid(false);
    this->listViewWidget->setSortingEnabled(true);
    this->listViewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->listViewWidget->setSelectionBehavior(QTableView::SelectRows);
    this->listViewWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    this->listViewWidget->verticalHeader()->hide();
    this->listViewWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->listViewWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    this->listViewWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    this->listViewWidget->horizontalHeader()->setStretchLastSection(true); // TODO: remove?
    this->listViewWidget->horizontalHeader()->setSortIndicatorShown(false);
    this->listViewWidget->horizontalHeader()->setHighlightSections(false);
    this->addWidget(this->listViewWidget);
    connect(this->listViewWidget, &QTableView::doubleClicked, this, &RomBrowserWidget::on_DoubleClicked);
    connect(this->listViewWidget, &Widget::RomBrowserListViewWidget::ZoomIn, this, &RomBrowserWidget::on_ZoomIn);
    connect(this->listViewWidget, &Widget::RomBrowserListViewWidget::ZoomOut, this, &RomBrowserWidget::on_ZoomOut);

    // configure grid view widget
    this->gridViewWidget = new Widget::RomBrowserGridViewWidget(this);
    this->gridViewModel  = new QStandardItemModel(this);
    this->gridViewWidget->setModel(this->gridViewModel);
    this->gridViewWidget->setItemDelegate(new NoFocusDelegate(this));
    this->gridViewWidget->setFlow(QListView::Flow::LeftToRight);
    this->gridViewWidget->setResizeMode(QListView::Adjust);
    this->gridViewWidget->setViewMode(QListView::ViewMode::IconMode);
    this->gridViewWidget->setTextElideMode(Qt::ElideNone);    
    this->gridViewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gridViewWidget->setWordWrap(true);
    this->gridViewWidget->setIconSize(QSize(180, 126)); // TODO: load this from settings
    this->addWidget(this->gridViewWidget);
    connect(this->gridViewWidget, &QTableView::doubleClicked, this, &RomBrowserWidget::on_DoubleClicked);
    connect(this->gridViewWidget, &Widget::RomBrowserGridViewWidget::ZoomIn, this, &RomBrowserWidget::on_ZoomIn);
    connect(this->gridViewWidget, &Widget::RomBrowserGridViewWidget::ZoomOut, this, &RomBrowserWidget::on_ZoomOut);

    this->setCurrentWidget(this->gridViewWidget);
}

RomBrowserWidget::~RomBrowserWidget()
{
}

void RomBrowserWidget::RefreshRomList(void)
{
    this->listViewModel->clear();
    this->gridViewModel->clear();
    
    this->coversDirectory = QString::fromStdString(CoreGetUserDataDirectory().string());
    this->coversDirectory += "/Covers";

    this->romSearcherThread->SetMaximumFiles(CoreSettingsGetIntValue(SettingsID::RomBrowser_MaxItems));
    this->romSearcherThread->SetRecursive(CoreSettingsGetBoolValue(SettingsID::RomBrowser_Recursive));
    this->romSearcherThread->SetDirectory(QString::fromStdString(CoreSettingsGetStringValue(SettingsID::RomBrowser_Directory)));
    this->romSearcherThread->start();
}

bool RomBrowserWidget::IsRefreshingRomList(void)
{
    return this->romSearcherThread->isRunning();
}

void RomBrowserWidget::StopRefreshRomList(void)
{
    return this->romSearcherThread->Stop();
}

void RomBrowserWidget::on_DoubleClicked(const QModelIndex& index)
{
    QStandardItemModel* model = model = this->listViewModel;;
    if (this->currentWidget() == this->gridViewWidget)
    {
        model = this->gridViewModel;
    }

    QString rom = model->itemData(index).last().toString();
    emit this->PlayGame(rom);
}

void RomBrowserWidget::on_ZoomIn(void)
{
    QAbstractItemView* view = this->listViewWidget;
    if (this->currentWidget() == this->gridViewWidget)
    {
        view = this->gridViewWidget;
    }

    view->setIconSize(view->iconSize() + QSize(20, 20));
}

void RomBrowserWidget::on_ZoomOut(void)
{
    QAbstractItemView* view = this->listViewWidget;
    if (this->currentWidget() == this->gridViewWidget)
    {
        view = this->gridViewWidget;
    }

    view->setIconSize(view->iconSize() - QSize(20, 20));
}

#include <iostream>
void RomBrowserWidget::on_RomBrowserThread_RomFound(QString file, CoreRomHeader header, CoreRomSettings settings)
{
    QString name;
    QPixmap pixmap;

    // generate name to use in UI
    name = QString::fromStdString(settings.GoodName);
    if (name.endsWith("(unknown rom)"))
    {
        name = QFileInfo(file).fileName();
    }

    // try to load cover
    QString coverPath = this->coversDirectory;
    coverPath += "/";
    coverPath += QString::fromStdString(header.Name);
    coverPath += ".jpg";

    if (!QFile::exists(coverPath) ||
        !pixmap.load(coverPath))
    {
        pixmap.load(":Resource/CoverFallback.png");
    }

    QStandardItem* listViewItem = new QStandardItem(QIcon(pixmap), name);
    listViewItem->setData(file);

    QStandardItem* gridViewItem = new QStandardItem(QIcon(pixmap), name);
    gridViewItem->setData(file);

    this->listViewModel->appendRow(listViewItem);
    this->listViewModel->sort(0, Qt::AscendingOrder);

    this->gridViewModel->appendRow(gridViewItem);
    this->gridViewModel->sort(0, Qt::AscendingOrder);
}
