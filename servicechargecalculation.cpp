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

#include "servicechargecalculation.h"
#include "./ui_servicechargecalculation.h"

#include <cmath>
#include <functional>
#include <thread>

#include <QButtonGroup>
#include <QRadioButton>
#include <QTableWidget>
#include <QVector>

template<typename T,
        typename Comparator = std::less<T>,
        typename Func>
static T get_best(T begin, T end, Func&& func, size_t n)
{
    T mid = (begin + end) / 2;
    if (!n || mid == begin || mid == end) return mid;

    Comparator comparator{};
    T calbegin = func(begin), calmid = func(mid), calend = func(end);
    if (comparator(calbegin, calend) && comparator(calbegin, calmid))
    {
        return get_best<T, Comparator>(mid, end, func, n - 1);
    }
    else if (comparator(calend, calbegin) && comparator(calend, calmid))
    {
        return get_best<T, Comparator>(begin, mid, func, n - 1);
    }
    else
    {
        T left = get_best<T, Comparator>(begin, mid, func, 1),
            right = get_best<T, Comparator>(mid, end, func, 1);
        T max{};
        if (left == right)
            return mid;
        else if (comparator(left, right))
            return get_best<T, Comparator>(begin, mid, func, n - 1);
        else
            get_best<T, Comparator>(mid, end, func, n - 1);
    }
    return mid;
}

ServiceChargeCalculation::ServiceChargeCalculation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServiceChargeCalculation)
    , times(0.0)
{
    ui->setupUi(this);

    // QCustomPlot
    ui->customPlot->setHidden(true);
    ui->customPlot->setMinimumWidth(500);
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    connect(this, &ServiceChargeCalculation::showDataSignal, this, &ServiceChargeCalculation::showDataSlot);

    // QButtonGroup
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->Therapist);
    buttonGroup->addButton(ui->Ragman);
    buttonGroup->addButton(ui->Jaeger);
    buttonGroup->addButton(ui->Mechanic);
    buttonGroup->addButton(ui->Prapor);
    buttonGroup->addButton(ui->Peacekeeper);
    buttonGroup->addButton(ui->Skier);
    buttonGroup->addButton(ui->Fence);

    // QRadioButton
    connect(ui->Therapist, &QRadioButton::toggled, this, &ServiceChargeCalculation::onTherapistToggled);
    connect(ui->Ragman, &QRadioButton::toggled, this, &ServiceChargeCalculation::onRagmanToggled);
    connect(ui->Jaeger, &QRadioButton::toggled, this, &ServiceChargeCalculation::onJaegerToggled);
    connect(ui->Mechanic, &QRadioButton::toggled, this, &ServiceChargeCalculation::onMechanicToggled);
    connect(ui->Peacekeeper, &QRadioButton::toggled, this, &ServiceChargeCalculation::onPeacekeeperToggled);
    connect(ui->Skier, &QRadioButton::toggled, this, &ServiceChargeCalculation::onSkierToggled);
    connect(ui->Fence, &QRadioButton::toggled, this, &ServiceChargeCalculation::onFenceToggled);

    // QTableWidget
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &ServiceChargeCalculation::onValueCellChanged);
    connect(ui->tableWidget_2, &QTableWidget::cellChanged, this, &ServiceChargeCalculation::onRequirementCellChanged);

    // QPushButton
    connect(ui->calculateButton, &QPushButton::clicked, this, &ServiceChargeCalculation::onCalculateClicked);
}

