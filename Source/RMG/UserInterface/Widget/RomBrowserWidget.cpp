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
#include <vector>
#include <QList>

using namespace UserInterface::Widget;


RomBrowserWidget::RomBrowserWidget(QWidget *parent) : QStackedWidget(parent)
{
    // configure rom searcher thread
    this->romSearcherThread = new Thread::RomSearcherThread(this);
    connect(this->romSearcherThread, &Thread::RomSearcherThread::RomFound, this, &RomBrowserWidget::on_RomBrowserThread_RomFound);
    connect(this->romSearcherThread, &Thread::RomSearcherThread::Finished, this, &RomBrowserWidget::on_RomBrowserThread_Finished);

    // configure empty widget
    this->emptyWidget = new Widget::RomBrowserEmptyWidget(this);
    this->addWidget(this->emptyWidget);
    connect(this->emptyWidget, &RomBrowserEmptyWidget::SelectRomDirectory, this, &RomBrowserWidget::on_Action_ChooseRomDirectory);
    connect(this->emptyWidget, &RomBrowserEmptyWidget::Refresh, this, &RomBrowserWidget::on_Action_RefreshRomList);

    // configure loading widget
    this->loadingWidget = new Widget::RomBrowserLoadingWidget(this);
    this->addWidget(this->loadingWidget);

    // configure list view widget
    this->listViewWidget = new Widget::RomBrowserListViewWidget(this);
    this->listViewModel  = new QStandardItemModel(this);
    this->listViewWidget->setModel(this->listViewModel);
    this->listViewWidget->setFrameStyle(QFrame::NoFrame);
    //this->listViewWidget->setItemDelegate(new NoFocusDelegate(this));
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
    this->listViewWidget->horizontalHeader()->setStretchLastSection(true);
    this->listViewWidget->horizontalHeader()->setSortIndicatorShown(false);
    this->listViewWidget->horizontalHeader()->setHighlightSections(false);
    this->listViewWidget->horizontalHeader()->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    this->addWidget(this->listViewWidget);
    connect(this->listViewWidget, &QTableView::doubleClicked, this, &RomBrowserWidget::on_DoubleClicked);
    connect(this->listViewWidget, &QTableView::customContextMenuRequested, this, &RomBrowserListViewWidget::customContextMenuRequested);
    connect(this->listViewWidget->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &RomBrowserWidget::on_listViewWidget_sortIndicatorChanged);
    connect(this->listViewWidget, &Widget::RomBrowserListViewWidget::ZoomIn, this, &RomBrowserWidget::on_ZoomIn);
    connect(this->listViewWidget, &Widget::RomBrowserListViewWidget::ZoomOut, this, &RomBrowserWidget::on_ZoomOut);


    // TODO
    QStringList labels;
    labels << "Name";
    labels << "Internal Name";
    labels << "MD5";
    this->listViewModel->setColumnCount(labels.size());
    this->listViewModel->setHorizontalHeaderLabels(labels);

    // configure grid view widget
    this->gridViewWidget = new Widget::RomBrowserGridViewWidget(this);
    this->gridViewModel  = new QStandardItemModel(this);
    this->gridViewWidget->setModel(this->gridViewModel);
    this->gridViewWidget->setFlow(QListView::Flow::LeftToRight);
    this->gridViewWidget->setResizeMode(QListView::Adjust);
    this->gridViewWidget->setUniformItemSizes(false);
    this->gridViewWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    this->gridViewWidget->setViewMode(QListView::ViewMode::IconMode);
    this->gridViewWidget->setTextElideMode(Qt::ElideNone);    
    this->gridViewWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->gridViewWidget->setWordWrap(true);
    this->gridViewWidget->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    this->gridViewWidget->setFrameStyle(QFrame::NoFrame);
    int iconWidth = CoreSettingsGetIntValue(SettingsID::RomBrowser_GridViewIconWidth);
    int iconHeight = CoreSettingsGetIntValue(SettingsID::RomBrowser_GridViewIconHeight);
    this->gridViewWidget->setIconSize(QSize(iconWidth, iconHeight));
    //this->gridViewWidget->setSpacing(32); // TODO
    this->addWidget(this->gridViewWidget);
    connect(this->gridViewWidget, &QListView::doubleClicked, this, &RomBrowserWidget::on_DoubleClicked);
    connect(this->gridViewWidget, &QListView::customContextMenuRequested, this, &RomBrowserListViewWidget::customContextMenuRequested);
    connect(this->gridViewWidget, &Widget::RomBrowserGridViewWidget::ZoomIn, this, &RomBrowserWidget::on_ZoomIn);
    connect(this->gridViewWidget, &Widget::RomBrowserGridViewWidget::ZoomOut, this, &RomBrowserWidget::on_ZoomOut);

    // configure context menu policy
    this->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    connect(this, &QStackedWidget::customContextMenuRequested, this, &RomBrowserWidget::customContextMenuRequested);

    // configure context menu actions
    this->action_PlayGame = new QAction(this);
    this->action_PlayGameWithDisk = new QAction(this);
    this->action_RefreshRomList = new QAction(this);
    this->action_ChooseRomDirectory = new QAction(this);
    this->action_RomInformation = new QAction(this);
    this->action_EditGameSettings = new QAction(this);
    this->action_EditCheats = new QAction(this);
    this->action_PlayGame->setText("Play Game");
    this->action_PlayGameWithDisk->setText("Play Game with Disk");
    this->action_RefreshRomList->setText("Refresh ROM List");
    this->action_ChooseRomDirectory->setText("Choose ROM Directory...");
    this->action_RomInformation->setText("ROM Information");
    this->action_EditGameSettings->setText("Edit Game Settings");
    this->action_EditCheats->setText("Edit Cheats");
    connect(this->action_PlayGame, &QAction::triggered, this, &RomBrowserWidget::on_Action_PlayGame);
    connect(this->action_PlayGameWithDisk, &QAction::triggered, this, &RomBrowserWidget::on_Action_PlayGameWithDisk);
    connect(this->action_RefreshRomList, &QAction::triggered, this, &RomBrowserWidget::on_Action_RefreshRomList);
    connect(this->action_ChooseRomDirectory, &QAction::triggered, this,
            &RomBrowserWidget::on_Action_ChooseRomDirectory);
    connect(this->action_RomInformation, &QAction::triggered, this, &RomBrowserWidget::on_Action_RomInformation);
    connect(this->action_EditGameSettings, &QAction::triggered, this, &RomBrowserWidget::on_Action_EditGameSettings);
    connect(this->action_EditCheats, &QAction::triggered, this, &RomBrowserWidget::on_Action_EditCheats);

    // configure context menu
    this->contextMenu = new QMenu(this);
    this->contextMenu->addAction(this->action_PlayGame);
    this->contextMenu->addAction(this->action_PlayGameWithDisk);
    this->contextMenu->addSeparator();
    this->contextMenu->addAction(this->action_RefreshRomList);
    this->contextMenu->addAction(this->action_ChooseRomDirectory);
    this->contextMenu->addSeparator();
    this->contextMenu->addAction(this->action_RomInformation);
    this->contextMenu->addSeparator();
    this->contextMenu->addAction(this->action_EditGameSettings);
    this->contextMenu->addAction(this->action_EditCheats);
}

