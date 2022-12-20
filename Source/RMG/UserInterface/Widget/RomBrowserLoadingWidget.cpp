/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "RomBrowserLoadingWidget.hpp"

#include <QLabel>
#include <QHBoxLayout>

using namespace UserInterface::Widget;

RomBrowserLoadingWidget::RomBrowserLoadingWidget(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);

    QLabel* loadingLabel = new QLabel(this);
    loadingLabel->setText("Loading...");
    loadingLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addWidget(loadingLabel);

    this->setLayout(layout);
}

RomBrowserLoadingWidget::~RomBrowserLoadingWidget()
{
}