void ServiceChargeCalculation::onCalculateClicked()
{
    ui->calculateButton->setDisabled(true);
    // check all of numbers
    {
        int row = ui->tableWidget->rowCount();
        int col = ui->tableWidget->columnCount();
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                bool ok = false;
                ui->tableWidget->item(i, j)->text().toLongLong(&ok);
                if (!ok)
                {
                    ui->resultLabel->setText(" 表格数据读取错误，可能含有非数字 ");
                    return;
                }
            }
        }
    }
    {
        int row = ui->tableWidget_2->rowCount();
        int col = ui->tableWidget_2->columnCount();
        for (int i = 0; i < row; ++i)
        {
            for (int j = 0; j < col; ++j)
            {
                bool ok = false;
                ui->tableWidget_2->item(i, j)->text().toLongLong(&ok);
                if (!ok)
                {
                    ui->resultLabel->setText(" 表格数据读取错误，可能含有非数字 ");
                    return;
                }
            }
        }
    }

    // check radio
    if (times == 0.0)
    {
        ui->resultLabel->setText(" 请选择商人 ");
        return;
    }

    // prepare data
    auto count_calculate = [=]() {
        long long count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            count += ui->tableWidget->item(i, 1)->text().toLongLong();
        }
        return count;
        };

    auto count_value = [=]() {
        long double count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            long long amount = ui->tableWidget->item(i, 1)->text().toLongLong();
            if (!amount) continue;
            count += ui->tableWidget->item(i, 0)->text().toLongLong() * ui->tableWidget->item(i, 1)->text().toLongLong() / times;
        }
        return count;
        };

    long long Number = ui->tableWidget->item(0, 1)->text().toLongLong();
    auto count_need = [=]() {
        long double count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            long long amount = ui->tableWidget->item(i, 1)->text().toLongLong();
            if (!amount) continue;
            count += ui->tableWidget_2->item(i, 0)->text().toLongLong() * 1.0l * ui->tableWidget->item(i, 1)->text().toLongLong();
        }
        return count;
        };

    long long Q = ui->checkBox_2->isChecked() ? 1 : count_calculate();
    if (!Q)
    {
        ui->resultLabel->setText(" 商品总数为0 ");
        return;
    }

    long double VO = count_value() / Q, VR = count_need() / Q;
    if (!VO || !VR)
    {
        ui->resultLabel->setText(" 商品总值为0 ");
        return;
    }

    const long double Ti = 0.03, Tr = 0.03;
    bool isChecked = ui->checkBox->isChecked();
    int value = ui->spinBox->value();
    auto function = [=](long double VR) {
        long double PO = VO > VR ? std::pow(std::log10l(VO / VR), 1.08l) : std::log10l(VO / VR),
            PR = VR >= VO ? std::pow(std::log10l(VR / VO), 1.08l) : std::log10l(VR / VO);

        long double s = VO * Ti * std::pow(4.0l, PO) * Q + VR * Tr * std::pow(4.0l, PR) * Q;

        if (isChecked) s = s * (0.7l - value * 0.003l);

        return s;
        };

    auto derivativeOfVRRFunction = [=](long double VR) {
        return 1 + (VO * Ti * Q * std::log10l(4) * std::pow(4.0l, std::log10l(VO / VR)) / VR -
            Tr * Q * std::log10l(40) * std::pow(4.0l, std::log10l(VR / VO))) * (0.7l - ui->spinBox->value() * 0.003l);
        };

    auto secondDerivativeOfVRRFunction = [=](long double VR) {
        return (-1 * VO * Ti * Q * std::pow(std::log10l(4), 2) * std::pow(4.0l, std::log10l(VO / VR)) / std::pow(VR, 2) -
            Tr * Q * std::log10l(40) * std::log10l(4) * std::pow(4.0l, std::log10l(VR / VO)) / VR) * (0.7l - ui->spinBox->value() * 0.003l);
        };

    // calculate
    long double bestVR = VR;
    for (int i = 0; i < 100; ++i)
    {
        bestVR = bestVR - derivativeOfVRRFunction(bestVR) / secondDerivativeOfVRRFunction(bestVR);
    }
    long double bestRequirement = get_best(10.0l, bestVR, [&](long double V) { return V - function(V); }, 100) / Q;

    // show data
    std::thread([=]() {
        const auto logl2 = std::logl(bestRequirement * 2);
        const size_t size = static_cast<size_t>(bestRequirement * 2) / static_cast<size_t>(logl2);
        QVector<double> x(size), y(size);
        double max = -1e10;
        for (size_t i = 1; i < size; ++i)
        {
            x[i] = static_cast<double>(static_cast<long double>(i) * logl2);
            y[i] = static_cast<double>(x[i] * Number - function(x[i]));
            if (y[i] > max) max = y[i];
        }
        showDataSignal(x, y, max);
        }).detach();

    long double result = function(VR);
    ui->resultLabel->setText("手续费: " +
        QString::number(static_cast<long long>(std::round(result))) +
        " 总利润: " +
        QString::number(static_cast<long long>(std::round(VR - result))) +
        " 最高利润卖价: " +
        QString::number(static_cast<long long>(std::round(bestRequirement))));
}

/*Therapist为0.63，Ragman为0.62，Jaeger为0.6，Mechanic为0.56，Prapor为=0.5，Peacekeeper为大约0.495，Skier为0.49，Fence为0.4*/

void ServiceChargeCalculation::onTherapistToggled()
{
    times = 0.63l;
}

void ServiceChargeCalculation::onRagmanToggled()
{
    times = 0.62l;
}

void ServiceChargeCalculation::onJaegerToggled()
{
    times = 0.6l;
}

void ServiceChargeCalculation::onMechanicToggled()
{
    times = 0.56l;
}

void ServiceChargeCalculation::onPraporToggled()
{
    times = 0.5l;
}

void ServiceChargeCalculation::onPeacekeeperToggled()
{
    times = 0.495l;
}

void ServiceChargeCalculation::onSkierToggled()
{
    times = 0.49l;
}

void ServiceChargeCalculation::onFenceToggled()
{
    times = 0.4;
}

void ServiceChargeCalculation::onValueCellChanged(int row, int column)
{
    auto item = ui->tableWidget->item(row, column);
}

void ServiceChargeCalculation::onRequirementCellChanged(int row, int column)
{
    auto item = ui->tableWidget_2->item(row, column);
}

void ServiceChargeCalculation::showDataSlot(QVector<double> x, QVector<double> y, double max)
{
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->customPlot->xAxis->setLabel("x");
    ui->customPlot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->customPlot->xAxis->setRange(-10.0, x[x.size() - 1]);
    ui->customPlot->yAxis->setRange(y[0], max);
    ui->customPlot->replot();
    ui->customPlot->setHidden(false);
    ui->calculateButton->setDisabled(false);
}

ServiceChargeCalculation::~ServiceChargeCalculation()
{
    delete ui;
}

void ServiceChargeCalculation::showData(QVector<double> x, QVector<double> y, double max)
{
    emit showDataSignal(std::move(x), std::move(y), max);
}

