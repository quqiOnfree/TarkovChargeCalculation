#include "servicechargecalculation.h"
#include "./ui_servicechargecalculation.h"

#include <cmath>
#include <functional>

#include <QButtonGroup>
#include <QRadioButton>
#include <QTableWidget>

template<typename T, typename Func>
T getBestMax(T begin, T end, Func&& func, size_t n)
{
    T mid = (begin + end) / 2;
    if (!n || mid == begin || mid == end) return mid;

    T calbegin = func(begin), calmid = func(mid), calend = func(end);
    if (calbegin < calend && calbegin < calmid)
    {
        return getBestMax(mid, end, func, n - 1);
    }
    else if (calbegin > calend && calmid > calend)
    {
        return getBestMax(begin, mid, func, n - 1);
    }
    else
    {
        T left = getBestMax(begin, mid, func, 1),
            right = getBestMax(mid, end, func, 1);
        T max = std::max(left, right);
        if (max == left && max == right) return mid;
        else if (max == left) return getBestMax(begin, mid, func, n - 1);
        else return getBestMax(mid, end, func, n - 1);
    }
    return mid;
}

ServiceChargeCalculation::ServiceChargeCalculation(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServiceChargeCalculation)
    , times(0.0)
{
    ui->setupUi(this);

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

    auto count_calculate = [this]() {
        long long count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            count += ui->tableWidget->item(i, 1)->text().toLongLong();
        }
        return count;
        };

    auto count_value = [this]() {
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

    auto count_need = [this]() {
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

    long double VO = count_value() / Q, VR = count_need();
    if (!VO || !VR)
    {
        ui->resultLabel->setText(" 商品总值为0 ");
        return;
    }

    const long double Ti = 0.03, Tr = 0.03;

    auto function = [&](long double VR) {
        long double PO = VO > VR ? std::pow(std::log10l(VO / VR), 1.08l) : std::log10l(VO / VR),
            PR = VR >= VO ? std::pow(std::log10l(VR / VO), 1.08l) : std::log10l(VR / VO);

        long double s = VO * Ti * std::pow(4.0l, PO) * Q + VR * Tr * std::pow(4.0l, PR) * Q;

        if (ui->checkBox->isChecked()) s = s * (0.7l - ui->spinBox->value() * 0.003l);

        return s;
        };

    auto derivativeOfVRRFunction = [&](long double VR) {
        return 1 + (VO * Ti * Q * std::log10l(4) * std::pow(4.0l, std::log10l(VO / VR)) / VR -
            Tr * Q * std::log10l(40) * std::pow(4.0l, std::log10l(VR / VO))) * (0.7l - ui->spinBox->value() * 0.003l);
        };

    auto secondDerivativeOfVRRFunction = [&](long double VR) {
        return (-1 * VO * Ti * Q * std::pow(std::log10l(4), 2) * std::pow(4.0l, std::log10l(VO / VR)) / std::pow(VR, 2) -
            Tr * Q * std::log10l(40) * std::log10l(4) * std::pow(4.0l, std::log10l(VR / VO)) / VR) * (0.7l - ui->spinBox->value() * 0.003l);
        };

    long double bestVR = VR;
    for (int i = 0; i < 1000; ++i)
    {
        bestVR = bestVR - derivativeOfVRRFunction(bestVR) / secondDerivativeOfVRRFunction(bestVR);
    }
    long double bestRequirement = getBestMax(10.0l, bestVR, [&](long double V) { return V - function(V); }, 100) / Q;

    long double result = function(VR);
    ui->resultLabel->setText("----手续费: " +
        QString::number(static_cast<long long>(std::round(result))) +
        " 总利润: " +
        QString::number(static_cast<long long>(std::round(VR - result))) +
        " 建议卖价(不精确): " +
        QString::number(static_cast<long long>(std::round(bestRequirement))) +
        "----");
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

ServiceChargeCalculation::~ServiceChargeCalculation()
{
    delete ui;
}

