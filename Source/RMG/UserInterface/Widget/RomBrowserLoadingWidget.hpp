/*
 * Rosalie's Mupen GUI - https://github.com/Rosalie241/RMG
 *  Copyright (C) 2020 Rosalie Wanders <rosalie@mailbox.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3.
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef ROMBROWSERLOADINGWIDGET_HPP
#define ROMBROWSERLOADINGWIDGET_HPP

#include <QWidget>

namespace UserInterface
{
namespace Widget
{
class RomBrowserLoadingWidget : public QWidget
{
    Q_OBJECT
public:
    RomBrowserLoadingWidget(QWidget* parent);
    ~RomBrowserLoadingWidget();
};
} // namespace Widget
} // namespace UserInterface

#endif // ROMBROWSERLOADINGWIDGET_HPP
