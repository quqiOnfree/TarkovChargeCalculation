#include "servicechargecalculation.h"
#include "./ui_servicechargecalculation.h"

#include <cmath>

#include <QButtonGroup>
#include <QRadioButton>
#include <QTableWidget>

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
    connect(ui->tableWidget_2, &QTableWidget::cellChanged, this, &ServiceChargeCalculation::onNeedCellChanged);

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
        long long count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            count += ui->tableWidget->item(i, 0)->text().toLongLong() * ui->tableWidget->item(i, 1)->text().toLongLong();
        }
        return count;
        };

    auto count_need = [this]() {
        long long count = 0;
        int row = ui->tableWidget->rowCount();
        for (int i = 0; i < row; ++i)
        {
            count += ui->tableWidget_2->item(i, 0)->text().toLongLong() * ui->tableWidget->item(i, 1)->text().toLongLong();
        }
        return count;
        };

    long long Q = ui->checkBox_2->isChecked() ? 1 : count_calculate();
    if (!Q)
    {
        ui->resultLabel->setText(" 商品总数为0 ");
        return;
    }

    long double VO = count_value() * 1.0l / Q, VR = count_need() * 1.0l;
    if (!VO || !VR)
    {
        ui->resultLabel->setText(" 商品总值为0 ");
        return;
    }

    long double PO = VO > VR ? std::pow(std::log10l(VO / VR), 1.08l) : std::log10l(VO / VR),
                PR = VR >= VO ? std::pow(std::log10l(VR / VO), 1.08l) : std::log10l(VR / VO);

    long double Ti = 0.07, Tr = 0.05;
    long double s = VO * Ti * 4 * PO * Q + VR * Tr * 4 * PR * Q;

    if (ui->checkBox->isChecked()) s = s * 0.7l;

    ui->resultLabel->setText(QString::number(static_cast<double>(s)));
}

/*Therapist为0.63，Ragman为0.62，Jaeger为0.6，Mechanic为0.56，Prapor为=0.5，Peacekeeper为大约0.495，Skier为0.49，Fence为0.4*/

void ServiceChargeCalculation::onTherapistToggled()
{
    times = 0.63;
}

void ServiceChargeCalculation::onRagmanToggled()
{
    times = 0.62;
}

void ServiceChargeCalculation::onJaegerToggled()
{
    times = 0.6;
}

void ServiceChargeCalculation::onMechanicToggled()
{
    times = 0.56;
}

void ServiceChargeCalculation::onPraporToggled()
{
    times = 0.5;
}

void ServiceChargeCalculation::onPeacekeeperToggled()
{
    times = 0.495;
}

void ServiceChargeCalculation::onSkierToggled()
{
    times = 0.49;
}

void ServiceChargeCalculation::onFenceToggled()
{
    times = 0.4;
}

void ServiceChargeCalculation::onValueCellChanged(int row, int column)
{
    auto item = ui->tableWidget->item(row, column);
}

void ServiceChargeCalculation::onNeedCellChanged(int row, int column)
{
    auto item = ui->tableWidget_2->item(row, column);
}

ServiceChargeCalculation::~ServiceChargeCalculation()
{
    delete ui;
}

