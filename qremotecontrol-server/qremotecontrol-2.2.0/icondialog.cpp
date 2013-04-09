#include "icondialog.h"
#include "ui_icondialog.h"

IconDialog::IconDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IconDialog)
{
    ui->setupUi(this);

    QPushButton *button = new QPushButton(tr("Browse..."), this);
    ui->buttonBox->addButton(button, QDialogButtonBox::HelpRole);
}

IconDialog::~IconDialog()
{
    delete ui;
}

void IconDialog::on_buttonBox_accepted()
{
    QString fileName = QString("%1/%2.png").arg(m_settingsDir).arg(ui->listWidget->currentRow());
    m_icon = ui->listWidget->currentItem()->icon();
    m_icon.pixmap(64).save(fileName);
    m_fileName = fileName;
}

void IconDialog::on_buttonBox_helpRequested()
{
    QString fileName = QFileDialog::getOpenFileName( this,
                                                     tr("Open icon"),
                                                     QDesktopServices::storageLocation(QDesktopServices::HomeLocation),
                                                     tr("Images (*.png *.xpm *.jpg *.bmp)"));
    if (!fileName.isNull()) {
        m_icon = QIcon(fileName);
        m_fileName = fileName;
        this->accept();
    }
}