RomBrowserWidget::~RomBrowserWidget()
{
}

void RomBrowserWidget::RefreshRomList(void)
{
    this->listViewModel->removeRows(0, this->listViewModel->rowCount());
    this->gridViewModel->removeRows(0, this->gridViewModel->rowCount());

    this->coversDirectory = QString::fromStdString(CoreGetUserDataDirectory().string());
    this->coversDirectory += "/Covers";

    this->listViewSortSection = CoreSettingsGetIntValue(SettingsID::RomBrowser_ListViewSortSection);
    this->listViewSortOrder   = CoreSettingsGetIntValue(SettingsID::RomBrowser_ListViewSortOrder);

    QString directory = QString::fromStdString(CoreSettingsGetStringValue(SettingsID::RomBrowser_Directory));
    if (directory.isEmpty())
    {
        this->setCurrentWidget(this->emptyWidget);
        return;
    }

    this->romSearcherThread->SetMaximumFiles(CoreSettingsGetIntValue(SettingsID::RomBrowser_MaxItems));
    this->romSearcherThread->SetRecursive(CoreSettingsGetBoolValue(SettingsID::RomBrowser_Recursive));
    this->romSearcherThread->SetDirectory(directory);
    this->romSearcherThread->start();

    this->setCurrentWidget(this->loadingWidget);
    this->romSearcherTimer.start();
}

