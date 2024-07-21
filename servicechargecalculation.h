/*
    TarkovChargeCalculation, an easy to use, calcuating tax of items application for Escape from Tarkov
    Copyright (C) 2024  Xuan Xiao

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SERVICECHARGECALCULATION_H
#define SERVICECHARGECALCULATION_H

#include <QWidget>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class ServiceChargeCalculation; }
QT_END_NAMESPACE

class ServiceChargeCalculation : public QWidget
{
    Q_OBJECT

public:
    ServiceChargeCalculation(QWidget *parent = nullptr);
    ~ServiceChargeCalculation();

    void showData(QVector<double> x, QVector<double> y, double max);

signals:
    void showDataSignal(QVector<double> x, QVector<double> y, double max);

private slots:
    void onCalculateClicked();

    /*Therapist为0.63，Ragman为0.62，Jaeger为0.6，Mechanic为0.56，Prapor为=0.5，Peacekeeper为大约0.495，Skier为0.49，Fence为0.4*/
    void onTherapistToggled();
    void onRagmanToggled();
    void onJaegerToggled();
    void onMechanicToggled();
    void onPraporToggled();
    void onPeacekeeperToggled();
    void onSkierToggled();
    void onFenceToggled();

    // table widget
    void onValueCellChanged(int row, int column);
    void onRequirementCellChanged(int row, int column);

    // custom plot
    void showDataSlot(QVector<double> x, QVector<double> y, double max);

private:
    long double times;
    Ui::ServiceChargeCalculation *ui;
};
#endif // SERVICECHARGECALCULATION_H
