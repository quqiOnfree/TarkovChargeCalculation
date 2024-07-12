#ifndef SERVICECHARGECALCULATION_H
#define SERVICECHARGECALCULATION_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ServiceChargeCalculation; }
QT_END_NAMESPACE

class ServiceChargeCalculation : public QWidget
{
    Q_OBJECT

public:
    ServiceChargeCalculation(QWidget *parent = nullptr);
    ~ServiceChargeCalculation();

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
    void onNeedCellChanged(int row, int column);

private:
    double times;
    Ui::ServiceChargeCalculation *ui;
};
#endif // SERVICECHARGECALCULATION_H