bool RomBrowserWidget::IsRefreshingRomList(void)
{
    return this->romSearcherThread->isRunning();
}

void RomBrowserWidget::StopRefreshRomList(void)
{
    return this->romSearcherThread->Stop();
}

void RomBrowserWidget::ShowList(void)
{
    this->currentViewWidget = this->listViewWidget;

    // only change widget now when we're not refreshing
    if (!this->IsRefreshingRomList() &&
        this->currentWidget() != this->emptyWidget)
    {
        this->setCurrentWidget(this->listViewWidget);        
    }
}

void RomBrowserWidget::ShowGrid(void)
{
    this->currentViewWidget = this->gridViewWidget;

    // only change widget now when we're not refreshing
    if (!this->IsRefreshingRomList() &&
        this->currentWidget() != this->emptyWidget)
    {
        this->setCurrentWidget(this->gridViewWidget);        
    }
}

QStandardItemModel* RomBrowserWidget::getCurrentModel(void)
{
    QWidget* currentWidget = this->currentWidget();
    if (currentWidget == this->listViewWidget)
    {
        return this->listViewModel;
    }
    else if (currentWidget == this->gridViewWidget)
    {
        return this->gridViewModel;
    }

    return nullptr;
}

QAbstractItemView* RomBrowserWidget::getCurrentModelView(void)
{
    QWidget* currentWidget = this->currentWidget();
    if (currentWidget == this->listViewWidget)
    {
        return this->listViewWidget;
    }
    else if (currentWidget == this->gridViewWidget)
    {
        return this->gridViewWidget;
    }

    return nullptr;
}

QString RomBrowserWidget::getCurrentRom(void)
{
    QStandardItemModel* model = this->getCurrentModel();
    QAbstractItemView*  view  = this->getCurrentModelView();
    if (model == nullptr || view == nullptr)
    {
        return "";
    }

    QModelIndex index = view->currentIndex();
    return model->itemData(index).last().toString();
}

QIcon RomBrowserWidget::getCurrentCover(CoreRomHeader header, CoreRomSettings settings)
{
    QPixmap pixmap;

    // try to load cover using
    // 1) MD5
    // 2) good name
    // 3) internal name
    bool foundCover = false;
    for (QString name : { 
        QString::fromStdString(settings.MD5), 
        QString::fromStdString(settings.GoodName), 
        QString::fromStdString(header.Name) })
    {
        // we support jpg & png as file extensions
        for (QString ext : { ".jpg", ".jpeg", ".png" })
        {
            QString coverPath = this->coversDirectory;
            coverPath += "/";
            coverPath += name;
            coverPath += ext;

            if (QFile::exists(coverPath) && 
                pixmap.load(coverPath))
            {
                foundCover = true;
                break;
            }
        }

        if (foundCover)
        {
            break;
        }
    }
    // fallback
    if (!foundCover)
    {
        pixmap.load(":Resource/CoverFallback.png");
    }

    return QIcon(pixmap);
}

void RomBrowserWidget::timerEvent(QTimerEvent* event)
{
    this->killTimer(event->timerId());
    this->setCurrentWidget(this->currentViewWidget);
}

void RomBrowserWidget::on_DoubleClicked(const QModelIndex& index)
{
    emit this->PlayGame(this->getCurrentRom());
}

void RomBrowserWidget::customContextMenuRequested(QPoint position)
{
    QAbstractItemView* view = this->getCurrentModelView();
    if (view == nullptr)
    {
        return;
    }

    if (!view->selectionModel()->hasSelection())
    {
        return;
    }

    this->contextMenu->popup(this->mapToGlobal(position));
}

void RomBrowserWidget::on_listViewWidget_sortIndicatorChanged(int logicalIndex, Qt::SortOrder sortOrder)
{
    CoreSettingsSetValue(SettingsID::RomBrowser_ListViewSortSection, logicalIndex);
    CoreSettingsSetValue(SettingsID::RomBrowser_ListViewSortOrder, (int)sortOrder);
}

void RomBrowserWidget::on_ZoomIn(void)
{
    QAbstractItemView* view = this->getCurrentModelView();
    if (view == nullptr)
    {
        return;
    }

    view->setIconSize(view->iconSize() + QSize(20, 20));
}

void RomBrowserWidget::on_ZoomOut(void)
{
    QAbstractItemView* view = this->getCurrentModelView();
    if (view == nullptr)
    {
        return;
    }

    view->setIconSize(view->iconSize() - QSize(20, 20));
}

#include <iostream>
void RomBrowserWidget::on_RomBrowserThread_RomFound(QString file, CoreRomHeader header, CoreRomSettings settings)
{
    QString name;
    QIcon coverIcon;

    // generate name to use in UI
    name = QString::fromStdString(settings.GoodName);
    if (name.endsWith("(unknown rom)"))
    {
        name = QFileInfo(file).fileName();
    }

    coverIcon = this->getCurrentCover(header, settings);

    QList<QStandardItem*> listViewRow;
    // goodname
    QStandardItem* listViewItem1 = new QStandardItem();
    listViewItem1->setText(name);
    listViewItem1->setData(file);
    // internal name
    QStandardItem* listViewItem2 = new QStandardItem();
    listViewItem2->setText(QString::fromStdString(header.Name));
    listViewItem2->setData(file);
    // MD5
    QStandardItem* listViewItem3 = new QStandardItem();
    listViewItem3->setText(QString::fromStdString(settings.MD5));
    listViewItem3->setData(file);
    listViewRow.append(listViewItem1);
    listViewRow.append(listViewItem2);
    listViewRow.append(listViewItem3);
    this->listViewModel->appendRow(listViewRow);

    std::cout << "name: " << name.toStdString() << std::endl;

    QStandardItem* gridViewItem = new QStandardItem(coverIcon, name);
    //gridViewItem->setIcon(coverIcon);
   // gridViewItem->setText(name);
    gridViewItem->setData(file);

    this->gridViewModel->appendRow(gridViewItem);
}

void RomBrowserWidget::on_RomBrowserThread_Finished(bool canceled)
{
    // sort data
    this->listViewModel->sort(this->listViewSortSection, (Qt::SortOrder)this->listViewSortOrder);
    this->gridViewModel->sort(0, Qt::SortOrder::AscendingOrder);

    // resize list view's columns
    this->listViewWidget->resizeColumnsToContents();

    if (!canceled)
    {
        if (this->listViewModel->rowCount() == 0)
        {
            this->setCurrentWidget(this->emptyWidget);
            return;
        }
    }

    // prevent flicker by forcing the loading screen
    // to be shown at least 300ms
    qint64 elapsedTime = this->romSearcherTimer.elapsed();
    if (elapsedTime < 300)
    {
        this->startTimer(300 - elapsedTime);
        return;
    }

    this->setCurrentWidget(this->currentViewWidget);
}

void RomBrowserWidget::on_Action_PlayGame(void)
{
    emit this->PlayGame(this->getCurrentRom());
}

void RomBrowserWidget::on_Action_PlayGameWithDisk(void)
{
    emit this->PlayGameWithDisk(this->getCurrentRom());
}

void RomBrowserWidget::on_Action_RefreshRomList(void)
{
    this->RefreshRomList();
}

void RomBrowserWidget::on_Action_ChooseRomDirectory(void)
{
    emit this->ChooseRomDirectory();
}

void RomBrowserWidget::on_Action_RomInformation(void)
{
    emit this->RomInformation(this->getCurrentRom());
}

void RomBrowserWidget::on_Action_EditGameSettings(void)
{
    emit this->EditGameSettings(this->getCurrentRom());
}

void RomBrowserWidget::on_Action_EditCheats(void)
{
    emit this->Cheats(this->getCurrentRom());
}
